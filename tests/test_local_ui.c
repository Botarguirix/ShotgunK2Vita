#include "runtime.h"
#include "graphics.h"
#include "local_ui.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv){if(argc<3)return 2;int ok=sg_boot(argv[1],stdout);for(int i=0;ok&&i<90;i++)ok=sg_step(0,128,128,128,128,0,160,90);if(ok)ok=sg_host_eval("mode=nil; DEN.achievements[ACHIEVEMENTS[1].id]=nil; trig_achievement(ACHIEVEMENTS[1].id)");for(int i=0;ok&&i<35;i++)ok=sg_step(0,128,128,128,128,0,160,90);if(ok)ok=sg_ui_current_notice()!=NULL;uint32_t *p=malloc(320*180*4);sg_present(p,320,180);unsigned dark=0,white=0;for(int y=4;y<39;y++)for(int x=16;x<304;x++){dark+=p[y*320+x]==0xff201018u;white+=p[y*320+x]==0xffffffffu;}ok=ok&&dark>1000&&white>80;printf("NATIVE_POPUP_CONTRAST=%d dark=%u white=%u\n",ok,dark,white);
FILE*f=fopen(argv[2],"wb");fprintf(f,"P6\n320 180\n255\n");for(int i=0;i<320*180;i++){unsigned char c[3]={p[i],p[i]>>8,p[i]>>16};fwrite(c,1,3,f);}fclose(f);free(p);if(ok)ok=sg_host_eval("init_codex(); assert(codex_mode==\"codex\"); assert(DEN.achievements[ACHIEVEMENTS[1].id]); url(\"https://example.invalid\"); twitch(); discord()");for(int i=0;ok&&i<10;i++)ok=sg_step(SG_RPAGE,128,128,128,128,0,160,90);if(ok)ok=sg_host_eval("assert(codex_mode==\"achievement\")");printf("LOCAL_CODEX_AND_DISABLED_LINKS=%d\n",ok);if(!ok)puts(sg_error());sg_close();return !ok;}
