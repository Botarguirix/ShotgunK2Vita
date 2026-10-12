#pragma once
#include <stdio.h>
int sg_boot(const char *root, FILE *log);
const char *sg_error(void);
int sg_should_quit(void);
void sg_close(void);
int sg_step(unsigned buttons,int lx,int ly,int rx,int ry,int touch,int x,int y);
void sg_report_state(void);
unsigned sg_lua_memory_kb(void);
#ifndef SG_VITA
int sg_host_eval(const char *script);
#endif
#define SG_CROSS 1u
#define SG_CIRCLE 2u
#define SG_SQUARE 4u
#define SG_TRIANGLE 8u
#define SG_LEFT 16u
#define SG_RIGHT 32u
#define SG_UP 64u
#define SG_DOWN 128u
#define SG_L 256u
#define SG_R 512u
#define SG_START 1024u
#define SG_SELECT 2048u
#define SG_LPAGE 4096u
#define SG_RPAGE 8192u
#define SG_LSTICK 16384u
#define SG_RSTICK 32768u

/* Low-rate physical movement diagnostics; no on-screen overlay. */
void sg_report_movement(void);
