#pragma once
/* Physical buttons pass through without SELECT shortcuts. */
unsigned sg_controls(unsigned pad,int *toggle_hud);
/* Front touch is ignored; L1/R1 use two rear touch zones. */
unsigned sg_touch_shoulder(int x,int y);
unsigned sg_touch_back_shoulder(int x,int y);
