#pragma once
#include "lua.h"
#include <stdint.h>
#include <stdio.h>
void sg_audio_init(lua_State *L,const char *root,FILE *log);
int sg_audio_start(FILE *log);
void sg_audio_mix(int16_t *out,unsigned frames);
void sg_audio_close(void);
typedef struct {unsigned music_starts,music_loops,music_failures,fx_starts,queue_drops;} SgAudioStats;
SgAudioStats sg_audio_stats(void);
