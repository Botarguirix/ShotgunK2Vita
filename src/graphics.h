#pragma once
unsigned sg_graphics_memory_kb(void);
#include "lua.h"
#include <stdint.h>
void sg_graphics_init(lua_State *L,const char *root);
void sg_graphics_close(void);
void sg_present(uint32_t *pixels,int width,int height);
/* Logical coordinates stay unchanged; sample the screen on a 1x or 2x grid. */
void sg_graphics_set_output_divisor(int divisor);
uint32_t sg_present_background(void);
/* Surfaces and palette must remain immutable until all row jobs finish. */
void sg_present_rows(uint32_t *pixels,int width,int height,int first,int last);

/* Install after game bootstrap; preserves pause palette suppression. */
void sg_graphics_fast_api(lua_State *L);
