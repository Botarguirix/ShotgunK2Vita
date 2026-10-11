#pragma once
#include <stdio.h>
#include <stdint.h>

enum { SG_DIAG_BOOT, SG_DIAG_INPUT, SG_DIAG_UPDATE, SG_DIAG_DRAW,
       SG_DIAG_PRESENT, SG_DIAG_DISPLAY, SG_DIAG_IDLE, SG_DIAG_ERROR, SG_DIAG_EXIT };
int sg_diag_start(const char *root);
const char *sg_diag_directory(void);
FILE *sg_diag_open(const char *name);
void sg_diag_phase(unsigned phase);
void sg_diag_frame(unsigned frame, unsigned buttons, int lx, int ly, int rx, int ry);
void sg_diag_location(const char *source, int line);
void sg_diag_event(const char *kind, const char *detail);
void sg_diag_memory(unsigned lua_kb, unsigned graphics_kb);
void sg_diag_audio(unsigned blocks, int rc);
void sg_diag_worker(unsigned worker, unsigned phase);
void sg_diag_stop(const char *reason);
