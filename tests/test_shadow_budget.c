#include "graphics.h"
#include "lauxlib.h"
#include "lualib.h"
#include "perf.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
static void eval(lua_State*L,const char*s){if(luaL_dostring(L,s)){puts(lua_tostring(L,-1));exit(1);}}
static unsigned hash(uint32_t*p){unsigned h=2166136261u;for(int i=0;i<320*180;i++)h=(h^p[i])*16777619u;return h;}
int main(int argc,char**argv){if(argc<2)return 2;lua_State*L=luaL_newstate();luaL_openlibs(L);sg_graphics_init(L,argv[1]);
 eval(L,"function bright(k,n) return sget(16+k,math.min(8,math.max(1,4+n))) end; function shpr(px,py,pw,ph,x,y,inc) inc=inc or -1; for dx=0,pw-1 do for dy=0,ph-1 do if sget(px+dx,py+dy)>0 then pset(x+dx,y+dy,bright(pget(x+dx,y+dy),inc)) end end end end; reference_shpr=shpr;saved_bright=bright;newsrf(64,16,'shadow_sheet');target('shadow_sheet');for x=0,63 do for y=0,15 do pset(x,y,(x+y)%6) end end;target()");
 sg_graphics_fast_api(L);eval(L,"native_shpr=shpr");uint32_t*p=malloc(320*180*4);assert(p);
 for(int mode=0;mode<6;mode++){unsigned h[2];for(int native=0;native<2;native++){
  eval(L,"bright=saved_bright;pal();palt();sfillp();fillp();blend();camera();clip();target();cls(3);spritesheet('shadow_sheet')");
  if(mode==1)eval(L,"camera(3,5);clip(20,19,18,20)");
  if(mode==2)eval(L,"fillp(43690,true);pal(2,6);blend(1,3,8)");
  if(mode==4)eval(L,"spritesheet('screen')");
  if(mode==5)eval(L,"bright=function(k,n)return (saved_bright(k,n)+1)%16 end");
  eval(L,native?(mode==3?"native_shpr(-.5,1.5,15.5,9.5,-.5,20.5,-1.5)":"native_shpr(0,0,16,12,20,20,-1)"):(mode==3?"reference_shpr(-.5,1.5,15.5,9.5,-.5,20.5,-1.5)":"reference_shpr(0,0,16,12,20,20,-1)"));sg_present(p,320,180);h[native]=hash(p);
 }assert(h[0]==h[1]);printf("SHADOW_EQUIVALENCE mode=%d hash=%08x\n",mode,h[0]);}
 eval(L,"bright=saved_bright;fillp();blend();camera();clip();pal();target();cls(3);spritesheet('shadow_sheet')");uint64_t t=sg_now_us();eval(L,"for i=1,3000 do reference_shpr(0,0,16,12,20,20,-1) end");uint64_t old=sg_now_us()-t;t=sg_now_us();eval(L,"for i=1,3000 do native_shpr(0,0,16,12,20,20,-1) end");uint64_t fast=sg_now_us()-t;printf("SHADOW_3000_REFERENCE_US=%llu NATIVE_US=%llu PC_NOT_VITA\n",(unsigned long long)old,(unsigned long long)fast);free(p);sg_graphics_close();lua_close(L);return 0;}
