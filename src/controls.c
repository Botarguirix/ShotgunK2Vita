#include "controls.h"
#include "runtime.h"
unsigned sg_touch_shoulder(int x,int y){
    return 0;
}
unsigned sg_touch_back_shoulder(int x,int y){
    if(x<0||x>=1920||y<0||y>=1088)return 0;
    /* Two wide horizontal pads matching the rear-touch layout: upper-left
     * is L1 and upper-right is R1. Keep an outer margin and a large center
     * gap so the user's palms do not trigger either shoulder accidentally. */
    if(x>=120&&x<820&&y>=40&&y<420)return SG_LPAGE;
    if(x>=1100&&x<1800&&y>=40&&y<420)return SG_RPAGE;
    return 0;
}
unsigned sg_controls(unsigned pad,int *toggle_hud){
    *toggle_hud=0;return pad;
}
