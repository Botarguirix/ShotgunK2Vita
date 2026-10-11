#include "runtime.h"
#include "graphics.h"
#include "perf.h"
#include "render_pool.h"
#include "diagnostics.h"
#include <stdio.h>
#include <stdlib.h>

/* Same deterministic menu/intro inputs as sg_probe, with presentation every frame. */
int main(int argc, char **argv) {
    if (argc < 3) { fprintf(stderr,"Usage: sg_bench data-root output.csv [frames]\n"); return 2; }
    if(argc>6&&!sg_diag_start(argv[6]))return 2;
    FILE *csv = fopen(argv[2], "w");
    if (!csv) return 2;
    fprintf(csv,"frame,update_ms,draw_ms,present_ms,wait_ms,frame_ms,glyph_hits,glyph_misses,font_build_ms,blit_pixels,workers,cpu_mhz\n");
    uint32_t *pixels = malloc(960*544*sizeof(*pixels));
    int ok = pixels && sg_boot(argv[1], stdout), frames = argc > 3 ? atoi(argv[3]) : 4800;
    int workers=sg_render_pool_start(argc>5?atoi(argv[5]):1,stdout);
    for (int i=0; ok && i<frames; i++) {
        unsigned buttons=0;
        if(i==300||i==400||i==650||i==900||i==1200||i==1500||i==1800||i==2100||i==2400||i==2700||i==3000||i==3300) buttons=SG_CROSS;
        if(i==1600||i==1700) buttons=SG_DOWN;
        if(i==4200) buttons=SG_UP;
        if(i==4300) buttons=SG_CROSS;
        if(i==4500) buttons=SG_R;
        uint64_t begin=sg_now_us();
        sg_diag_frame((unsigned)i,buttons,128,128,128,128);
        ok=sg_step(buttons,128,128,128,128,0,160,90);
        sg_diag_phase(SG_DIAG_PRESENT);
        uint64_t present=sg_now_us();
        sg_render_pool_present(pixels,960,544);
        uint64_t end=sg_now_us();
        fprintf(csv,"%d,%.3f,%.3f,%.3f,0,%.3f,%llu,%llu,%.3f,%llu,%d,0\n",i,
            sg_perf.update_us/1000.0,sg_perf.draw_us/1000.0,(end-present)/1000.0,(end-begin)/1000.0,
            (unsigned long long)sg_perf.glyph_hits,(unsigned long long)sg_perf.glyph_misses,
            sg_perf.font_build_us/1000.0,(unsigned long long)sg_perf.blit_pixels,workers);
    }
    if(ok&&argc>4){FILE *f=fopen(argv[4],"wb");if(f){fprintf(f,"P6\n960 544\n255\n");for(int i=0;i<960*544;i++){unsigned char rgb[3]={pixels[i],pixels[i]>>8,pixels[i]>>16};fwrite(rgb,1,3,f);}fclose(f);}}
    sg_report_state();sg_render_pool_stop(); sg_close(); free(pixels); fclose(csv);
    sg_diag_stop(ok?"clean":"lua_error");
    return ok ? 0 : 1;
}
