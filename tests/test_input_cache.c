#include "runtime.h"
#include "perf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
static double value(unsigned b,int x,int touch){
 double v=(b&SG_CROSS)||touch?1:0;
 double left=(128-x)>24?((128-x)-24)/103.0:0;
 return left>v?left:v;
}
int main(int argc,char **argv){
 if(argc<3)return 2;char from[1600],to[1600];mkdir(argv[2],0777);
 snprintf(to,sizeof(to),"%s/assets",argv[2]);mkdir(to,0777);
 snprintf(to,sizeof(to),"%s/assets/fonts",argv[2]);mkdir(to,0777);
 snprintf(from,sizeof(from),"%s/assets/fonts/Terminus.ttf",argv[1]);
 snprintf(to,sizeof(to),"%s/assets/fonts/Terminus.ttf",argv[2]);
 FILE *a=fopen(from,"rb"),*f=fopen(to,"wb");if(!a||!f)return 2;
 char bytes[4096];size_t n;while((n=fread(bytes,1,sizeof(bytes),a)))fwrite(bytes,1,n,f);fclose(a);fclose(f);
 snprintf(to,sizeof(to),"%s/code.lua",argv[2]);f=fopen(to,"w");if(!f)return 2;
 fputs("local spec=', C:CROSS,, C:LSTICK:LEFT, TOUCH, '\n"
       "function _init() defbtn('mixed',0,spec) end\n"
       "function _update()\n"
       " for i=1,100 do\n"
       "  assert(btn('mixed')==(expect>0)); assert(math.abs(btnv('mixed')-expect)<1e-12)\n"
       "  assert(btnp('mixed')==(edge>0)); assert(btnr('mixed')==release)\n"
       "  assert(math.abs(btnv(spec)-expect)<1e-12)\n"
       " end\n"
       " defbtn('mixed',0,'c:rtrigger'); assert(btn('mixed')==right)\n"
       " defbtn('mixed',0,spec); assert(math.abs(btnv('mixed')-expect)<1e-12)\n"
       "end\nfunction _draw() end\n",f);fclose(f);
 int ok=sg_boot(argv[2],stdout),px=0,pt=0;unsigned pb=0,seed=12345;
 uint64_t update=0,calls=0,evals=0,parses=0;
 for(int i=0;ok&&i<1000;i++){
  seed=seed*1664525u+1013904223u;unsigned b=((seed&1)?SG_CROSS:0)|((seed&2)?SG_R:0);
  int x=(seed>>8)&255,touch=(seed>>16)&1;
  double expected=value(b,x,touch),previous=value(pb,px,pt);
  double edge=value((b&SG_CROSS)&&!(pb&SG_CROSS)?SG_CROSS:0,x,touch&&!pt);
  if(128-px>24)edge=((b&SG_CROSS)&&!(pb&SG_CROSS))||(touch&&!pt)?1:0;
  char script[512];snprintf(script,sizeof(script),"expect=%.17g;edge=%.17g;release=%s;right=%s",expected,edge,previous>0&&expected==0?"true":"false",b&SG_R?"true":"false");
  ok=sg_host_eval(script)&&sg_step(b,x,128,128,128,touch,160,90);
  update+=sg_perf.update_us;
#ifndef SG_LEGACY_INPUTS
  calls+=sg_perf.input_calls;evals+=sg_perf.input_evaluations;parses+=sg_perf.input_parses;
  if(sg_perf.input_evaluations>4)ok=0;
#endif
  pb=b;px=x;pt=touch;
 }
 if(!ok)fprintf(stderr,"%s\n",sg_error());
 printf("INPUT_EQUIVALENCE=%s frames=1000 update_us=%llu calls=%llu evaluations=%llu parses=%llu\n",ok?"passed":"failed",(unsigned long long)update,(unsigned long long)calls,(unsigned long long)evals,(unsigned long long)parses);
 sg_close();return !ok;
}
