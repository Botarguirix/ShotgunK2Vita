#include "runtime.h"
#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
int main(int argc,char **argv){
    if(argc<2)return 2;int ok=sg_boot(argv[1],stdout),frame=0;
    uint32_t *pixels=malloc(320*180*sizeof(*pixels));if(!pixels){sg_close();return 2;}
    for(;ok&&frame<5400;frame++){
        unsigned buttons=0;
        if(frame==300||frame==400||frame==650||frame==900||frame==1200||frame==1500||frame==1800||frame==2100||frame==2400||frame==2700||frame==3000||frame==3300)buttons=SG_CROSS;
        if(frame==1600||frame==1700)buttons=SG_DOWN;
        int rx=128,ry=128;
        if(frame>=3800){double angle=(frame-3800)*6.283185307179586/240;rx=128+(int)(120*cos(angle));ry=128+(int)(120*sin(angle));}
        ok=sg_step(buttons,128,128,rx,ry,0,160,90);sg_present(pixels,320,180);
    }
    int expected=argc>2&&!strcmp(argv[2],"--expect-missing-trifill");
    if(expected){int reproduced=!ok&&strstr(sg_error(),"trifill")&&strstr(sg_error(),"nil value");printf("OLD_TRIFILL_FAILURE_REPRODUCED=%d frame=%d\n",reproduced,frame);free(pixels);sg_close();return reproduced?0:1;}
    if(!ok)fprintf(stderr,"%s\n",sg_error());
    if(ok&&argc>2){FILE *file=fopen(argv[2],"wb");if(file){fprintf(file,"P6\n320 180\n255\n");for(int i=0;i<320*180;i++){unsigned char rgb[3]={pixels[i],pixels[i]>>8,pixels[i]>>16};fwrite(rgb,1,3,file);}fclose(file);}}
    sg_report_state();free(pixels);sg_close();printf("RIGHT_STICK_REGRESSION=%s frames=%d\n",ok?"passed":"failed",frame);return ok?0:1;
}
