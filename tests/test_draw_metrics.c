#include "runtime.h"
#include "perf.h"
#include <stdio.h>
int main(int argc,char**argv){if(argc<2)return 2;int ok=sg_boot(argv[1],stdout);for(int i=0;ok&&i<90;i++)ok=sg_step(0,128,128,128,128,0,160,90);printf("DRAW_METRICS entities=%llu updates=%llu draw_calls=%llu palette_resets=%llu sprites=%llu pixels=%llu\n",(unsigned long long)sg_perf.entities,(unsigned long long)sg_perf.entity_updates,(unsigned long long)sg_perf.entity_draws,(unsigned long long)sg_perf.palette_resets,(unsigned long long)sg_perf.sprite_calls,(unsigned long long)sg_perf.blit_pixels);ok=ok&&sg_perf.entities>0&&sg_perf.entity_draws>0&&sg_perf.entity_updates>0&&sg_perf.palette_resets>0&&sg_perf.sprite_calls>0;if(!ok)puts(sg_error());sg_close();return !ok;}
