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

#define WIDTH 960
#define HEIGHT 544
static uint32_t *pixels;
static void rect(int x,int y,int w,int h,uint32_t color) {
    for(int j=y;j<y+h && j<HEIGHT;j++)
        for(int i=x;i<x+w && i<WIDTH;i++)
            if(i>=0 && j>=0) pixels[j*WIDTH+i]=color;
}
int main(void) {
    SceUID block=sceKernelAllocMemBlock("ShotgunK2Vita frame",SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,0x200000,NULL);
    if(block<0) return 1;
    sceKernelGetMemBlockBase(block,(void **)&pixels);
    SceDisplayFrameBuf frame={sizeof(frame),pixels,WIDTH,SCE_DISPLAY_PIXELFORMAT_A8B8G8R8,WIDTH,HEIGHT};
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,SCE_TOUCH_SAMPLING_STATE_START);
    mkdir("ux0:data/sg2vita",0777);
    FILE *log=fopen("ux0:data/sg2vita/diagnostic.log","w");
    if(log) { fputs("ShotgunK2Vita 0.1 diagnostic. Game engine NOT integrated.\n",log); fflush(log); }
    int x=480,y=272; uint32_t previous=0; unsigned frames=0;
    while(1) {
        SceCtrlData pad={0}; SceTouchData touch={0};
        sceCtrlPeekBufferPositive(0,&pad,1);
        sceTouchPeek(0,&touch,1);
        if(pad.buttons&SCE_CTRL_START) break;
        if(pad.lx<90 || pad.lx>166) x+=(pad.lx-128)/20;
        if(pad.ly<90 || pad.ly>166) y+=(pad.ly-128)/20;
        if(pad.buttons&SCE_CTRL_LEFT) x-=3;
        if(pad.buttons&SCE_CTRL_RIGHT) x+=3;
        if(pad.buttons&SCE_CTRL_UP) y-=3;
        if(pad.buttons&SCE_CTRL_DOWN) y+=3;
        if(touch.reportNum) { x=touch.report[0].x*WIDTH/1920; y=touch.report[0].y*HEIGHT/1088; }
        if(x<0)x=0;
        if(x>=WIDTH)x=WIDTH-1;
        if(y<0)y=0;
        if(y>=HEIGHT)y=HEIGHT-1;
        rect(0,0,WIDTH,HEIGHT,0xff241b16);
        for(int row=0;row<8;row++) for(int col=0;col<8;col++)
            rect(240+col*60,32+row*60,60,60,(row+col)%2?0xff51423c:0xffd3c6b5);
        rect(x-6,y-6,13,13,pad.buttons&SCE_CTRL_CROSS?0xff44ee44:0xff4488ff);
        rect(16,16,160,16,0xff4488ff);
        rect(16,48,160,16,touch.reportNum?0xff44ee44:0xff555555);
        if(log && pad.buttons!=previous) { fprintf(log,"frame=%u buttons=%08lx cursor=%d,%d\n",frames,(unsigned long)pad.buttons,x,y); fflush(log); }
        previous=pad.buttons; frames++;
        sceDisplaySetFrameBuf(&frame,SCE_DISPLAY_SETBUF_NEXTFRAME);
        sceDisplayWaitVblankStart();
    }
    if(log) { fputs("Normal exit via START\n",log); fclose(log); }
    sceKernelExitProcess(0);
    return 0;
}
