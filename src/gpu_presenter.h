#pragma once
#include <stdint.h>
#include <stdio.h>
typedef struct {uint64_t upload_us,submit_us,wait_us;} SgGpuStats;
int sg_gpu_init(FILE *log);
int sg_gpu_present(const uint32_t *pixels,int width,int height,unsigned buttons,int controls,uint32_t background,SgGpuStats *stats);
void sg_gpu_close(void);
