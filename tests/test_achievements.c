#include "runtime.h"
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv){
 if(argc<2)return 2;
 int ok=sg_boot(argv[1],stdout);
 for(int i=0;ok&&i<90;i++)ok=sg_step(0,128,128,128,128,0,160,90);
 if(ok)ok=sg_host_eval("mode=nil; local id=ACHIEVEMENTS[1].id; assert(id); DEN.achievements[id]=nil; trig_achievement(id); assert(DEN.achievements[id]); DEN.achievements.save(); local s=file('save/achievements.sav'); assert(type(s)=='string' and s:find(id,1,true))");
 if(argc>2&&!strcmp(argv[2],"--expect-old-error")){
  int reproduced=!ok&&strstr(sg_error(),"fb_ev");printf("OLD_ACHIEVEMENT_ERROR=%d error=%s\n",reproduced,sg_error());sg_close();return reproduced?0:1;
 }
 if(!ok)fprintf(stderr,"%s\n",sg_error());
 printf("ACHIEVEMENT_UNLOCK_AND_SAVE=%s\n",ok?"passed":"failed");sg_close();return ok?0:1;
}
