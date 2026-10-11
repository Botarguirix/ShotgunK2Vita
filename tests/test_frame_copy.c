#include <assert.h>
#include "frame_copy.h"
static int behavior;
uint64_t sg_now_us(void){static uint64_t now;return now+=10;}
int sceDmacMemcpy(void *dst,const void *src,size_t bytes){
 if(behavior==3)return -7;
 if(behavior!=0)memcpy(dst,src,bytes);
 if(behavior==2)((unsigned char *)dst)[bytes/2]^=1;
 return 0;
}
int main(void){
 unsigned char src[4096],dst[4096];
 behavior=0;SgFrameCopy c=sg_copy_select(dst,src,sizeof(src),1,stdout);assert(c.mode==0);
 behavior=2;c=sg_copy_select(dst,src,sizeof(src),1,stdout);assert(c.mode==0);
 behavior=1;c=sg_copy_select(dst,src,sizeof(src),1,stdout);assert(c.mode==1);
 memset(src,0x71,sizeof(src));behavior=3;
 sg_copy_frame(&c,dst,src,sizeof(src),stdout);
 assert(c.mode==0&&c.last_rc==-7&&!memcmp(dst,src,sizeof(src)));
 behavior=0;memset(src,0x29,sizeof(src));sg_copy_frame(&c,dst,src,sizeof(src),stdout);
 assert(!memcmp(dst,src,sizeof(src)));
 behavior=1;c=sg_copy_select(dst,src,sizeof(src),0,stdout);assert(c.mode==0);
 puts("FRAME_COPY_INTEGRITY_AND_FALLBACK_PASSED");return 0;
}
