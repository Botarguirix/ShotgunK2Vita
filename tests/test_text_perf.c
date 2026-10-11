#include "graphics.h"
#include "perf.h"
#include "lauxlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
int main(int argc,char **argv){
 if(argc<2)return 2;lua_State *L=luaL_newstate();sg_graphics_init(L,argv[1]);
 const char *script="cls(0);camera(11,7);clip(4,5,250,140);pal(7,12);fillp(43690,true);print('Shotgun King - Vita 0123456789',-4,20,7);fillp();print('Testing glyph clipping and palette',5,40,7);blend(7,0,5);print('A second line with accents: áéíóú',10,60,7);camera();clip();blend();pal()";
 if(luaL_loadstring(L,script))return 2;lua_pushvalue(L,-1);if(lua_pcall(L,0,0,0))return 2;
 uint64_t start=sg_now_us();int iterations=5000;
 for(int i=0;i<iterations;i++){lua_pushvalue(L,-1);if(lua_pcall(L,0,0,0))return 2;}
 uint64_t elapsed=sg_now_us()-start;uint32_t *p=malloc(320*180*4);if(!p)return 2;sg_present(p,320,180);
 uint64_t hash=14695981039346656037ULL;for(int i=0;i<320*180*4;i++){hash^=((unsigned char *)p)[i];hash*=1099511628211ULL;}
 printf("TEXT_BITMAP_HASH=%016llx iterations=%d draw_us=%llu\n",(unsigned long long)hash,iterations,(unsigned long long)elapsed);
 free(p);sg_graphics_close();lua_close(L);return 0;
}
