#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include "runtime.h"
#include "graphics.h"
#include "perf.h"
#include "frame_copy.h"
#include "gpu_presenter.h"
#include "render_pool.h"
#include "controls.h"
#include "diagnostics.h"
#include "input_replay.h"
#include <psp2/power.h>
#include <psp2/kernel/cpu.h>
#define WIDTH 960
#define HEIGHT 544
unsigned int _newlib_heap_size_user=96*1024*1024;
static const unsigned char alphabet[26][7]={
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
 {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},
 {14,4,4,4,4,4,14},{7,2,2,2,2,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
 {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
 {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31}};
static const unsigned char digits[10][7]={
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},{2,6,10,18,31,2,2},
 {31,16,16,30,1,1,30},{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},{14,17,17,15,1,1,14}};
static void text_at(uint32_t *p,const char *s,int start_x,int y){int x=start_x;for(;*s&&y<HEIGHT-16;s++){unsigned c=(unsigned char)*s;if(c>='a'&&c<='z')c-=32;if(c=='\n'||x>WIDTH-24){x=start_x;y+=20;if(c=='\n')continue;if(y>=HEIGHT-16)break;}const unsigned char *glyph=c>='A'&&c<='Z'?alphabet[c-'A']:c>='0'&&c<='9'?digits[c-'0']:NULL;if(glyph){for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(glyph[yy]&(1<<(4-xx)))for(int j=0;j<2;j++)for(int i=0;i<2;i++)p[(y+yy*2+j)*WIDTH+x+xx*2+i]=0xffffffffu;}x+=12;}}
static void text(uint32_t *p,const char *s,int y){text_at(p,s,18,y);}
static void loading_screen(uint32_t *p){
 for(int i=0;i<WIDTH*HEIGHT;i++)p[i]=0xff101827u;
 for(int y=72;y<474;y++)for(int x=96;x<864;x++)p[y*WIDTH+x]=0xff182b45u;
 for(int x=96;x<864;x++){p[72*WIDTH+x]=0xff4c8dccu;p[473*WIDTH+x]=0xff4c8dccu;}
 for(int y=72;y<474;y++){p[y*WIDTH+96]=0xff4c8dccu;p[y*WIDTH+863]=0xff4c8dccu;}
 text_at(p,"SHOTGUN KING",250,150);text_at(p,"VITA EDITION",310,205);
 text_at(p,"INITIALIZING GAME",250,300);
 for(int y=348;y<372;y++)for(int x=220;x<740;x++)p[y*WIDTH+x]=0xff0b1524u;
 for(int y=352;y<368;y++)for(int x=224;x<736;x++)p[y*WIDTH+x]=0xff56b4e9u;
 text_at(p,"PLEASE WAIT",370,410);
}
static unsigned input(unsigned v){unsigned out=0;
 if(v&SCE_CTRL_CROSS)out|=SG_CROSS;
 if(v&SCE_CTRL_CIRCLE)out|=SG_CIRCLE;
 if(v&SCE_CTRL_SQUARE)out|=SG_SQUARE;
 if(v&SCE_CTRL_TRIANGLE)out|=SG_TRIANGLE;
 if(v&SCE_CTRL_LEFT)out|=SG_LEFT;
 if(v&SCE_CTRL_RIGHT)out|=SG_RIGHT;
 if(v&SCE_CTRL_UP)out|=SG_UP;
 if(v&SCE_CTRL_DOWN)out|=SG_DOWN;
 if(v&SCE_CTRL_LTRIGGER)out|=SG_L;
 if(v&SCE_CTRL_RTRIGGER)out|=SG_R;
 if(v&SCE_CTRL_START)out|=SG_START;
 if(v&SCE_CTRL_SELECT)out|=SG_SELECT;
 return out;}
typedef struct {int cpu,workers,copy_mode,profile,renderer,divisor,fps_limit,lua_jit,replay;} Config;
static Config settings(void){
 /* Safe defaults for the shipped profiling build.  A stale CFG must not
  * silently disable measurements or revert the requested 500 MHz profile. */
 Config c={500,3,2,1,1,2,60,0,0};FILE *f=fopen("ux0:data/shotgunk2vita/performance.cfg","r");
 if(f){char line[128],key[80];int value;while(fgets(line,sizeof(line),f))if(sscanf(line,"%79[^=]=%d",key,&value)==2){
  if(!strcmp(key,"cpu_mhz")&&(value==333||value==444))c.cpu=value;
  if(!strcmp(key,"workers")&&value>=1&&value<=3)c.workers=value;
  if(!strcmp(key,"copy_mode")&&value>=0&&value<=2)c.copy_mode=value;
  if(!strcmp(key,"renderer")&&(value==0||value==1))c.renderer=value;
  if(!strcmp(key,"profile"))c.profile=value!=0;
  if(!strcmp(key,"resolution_divisor")&&(value==1||value==2))c.divisor=value;
  if(!strcmp(key,"fps_limit")&&(value==30||value==60))c.fps_limit=value;
  if(!strcmp(key,"lua_jit"))c.lua_jit=value!=0;
  if(!strcmp(key,"profile_funcs"))sg_set_profile_funcs(value);
  if(!strcmp(key,"profile_lines"))sg_set_profile_lines(value);
  /* input_replay wins over input_record when both are set. */
  if(!strcmp(key,"input_record")&&value&&c.replay!=2)c.replay=1;
  if(!strcmp(key,"input_replay")&&value)c.replay=2;
 }fclose(f);}sg_set_jit_mode(c.lua_jit);return c;
}
static const char *run_gpu(Config config,FILE *log){
 int scene_width=320/config.divisor,scene_height=180/config.divisor;
 uint32_t *pixels=malloc(scene_width*scene_height*4),*error_pixels=malloc(WIDTH*HEIGHT*4);
 if(!pixels||!error_pixels){free(pixels);free(error_pixels);return "gpu_staging_error";}
 sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
 sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,SCE_TOUCH_SAMPLING_STATE_START);sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK,SCE_TOUCH_SAMPLING_STATE_START);
 mkdir("ux0:data/shotgunk2vita",0777);
 if(log){fprintf(log,"cpu_requested=%d cpu_actual=%d renderer=gpu\n",config.cpu,scePowerGetArmClockFrequency());fflush(log);}
 FILE *csv=config.profile?sg_diag_open("performance.csv"):NULL;
 if(config.profile&&!csv)csv=fopen("ux0:data/shotgunk2vita/performance.csv","a");
 if(csv){setvbuf(csv,NULL,_IOFBF,16384);fprintf(csv,"frame,update_ms,draw_ms,present_ms,wait_ms,frame_ms,glyph_hits,glyph_misses,font_build_ms,blit_pixels,workers,cpu_mhz,render0_ms,render1_ms,render2_ms,copy_ms,running,copy_mode,copy_rc,input_calls,input_evaluations,input_parses,renderer,gpu_submit_ms,entities,entity_updates,entity_draws,palette_resets,palette_suppressed,sprite_calls,shadow_calls,shadow_pixels,shadow_fallbacks");sg_perf_extra_header(csv);fflush(csv);}
 loading_screen(error_pixels);
 SgGpuStats gpu={0};
 if(!sg_gpu_present(error_pixels,WIDTH,HEIGHT,0,0,0xff000000,&gpu)){free(pixels);free(error_pixels);if(csv)fclose(csv);return "gpu_startup_error";}
 SgReplay replay;sg_replay_start(&replay,config.replay,"ux0:data/shotgunk2vita",log);
 int running=sg_boot("ux0:data/shotgunk2vita",log);unsigned frames=0;
 if(log){fprintf(log,"bootstrap_result=%d error=%s\n",running,running?"none":sg_error());fflush(log);}
 int workers=sg_render_pool_start(config.workers,log);
 const char *exit_reason=NULL;
 uint64_t flush_time=sg_now_us(),window_time=flush_time,elapsed=0;uint64_t last_end=flush_time;unsigned window_frames=0;char metrics[100]="FPS MEASURING";
 while(1){
  uint64_t frame_start=sg_now_us();
  sg_diag_phase(SG_DIAG_INPUT);
  SceCtrlData pad={0};SceTouchData touch={0},back_touch={0};sceCtrlPeekBufferPositive(0,&pad,1);sceTouchPeek(SCE_TOUCH_PORT_FRONT,&touch,1);sceTouchPeek(SCE_TOUCH_PORT_BACK,&back_touch,1);
  if(!running&&(pad.buttons&SCE_CTRL_START)){exit_reason="error_exit_start";break;}
  int toggle;unsigned buttons=sg_controls(input(pad.buttons),&toggle);(void)toggle;
  for(unsigned i=0;i<back_touch.reportNum;i++)buttons|=sg_touch_back_shoulder(back_touch.report[i].x,back_touch.report[i].y);
  /* Recorded/replayed inputs are the ones the game step consumes. */
  if(running)sg_replay_frame(&replay,&buttons,&pad.lx,&pad.ly,&pad.rx,&pad.ry,buttons,log);
  sg_diag_frame(frames,buttons,pad.lx,pad.ly,pad.rx,pad.ry);
  /* Front touch is intentionally disabled; rear touch remains L1/R1 above. */
  if(running)running=sg_step(buttons,pad.lx,pad.ly,pad.rx,pad.ry,0,160,90);
  if(sg_should_quit()){exit_reason="menu_exit";break;}
  sg_diag_phase(SG_DIAG_PRESENT);
  uint64_t present_start=sg_now_us();if(running)sg_render_pool_present(pixels,scene_width,scene_height);uint64_t present_end=sg_now_us();SgRenderStats render=sg_render_pool_stats();
  if(!running){sg_diag_phase(SG_DIAG_ERROR);for(int i=0;i<WIDTH*HEIGHT;i++)error_pixels[i]=0xff201018;text(error_pixels,"SHOTGUNK VITA STOPPED\nLOGS IN DATA SHOTGUNK2VITA LOGS\nSTART TO EXIT",28);text(error_pixels,sg_error(),110);}
  sg_diag_phase(SG_DIAG_DISPLAY);
  if(replay.mode==2&&log&&frames%600==0)fprintf(log,"replay_check frame=%u entities=%llu lua_kb=%u\n",frames,(unsigned long long)sg_perf.entities,sg_lua_memory_kb());
  int display_ok=sg_gpu_present(running?pixels:error_pixels,running?scene_width:WIDTH,running?scene_height:HEIGHT,buttons,running,running?sg_present_background():0xff201018,&gpu);
  uint64_t end=sg_now_us();uint64_t copy_us=gpu.upload_us;
  if(!display_ok){if(log)fprintf(log,"gpu_display_failed\n");exit_reason="gpu_display_error";break;}
  sg_diag_phase(running?SG_DIAG_IDLE:SG_DIAG_ERROR);
  uint64_t frame_duration=end-last_end;last_end=end;elapsed+=frame_duration;window_frames++;
  if(end-window_time>=500000){snprintf(metrics,sizeof(metrics),"FPS %u U %u D %u P %u CPU %d W %d",elapsed?(unsigned)(window_frames*1000000ULL/elapsed):0,(unsigned)(sg_perf.update_us/1000),(unsigned)(sg_perf.draw_us/1000),(unsigned)((present_end-present_start)/1000),scePowerGetArmClockFrequency(),workers);window_time=end;window_frames=0;elapsed=0;}
  if(csv)fprintf(csv,"%u,%.3f,%.3f,%.3f,%.3f,%.3f,%llu,%llu,%.3f,%llu,%d,%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%d,%llu,%llu,%llu,1,%.3f,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu",frames,sg_perf.update_us/1000.0,sg_perf.draw_us/1000.0,(present_end-present_start)/1000.0,gpu.wait_us/1000.0,frame_duration/1000.0,(unsigned long long)sg_perf.glyph_hits,(unsigned long long)sg_perf.glyph_misses,sg_perf.font_build_us/1000.0,(unsigned long long)sg_perf.blit_pixels,workers,scePowerGetArmClockFrequency(),render.worker_us[0]/1000.0,render.worker_us[1]/1000.0,render.worker_us[2]/1000.0,copy_us/1000.0,running,0,0,(unsigned long long)sg_perf.input_calls,(unsigned long long)sg_perf.input_evaluations,(unsigned long long)sg_perf.input_parses,gpu.submit_us/1000.0, (unsigned long long)sg_perf.entities,(unsigned long long)sg_perf.entity_updates,(unsigned long long)sg_perf.entity_draws,(unsigned long long)sg_perf.palette_resets,(unsigned long long)sg_perf.palette_suppressed,(unsigned long long)sg_perf.sprite_calls,(unsigned long long)sg_perf.shadow_calls,(unsigned long long)sg_perf.shadow_pixels,(unsigned long long)sg_perf.shadow_fallbacks);if(csv)sg_perf_extra_row(csv,config.divisor==2?160:320,config.divisor==2?90:180);
  if(end-flush_time>=250000){if(running)sg_report_movement();if(csv)fflush(csv);sg_diag_memory(sg_lua_memory_kb(),sg_graphics_memory_kb());if(log){fprintf(log,"native_frames=%u running=%d renderer=gpu render_workers=%d upload_us=%llu submit_us=%llu wait_us=%llu lua_kb=%u surface_kb=%u %s\n",frames,running,workers,(unsigned long long)gpu.upload_us,(unsigned long long)gpu.submit_us,(unsigned long long)gpu.wait_us,sg_lua_memory_kb(),sg_graphics_memory_kb(),metrics);fflush(log);}flush_time=end;}
  uint64_t frame_budget=1000000ULL/(unsigned)sg_get_fps_limit();uint64_t frame_spent=sg_now_us()-frame_start;if(frame_spent<frame_budget)sceKernelDelayThread((unsigned)(frame_budget-frame_spent));
  frames++;
 }
 sg_diag_phase(SG_DIAG_EXIT);sg_diag_event("shutdown",exit_reason?exit_reason:"clean");
 if(log)fprintf(log,"exit_reason=%s\n",exit_reason?exit_reason:running?"clean":"lua_error");
 sg_replay_stop(&replay,log);sg_set_save_redirect(NULL);
 sg_render_pool_stop();sg_close();if(csv)fclose(csv);free(pixels);free(error_pixels);
 return exit_reason?exit_reason:running?"clean":"lua_error";
}
int main(void){
 int diagnostic_started=sg_diag_start("ux0:data/shotgunk2vita");
 if(sg_diag_directory()[0]){char console[1400];snprintf(console,sizeof(console),"%s/console.log",sg_diag_directory());
  if(freopen(console,"a",stdout))setvbuf(stdout,NULL,_IOLBF,4096);
  if(freopen(console,"a",stderr))setvbuf(stderr,NULL,_IONBF,0);
 }
 FILE *log=sg_diag_open("runtime.log");
 if(!log)log=fopen("ux0:data/shotgunk2vita/runtime.log","a");
 Config config=settings();
 if(log){setvbuf(log,NULL,_IOLBF,4096);fprintf(log,"build=performance-%s name=%s fps_limit=%d hud=removed diagnostics_started=%d session=%s\n",SG_BUILD_VERSION,SG_BUILD_NAME,config.fps_limit,diagnostic_started,sg_diag_directory());}
 int old_clock=scePowerGetArmClockFrequency();int clock_rc=scePowerSetArmClockFrequency(config.cpu);
 sg_graphics_set_output_divisor(config.divisor);
 sg_diag_thread(SG_THREAD_MAIN,sceKernelGetThreadId());
 int affinity_rc=sceKernelChangeThreadCpuAffinityMask(sceKernelGetThreadId(),SCE_KERNEL_CPU_MASK_USER_0);
 if(log)fprintf(log,"cpu_requested=%d cpu_actual=%d clock_rc=%d affinity_rc=%d renderer_requested=%d\n",config.cpu,scePowerGetArmClockFrequency(),clock_rc,affinity_rc,config.renderer);
 if(config.renderer&&sg_gpu_init(log,config.divisor)){
  const char *reason=run_gpu(config,log);sg_gpu_close();if(log)fclose(log);fflush(stdout);fflush(stderr);sg_diag_stop(reason);
  if(old_clock>0)scePowerSetArmClockFrequency(old_clock);
  sceKernelExitProcess(0);return 0;
 }
 SceUID block=sceKernelAllocMemBlock("SGK double framebuffer",SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,0x400000,NULL);
 if(block<0){if(log){fprintf(log,"framebuffer_alloc_failed=0x%08X\n",(unsigned)block);fclose(log);}sg_diag_stop("startup_error");if(old_clock>0)scePowerSetArmClockFrequency(old_clock);return 1;}
 uint32_t *display_base=NULL;
 int framebuffer_rc=sceKernelGetMemBlockBase(block,(void **)&display_base);
 if(framebuffer_rc<0){if(log){fprintf(log,"framebuffer_base_failed=0x%08X\n",(unsigned)framebuffer_rc);fclose(log);}sg_diag_stop("startup_error");sceKernelFreeMemBlock(block);if(old_clock>0)scePowerSetArmClockFrequency(old_clock);return 1;}
 /* CPU compositing and repeated-row reads use cached RAM. Only the completed
  * image is copied into the inactive CDRAM buffer; scanout stays immutable. */
 uint32_t *pixels=memalign(64,WIDTH*HEIGHT*4);
 if(!pixels){if(log){fprintf(log,"staging_alloc_failed\n");fclose(log);}sg_diag_stop("startup_error");sceKernelFreeMemBlock(block);if(old_clock>0)scePowerSetArmClockFrequency(old_clock);return 1;}
 uint32_t *display_buffers[2]={display_base,(uint32_t *)((unsigned char *)display_base+0x200000)};int back=1;
 SgFrameCopy copier=sg_copy_select(display_buffers[1],(unsigned char *)pixels,WIDTH*HEIGHT*4,config.copy_mode,log);
 SceDisplayFrameBuf frame={sizeof(frame),display_buffers[0],WIDTH,SCE_DISPLAY_PIXELFORMAT_A8B8G8R8,WIDTH,HEIGHT};
 if(log)fprintf(log,"presentation=cached_staging double_buffers=2 resolution=960x544 game_resolution=320x180\n");
 sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
 sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,SCE_TOUCH_SAMPLING_STATE_START);sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK,SCE_TOUCH_SAMPLING_STATE_START);
 mkdir("ux0:data/shotgunk2vita",0777);
 if(log){fprintf(log,"cpu_requested=%d cpu_actual=%d clock_rc=%d affinity_rc=%d framebuffer_rc=%d\n",config.cpu,scePowerGetArmClockFrequency(),clock_rc,affinity_rc,framebuffer_rc);fflush(log);}
 FILE *csv=config.profile?sg_diag_open("performance.csv"):NULL;
 if(config.profile&&!csv)csv=fopen("ux0:data/shotgunk2vita/performance.csv","a");
 if(csv){setvbuf(csv,NULL,_IOFBF,16384);fprintf(csv,"frame,update_ms,draw_ms,present_ms,wait_ms,frame_ms,glyph_hits,glyph_misses,font_build_ms,blit_pixels,workers,cpu_mhz,render0_ms,render1_ms,render2_ms,copy_ms,running,copy_mode,copy_rc,input_calls,input_evaluations,input_parses,renderer,gpu_submit_ms,entities,entity_updates,entity_draws,palette_resets,palette_suppressed,sprite_calls,shadow_calls,shadow_pixels,shadow_fallbacks");sg_perf_extra_header(csv);fflush(csv);}
 loading_screen(pixels);
 memcpy(display_buffers[0],pixels,WIDTH*HEIGHT*4);
 sceDisplaySetFrameBuf(&frame,SCE_DISPLAY_SETBUF_NEXTFRAME);sceDisplayWaitVblankStart();
 int running=sg_boot("ux0:data/shotgunk2vita",log);unsigned frames=0;
 if(log){fprintf(log,"bootstrap_result=%d error=%s\n",running,running?"none":sg_error());fflush(log);}
 int workers=sg_render_pool_start(config.workers,log);
 const char *exit_reason=NULL;
 uint64_t flush_time=sg_now_us(),window_time=flush_time,elapsed=0;uint64_t last_end=flush_time;unsigned window_frames=0;char metrics[100]="FPS MEASURING";
 while(1){
  sg_diag_phase(SG_DIAG_INPUT);
  SceCtrlData pad={0};SceTouchData touch={0},back_touch={0};sceCtrlPeekBufferPositive(0,&pad,1);sceTouchPeek(SCE_TOUCH_PORT_FRONT,&touch,1);sceTouchPeek(SCE_TOUCH_PORT_BACK,&back_touch,1);
  if(!running&&(pad.buttons&SCE_CTRL_START)){exit_reason="error_exit_start";break;}
  int toggle;unsigned buttons=sg_controls(input(pad.buttons),&toggle);(void)toggle;
  for(unsigned i=0;i<back_touch.reportNum;i++)buttons|=sg_touch_back_shoulder(back_touch.report[i].x,back_touch.report[i].y);
  sg_diag_frame(frames,buttons,pad.lx,pad.ly,pad.rx,pad.ry);
  /* Front touch is intentionally disabled; rear touch remains L1/R1 above. */
  if(running)running=sg_step(buttons,pad.lx,pad.ly,pad.rx,pad.ry,0,160,90);
  if(sg_should_quit()){exit_reason="menu_exit";break;}
  sg_diag_phase(SG_DIAG_PRESENT);
  uint64_t present_start=sg_now_us();if(running)sg_render_pool_present(pixels,WIDTH,HEIGHT);uint64_t present_end=sg_now_us();SgRenderStats render=sg_render_pool_stats();
  if(!running){sg_diag_phase(SG_DIAG_ERROR);for(int i=0;i<WIDTH*HEIGHT;i++)pixels[i]=0xff201018;text(pixels,"SHOTGUNK VITA STOPPED\nLOGS IN DATA SHOTGUNK2VITA LOGS\nSTART TO EXIT",28);text(pixels,sg_error(),110);}
  sg_diag_phase(SG_DIAG_DISPLAY);
  uint64_t copy_start=sg_now_us();sg_copy_frame(&copier,display_buffers[back],pixels,WIDTH*HEIGHT*4,log);uint64_t copy_us=sg_now_us()-copy_start;
  frame.base=display_buffers[back];
  uint64_t wait_start=sg_now_us();int display_rc=sceDisplaySetFrameBuf(&frame,SCE_DISPLAY_SETBUF_NEXTFRAME);int wait_rc=sceDisplayWaitVblankStart();uint64_t end=sg_now_us();
  SceDisplayFrameBuf active={0};active.size=sizeof(active);
  int active_rc=sceDisplayGetFrameBuf(&active,SCE_DISPLAY_SETBUF_IMMEDIATE);
  if(display_rc>=0&&wait_rc>=0&&active_rc>=0&&active.base!=frame.base){wait_rc=sceDisplayWaitVblankStart();active_rc=sceDisplayGetFrameBuf(&active,SCE_DISPLAY_SETBUF_IMMEDIATE);end=sg_now_us();}
  if(display_rc<0||wait_rc<0||active_rc<0||active.base!=frame.base){if(log)fprintf(log,"display_swap_failed set=%d wait=%d get=%d active=%p expected=%p\n",display_rc,wait_rc,active_rc,active.base,frame.base);exit_reason="display_error";break;}
  back^=1;
  sg_diag_phase(running?SG_DIAG_IDLE:SG_DIAG_ERROR);
  if((display_rc<0||wait_rc<0)&&log){fprintf(log,"display_error=0x%08X wait_error=0x%08X\n",(unsigned)display_rc,(unsigned)wait_rc);fflush(log);}
  uint64_t frame_duration=end-last_end;last_end=end;elapsed+=frame_duration;window_frames++;
  if(end-window_time>=500000){snprintf(metrics,sizeof(metrics),"FPS %u U %u D %u P %u CPU %d W %d",elapsed?(unsigned)(window_frames*1000000ULL/elapsed):0,(unsigned)(sg_perf.update_us/1000),(unsigned)(sg_perf.draw_us/1000),(unsigned)((present_end-present_start)/1000),scePowerGetArmClockFrequency(),workers);window_time=end;window_frames=0;elapsed=0;}
  if(csv)fprintf(csv,"%u,%.3f,%.3f,%.3f,%.3f,%.3f,%llu,%llu,%.3f,%llu,%d,%d,%.3f,%.3f,%.3f,%.3f,%d,%d,%d,%llu,%llu,%llu,0,0.000,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu",frames,sg_perf.update_us/1000.0,sg_perf.draw_us/1000.0,(present_end-present_start)/1000.0,(end-wait_start)/1000.0,frame_duration/1000.0,(unsigned long long)sg_perf.glyph_hits,(unsigned long long)sg_perf.glyph_misses,sg_perf.font_build_us/1000.0,(unsigned long long)sg_perf.blit_pixels,workers,scePowerGetArmClockFrequency(),render.worker_us[0]/1000.0,render.worker_us[1]/1000.0,render.worker_us[2]/1000.0,copy_us/1000.0,running,copier.mode,copier.last_rc,(unsigned long long)sg_perf.input_calls,(unsigned long long)sg_perf.input_evaluations,(unsigned long long)sg_perf.input_parses, (unsigned long long)sg_perf.entities,(unsigned long long)sg_perf.entity_updates,(unsigned long long)sg_perf.entity_draws,(unsigned long long)sg_perf.palette_resets,(unsigned long long)sg_perf.palette_suppressed,(unsigned long long)sg_perf.sprite_calls,(unsigned long long)sg_perf.shadow_calls,(unsigned long long)sg_perf.shadow_pixels,(unsigned long long)sg_perf.shadow_fallbacks);if(csv)sg_perf_extra_row(csv,config.divisor==2?160:320,config.divisor==2?90:180);
  if(end-flush_time>=250000){if(running)sg_report_movement();if(csv)fflush(csv);sg_diag_memory(sg_lua_memory_kb(),sg_graphics_memory_kb());if(log){fprintf(log,"native_frames=%u running=%d render_workers=%d cpus=%d/%d/%d row_us=%llu/%llu/%llu lua_kb=%u surface_kb=%u %s\n",frames,running,workers,render.cpu[0],render.cpu[1],render.cpu[2],(unsigned long long)render.worker_us[0],(unsigned long long)render.worker_us[1],(unsigned long long)render.worker_us[2],sg_lua_memory_kb(),sg_graphics_memory_kb(),metrics);fflush(log);}flush_time=end;}
  frames++;
 }
 sg_diag_phase(SG_DIAG_EXIT);sg_diag_event("shutdown",exit_reason?exit_reason:"clean");
 if(log)fprintf(log,"exit_reason=%s\n",exit_reason?exit_reason:running?"clean":"lua_error");
 sg_render_pool_stop();sg_close();if(csv)fclose(csv);if(log)fclose(log);fflush(stdout);fflush(stderr);sg_diag_stop(exit_reason?exit_reason:running?"clean":"lua_error");sceDisplaySetFrameBuf(NULL,SCE_DISPLAY_SETBUF_NEXTFRAME);sceDisplayWaitVblankStart();free(pixels);sceKernelFreeMemBlock(block);
 if(old_clock>0){scePowerSetArmClockFrequency(old_clock);}
 sceKernelExitProcess(0);return 0;
}
