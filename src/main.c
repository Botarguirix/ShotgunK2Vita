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
static void text(uint32_t *p,const char *s,int y){int x=18;for(;*s&&y<HEIGHT-16;s++){unsigned c=(unsigned char)*s;if(c>='a'&&c<='z')c-=32;if(c=='\n'||x>WIDTH-24){x=18;y+=20;if(c=='\n')continue;}if(c>='A'&&c<='Z'){for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(alphabet[c-'A'][yy]&(1<<(4-xx)))for(int j=0;j<2;j++)for(int i=0;i<2;i++)p[(y+yy*2+j)*WIDTH+x+xx*2+i]=0xffffffffu;}x+=12;}}
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
int main(void){
 SceUID block=sceKernelAllocMemBlock("SGK framebuffer",SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,0x200000,NULL);
 if(block<0)return 1;
 uint32_t *pixels=NULL;
 if(sceKernelGetMemBlockBase(block,(void **)&pixels)<0)return 1;
 SceDisplayFrameBuf frame={sizeof(frame),pixels,WIDTH,SCE_DISPLAY_PIXELFORMAT_A8B8G8R8,WIDTH,HEIGHT};
 sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
 sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,SCE_TOUCH_SAMPLING_STATE_START);
 mkdir("ux0:data/shotgunk2vita",0777);
 FILE *log=fopen("ux0:data/shotgunk2vita/runtime.log","w");
 memset(pixels,0,WIDTH*HEIGHT*4);text(pixels,"SHOTGUNK VITA\nLOADING LUA AND GAME DATA",40);
 sceDisplaySetFrameBuf(&frame,SCE_DISPLAY_SETBUF_NEXTFRAME);sceDisplayWaitVblankStart();
 int running=sg_boot("ux0:data/shotgunk2vita",log);unsigned frames=0;
 while(1){SceCtrlData pad={0};SceTouchData touch={0};sceCtrlPeekBufferPositive(0,&pad,1);sceTouchPeek(0,&touch,1);
  if((pad.buttons&(SCE_CTRL_START|SCE_CTRL_SELECT))==(SCE_CTRL_START|SCE_CTRL_SELECT))break;
  if(running)running=sg_step(input(pad.buttons),pad.lx,pad.ly,pad.rx,pad.ry,touch.reportNum>0,touch.reportNum?touch.report[0].x*320/1920:160,touch.reportNum?touch.report[0].y*180/1088:90);
  sg_present(pixels,WIDTH,HEIGHT);
  if(!running){for(int i=0;i<WIDTH*HEIGHT;i++)pixels[i]=0xff201018;text(pixels,"SHOTGUNK VITA BOOTSTRAP STOPPED\nREAD RUNTIME LOG IN DATA FOLDER\nSTART AND SELECT TO EXIT",28);text(pixels,sg_error(),110);}
  sceDisplaySetFrameBuf(&frame,SCE_DISPLAY_SETBUF_NEXTFRAME);sceDisplayWaitVblankStart();
  if(log&&frames%300==0){fprintf(log,"native_frames=%u running=%d\n",frames,running);fflush(log);}frames++;
 }
 sg_close();if(log)fclose(log);sceKernelExitProcess(0);return 0;
}
