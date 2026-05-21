#include "sandbox.h"
#include <sys/resource.h>
#include <csignal>
#include <cstdlib>
#include <iostream>

void Sandbox::applyMemoryLimit(int memory_limit_mb) {
    struct rlimit rl;
    rl.rlim_cur = (rlim_t)memory_limit_mb * 1024 * 1024;
    rl.rlim_max = rl.rlim_cur;
    if (setrlimit(RLIMIT_AS, &rl) != 0) {
        _exit(EXIT_FAILURE);
    }
}

void Sandbox::applyTimeout(int time_limit_sec) {
    signal(SIGALRM, SIG_DFL);
    alarm(static_cast<unsigned int>(time_limit_sec));
}

void Sandbox::applyLimits(int time_limit_sec, int memory_limit_mb) {
    applyMemoryLimit(memory_limit_mb);
    if (time_limit_sec > 0) {
        applyTimeout(time_limit_sec);
    }
}
