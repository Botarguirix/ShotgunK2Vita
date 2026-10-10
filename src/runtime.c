#include "runtime.h"
#include "graphics.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

static lua_State *state;
static FILE *output;
static char rootpath[1024], failure[1024];
static unsigned instructions;
static double frame_time;
typedef struct{char id[80],tokens[512];} Binding;
static Binding bindings[128];static int binding_count;
static unsigned buttons_now,buttons_prev;static int axes[4],axes_prev[4],touched,touch_prev,mx,my;
static char control_string[8192];
static void bind(const char *id,const char *tokens){int i;for(i=0;i<binding_count;i++)if(!strcmp(bindings[i].id,id))break;if(i==binding_count){if(i==128)return;binding_count++;}snprintf(bindings[i].id,80,"%s",id);snprintf(bindings[i].tokens,512,"%s",tokens);}
static unsigned token_bit(const char *token){if(strstr(token,"cross")||!strcmp(token,"c:a"))return SG_CROSS;if(strstr(token,"circle")||!strcmp(token,"c:b"))return SG_CIRCLE;if(strstr(token,"square")||!strcmp(token,"c:x"))return SG_SQUARE;if(strstr(token,"triangle")||!strcmp(token,"c:y"))return SG_TRIANGLE;if(strstr(token,"dpad:left"))return SG_LEFT;if(strstr(token,"dpad:right"))return SG_RIGHT;if(strstr(token,"dpad:up"))return SG_UP;if(strstr(token,"dpad:down"))return SG_DOWN;if(strstr(token,"lshoulder")||strstr(token,"ltrigger")||strstr(token,"l1")||strstr(token,"l2"))return SG_L;if(strstr(token,"rshoulder")||strstr(token,"rtrigger")||strstr(token,"r1")||strstr(token,"r2"))return SG_R;if(strstr(token,"start")||strstr(token,"options"))return SG_START;if(strstr(token,"back")||strstr(token,"touchpad"))return SG_SELECT;return 0;}
static double input_value(const char *id,int edge){const char *tokens=id;for(int i=0;i<binding_count;i++)if(!strcmp(id,bindings[i].id)){tokens=bindings[i].tokens;break;}char copy[512];snprintf(copy,sizeof(copy),"%s",tokens);double value=0;char *token=strtok(copy,",");while(token){while(*token==' '||*token=='\t')token++;size_t n=strlen(token);while(n&&token[n-1]==' ')token[--n]=0;unsigned mask=token_bit(token);double v=mask?((buttons_now&mask)&&(!edge||!(buttons_prev&mask))):0;if(!strcmp(token,"m:left")||!strcmp(token,"m:lb")||!strcmp(token,"touch"))v=touched&&(!edge||!touch_prev);if(strstr(token,"stick:")){int right=strstr(token,"rstick")!=NULL;int horizontal=strstr(token,"left")||strstr(token,"right");int a=axes[(right?2:0)+(horizontal?0:1)]-128;int sign=strstr(token,"left")||strstr(token,"up")?-1:1;v=a*sign>24?(a*sign-24)/103.0:0;if(edge&&(axes_prev[(right?2:0)+(horizontal?0:1)]-128)*sign>24)v=0;}if(v>value)value=v;token=strtok(NULL,",");}return value;}
static int btn(lua_State *L){lua_pushboolean(L,input_value(luaL_checkstring(L,1),0)>0);return 1;}
static int btnp(lua_State *L){lua_pushboolean(L,input_value(luaL_checkstring(L,1),1)>0);return 1;}
static int btnr(lua_State *L){const char *id=luaL_checkstring(L,1);double now=input_value(id,0);unsigned current=buttons_now;buttons_now=buttons_prev;int current_touch=touched;touched=touch_prev;double previous=input_value(id,0);buttons_now=current;touched=current_touch;lua_pushboolean(L,previous>0&&now==0);return 1;}
static int btnv(lua_State *L){lua_pushnumber(L,input_value(luaL_checkstring(L,1),0));return 1;}
static int controls(lua_State *L){if(lua_gettop(L)){const char *s=luaL_checkstring(L,1);snprintf(control_string,sizeof(control_string),"%s",s);char copy[8192];snprintf(copy,sizeof(copy),"%s",s);char *line=copy;while(*line){char *end=strchr(line,'\n');if(end)*end=0;char *sep=strchr(line,'>');if(sep){*sep=0;while(*line==' '||*line=='\t'||*line=='\r')line++;size_t n=strlen(line);while(n&&(line[n-1]==' '||line[n-1]=='\r'||line[n-1]=='\t'))line[--n]=0;bind(line,sep+1);}if(!end)break;line=end+1;}}lua_pushstring(L,control_string);return 1;}
static int mouse(lua_State *L){if(!lua_gettop(L)){lua_pushinteger(L,mx);lua_pushinteger(L,my);return 2;}const char *op=luaL_checkstring(L,1);if(!strcmp(op,"show"))return 0;if(!strcmp(op,"pos")){lua_pushinteger(L,mx);lua_pushinteger(L,my);return 2;}return luaL_error(L,"Unsupported mouse operation %s",op);}
static void limit(lua_State *L, lua_Debug *ar) {
    (void)ar;
    if(++instructions>2000) luaL_error(L,"Instruction limit exceeded during bootstrap");
}
static void logline(const char *text) { if(output){fprintf(output,"%s\n",text);fflush(output);} }
static void resolve(const char *name,char *path,size_t cap) {
    /* APK scripts refer to the shared library directory two levels above. */
    while(!strncmp(name,"../",3)) name+=3;
    if(name[0]=='/' || strchr(name,':') || strstr(name,"..")) {path[0]=0;return;}
    snprintf(path,cap,"%s/%s",rootpath,name);
}
static int require_file(lua_State *L) {
    const char *name=luaL_checkstring(L,1);char path[2048];resolve(name,path,sizeof(path));
    lua_getfield(L,LUA_REGISTRYINDEX,"sg.loaded");
    lua_getfield(L,-1,path);
    if(!lua_isnil(L,-1))return 1;
    lua_pop(L,1);
    if(luaL_loadfile(L,path))return lua_error(L);
    logline(path);
    lua_call(L,0,1);
    if(lua_isnil(L,-1)){lua_pop(L,1);lua_pushboolean(L,1);}
    lua_pushvalue(L,-1);lua_setfield(L,-3,path);
    return 1;
}
static int logger(lua_State *L) {
    for(int i=1;i<=lua_gettop(L);i++) {
        const char *text=lua_tostring(L,i);
        logline(text?text:lua_typename(L,lua_type(L,i)));
    }
    return 0;
}
static int missing(lua_State *L) {return luaL_error(L,"Unimplemented SUGAR API: %s",lua_tostring(L,lua_upvalueindex(1)));}
static int file_read(lua_State *L) {
    char path[2048];resolve(luaL_checkstring(L,1),path,sizeof(path));
    if(lua_gettop(L)>1){if(strncmp(luaL_checkstring(L,1),"save/",5))return luaL_error(L,"Writes restricted to save/");size_t n;const char *s=luaL_checklstring(L,2,&n);FILE *f=fopen(path,"wb");if(!f)return luaL_error(L,"Cannot write save %s",path);size_t got=fwrite(s,1,n,f);fclose(f);if(got!=n)return luaL_error(L,"Save write incomplete");return 0;}
    FILE *f=fopen(path,"rb");if(!f){lua_pushnil(L);return 1;}
    fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    if(size<0 || size>16*1024*1024){fclose(f);return luaL_error(L,"Invalid file size");}
    char *data=malloc((size_t)size+1);if(!data){fclose(f);return luaL_error(L,"Out of memory");}
    size_t got=fread(data,1,(size_t)size,f);fclose(f);
    lua_pushlstring(L,data,got);free(data);return 1;
}
static int add(lua_State *L){luaL_checktype(L,1,LUA_TTABLE);int n=(int)lua_objlen(L,1)+1;int pos=luaL_optint(L,3,n);for(int i=n;i>pos;i--){lua_rawgeti(L,1,i-1);lua_rawseti(L,1,i);}lua_pushvalue(L,2);lua_rawseti(L,1,pos);lua_pushvalue(L,2);return 1;}
static int del(lua_State *L){int n=(int)lua_objlen(L,1);for(int i=1;i<=n;i++){lua_rawgeti(L,1,i);int same=lua_equal(L,-1,2);lua_pop(L,1);if(same){for(int j=i;j<n;j++){lua_rawgeti(L,1,j+1);lua_rawseti(L,1,j);}lua_pushnil(L);lua_rawseti(L,1,n);lua_pushvalue(L,2);return 1;}}return 0;}
static int iterator(lua_State *L){int i=lua_tointeger(L,lua_upvalueindex(2))+1;lua_pushinteger(L,i);lua_replace(L,lua_upvalueindex(2));lua_rawgeti(L,lua_upvalueindex(1),i);return 1;}
static int all(lua_State *L){luaL_checktype(L,1,LUA_TTABLE);lua_pushvalue(L,1);lua_pushinteger(L,0);lua_pushcclosure(L,iterator,2);return 1;}
static int rnd(lua_State *L){if(lua_istable(L,1)){int len=(int)lua_objlen(L,1);if(!len){lua_pushnil(L);return 1;}lua_rawgeti(L,1,1+rand()%len);return 1;}double n=luaL_optnumber(L,1,1);lua_pushnumber(L,(rand()/(RAND_MAX+1.0))*n);return 1;}
static int irnd(lua_State *L){lua_pushinteger(L,(int)floor((rand()/(RAND_MAX+1.0))*luaL_checknumber(L,1)));return 1;}
static int mid(lua_State *L){double a=luaL_checknumber(L,1),b=luaL_checknumber(L,2),c=luaL_checknumber(L,3);lua_pushnumber(L,fmax(fmin(a,b),fmin(fmax(a,b),c)));return 1;}
static int false_value(lua_State *L){lua_pushboolean(L,0);return 1;}
static int timestamp(lua_State *L){lua_pushnumber(L,frame_time);return 1;}
static int platform(lua_State *L){lua_pushstring(L,"VITA");return 1;}
static int desktop(lua_State *L){lua_pushstring(L,"save");return 1;}
static int isfile(lua_State *L){char path[2048];struct stat s;resolve(luaL_checkstring(L,1),path,sizeof(path));lua_pushboolean(L,!stat(path,&s)&&S_ISREG(s.st_mode));return 1;}
static int isfolder(lua_State *L){char path[2048];struct stat s;resolve(luaL_checkstring(L,1),path,sizeof(path));lua_pushboolean(L,!stat(path,&s)&&S_ISDIR(s.st_mode));return 1;}
static int makedir(lua_State *L){char path[2048];resolve(luaL_checkstring(L,1),path,sizeof(path));if(!path[0])return luaL_error(L,"Invalid directory");mkdir(path,0777);return 0;}
static int ctrlr(lua_State *L){const char *op=luaL_checkstring(L,1);if(!strcmp(op,"list")){lua_newtable(L);lua_pushinteger(L,0);lua_rawseti(L,-2,1);return 1;}if(!strcmp(op,"which")){lua_pushinteger(L,0);return 1;}if(!strcmp(op,"type")){lua_pushstring(L,"PS4");return 1;}if(!strcmp(op,"hasrumble")){lua_pushboolean(L,0);return 1;}if(!strcmp(op,"assign"))return 0;return luaL_error(L,"Unsupported ctrlr command %s",op);}
static int defbtn(lua_State *L){bind(luaL_checkstring(L,1),luaL_checkstring(L,3));return 0;}
static int audio_asset(lua_State *L){char path[2048];resolve(luaL_checkstring(L,1),path,sizeof(path));FILE *f=fopen(path,"rb");if(!f)return luaL_error(L,"Missing audio resource: %s",path);fclose(f);lua_pushvalue(L,lua_gettop(L)>1?2:1);return 1;}
static int muted_audio(lua_State *L){(void)L;return 0;}
static int foreach(lua_State *L){luaL_checktype(L,1,LUA_TTABLE);luaL_checktype(L,2,LUA_TFUNCTION);int n=(int)lua_objlen(L,1);lua_newtable(L);for(int i=1;i<=n;i++){lua_rawgeti(L,1,i);lua_rawseti(L,3,i);}for(int i=1;i<=n;i++){lua_pushvalue(L,2);lua_rawgeti(L,3,i);if(lua_isnil(L,-1)){lua_pop(L,2);continue;}lua_call(L,1,0);}return 0;}
static int split(lua_State *L){size_t len;const char *s=luaL_checklstring(L,1,&len),*delimiter=luaL_optstring(L,2,",");size_t dl=strlen(delimiter);if(!dl)return luaL_error(L,"Empty split delimiter");lua_newtable(L);int i=1;const char *start=s,*end=s+len;while(start<=end){const char *next=strstr(start,delimiter);size_t n=next?(size_t)(next-start):(size_t)(end-start);lua_pushlstring(L,start,n);lua_rawseti(L,-2,i++);if(!next)break;start=next+dl;}return 1;}
static int async_file(lua_State *L){if(lua_toboolean(L,3))return luaL_error(L,"Encoded saves are not supported");file_read(L);lua_pushcfunction(L,false_value);return 1;}
static int encoded_file(lua_State *L){char path[2048];resolve(luaL_checkstring(L,1),path,sizeof(path));FILE *f=fopen(path,"rb");if(f){fclose(f);return luaL_error(L,"Legacy encoded save import is not supported");}lua_pushnil(L);return 1;}
static int mapvalue(lua_State *L){double v=luaL_checknumber(L,1),a=luaL_checknumber(L,2),b=luaL_checknumber(L,3),c=luaL_checknumber(L,4),d=luaL_checknumber(L,5);lua_pushnumber(L,b==a?c:c+(v-a)*(d-c)/(b-a));return 1;}
static int syslang(lua_State *L){lua_pushstring(L,"en");lua_pushstring(L,"US");return 2;}
static int delta(lua_State *L){lua_pushnumber(L,1.0/60);return 1;}
static int sugarstep(lua_State *L){(void)L;frame_time+=1.0/60;return 0;}
static int lerp(lua_State *L){double a=luaL_checknumber(L,1),b=luaL_checknumber(L,2),c=luaL_checknumber(L,3);lua_pushnumber(L,a+(b-a)*c);return 1;}
static int turn_sin(lua_State *L){lua_pushnumber(L,sin(luaL_checknumber(L,1)*6.283185307179586));return 1;}
static int turn_cos(lua_State *L){lua_pushnumber(L,cos(luaL_checknumber(L,1)*6.283185307179586));return 1;}
static int roundvalue(lua_State *L){lua_pushnumber(L,floor(luaL_checknumber(L,1)+.5));return 1;}
static int signvalue(lua_State *L){double v=luaL_checknumber(L,1);lua_pushinteger(L,v<0?-1:v>0?1:0);return 1;}
static int turn_atan(lua_State *L){double x=luaL_checknumber(L,1),y=luaL_checknumber(L,2);double v=atan2(y,x)/6.283185307179586;if(v<0)v+=1;lua_pushnumber(L,v);return 1;}
static int remove_save(lua_State *L){const char *name=luaL_checkstring(L,1);if(strncmp(name,"save/",5))return luaL_error(L,"Removal restricted to save/");char path[2048];resolve(name,path,sizeof(path));if(path[0])remove(path);return 0;}
static int lib_callbacks(lua_State *L,const char *name){lua_getglobal(L,name);if(lua_istable(L,-1)){lua_pushnil(L);while(lua_next(L,-2)){lua_pushvalue(L,-1);lua_call(L,0,0);lua_pop(L,1);}}lua_pop(L,1);return 0;}
static int shallow_copy(lua_State *L){luaL_checktype(L,1,LUA_TTABLE);lua_newtable(L);lua_pushnil(L);while(lua_next(L,1)){lua_pushvalue(L,-2);lua_pushvalue(L,-2);lua_settable(L,2);lua_pop(L,1);}return 1;}
static int init_libs(lua_State *L){return lib_callbacks(L,"_lib_init");}
static int frame_call(lua_State *L){lib_callbacks(L,"_lib_update");lua_getglobal(L,"_update");if(lua_isfunction(L,-1))lua_call(L,0,0);else lua_pop(L,1);lua_getglobal(L,"_draw");if(lua_isfunction(L,-1))lua_call(L,0,0);else lua_pop(L,1);lib_callbacks(L,"_lib_draw");return 0;}
static int graphics_start(lua_State *L){sg_graphics_init(L,rootpath);return 0;}
static int list(lua_State *L){char path[2048];resolve(luaL_optstring(L,1,""),path,sizeof(path));DIR *d=opendir(path);lua_newtable(L);if(d){struct dirent *e;int i=1;while((e=readdir(d))){if(e->d_name[0]=='.')continue;lua_pushstring(L,e->d_name);lua_rawseti(L,-2,i++);}closedir(d);}return 1;}
static int gameplay(lua_State *L){lua_getglobal(L,"require");lua_pushvalue(L,1);lua_call(L,1,1);return 1;}
static void reg(const char *name,lua_CFunction fn){lua_pushcfunction(state,fn);lua_setglobal(state,name);}
static void boolean(const char *name,int value){lua_pushboolean(state,value);lua_setglobal(state,name);}
static void alias(const char *name,const char *table,const char *field){lua_getglobal(state,table);lua_getfield(state,-1,field);lua_setglobal(state,name);lua_pop(state,1);}
int sg_boot(const char *root,FILE *log) {
    output=log;failure[0]=0;instructions=0;snprintf(rootpath,sizeof(rootpath),"%s",root);
    state=luaL_newstate();if(!state){snprintf(failure,sizeof(failure),"Lua allocation failed");return 0;}
    luaL_openlibs(state);
    lua_pushnil(state);lua_setglobal(state,"io");lua_pushnil(state);lua_setglobal(state,"os");
    lua_pushnil(state);lua_setglobal(state,"debug");lua_pushnil(state);lua_setglobal(state,"dofile");
    lua_pushnil(state);lua_setglobal(state,"loadfile");lua_pushnil(state);lua_setglobal(state,"package");
    lua_newtable(state);lua_setfield(state,LUA_REGISTRYINDEX,"sg.loaded");
    reg("require",require_file);reg("log",logger);reg("wlog",logger);reg("rlog",logger);reg("print",logger);
    reg("file",file_read);reg("all",all);reg("add",add);reg("del",del);reg("rnd",rnd);reg("irnd",irnd);reg("mid",mid);
    reg("time",timestamp);reg("ls",list);reg("read_gameplay_file",gameplay);
    reg("runtime",platform);
    reg("desktop_path",desktop);
    reg("isfile",isfile);
    reg("isfolder",isfolder);reg("mkdir",makedir);
    reg("ctrlr",ctrlr);reg("defbtn",defbtn);
    reg("foreach",foreach);reg("split",split);
    reg("shallow_copy",shallow_copy);
    reg("afile",async_file);
    reg("encfile",encoded_file);reg("map",mapvalue);
    reg("syslang",syslang);
    reg("dt",delta);reg("t",timestamp);reg("sugar_step",sugarstep);reg("lerp",lerp);
    reg("round",roundvalue);
    reg("sgn",signvalue);
    reg("rm",remove_save);
    reg("live",false_value);reg("psn",false_value);reg("steam",false_value);
    const char *maths[]={"abs","min","max","sqrt","floor","ceil","sin","cos","tan","atan2","pow","log",NULL};
    for(int i=0;maths[i];i++)if(strcmp(maths[i],"log"))alias(maths[i],"math",maths[i]);
    reg("sin",turn_sin);reg("cos",turn_cos);
    reg("atan2",turn_atan);
    alias("flr","math","floor");alias("sub","string","sub");alias("chr","string","char");alias("ord","string","byte");
    alias("find","string","find");alias("sbs","string","gsub");alias("match","string","match");alias("format","string","format");
    alias("deli","table","remove");
    lua_getglobal(state,"tostring");lua_setglobal(state,"tostr");lua_getglobal(state,"tonumber");lua_setglobal(state,"tonum");
    boolean("PC",1);boolean("MOBILE",0);boolean("DEV",0);boolean("IN_EXPORT",1);boolean("IN_LOOSE_FILES",1);boolean("SHADERLESS",1);
    lua_newtable(state);lua_setglobal(state,"EXE_ARGS");
    lua_newtable(state);lua_setglobal(state,"_lib_init");lua_newtable(state);lua_setglobal(state,"_lib_update");lua_newtable(state);lua_setglobal(state,"_lib_draw");
    lua_pushstring(state,"PC");lua_setglobal(state,"BUILD_TYPE");lua_pushstring(state,"PS");lua_setglobal(state,"GAMEPAD_LAYOUT");
    const char *apis[]={"bprint","newfnt","font","strwidth","pal","newwin","camera","tcamera","rumble","newsrf","palette","sfx","newmus","newsfx","target","spritesheet","sspr","spr","rectfill","line","btn","btnp","btnv","controls","winspec","fpslimit","shdrf","newbnk","delbnk",NULL};
    for(int i=0;apis[i];i++){lua_pushstring(state,apis[i]);lua_pushcclosure(state,missing,1);lua_setglobal(state,apis[i]);}
    lua_pushcfunction(state,graphics_start);
    if(lua_pcall(state,0,0,0)){snprintf(failure,sizeof(failure),"%s",lua_tostring(state,-1));logline(failure);return 0;}
    reg("controls",controls);reg("btn",btn);reg("btnp",btnp);reg("btnr",btnr);reg("btnv",btnv);reg("mouse",mouse);
    reg("newsfx",audio_asset);reg("newmus",audio_asset);reg("sfx",muted_audio);reg("music",muted_audio);reg("mus",muted_audio);reg("musvol",muted_audio);reg("sfxvol",muted_audio);reg("rumble",muted_audio);
    reg("delsfx",muted_audio);
    reg("nxtmusic",muted_audio);
    logline("Audio output disabled in bootstrap build");
    lua_sethook(state,limit,LUA_MASKCOUNT,10000);
    char path[2048];resolve("code.lua",path,sizeof(path));
    logline("ShotgunK2Vita Lua 5.1 bootstrap; unsupported APIs stop execution");
    int status=luaL_loadfile(state,path);if(!status)status=lua_pcall(state,0,0,0);
    if(!status){logline("code.lua loaded; invoking _init");lua_getglobal(state,"_init");status=lua_pcall(state,0,0,0);}
    if(!status){lua_pushcfunction(state,init_libs);status=lua_pcall(state,0,0,0);}
    if(status){snprintf(failure,sizeof(failure),"%s",lua_tostring(state,-1));logline(failure);return 0;}
    logline("_init completed");return 1;
}
const char *sg_error(void){return failure;}
void sg_report_state(void){if(!state)return;const char *names[]={"ingame","playing","game_mode","ctrl_mode","selectedOption","MOUSE","turn",NULL};for(int i=0;names[i];i++){lua_getglobal(state,names[i]);char line[256];const char *s=lua_tostring(state,-1);snprintf(line,sizeof(line),"state.%s=%s",names[i],s?s:lua_isboolean(state,-1)?(lua_toboolean(state,-1)?"true":"false"):lua_typename(state,lua_type(state,-1)));logline(line);lua_pop(state,1);}}
int sg_step(unsigned buttons,int lx,int ly,int rx,int ry,int touch,int x,int y){if(!state||failure[0])return 0;buttons_prev=buttons_now;buttons_now=buttons;memcpy(axes_prev,axes,sizeof(axes));touch_prev=touched;touched=touch;mx=x;my=y;axes[0]=lx;axes[1]=ly;axes[2]=rx;axes[3]=ry;frame_time+=1.0/60;instructions=0;lua_pushcfunction(state,frame_call);if(lua_pcall(state,0,0,0)){snprintf(failure,sizeof(failure),"%s",lua_tostring(state,-1));logline(failure);return 0;}return 1;}
void sg_close(void){if(state)lua_close(state);state=NULL;sg_graphics_close();}
