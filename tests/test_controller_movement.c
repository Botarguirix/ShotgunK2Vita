#include "runtime.h"
#include "controls.h"
#include <stdio.h>
static int step(unsigned buttons,int analog){int toggle;unsigned front=sg_touch_shoulder(960,544);return sg_step(sg_controls(buttons,&toggle)|front,128,analog?0:128,94,129,0,160,90);}
int main(int argc,char**argv){if(argc<2)return 2;int ok=sg_boot(argv[1],stdout);for(int i=0;ok&&i<90;i++)ok=step(0,0);
 for(int analog=0;ok&&analog<2;analog++){
  ok=sg_host_eval("NO_INTRO=true;set_mode('throne');mode.start()");for(int i=0;ok&&i<720;i++)ok=step(0,0);
  if(ok)ok=sg_host_eval("assert(hero and hero.sq and playing); test_start_sq=hero.sq");
  for(int i=0;ok&&i<9;i++)ok=step(analog?0:SG_UP,analog);
  if(ok)ok=step(SG_CROSS|(analog?0:SG_UP),analog);
  for(int i=0;ok&&i<120;i++)ok=step(0,0);
  if(ok)ok=sg_host_eval("assert(GAMEPAD and not VPAD and not TOUCH); assert(hero.sq~=test_start_sq); assert(hero.x==hero.sq.x and hero.y==hero.sq.y)");
  sg_report_movement();printf("KING_CELL_AND_SPRITE analog=%d result=%d\n",analog,ok);
 }
 if(!ok)puts(sg_error());sg_close();return !ok;
}
