#include "runtime.h"
#include "controls.h"
#include <assert.h>
#include <stdio.h>

int main(int argc,char **argv){
 int toggle;unsigned bits;
 assert(sg_touch_shoulder(0,968)==SG_LPAGE);
 assert(sg_touch_shoulder(179,1087)==SG_LPAGE);
 assert(sg_touch_shoulder(1740,968)==SG_RPAGE);
 assert(sg_touch_shoulder(1919,1087)==SG_RPAGE);
 assert(!sg_touch_shoulder(180,968)&&!sg_touch_shoulder(1739,968));
 assert(!sg_touch_shoulder(0,967)&&!sg_touch_shoulder(1920,1087));
 bits=sg_controls(SG_R,&toggle);assert(bits==SG_R&&!toggle);
 bits=sg_controls(SG_SELECT|SG_R,&toggle);assert(bits==SG_RPAGE&&!toggle);
 bits=sg_controls(SG_R,&toggle);assert(bits==0); /* Chord release must not fire. */
 sg_controls(0,&toggle);
 bits=sg_controls(SG_SELECT|SG_TRIANGLE,&toggle);assert(bits==0&&toggle);
 bits=sg_controls(SG_SELECT|SG_TRIANGLE,&toggle);assert(bits==0&&!toggle);
 bits=sg_controls(SG_TRIANGLE,&toggle);assert(bits==0&&!toggle);
 sg_controls(0,&toggle);
 bits=sg_controls(SG_SELECT|SG_SQUARE,&toggle);assert(bits==SG_LSTICK);
 sg_controls(0,&toggle);
 bits=sg_controls(SG_SELECT|SG_CIRCLE,&toggle);assert(bits==SG_RSTICK);
 sg_controls(0,&toggle);
 if(argc<2||!sg_boot(argv[1],stdout))return 1;
 unsigned buttons[7]={0,SG_R,SG_R,SG_RPAGE,0,0,0};
 for(int i=0;i<7;i++)if(!sg_step(buttons[i],i==4||i==5?0:128,128,128,128,0,160,90)){fprintf(stderr,"%s\n",sg_error());sg_close();return 1;}
 sg_close();puts("INPUT_AND_CHORD_TESTS_PASSED");return 0;
}
