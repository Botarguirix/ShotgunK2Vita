#include "runtime.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
int main(int argc,char **argv){
 if(argc<3)return 2;mkdir(argv[2],0777);char path[1600];snprintf(path,sizeof(path),"%s/code.lua",argv[2]);
 FILE *f=fopen(path,"w");if(!f)return 2;
 fputs("SET={music=7,sfx=4}; OPTIONS={music={nid='music',def=10},sfx={nid='sfx',def=10},missing={nid='shields',def=2}}\n"
       "function _init()\n"
       " assert(bget('save',OPTIONS.music,0)==7)\n"
       " bset('save',OPTIONS.music,0,3); assert(SET.music==3)\n"
       " assert(bget('save',OPTIONS.missing,0)==2)\n"
       " assert(not pcall(bget,'save',0,0))\n"
       " assert(not pcall(bget,'stats',OPTIONS.music,0))\n"
       "end\nfunction _update() end\nfunction _draw() end\n",f);fclose(f);
 /* Default font is still loaded from the real prepared root. */
 char fontdir[1600],from[1600],to[1600];snprintf(fontdir,sizeof(fontdir),"%s/assets",argv[2]);mkdir(fontdir,0777);snprintf(fontdir,sizeof(fontdir),"%s/assets/fonts",argv[2]);mkdir(fontdir,0777);
 snprintf(from,sizeof(from),"%s/assets/fonts/Terminus.ttf",argv[1]);snprintf(to,sizeof(to),"%s/assets/fonts/Terminus.ttf",argv[2]);
 FILE *a=fopen(from,"rb"),*b=fopen(to,"wb");if(!a||!b)return 2;char bytes[4096];size_t n;while((n=fread(bytes,1,sizeof(bytes),a)))if(fwrite(bytes,1,n,b)!=n)return 2;fclose(a);fclose(b);
 int ok=sg_boot(argv[2],stdout);if(ok)ok=sg_step(0,128,128,128,128,0,160,90);
 if(!ok)fprintf(stderr,"%s\n",sg_error());sg_close();puts(ok?"OPTIONS_BRIDGE_PASSED":"OPTIONS_BRIDGE_FAILED");return ok?0:1;
}
