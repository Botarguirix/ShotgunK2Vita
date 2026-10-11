#include "controls.h"
#include "runtime.h"
unsigned sg_controls(unsigned pad,int *toggle_hud){
    static unsigned previous,consumed;
    unsigned out=pad;
    *toggle_hud=0;
    consumed&=pad;
    if(pad&SG_SELECT){
        unsigned chords=pad&(SG_L|SG_R|SG_SQUARE|SG_CIRCLE|SG_TRIANGLE);
        consumed|=chords;
        if(chords)out&=~SG_SELECT;
        if(pad&SG_L)out|=SG_LPAGE;
        if(pad&SG_R)out|=SG_RPAGE;
        if(pad&SG_SQUARE)out|=SG_LSTICK;
        if(pad&SG_CIRCLE)out|=SG_RSTICK;
        if((pad&SG_TRIANGLE)&&((previous&(SG_SELECT|SG_TRIANGLE))!=(SG_SELECT|SG_TRIANGLE)))*toggle_hud=1;
    }
    previous=pad;
    return out&~consumed;
}
