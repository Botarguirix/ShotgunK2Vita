#include "controls.h"
#include "runtime.h"
unsigned sg_touch_shoulder(int x,int y){
    return 0;
}
unsigned sg_touch_back_shoulder(int x,int y){
    if(x<0||x>=1920||y<0||y>=1088)return 0;
    if(x<640)return SG_LPAGE;
    if(x>=1280)return SG_RPAGE;
    return 0;
}
unsigned sg_controls(unsigned pad,int *toggle_hud){
    *toggle_hud=0;return pad;
}
