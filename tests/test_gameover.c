#include "runtime.h"
#include <stdio.h>
#include <string.h>
static int idle(int frames){for(int i=0;i<frames;i++)if(!sg_step(0,128,128,128,128,0,160,90))return 0;return 1;}
int main(int argc,char**argv){
 if(argc<2)return 2;int old=argc>2&&!strcmp(argv[2],"--expect-old-error");
 int ok=sg_boot(argv[1],stdout)&&idle(90);
 if(ok)ok=sg_host_eval("NO_INTRO=true;set_mode('throne');mode.start()")&&idle(720);
 if(ok)ok=sg_host_eval("assert(hero and hero.sq and playing);remove_buts();test_left=0;for e in all(ents) do if e.button then test_left=test_left+1 end end;_log('BUTTONS_LEFT='..test_left);play();xpl(hero);assert(hero.dead and not hero.sq)");
 if(old){
  int error=ok&&!sg_host_eval("shoot(false,squares[1])")&&strstr(sg_error(),"nil value");
  printf("OLD_GAMEOVER_ERROR=%d\n",error);sg_close();return !error;
 }
 if(ok)ok=sg_host_eval("assert(test_left==0);shoot(false,squares[1]);gameover()");
 for(int i=0;ok&&i<300;i++)ok=sg_step(i%30==0?SG_CROSS|SG_R:0,128,128,220,128,0,160,90);
 if(ok)ok=sg_host_eval("NO_INTRO=true;set_mode('throne');mode.start()")&&idle(720);
 if(ok)ok=sg_host_eval("assert(hero and hero.sq and not hero.dead and playing);test_sq=hero.sq");
 if(ok)ok=sg_step(SG_UP,128,128,128,128,0,160,90)&&idle(8)&&sg_step(SG_CROSS|SG_UP,128,128,128,128,0,160,90)&&idle(120);
 if(ok)ok=sg_host_eval("assert(hero.sq~=test_sq);assert(hero.x==hero.sq.x and hero.y==hero.sq.y)");
 if(!ok)puts(sg_error());printf("GAMEOVER_RESTART=%d\n",ok);sg_close();return !ok;
}
