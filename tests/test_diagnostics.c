#include "diagnostics.h"
#include "runtime.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static char *read_all(const char *directory,const char *name){
    char path[1600];snprintf(path,sizeof(path),"%s/%s",directory,name);
    FILE *file=fopen(path,"rb");assert(file);fseek(file,0,SEEK_END);long size=ftell(file);rewind(file);
    assert(size>=0);char *text=calloc((size_t)size+1,1);assert(text);
    assert(fread(text,1,(size_t)size,file)==(size_t)size);fclose(file);return text;
}
int main(int argc,char **argv){
    assert(argc>=2);
    if(argc>2&&!strcmp(argv[2],"abrupt")){
        assert(sg_diag_start(argv[1]));sg_diag_frame(37,256,64,128,192,128);
        sg_diag_phase(SG_DIAG_DRAW);sg_diag_location("fixture/native-stage.lua",73);
        sg_diag_event("fixture","before abrupt termination");usleep(600000);
        _exit(77); /* Simulate a process ending without fclose or sg_diag_stop. */
    }
    char code[1400];snprintf(code,sizeof(code),"%s/code.lua",argv[1]);
    assert(argc>=3);mkdir(argv[1],0777);
    char assets[1400];snprintf(assets,sizeof(assets),"%s/assets",argv[1]);mkdir(assets,0777);
    snprintf(assets,sizeof(assets),"%s/assets/fonts",argv[1]);mkdir(assets,0777);
    snprintf(assets,sizeof(assets),"%s/assets/fonts/Terminus.ttf",argv[1]);
    FILE *font=fopen(argv[2],"rb"),*copy=fopen(assets,"wb");assert(font&&copy);
    char buffer[4096];size_t bytes;while((bytes=fread(buffer,1,sizeof(buffer),font)))assert(fwrite(buffer,1,bytes,copy)==bytes);
    fclose(font);fclose(copy);
    FILE *fixture=fopen(code,"w");assert(fixture);
    fputs("function _init() end\nlocal function inner() error('DIAG_FIXTURE_FAILURE') end\nlocal function outer() inner() end\nfunction _update() outer() end\nfunction _draw() end\n",fixture);fclose(fixture);
    assert(sg_diag_start(argv[1]));char first[1200];snprintf(first,sizeof(first),"%s",sg_diag_directory());
    FILE *log=sg_diag_open("runtime.log");assert(log);
    assert(sg_boot(argv[1],log));assert(!sg_step(0,128,128,128,128,0,0,0));
    assert(strstr(sg_error(),"DIAG_FIXTURE_FAILURE"));assert(strstr(sg_error(),"stack traceback"));
    sg_close();fclose(log);sg_diag_stop("lua_error");
    char *content=read_all(first,"runtime.log");assert(strstr(content,"inner"));assert(strstr(content,"outer"));free(content);
    content=read_all(first,"crash-trace.log");assert(strstr(content,"session_end=lua_error"));free(content);
    pid_t child=fork();assert(child>=0);
    if(!child){execl(argv[0],argv[0],argv[1],"abrupt",NULL);_exit(99);}
    int status;assert(waitpid(child,&status,0)==child);assert(WIFEXITED(status)&&WEXITSTATUS(status)==77);
    char logs[1400];snprintf(logs,sizeof(logs),"%s/logs",argv[1]);
    char *previous=read_all(logs,"latest.txt");previous[strcspn(previous,"\r\n")]=0;
    assert(strcmp(previous,first));content=read_all(previous,"crash-trace.log");
    assert(strstr(content,"frame=37"));assert(strstr(content,"phase=lua_draw"));
    assert(strstr(content,"native-stage.lua:73"));assert(!strstr(content,"session_end="));
    assert(sg_diag_start(argv[1]));assert(strcmp(previous,sg_diag_directory()));sg_diag_stop("clean");
    char *unchanged=read_all(previous,"crash-trace.log");assert(!strcmp(content,unchanged));free(content);free(unchanged);free(previous);
    puts("DIAGNOSTICS_PASSED: Lua traceback, durable abrupt exit, distinct sessions, preservation");return 0;
}
