#include "graphics.h"
#include "lauxlib.h"
#include "perf.h"
#include <stdlib.h>
#include <stdio.h>
int main(int argc,char **argv){if(argc<2)return 2;lua_State*L=luaL_newstate();sg_graphics_init(L,argv[1]);if(luaL_dostring(L,"cls(3);rectfill(20,30,140,110,7);newsrf(640,360,'hdtext');target('hdtext');cls(255);rectfill(60,80,90,100,11);target()"))return 1;uint32_t*p=malloc(320*180*4);uint64_t t=sg_now_us();for(int i=0;i<400;i++)sg_present(p,320,180);t=sg_now_us()-t;unsigned h=2166136261u;for(int i=0;i<320*180;i++)h=(h^p[i])*16777619u;printf("logical_compose_400_us=%llu hash=%08x platform=PC_NOT_VITA\n",(unsigned long long)t,h);free(p);sg_graphics_close();lua_close(L);return 0;}
