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

void sg_perf_begin(void) {static unsigned frame;memset(&sg_perf,0,sizeof(sg_perf));sg_perf.gfx_sample=(++frame%30)==0;}
void sg_perf_extra_header(FILE*f){fputs(",output_width,output_height,gfx_sample,gfx_blit_ms,gfx_rotate_ms,gfx_fill_ms,gfx_text_ms,gfx_blit_calls,gfx_rotate_calls,gfx_fill_calls,gfx_text_calls\n",f);}
void sg_perf_extra_row(FILE*f,int w,int h){fprintf(f,",%d,%d,%u,%.3f,%.3f,%.3f,%.3f,%llu,%llu,%llu,%llu\n",w,h,sg_perf.gfx_sample,sg_perf.gfx_us[0]/1000.,sg_perf.gfx_us[1]/1000.,sg_perf.gfx_us[2]/1000.,sg_perf.gfx_us[3]/1000.,(unsigned long long)sg_perf.gfx_calls[0],(unsigned long long)sg_perf.gfx_calls[1],(unsigned long long)sg_perf.gfx_calls[2],(unsigned long long)sg_perf.gfx_calls[3]);}
