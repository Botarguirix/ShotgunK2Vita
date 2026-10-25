#include "runtime.h"
#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char **argv){
 if(argc<2)return 2;
 int frames=argc>2?atoi(argv[2]):600;
 if(frames<0||frames>6000)return 2;
 if(argc>4)sg_set_save_redirect(argv[4]);
 sg_set_jit_mode(1);
 if(!sg_boot(argv[1],stdout)){fprintf(stderr,"BOOT_FAILED: %s\n",sg_error());sg_close();return 1;}
 int ok=sg_host_eval("assert(VERSION=='1.623b');assert(SG_STEAM_BRIDGE);"
  "local f=font();font('Terminus');assert(strheight()>0);"
  "assert(strheight('a\\nb')==2*strheight());"
  "assert(fntspec('fallback')=='TerminusThai');"
  "assert(not pcall(fntspec,'fallback','Terminus'));font(f)");
 if(ok&&argc>3&&!strcmp(argv[3],"game")){
  ok=sg_host_eval("srand(1);NO_INTRO=true;set_mode('throne');mode.start()");
 }
 int i=0;
 for(;ok&&i<frames;i++){
  unsigned buttons=i==730?SG_UP:i==740?SG_CROSS:i==800?SG_R:i==850?SG_TRIANGLE:0;
  ok=sg_step(buttons,128,128,128,128,0,160,90);
  if(ok&&argc>3&&i==719)ok=sg_host_eval("assert(hero and hero.sq and playing,'Steam game state not ready')");
 }
 if(!ok)fprintf(stderr,"STEAM_SMOKE_FAILED frame=%d: %s\n",i,sg_error());
 printf("STEAM_SMOKE frames=%d success=%d lua_kb=%u surfaces_kb=%u\n",i,ok,sg_lua_memory_kb(),sg_graphics_memory_kb());
 sg_report_state();sg_close();return ok?0:1;
}
