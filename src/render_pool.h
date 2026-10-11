#pragma once
#include <stdint.h>
#include <stdio.h>
int sg_render_pool_start(int workers, FILE *log);
void sg_render_pool_present(uint32_t *pixels, int width, int height);
void sg_render_pool_stop(void);
typedef struct {uint64_t worker_us[3];int cpu[3];} SgRenderStats;
SgRenderStats sg_render_pool_stats(void);
