#include "steam_install.h"
#include <stdio.h>

/* Usage: sg_steam_install_probe ROOT  (ROOT holds a copy of data.sgr) */
static void progress(unsigned percent,void *user){(void)user;if(percent%10==0)printf("progress=%u\n",percent);}
int main(int argc,char **argv){
 if(argc<2)return 2;
 int ok=sg_steam_install(argv[1],stdout,progress,NULL);
 printf("STEAM_INSTALL first=%d\n",ok);
 /* The second boot must find the marker and do no work. */
 int again=ok&&sg_steam_install(argv[1],stdout,progress,NULL);
 printf("STEAM_INSTALL second=%d\n",again);
 return ok&&again?0:1;
}
