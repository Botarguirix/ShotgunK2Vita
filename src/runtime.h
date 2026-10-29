#pragma once
#include <stdio.h>
/* Data folder on the Vita; the Steam build is a separate app with its own. */
#ifndef SG_DATA_ROOT
#define SG_DATA_ROOT "ux0:data/shotgunk2vita"
#endif
int sg_boot(const char *root, FILE *log);
const char *sg_error(void);
int sg_should_quit(void);
/* The game called run() (SUGAR cart reboot): relaunch the app. */
int sg_restart_requested(void);
void sg_close(void);
int sg_step(unsigned buttons,int lx,int ly,int rx,int ry,int touch,int x,int y);
void sg_report_state(void);
unsigned sg_lua_memory_kb(void);
void sg_set_fps_limit(int limit);
int sg_get_fps_limit(void);
/* Before sg_boot: drop the Lua hook and entity wrappers after bootstrap. */
void sg_set_jit_mode(int on);
/* Before sg_boot: time global Lua functions and log the top ones on spikes. */
void sg_set_profile_funcs(int on);
/* Before sg_boot: time-weighted Lua line sampler, top lines every 600 frames. */
void sg_set_profile_lines(int on);
/* Map "save/..." to this directory (NULL or "" restores the real save/). */
void sg_set_save_redirect(const char *dir);
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
