#include "runtime.h"
#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
int main(int argc,char **argv){
 if(argc<2)return 2;
 int ok=sg_boot(argv[1],stdout),frame=0;unsigned shots=0;
 uint32_t *pixels=malloc(320*180*4);if(!pixels){sg_close();return 2;}
 for(;ok&&frame<6000;frame++){
  unsigned buttons=0;
  if(frame==300||frame==400||frame==650||frame==900||frame==1200||frame==1500||frame==1800||frame==2100||frame==2400||frame==2700||frame==3000||frame==3300)buttons=SG_CROSS;
  if(frame==1600||frame==1700)buttons=SG_DOWN;
  int rx=128,ry=128;
  if(frame>=3800){double a=(frame-3800)*6.283185307179586/480;rx=128+(int)(120*cos(a));ry=128+(int)(120*sin(a));}
  if(frame>=4000&&frame%180==40){buttons|=SG_R;shots++;}
  if(frame>=4100&&frame%180==100)buttons|=SG_TRIANGLE;
  if(frame==5000||frame==5100)buttons=SG_START;
  ok=sg_step(buttons,128,128,rx,ry,0,160,90);sg_present(pixels,320,180);
 }
 int expected=argc>2&&!strcmp(argv[2],"--expect-old-error");
 if(ok){
  ok=sg_host_eval("assert(type(OPTIONS.music)==\"table\"); assert(ismobile()==MOBILE)");
  if(expected)ok=sg_host_eval("ismobile=nil");
  if(ok)ok=sg_host_eval("pause_game()");
  for(int i=0;ok&&i<30;i++)ok=sg_step(0,128,128,128,128,0,160,90);
  if(ok)ok=sg_host_eval("assert(pause); unpause(); assert(not pause)");
  printf("ACTUAL_PAUSE_ROUTE=%s\n",ok?"passed":"failed");
 }
 if(expected){int reproduced=!ok&&strstr(sg_error(),"nil value");printf("OLD_COMBAT_ERROR=%d frame=%d shots=%u\n",reproduced,frame,shots);free(pixels);sg_close();return reproduced?0:1;}
 if(!ok)fprintf(stderr,"%s\n",sg_error());
 sg_report_state();printf("COMBAT_REGRESSION=%s frames=%d pause_inputs=2 R_inputs=%u\n",ok?"passed":"failed",frame,shots);
 free(pixels);sg_close();return ok&&shots>5?0:1;
}
