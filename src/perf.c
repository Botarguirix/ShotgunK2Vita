#include "perf.h"
#include <string.h>
#ifdef SG_VITA
#include <psp2/kernel/processmgr.h>
#else
#include <time.h>
#endif

SgPerf sg_perf;

uint64_t sg_now_us(void) {
#ifdef SG_VITA
    return sceKernelGetProcessTimeWide();
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000 + t.tv_nsec / 1000;
#endif
}

void sg_perf_begin(void) { memset(&sg_perf, 0, sizeof(sg_perf)); }
