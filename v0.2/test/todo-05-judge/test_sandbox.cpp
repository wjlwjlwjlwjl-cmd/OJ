#include <gtest/gtest.h>
#include <sys/resource.h>
#include <sys/time.h>
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

TEST(SandboxTest, ApplyTimeoutSetsTimer) {
    pid_t pid = fork();
    if (pid == 0) {
        Sandbox::applyTimeout(2000);
        struct itimerval timer;
        getitimer(ITIMER_REAL, &timer);
        long remaining_ms = timer.it_value.tv_sec * 1000
                          + timer.it_value.tv_usec / 1000;
        if (remaining_ms > 0 && remaining_ms <= 2000) {
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
        struct itimerval timer;
        getitimer(ITIMER_REAL, &timer);
        if (timer.it_value.tv_sec == 0 && timer.it_value.tv_usec == 0) {
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
        Sandbox::applyLimits(3000, 256);
        struct rlimit rl;
        getrlimit(RLIMIT_AS, &rl);
        if (rl.rlim_cur != (rlim_t)256 * 1024 * 1024) {
            _exit(1);
        }
        struct itimerval timer;
        getitimer(ITIMER_REAL, &timer);
        long remaining_ms = timer.it_value.tv_sec * 1000
                          + timer.it_value.tv_usec / 1000;
        if (remaining_ms > 0 && remaining_ms <= 3000) {
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
        struct itimerval timer;
        getitimer(ITIMER_REAL, &timer);
        struct rlimit rl;
        getrlimit(RLIMIT_AS, &rl);
        if (timer.it_value.tv_sec == 0 && timer.it_value.tv_usec == 0
            && rl.rlim_cur == (rlim_t)256 * 1024 * 1024) {
            _exit(0);
        }
        _exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
