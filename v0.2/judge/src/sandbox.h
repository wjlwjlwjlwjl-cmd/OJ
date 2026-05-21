#ifndef SANDBOX_H
#define SANDBOX_H

#include <sys/types.h>

class Sandbox {
public:
    static void applyMemoryLimit(int memory_limit_mb);
    static void applyTimeout(int time_limit_ms);
    static void applyLimits(int time_limit_ms, int memory_limit_mb);
};

#endif
