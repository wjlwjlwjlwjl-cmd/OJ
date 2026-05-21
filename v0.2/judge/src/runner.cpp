#include "runner.h"
#include "sandbox.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cassert>

static const char* COMPILER = "/usr/bin/g++";

JudgeResult Runner::judge(const std::string& code,
                           const std::vector<TestCase>& test_cases,
                           int time_limit_ms,
                           int memory_limit_mb) {
    JudgeResult result;
    result.status = "accepted";

    std::string tmp_dir = createTempDir();
    if (tmp_dir.empty()) {
        result.status = "runtime_error";
        result.message = "Failed to create temp directory";
        return result;
    }

    std::string source_path = tmp_dir + "/solution.cpp";
    std::string exe_path = tmp_dir + "/solution";

    std::string compile_error;
    if (!compile(code, source_path, exe_path, compile_error)) {
        result.status = "compile_error";
        result.message = compile_error;
        cleanup(tmp_dir);
        return result;
    }

    int passed_count = 0;
    bool any_timeout = false;
    bool any_runtime_error = false;

    for (size_t i = 0; i < test_cases.size(); ++i) {
        TestResult tr = runSingle(exe_path, test_cases[i], (int)i,
                                   time_limit_ms, memory_limit_mb);
        result.test_results.push_back(tr);
        if (tr.passed) {
            passed_count++;
        } else if (tr.status == "time_limit") {
            any_timeout = true;
        } else if (tr.status == "runtime_error") {
            any_runtime_error = true;
        }
    }

    cleanup(tmp_dir);

    if (passed_count == (int)test_cases.size()) {
        result.status = "accepted";
    } else if (any_timeout) {
        result.status = "time_limit";
    } else if (any_runtime_error) {
        result.status = "runtime_error";
    } else {
        result.status = "wrong_answer";
    }

    return result;
}

bool Runner::compile(const std::string& code,
                      const std::string& source_path,
                      const std::string& exe_path,
                      std::string& error_output) {
    std::ofstream src(source_path);
    if (!src) return false;
    src << code;
    src.close();

    int pipefd[2];
    if (pipe(pipefd) != 0) return false;

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execl(COMPILER, "g++", "-O2", "-std=c++17", "-o",
              exe_path.c_str(), source_path.c_str(), nullptr);
        _exit(1);
    }

    close(pipefd[1]);
    char buf[4096];
    ssize_t n;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
        error_output.append(buf, n);
    }
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

TestResult Runner::runSingle(const std::string& exe_path,
                              const TestCase& tc,
                              int index,
                              int time_limit_ms,
                              int memory_limit_mb) {
    TestResult tr;
    tr.index = index;
    tr.expected_output = tc.output;

    int stdin_pipe[2], stdout_pipe[2];
    if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0) {
        tr.status = "runtime_error";
        tr.your_output = "pipe() failed";
        return tr;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        Sandbox::applyLimits(time_limit_ms / 1000 + 1, memory_limit_mb);

        execl(exe_path.c_str(), "./solution", nullptr);
        _exit(1);
    }

    close(stdin_pipe[0]);
    close(stdout_pipe[1]);

    write(stdin_pipe[1], tc.input.data(), tc.input.size());
    close(stdin_pipe[1]);

    std::string output;
    char buf[65536];
    ssize_t n;

    fd_set set;
    struct timeval timeout;
    time_t start_time = time(nullptr);

    while (true) {
        FD_ZERO(&set);
        FD_SET(stdout_pipe[0], &set);
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        int r = select(stdout_pipe[0] + 1, &set, nullptr, nullptr, &timeout);
        if (r > 0) {
            n = read(stdout_pipe[0], buf, sizeof(buf));
            if (n > 0) {
                output.append(buf, n);
            } else {
                break;
            }
        } else if (r == 0) {
            if (time(nullptr) - start_time > time_limit_ms / 1000 + 2) {
                kill(pid, SIGKILL);
                tr.status = "time_limit";
                break;
            }
        } else {
            break;
        }
    }
    close(stdout_pipe[0]);

    if (tr.status != "time_limit") {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                tr.status = "runtime_error";
                tr.your_output = output;
                return tr;
            }
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            if (sig == SIGALRM || sig == SIGXCPU) {
                tr.status = "time_limit";
            } else if (sig == SIGSEGV || sig == SIGABRT) {
                tr.status = "runtime_error";
            } else {
                tr.status = "runtime_error";
            }
            tr.your_output = output;
            return tr;
        }
    }

    tr.your_output = output;
    tr.status = "accepted";
    tr.passed = (trim(tr.your_output) == trim(tr.expected_output));
    if (!tr.passed) {
        tr.status = "wrong_answer";
    }
    return tr;
}

std::string Runner::trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\n' ||
                                 s[start] == '\r' || s[start] == '\t')) {
        start++;
    }
    size_t end = s.size();
    while (end > start && (s[end-1] == ' ' || s[end-1] == '\n' ||
                            s[end-1] == '\r' || s[end-1] == '\t')) {
        end--;
    }
    return s.substr(start, end - start);
}

std::string Runner::createTempDir() {
    srand(time(nullptr) ^ getpid());
    for (int attempt = 0; attempt < 10; attempt++) {
        std::string path = "/tmp/oj_" + std::to_string(rand() % 100000000);
        if (mkdir(path.c_str(), 0700) == 0) {
            return path;
        }
    }
    return "";
}

void Runner::cleanup(const std::string& dir) {
    std::string cmd = "rm -rf " + dir;
    system(cmd.c_str());
}
