#pragma once
/* Physical buttons pass through without SELECT shortcuts. */
unsigned sg_controls(unsigned pad,int *toggle_hud);
/* Only L1/R1 front corners are active. All other touches are ignored. */
unsigned sg_touch_shoulder(int x,int y);
