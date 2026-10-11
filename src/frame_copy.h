#ifndef SG_FRAME_COPY_H
#define SG_FRAME_COPY_H
#include <stdio.h>
#include <stddef.h>
#ifdef SG_VITA
#include <psp2/kernel/dmac.h>
#else
int sceDmacMemcpy(void *dst,const void *src,size_t size);
#endif
#include <string.h>
#include "perf.h"
/* Synchronous copies only: never submit a framebuffer before completion. */
typedef struct { int mode, last_rc; } SgFrameCopy;
static void sg_copy_frame(SgFrameCopy *c, void *dst, const void *src, size_t bytes, FILE *log){
 if(c->mode){
  c->last_rc=sceDmacMemcpy(dst,src,bytes);
  if(c->last_rc>=0)return;
  if(log)fprintf(log,"copy_dma_failed=%d fallback=cpu\n",c->last_rc);
  c->mode=0;
 }
 memcpy(dst,src,bytes);
}
static SgFrameCopy sg_copy_select(void *dst, unsigned char *src, size_t bytes, int requested, FILE *log){
 SgFrameCopy c={0,0};uint64_t cpu=0,dma=0;int valid=1,passes=0;
 /* Changing the entire cached source each pass checks visibility as well as
  * integrity. Destination is the inactive buffer, before initial scanout. */
 for(int pass=0;pass<8;pass++){
  for(size_t i=0;i<bytes;i++)src[i]=(unsigned char)(i*13+pass*37);
  uint64_t start=sg_now_us();memcpy(dst,src,bytes);cpu+=sg_now_us()-start;
  /* Invalidate the CPU result so a no-op/stale DMA cannot pass integrity. */
  memset(dst,0xa5,bytes);
  start=sg_now_us();c.last_rc=sceDmacMemcpy(dst,src,bytes);dma+=sg_now_us()-start;
  passes++;
  if(c.last_rc<0||memcmp(dst,src,bytes)){valid=0;break;}
 }
 c.mode=valid&&requested!=0&&(requested==1||dma<cpu);
 if(log)fprintf(log,"copy_probe requested=%d valid=%d rc=%d passes=%d cpu_us=%llu dma_us=%llu selected=%s\n",requested,valid,c.last_rc,passes,(unsigned long long)cpu,(unsigned long long)dma,c.mode?"dma":"cpu");
 return c;
}
#endif
