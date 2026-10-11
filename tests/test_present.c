#include "graphics.h"
#include "render_pool.h"
#include "lauxlib.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char **argv){
 if(argc<2)return 2;lua_State *L=luaL_newstate();sg_graphics_init(L,argv[1]);
 assert(!luaL_dostring(L,"cls(3);rectfill(20,30,140,110,7);newsrf(640,360,'hdtext');target('hdtext');cls(255);rectfill(60,80,90,100,11);target()"));
 int width=960,height=544;size_t size=(size_t)width*height*4;
 uint32_t *serial=malloc(size),*parallel=malloc(size+64),*source=malloc(320*180*4);assert(serial&&parallel&&source);
 memset(parallel,0xa5,size+64);sg_present(serial,width,height);sg_present(source,320,180);
 for(int y=0;y<height;y++)for(int x=0;x<width;x++){
  int sx=x/3,sy=(y-2)/3;uint32_t expected=y>=2&&y<542?source[sy*320+sx]:0xff000000u;
  assert(serial[y*width+x]==expected);
 }
 for(int workers=1;workers<=3;workers++){
  assert(sg_render_pool_start(workers,NULL)==workers);sg_render_pool_present(parallel,width,height);sg_render_pool_stop();
  assert(!memcmp(serial,parallel,size));for(int i=0;i<64;i++)assert(((unsigned char *)parallel)[size+i]==0xa5);
 }
 assert(!luaL_dostring(L,"camera(4,5);cls(0);spritesheet('hdtext');sprgrid(16);aspr(0,40,40,0,.5,.5);circ(40,40,4,7)"));
 sg_present(serial,width,height);
 free(serial);free(parallel);free(source);sg_graphics_close();lua_close(L);puts("PRESENT_SERIAL_PARALLEL_AND_SCALE_PASSED");return 0;
}
