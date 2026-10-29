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
  "assert(not pcall(fntspec,'fallback','Terminus'));"
  "local has_old=pcall(font,'old_console');if has_old then assert(strheight()>0) end; font(f);"
  "assert(console_sheet=='console_PS4' and not MOUSE,'prompts '..tostring(console_sheet));"
  /* Boss-death rays punch the hdtext overlay with its key 255 under apal(5). */
  "assert(#palette()<255,'palette '..#palette());"
  "if OVERLAY_SURF then target(OVERLAY_SURF);apal(5);pset(0,0,OVERLAY_TKEY);local v=pget(0,0);pal();target();"
  "assert(v==OVERLAY_TKEY,'overlay key remapped to '..v) else log('PRIVATE_OVERLAY_TEST=not_applicable') end");
 /* Extra Lua checks after boot (e.g. API coverage for Workshop mods). */
 const char *extra=getenv("SG_SMOKE_EVAL");
 if(ok&&extra)ok=sg_host_eval(extra);
 if(ok&&argc>3&&!strcmp(argv[3],"game")){
  ok=sg_host_eval("srand(1);NO_INTRO=true;set_mode('throne');mode.start()");
 }
 /* "touch": the front touch drives the game's own mouse mode in the menu
  * (cursor at the finger, PC prompts) and a pad press returns to PS4. */
 int touch_test=argc>3&&!strcmp(argv[3],"touch");
 int i=0;
 for(;ok&&i<frames;i++){
  unsigned buttons=i==730?SG_UP:i==740?SG_CROSS:i==800?SG_R:i==850?SG_TRIANGLE:0;
  int touch=0,x=160,y=90;
  /* SG_SMOKE_PRESS="frame:bits,frame:bits": pad presses for screenshots. */
  const char *press=getenv("SG_SMOKE_PRESS");
  for(const char *p=press;p&&*p;){int at=0,bits=0;if(sscanf(p,"%d:%d",&at,&bits)==2&&at==i)buttons|=(unsigned)bits;p=strchr(p,',');if(p)p++;}
  if(touch_test){buttons=i==260?SG_DOWN:0;if(i>=200&&i<240){x=100+(i-200);y=50;}else if(i>=240){x=139;y=50;}}
  ok=sg_step(buttons,128,128,128,128,touch,x,y);
  if(ok&&touch_test&&i==230)ok=sg_host_eval("assert(MOUSE,'touch did not enter mouse mode');"
   "assert(btnv('mx')==130 and btnv('my')==50,'cursor '..btnv('mx')..','..btnv('my'));"
   "assert(btn('mouse_move'))");
  if(ok&&touch_test&&i==290)ok=sg_host_eval("assert(not MOUSE and console_sheet=='console_PS4','pad did not restore PS4 prompts')");
  if(ok&&argc>3&&!strcmp(argv[3],"game")&&i==719)ok=sg_host_eval("assert(hero and hero.sq and playing,'Steam game state not ready')");
 }
 if(ok&&touch_test){
  ok=sg_step(0,128,128,128,128,1,2,178)&&sg_host_eval("assert(btnp('m:lb') and not btn('m:rb'),'left click')")&&
   sg_step(0,128,128,128,128,0,2,178)&&sg_step(0,128,128,128,128,2,2,178)&&
   sg_host_eval("assert(btnp('m:rb') and not btn('m:lb'),'right click');run()")&&sg_restart_requested();
  printf("STEAM_TOUCH mouse_mode=1 pad_return=1 run_restart=%d\n",ok);}
 /* Optional capture of the last frame (320x180, 0xAABBGGRR) as a PPM. */
 const char *ppm=getenv("SG_SMOKE_PPM"),*draw=getenv("SG_SMOKE_DRAW"),*overlay=getenv("SG_SMOKE_OVERLAY");
 if(ok&&draw)ok=sg_host_eval(draw);
 /* As on the Vita GPU path: scene without hdtext, overlay exported apart. */
 if(ok&&overlay){static uint32_t layer[640*360];int w,h;sg_graphics_set_overlay_separate(1);
  if(sg_overlay_export(layer,640,360,&w,&h)){FILE *f=fopen(overlay,"wb");if(f){fprintf(f,"P7\nWIDTH %d\nHEIGHT %d\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n",w,h);fwrite(layer,4,(size_t)w*h,f);fclose(f);}}}
 if(ok&&ppm){static uint32_t frame[320*180];sg_present(frame,320,180);FILE *f=fopen(ppm,"wb");
  if(f){fprintf(f,"P6 320 180 255\n");for(int p=0;p<320*180;p++){unsigned char rgb[3]={frame[p]&255,frame[p]>>8&255,frame[p]>>16&255};fwrite(rgb,1,3,f);}fclose(f);}}
 if(!ok)fprintf(stderr,"STEAM_SMOKE_FAILED frame=%d: %s\n",i,sg_error());
 printf("STEAM_SMOKE frames=%d success=%d lua_kb=%u surfaces_kb=%u\n",i,ok,sg_lua_memory_kb(),sg_graphics_memory_kb());
 sg_report_state();sg_close();return ok?0:1;
}
