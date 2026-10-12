#pragma once
#include <stdint.h>
void sg_ui_reset(void);
void sg_ui_notice(const char *message);
void sg_ui_tick(void);
const char *sg_ui_current_notice(void);
void sg_ui_row(uint32_t *row,int width,int height,int y);
