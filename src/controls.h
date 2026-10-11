#pragma once
/* Input is the abstract Vita pad bits from runtime.h. Additional shoulder/
 * stick buttons use SELECT chords, since the Vita has no L2/R2/L3/R3. */
unsigned sg_controls(unsigned pad,int *toggle_hud);
/* Front panel coordinates: 1920 x 1088. Zero means a gameplay touch. */
unsigned sg_touch_shoulder(int x,int y);
