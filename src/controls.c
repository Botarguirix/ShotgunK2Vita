#include "controls.h"
#include "runtime.h"
unsigned sg_touch_shoulder(int x,int y){
    if(x<0||x>=1920||y<968||y>=1088)return 0;
    if(x<180)return SG_LPAGE;
    if(x>=1740)return SG_RPAGE;
    return 0;
}
unsigned sg_controls(unsigned pad,int *toggle_hud){
    *toggle_hud=0;return pad;
}
