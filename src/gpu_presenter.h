#pragma once
#include <stdint.h>
#include <stdio.h>
typedef struct {uint64_t upload_us,submit_us,wait_us;} SgGpuStats;
int sg_gpu_init(FILE *log,int divisor);
int sg_gpu_present(const uint32_t *pixels,int width,int height,unsigned buttons,int controls,uint32_t background,SgGpuStats *stats);
void sg_gpu_close(void);
/* Optional overlay drawn over the scene at full screen size: fill writes up
 * to max_w x max_h RGBA pixels (rows of *w) and returns 0 when there is none.
 * It runs inside sg_gpu_present once the GPU no longer reads the texture. */
typedef int (*SgOverlayFill)(uint32_t *pixels,int max_w,int max_h,int *w,int *h);
void sg_gpu_set_overlay(SgOverlayFill fill);
