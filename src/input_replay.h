#pragma once
#include <stdio.h>
#include <stdint.h>
/* Deterministic input recording/replay, adapted from Vagante2Vita's
 * input_replay. The game logic only depends on these inputs, the fixed 1/60
 * step and an unseeded rand(), so the same file replays the same run.
 * File SGR1: 8-byte header, then 10-byte runs {u32 buttons, u8 lx,ly,rx,ry,
 * u16 frames}, little endian, appended until EOF. */
typedef struct {
 int mode; /* 0 off, 1 record, 2 replay */
 FILE *out;
 unsigned char *data;size_t size,pos;
 uint32_t buttons;unsigned char axes[4];unsigned ticks,left;
 unsigned frames,runs,aborted,done;
} SgReplay;
#ifndef SG_DATA_ROOT
#define SG_DATA_ROOT "ux0:data/shotgunk2vita"
#endif
#ifndef SG_REPLAY_DIR
#define SG_REPLAY_DIR SG_DATA_ROOT "/replay"
#endif
/* Before sg_boot. Record: snapshot save/ and open the file. Replay: load the
 * file, restore the snapshot into replay/save and redirect save/ there. */
int sg_replay_start(SgReplay *r,int mode,const char *root,FILE *log);
/* Once per frame, before sg_step: records the inputs or replaces them. */
void sg_replay_frame(SgReplay *r,unsigned *buttons,unsigned char *lx,unsigned char *ly,unsigned char *rx,unsigned char *ry,unsigned physical,FILE *log);
void sg_replay_stop(SgReplay *r,FILE *log);
