#pragma once
#include <stdint.h>

typedef struct {
    uint64_t update_us, draw_us, font_build_us;
    uint64_t glyph_hits, glyph_misses, blit_pixels;
} SgPerf;

extern SgPerf sg_perf;
uint64_t sg_now_us(void);
void sg_perf_begin(void);
