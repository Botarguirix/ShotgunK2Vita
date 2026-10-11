#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int copy_file(const char *from,const char *to){
    FILE *a=fopen(from,"rb"),*b=fopen(to,"wb");if(!a||!b){if(a)fclose(a);if(b)fclose(b);return 0;}
    char buffer[4096];size_t size;int ok=1;
    while((size=fread(buffer,1,sizeof(buffer),a)))if(fwrite(buffer,1,size,b)!=size){ok=0;break;}
    if(ferror(a))ok=0;fclose(a);if(fclose(b))ok=0;return ok;
}
int main(int argc,char **argv){
    if(argc<3)return 2;char path[1600],from[1600];mkdir(argv[2],0777);
    const char *dirs[]={"assets","assets/fonts","code",NULL};
    for(int i=0;dirs[i];i++){snprintf(path,sizeof(path),"%s/%s",argv[2],dirs[i]);mkdir(path,0777);}
    const char *files[]={"assets/fonts/Terminus.ttf","code/gamepad.lua",NULL};
    for(int i=0;files[i];i++){snprintf(path,sizeof(path),"%s/%s",argv[2],files[i]);snprintf(from,sizeof(from),"%s/%s",argv[1],files[i]);if(!copy_file(from,path))return 2;}
    snprintf(path,sizeof(path),"%s/code.lua",argv[2]);FILE *file=fopen(path,"w");if(!file)return 2;
    fputs("_log=log\nrequire('code/gamepad.lua')\n"
          "local dirs={'leftStickX-','leftStickX+','leftStickY-','leftStickY+'}\nlocal hits={0,0,0,0}\n"
          "function _init() use_gamepad_layout('PS',true); controls(INPUT_ASSIGNEMENT) end\n"
          "function _update()\n"
          "  for i,id in ipairs(dirs) do if btnrp(id) then hits[i]=hits[i]+1 end end\n"
          "  if btnp('c:start') then\n"
          "    assert(dt()>0 and dt()==delta(),'delta/dt contract failed')\n"
          "    for i,n in ipairs(hits) do assert(n>=12,'Held direction did not repeat');log('direction '..i..' repeats='..n) end\n"
          "  end\nend\nfunction _draw() end\n",file);fclose(file);
    int ok=sg_boot(argv[2],stdout),frames=0;
    const unsigned pad[]={SG_LEFT,SG_RIGHT,SG_UP,SG_DOWN};
    for(int cycle=0;ok&&cycle<3;cycle++)for(int direction=0;ok&&direction<4;direction++)for(int analog=0;ok&&analog<2;analog++){
        for(int held=0;ok&&held<60;held++){
            int lx=128,ly=128;if(analog){if(direction<2)lx=direction?255:0;else ly=direction==2?0:255;}
            ok=sg_step(analog?0:pad[direction],lx,ly,128,128,0,0,0);frames++;
        }
        if(ok)ok=sg_step(0,128,128,128,128,0,0,0);
    }
    if(argc>3&&!strcmp(argv[3],"--expect-missing-delta")){
        int expected=!ok&&frames==2&&strstr(sg_error(),"delta")&&strstr(sg_error(),"nil value");
        printf("OLD_DELTA_FAILURE_REPRODUCED=%d at_frame=%d\n",expected,frames);sg_close();return expected?0:1;
    }
    if(ok)ok=sg_step(SG_START,128,128,128,128,0,0,0);
    if(!ok)fprintf(stderr,"%s\n",sg_error());sg_close();
    printf("PAD_REPEAT_REGRESSION=%s frames=%d (four D-pad and stick directions held/released)\n",ok?"passed":"failed",frames);
    return ok?0:1;
}
