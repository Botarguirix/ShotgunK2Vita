#include <psp2/display.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "runtime.h"
#include "graphics.h"
#include "perf.h"
#include "render_pool.h"
#include "controls.h"
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
static void text(uint32_t *p,const char *s,int y){int x=18;for(;*s&&y<HEIGHT-16;s++){unsigned c=(unsigned char)*s;if(c>='a'&&c<='z')c-=32;if(c=='\n'||x>WIDTH-24){x=18;y+=20;if(c=='\n')continue;}const unsigned char *glyph=c>='A'&&c<='Z'?alphabet[c-'A']:c>='0'&&c<='9'?digits[c-'0']:NULL;if(glyph){for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(glyph[yy]&(1<<(4-xx)))for(int j=0;j<2;j++)for(int i=0;i<2;i++)p[(y+yy*2+j)*WIDTH+x+xx*2+i]=0xffffffffu;}x+=12;}}
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
typedef struct {int cpu,workers,hud,profile;} Config;
static Config settings(void){
 Config c={444,3,1,1};FILE *f=fopen("ux0:data/shotgunk2vita/performance.cfg","r");
 if(f){char line[128],key[80];int value;while(fgets(line,sizeof(line),f))if(sscanf(line,"%79[^=]=%d",key,&value)==2){
  if(!strcmp(key,"cpu_mhz")&&(value==333||value==444))c.cpu=value;
  if(!strcmp(key,"workers")&&value>=1&&value<=3)c.workers=value;
  if(!strcmp(key,"hud"))c.hud=value!=0;
  if(!strcmp(key,"profile"))c.profile=value!=0;
 }fclose(f);}return c;
}
int main(void){
 Config config=settings();int old_clock=scePowerGetArmClockFrequency();int clock_rc=scePowerSetArmClockFrequency(config.cpu);
 int affinity_rc=sceKernelChangeThreadCpuAffinityMask(sceKernelGetThreadId(),SCE_KERNEL_CPU_MASK_USER_0);
 SceUID block=sceKernelAllocMemBlock("SGK framebuffer",SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,0x200000,NULL);
 if(block<0){if(old_clock>0)scePowerSetArmClockFrequency(old_clock);return 1;}
 uint32_t *pixels=NULL;
 if(sceKernelGetMemBlockBase(block,(void **)&pixels)<0){sceKernelFreeMemBlock(block);if(old_clock>0)scePowerSetArmClockFrequency(old_clock);return 1;}
 SceDisplayFrameBuf frame={sizeof(frame),pixels,WIDTH,SCE_DISPLAY_PIXELFORMAT_A8B8G8R8,WIDTH,HEIGHT};
 sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
 sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,SCE_TOUCH_SAMPLING_STATE_START);
 mkdir("ux0:data/shotgunk2vita",0777);
 FILE *log=fopen("ux0:data/shotgunk2vita/runtime.log","w");
 if(log){fprintf(log,"build=performance-00.03 cpu_requested=%d cpu_actual=%d clock_rc=%d affinity_rc=%d\n",config.cpu,scePowerGetArmClockFrequency(),clock_rc,affinity_rc);fflush(log);}
 FILE *csv=config.profile?fopen("ux0:data/shotgunk2vita/performance.csv","w"):NULL;
 if(csv){setvbuf(csv,NULL,_IOFBF,16384);fprintf(csv,"frame,update_ms,draw_ms,present_ms,wait_ms,frame_ms,glyph_hits,glyph_misses,font_build_ms,blit_pixels,workers,cpu_mhz,render0_ms,render1_ms,render2_ms\n");fflush(csv);}
 memset(pixels,0,WIDTH*HEIGHT*4);text(pixels,"SHOTGUNK VITA\nLOADING LUA AND GAME DATA",40);
 sceDisplaySetFrameBuf(&frame,SCE_DISPLAY_SETBUF_NEXTFRAME);sceDisplayWaitVblankStart();
 int running=sg_boot("ux0:data/shotgunk2vita",log);unsigned frames=0;
 int workers=sg_render_pool_start(config.workers,log);
 uint64_t flush_time=sg_now_us(),window_time=flush_time,elapsed=0;uint64_t last_end=flush_time;unsigned window_frames=0;char hud[100]="FPS MEASURING";
 while(1){
  SceCtrlData pad={0};SceTouchData touch={0};sceCtrlPeekBufferPositive(0,&pad,1);sceTouchPeek(0,&touch,1);
  if((pad.buttons&(SCE_CTRL_START|SCE_CTRL_SELECT))==(SCE_CTRL_START|SCE_CTRL_SELECT))break;
  int toggle;unsigned buttons=sg_controls(input(pad.buttons),&toggle);if(toggle)config.hud=!config.hud;
  if(running)running=sg_step(buttons,pad.lx,pad.ly,pad.rx,pad.ry,touch.reportNum>0,touch.reportNum?touch.report[0].x*320/1920:160,touch.reportNum?touch.report[0].y*180/1088:90);
  uint64_t present_start=sg_now_us();sg_render_pool_present(pixels,WIDTH,HEIGHT);uint64_t present_end=sg_now_us();SgRenderStats render=sg_render_pool_stats();
  if(!running){for(int i=0;i<WIDTH*HEIGHT;i++)pixels[i]=0xff201018;text(pixels,"SHOTGUNK VITA STOPPED\nREAD RUNTIME LOG IN DATA FOLDER\nSTART AND SELECT TO EXIT",28);text(pixels,sg_error(),110);}
  if(config.hud){for(int i=0;i<WIDTH*23;i++)pixels[i]=0xff201018;text(pixels,hud,3);}
  uint64_t wait_start=sg_now_us();sceDisplaySetFrameBuf(&frame,SCE_DISPLAY_SETBUF_NEXTFRAME);sceDisplayWaitVblankStart();uint64_t end=sg_now_us();
  uint64_t frame_duration=end-last_end;last_end=end;elapsed+=frame_duration;window_frames++;
  if(end-window_time>=500000){snprintf(hud,sizeof(hud),"FPS %u U %u D %u P %u CPU %d W %d",elapsed?(unsigned)(window_frames*1000000ULL/elapsed):0,(unsigned)(sg_perf.update_us/1000),(unsigned)(sg_perf.draw_us/1000),(unsigned)((present_end-present_start)/1000),scePowerGetArmClockFrequency(),workers);window_time=end;window_frames=0;elapsed=0;}
  if(csv)fprintf(csv,"%u,%.3f,%.3f,%.3f,%.3f,%.3f,%llu,%llu,%.3f,%llu,%d,%d,%.3f,%.3f,%.3f\n",frames,sg_perf.update_us/1000.0,sg_perf.draw_us/1000.0,(present_end-present_start)/1000.0,(end-wait_start)/1000.0,frame_duration/1000.0,(unsigned long long)sg_perf.glyph_hits,(unsigned long long)sg_perf.glyph_misses,sg_perf.font_build_us/1000.0,(unsigned long long)sg_perf.blit_pixels,workers,scePowerGetArmClockFrequency(),render.worker_us[0]/1000.0,render.worker_us[1]/1000.0,render.worker_us[2]/1000.0);
  if(end-flush_time>=1000000){if(csv)fflush(csv);if(log){fprintf(log,"native_frames=%u running=%d render_workers=%d cpus=%d/%d/%d row_us=%llu/%llu/%llu %s\n",frames,running,workers,render.cpu[0],render.cpu[1],render.cpu[2],(unsigned long long)render.worker_us[0],(unsigned long long)render.worker_us[1],(unsigned long long)render.worker_us[2],hud);fflush(log);}flush_time=end;}
  frames++;
 }
 sg_render_pool_stop();sg_close();if(csv)fclose(csv);if(log)fclose(log);sceKernelFreeMemBlock(block);
 if(old_clock>0){scePowerSetArmClockFrequency(old_clock);}
 sceKernelExitProcess(0);return 0;
}
