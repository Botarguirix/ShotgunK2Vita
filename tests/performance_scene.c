#include "runtime.h"
#include "perf.h"
#include "render_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fixed seed, input schedule and viewport; desktop timing, never Vita FPS. */
int main(int argc,char **argv){
 if(argc!=6)return 2;
 int frames=atoi(argv[4]),requested=atoi(argv[5]);
 int board=!strcmp(argv[3],"board"),actions=!strcmp(argv[3],"actions");
 if(frames<120||requested<1||requested>3||(!board&&!actions&&strcmp(argv[3],"menu")))return 2;
 srand(1234);int ok=sg_boot(argv[1],stdout);
 for(int i=0;ok&&i<90;i++)ok=sg_step(0,128,128,128,128,0,160,90);
 if(ok&&(board||actions)){
  ok=sg_host_eval("math.randomseed(1234);NO_INTRO=true;set_mode('throne');mode.start()");
  for(int i=0;ok&&i<720;i++)ok=sg_step(0,128,128,128,128,0,160,90);
  if(ok)ok=sg_host_eval("assert(hero and hero.sq and playing)");
 }
 if(ok&&!board&&!actions){
  ok=sg_host_eval("open_menu();assert(menu and menu_is_on_root)");
  for(int i=0;ok&&i<90;i++)ok=sg_step(0,128,128,128,128,0,160,90);
 }
 uint32_t *pixels=malloc(960*544*sizeof(*pixels));FILE *csv=fopen(argv[2],"w");
 if(!pixels||!csv){free(pixels);if(csv)fclose(csv);sg_close();return 2;}
 int workers=sg_render_pool_start(requested,stdout);unsigned hash=2166136261u;
 fprintf(csv,"frame,update_ms,draw_ms,present_ms,wait_ms,frame_ms,entities,entity_draws,shadow_calls,shadow_pixels,workers,cpu_mhz\n");
 for(int i=0;ok&&i<frames;i++){
  unsigned buttons=0;int rx=128;
  if(actions){rx=(i/120)%2?220:36;if(i%240==20)buttons=SG_R;if(i%240==100)buttons=SG_TRIANGLE;if(i%480==180)buttons=SG_UP;if(i%480==190)buttons=SG_CROSS;}
  uint64_t begin=sg_now_us();ok=sg_step(buttons,128,128,rx,128,0,160,90);
  uint64_t present=sg_now_us();sg_render_pool_present(pixels,960,544);uint64_t end=sg_now_us();
  fprintf(csv,"%d,%.3f,%.3f,%.3f,0,%.3f,%llu,%llu,%llu,%llu,%d,0\n",i,sg_perf.update_us/1000.,sg_perf.draw_us/1000.,(end-present)/1000.,(end-begin)/1000.,(unsigned long long)sg_perf.entities,(unsigned long long)sg_perf.entity_draws,(unsigned long long)sg_perf.shadow_calls,(unsigned long long)sg_perf.shadow_pixels,workers);
  /* Outside timed region: sequence fingerprint, all pixels every frame. */
  for(int p=0;p<960*544;p++)hash=(hash^pixels[p])*16777619u;
 }
 printf("SCENE_RESULT scene=%s success=%d frames=%d workers=%d fingerprint=%08x\n",argv[3],ok,frames,workers,hash);
 if(!ok)fprintf(stderr,"%s\n",sg_error());
 sg_report_movement();sg_render_pool_stop();sg_close();free(pixels);fclose(csv);return !ok;
}
