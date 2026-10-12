#include "graphics.h"
#include "lauxlib.h"
#include "lualib.h"
#include "perf.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
static void eval(lua_State*L,const char*s){if(luaL_dostring(L,s)){puts(lua_tostring(L,-1));exit(1);}}
static unsigned hash(uint32_t*p){unsigned h=2166136261u;for(int i=0;i<320*180;i++)h=(h^p[i])*16777619u;return h;}
int main(int argc,char**argv){if(argc<2)return 2;lua_State*L=luaL_newstate();luaL_openlibs(L);sg_graphics_init(L,argv[1]);sg_graphics_fast_api(L);
 eval(L,"native_reset=pal_rst; function reference_reset() for i=0,9 do pal(i,i); pal(i,i,1) end; for i=1,9 do palt(i,false) end; palt(0,true); sfillp(1,0,1) end; saved_pal=pal; newsrf(16,16,'probe');target('probe');cls(1);target()");
 uint32_t*a=malloc(320*180*4),*b=malloc(320*180*4);assert(a&&b);
 for(int paused=0;paused<3;paused++){
  unsigned values[2];for(int native=0;native<2;native++){
   eval(L,"pal=saved_pal;pal();palt();sfillp();fillp();blend();camera();clip();target();cls(0);pal(1,7);pal(7,3,true);pal(10,12);pal(10,13,true);palt(1,true);palt(10,true);sfillp(1,43690,513)");
   if(paused==1)eval(L,"pal=function()end");if(paused==2)eval(L,"pal=function(a,b,c) saved_pal(a,b,c) end");eval(L,native?"native_reset()":"reference_reset()");
   eval(L,"pal=saved_pal;spritesheet('probe');sspr(0,0,16,16,20,20);rectfill(60,60,75,75,10)");sg_present(native?b:a,320,180);values[native]=hash(native?b:a);
  }assert(values[0]==values[1]);printf("PALETTE_EQUIVALENCE paused=%d hash=%08x\n",paused,values[0]);
 }
 uint64_t t=sg_now_us();eval(L,"for i=1,60000 do reference_reset() end");uint64_t old=sg_now_us()-t;t=sg_now_us();eval(L,"for i=1,60000 do native_reset() end");uint64_t fast=sg_now_us()-t;
 printf("PALETTE_60000_REFERENCE_US=%llu NATIVE_US=%llu platform=PC_NOT_VITA\n",(unsigned long long)old,(unsigned long long)fast);free(a);free(b);sg_graphics_close();lua_close(L);return 0;
}
