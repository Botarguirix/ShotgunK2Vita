#include "graphics.h"
#include "lauxlib.h"
#include "lualib.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static uint32_t pixels[320*180],reference[320*180];
static void script(lua_State *L,const char *code){if(luaL_dostring(L,code)){fprintf(stderr,"%s\n",lua_tostring(L,-1));assert(0);}}
static int64_t edge(int ax,int ay,int bx,int by,int x,int y){return (int64_t)(bx-ax)*(y-ay)-(int64_t)(by-ay)*(x-ax);}
static void filled(lua_State *L,int ax,int ay,int bx,int by,int cx,int cy){
    char code[256];snprintf(code,sizeof(code),"camera();clip();fillp();pal();blend();cls(0);trifill(%d,%d,%d,%d,%d,%d,7)",ax,ay,bx,by,cx,cy);script(L,code);sg_present(pixels,320,180);
    int64_t area=edge(ax,ay,bx,by,cx,cy);assert(area);
    for(int y=0;y<180;y++)for(int x=0;x<320;x++){
        int64_t a=edge(ax,ay,bx,by,x,y),b=edge(bx,by,cx,cy,x,y),c=edge(cx,cy,ax,ay,x,y);
        int inside=area>0?(a>=0&&b>=0&&c>=0):(a<=0&&b<=0&&c<=0);
        assert((pixels[y*320+x]&255)==(inside?7:0));
    }
}
int main(int argc,char **argv){
    assert(argc>1);lua_State *L=luaL_newstate();assert(L);luaL_openlibs(L);sg_graphics_init(L,argv[1]);
    filled(L,2,2,9,2,2,9);memcpy(reference,pixels,sizeof(pixels));
    filled(L,2,2,2,9,9,2);assert(!memcmp(reference,pixels,sizeof(pixels)));
    filled(L,-4,-4,10,-4,-4,10);filled(L,320,180,-50,180,320,-50);
    filled(L,3,9,1,11,3,13);filled(L,19,9,21,11,19,13);
    filled(L,9,3,11,1,13,3);filled(L,9,19,11,21,13,19);
    filled(L,3,4,12,9,3,16);filled(L,-20,9,200,-30,400,200);
    filled(L,-1000000,-1000000,1000000,-1000000,0,1000000);
    script(L,"camera(10,20);cls(0);color(7);trifill(12,22,19,22,12,29)");sg_present(pixels,320,180);assert(!memcmp(reference,pixels,sizeof(pixels)));
    script(L,"camera();cls(0);clip(4,4,2,2);trifill(2,2,9,2,2,9,7)");sg_present(pixels,320,180);
    for(int y=0;y<180;y++)for(int x=0;x<320;x++)assert((pixels[y*320+x]&255)==(x>=4&&x<=5&&y>=4&&y<=5?7:0));
    script(L,"clip();cls(0);fillp(43690,true);trifill(2,2,9,2,2,9,7)");sg_present(pixels,320,180);
    for(int y=0;y<180;y++)for(int x=0;x<320;x++)assert((pixels[y*320+x]&255)==((reference[y*320+x]&255)&&!((43690>>((y&3)*4+(x&3)))&1)?7:0));
    script(L,"cls(0);fillp(43690);trifill(2,2,9,2,2,9,3079)");sg_present(pixels,320,180);
    for(int y=0;y<180;y++)for(int x=0;x<320;x++)assert((pixels[y*320+x]&255)==((reference[y*320+x]&255)?((43690>>((y&3)*4+(x&3)))&1?12:7):0));
    script(L,"fillp();cls(0);pal(7,9);blend(9,0,11);trifill(2,2,9,2,2,9,7)");sg_present(pixels,320,180);
    for(int i=0;i<320*180;i++)assert((pixels[i]&255)==((reference[i]&255)?11:0));
    script(L,"pal();blend();cls(0);trifill(5,5,5,5,5,5,7);assert(pget(5,5)==7);trifill(2,2,6,6,9,9,7);assert(pget(7,7)==7)");
    script(L,"cls(0);tri(2,2,9,2,2,9,7);assert(pget(2,2)==7 and pget(3,3)==0 and pget(5,6)==7)");
    script(L,"tri(-1000000,-1000000,1000000,1000000,1000000,-1000000,7);assert(not pcall(function() trifill(0/0,0,1,1,2,2) end))");
    sg_graphics_close();lua_close(L);puts("TRIANGLES_PASSED: coverage, winding, camera, clipping, patterns, palette, blend, degenerates and bounded edges");return 0;
}
