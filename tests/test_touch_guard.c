#include "runtime.h"
#include "controls.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){if(argc<2)return 2;
 if(sg_touch_back_shoulder(650,100)!=SG_LPAGE||sg_touch_back_shoulder(1250,100)!=SG_RPAGE||sg_touch_back_shoulder(100,540)!=0||sg_touch_back_shoulder(960,540)!=0)return 3;
 int ok=sg_boot(argv[1],stdout);for(int i=0;ok&&i<90;i++)ok=sg_step(0,128,128,128,128,0,160,90);
 if(!ok)return 1;
 if(ok)ok=sg_host_eval("hero=nil; TOUCH=true; board={x=0,y=0}; local e=mk_sq_but({px=0,py=0},function()end,8); e.dr(e,0,0)");
 if(argc>2&&!strcmp(argv[2],"--expect-old-error")){int hit=!ok&&strstr(sg_error(),"hero");printf("OLD_TOUCH_HERO_ERROR=%d error=%s\n",hit,sg_error());sg_close();return !hit;}
 if(ok)ok=sg_step(0,128,128,128,128,1,160,90);
 for(int i=0;ok&&i<20;i++)ok=sg_step(0,128,128,128,128,0,160,90);
 printf("TOUCH_WITHOUT_HERO=%s\n",ok?"passed":"failed");if(!ok)puts(sg_error());sg_close();return !ok;
}
