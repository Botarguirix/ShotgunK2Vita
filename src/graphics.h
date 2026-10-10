#pragma once
#include "lua.h"
#include <stdint.h>
void sg_graphics_init(lua_State *L,const char *root);
void sg_graphics_close(void);
void sg_present(uint32_t *pixels,int width,int height);
