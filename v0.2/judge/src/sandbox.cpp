#include "sandbox.h"
#include <sys/resource.h>
#include <sys/time.h>
#include <csignal>
#include <cstdlib>

void Sandbox::applyMemoryLimit(int memory_limit_mb) {
    struct rlimit rl;
    rl.rlim_cur = (rlim_t)memory_limit_mb * 1024 * 1024;
    rl.rlim_max = rl.rlim_cur;
    if (setrlimit(RLIMIT_AS, &rl) != 0) {
        _exit(EXIT_FAILURE);
    }
}

void Sandbox::applyTimeout(int time_limit_ms) {
    if (time_limit_ms <= 0) return;
    struct itimerval timer;
    timer.it_value.tv_sec = time_limit_ms / 1000;
    timer.it_value.tv_usec = (time_limit_ms % 1000) * 1000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    signal(SIGALRM, SIG_DFL);
    if (setitimer(ITIMER_REAL, &timer, nullptr) != 0) {
        _exit(EXIT_FAILURE);
    }
}

void Sandbox::applyLimits(int time_limit_ms, int memory_limit_mb) {
    applyMemoryLimit(memory_limit_mb);
    if (time_limit_ms > 0) {
        applyTimeout(time_limit_ms);
    }
}
