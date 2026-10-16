#pragma once
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint64_t update_us, draw_us, font_build_us;
    uint64_t input_calls, input_evaluations, input_parses;
    uint64_t shadow_calls,shadow_pixels,shadow_fallbacks;
    uint64_t entities, entity_updates, entity_draws, palette_resets, palette_suppressed, sprite_calls;
    uint64_t glyph_hits, glyph_misses, blit_pixels;
    unsigned gfx_sample;
    uint64_t gfx_calls[4],gfx_us[4];
} SgPerf;

extern SgPerf sg_perf;
uint64_t sg_now_us(void);
void sg_perf_begin(void);
void sg_perf_extra_header(FILE *file);
void sg_perf_extra_row(FILE *file,int width,int height);
