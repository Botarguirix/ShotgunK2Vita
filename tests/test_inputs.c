#include "runtime.h"
#include "controls.h"
#include <assert.h>
#include <stdio.h>

int main(int argc,char **argv){
 int toggle;unsigned bits;
 assert(sg_touch_shoulder(0,968)==0);
 assert(sg_touch_shoulder(1919,1087)==0);
 assert(sg_touch_back_shoulder(0,0)==SG_LPAGE);
 assert(sg_touch_back_shoulder(639,1087)==SG_LPAGE);
 assert(sg_touch_back_shoulder(1280,0)==SG_RPAGE);
 assert(sg_touch_back_shoulder(1919,1087)==SG_RPAGE);
 assert(!sg_touch_back_shoulder(640,544)&&!sg_touch_back_shoulder(1279,544));
 const unsigned pads[]={SG_R,SG_SELECT|SG_R,SG_SELECT|SG_L,SG_SELECT|SG_TRIANGLE,SG_SELECT|SG_SQUARE,SG_SELECT|SG_CIRCLE,SG_SELECT|SG_START};
 for(unsigned i=0;i<sizeof(pads)/sizeof(pads[0]);i++){bits=sg_controls(pads[i],&toggle);assert(bits==pads[i]&&!toggle);}
 assert(sg_touch_shoulder(960,544)==0);
 if(argc<2||!sg_boot(argv[1],stdout))return 1;
 unsigned buttons[7]={0,SG_R,SG_R,SG_RPAGE,0,0,0};
 for(int i=0;i<7;i++)if(!sg_step(buttons[i],i==4||i==5?0:128,128,128,128,0,160,90)){fprintf(stderr,"%s\n",sg_error());sg_close();return 1;}
 sg_close();puts("INPUT_NO_CHORDS_AND_SHOULDER_ZONES_PASSED");return 0;
}
