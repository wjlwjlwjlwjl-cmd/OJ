#include <gtest/gtest.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>

#include "../../judge/src/sandbox.h"

TEST(SandboxTest, ApplyMemoryLimitSetsRlimit) {
    pid_t pid = fork();
    if (pid == 0) {
        Sandbox::applyMemoryLimit(128);
        struct rlimit rl;
        getrlimit(RLIMIT_AS, &rl);
        if (rl.rlim_cur == (rlim_t)128 * 1024 * 1024 &&
            rl.rlim_max == rl.rlim_cur) {
            _exit(0);
        }
        _exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

TEST(SandboxTest, ApplyMemoryLimitZero) {
    pid_t pid = fork();
    if (pid == 0) {
        Sandbox::applyMemoryLimit(0);
        _exit(0);
    }
    int status;
    waitpid(pid, &status, 0);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

TEST(SandboxTest, ApplyTimeoutSetsAlarm) {
    pid_t pid = fork();
    if (pid == 0) {
        Sandbox::applyTimeout(2);
        unsigned remaining = alarm(0);
        if (remaining > 0 && remaining <= 2) {
            _exit(0);
        }
        _exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

TEST(SandboxTest, ApplyTimeoutZero) {
    pid_t pid = fork();
    if (pid == 0) {
        Sandbox::applyTimeout(0);
        unsigned remaining = alarm(0);
        if (remaining == 0) {
            _exit(0);
        }
        _exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

TEST(SandboxTest, ApplyLimitsBoth) {
    pid_t pid = fork();
    if (pid == 0) {
        Sandbox::applyLimits(3, 256);
        struct rlimit rl;
        getrlimit(RLIMIT_AS, &rl);
        if (rl.rlim_cur != (rlim_t)256 * 1024 * 1024) {
            _exit(1);
        }
        unsigned remaining = alarm(0);
        if (remaining > 0 && remaining <= 3) {
            _exit(0);
        }
        _exit(2);
    }
    int status;
    waitpid(pid, &status, 0);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

TEST(SandboxTest, ApplyLimitsNegativeTimeSkipsAlarm) {
    pid_t pid = fork();
    if (pid == 0) {
        Sandbox::applyLimits(-1, 256);
        unsigned remaining = alarm(0);
        struct rlimit rl;
        getrlimit(RLIMIT_AS, &rl);
        if (remaining == 0 && rl.rlim_cur == (rlim_t)256 * 1024 * 1024) {
            _exit(0);
        }
        _exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
