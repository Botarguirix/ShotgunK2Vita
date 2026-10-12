#include "runtime.h"
#include "local_ui.h"
#include "graphics.h"
#include "perf.h"
#include "audio.h"
#include "diagnostics.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <string.h>
#include <ctype.h>
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
typedef struct{char id[80],tokens[512];unsigned mask,axis_dirs;int touch;uint64_t epoch;double values[3];} Binding;
static uint64_t input_epoch=1;static int quit_requested;
static Binding raw_bindings[32];static unsigned raw_next;
static void compile_binding(Binding *b);
static Binding bindings[128];static int binding_count;
static unsigned buttons_now,buttons_prev;static int axes[4],axes_prev[4],touched,touch_prev,mx,my;
static char control_string[8192];
static void bind(const char *id,const char *tokens){int i;for(i=0;i<binding_count;i++)if(!strcmp(bindings[i].id,id))break;if(i==binding_count){if(i==128)return;binding_count++;}snprintf(bindings[i].id,80,"%s",id);snprintf(bindings[i].tokens,512,"%s",tokens);compile_binding(&bindings[i]);}
static unsigned token_bit(const char *token){if(strstr(token,"cross")||!strcmp(token,"c:a"))return SG_CROSS;if(strstr(token,"circle")||!strcmp(token,"c:b"))return SG_CIRCLE;if(strstr(token,"square")||!strcmp(token,"c:x"))return SG_SQUARE;if(strstr(token,"triangle")||!strcmp(token,"c:y"))return SG_TRIANGLE;if(strstr(token,"dpad:left"))return SG_LEFT;if(strstr(token,"dpad:right"))return SG_RIGHT;if(strstr(token,"dpad:up"))return SG_UP;if(strstr(token,"dpad:down"))return SG_DOWN;if(strstr(token,"lshoulder")||strstr(token,"l1"))return SG_LPAGE;if(strstr(token,"ltrigger")||strstr(token,"l2"))return SG_L;if(strstr(token,"rshoulder")||strstr(token,"r1"))return SG_RPAGE;if(strstr(token,"rtrigger")||strstr(token,"r2"))return SG_R;if(strstr(token,"lstickb")||strstr(token,"l3"))return SG_LSTICK;if(strstr(token,"rstickb")||strstr(token,"r3"))return SG_RSTICK;if(strstr(token,"start")||strstr(token,"options"))return SG_START;if(strstr(token,"back")||strstr(token,"touchpad"))return SG_SELECT;return 0;}
static void compile_binding(Binding *b){
 b->mask=b->axis_dirs=0;b->touch=0;b->epoch=0;sg_perf.input_parses++;
 char copy[512];snprintf(copy,sizeof(copy),"%s",b->tokens);char *token=copy;
 while(token){char *next=strchr(token,',');if(next)*next++=0;
  while(*token==' '||*token=='\t')token++;
  size_t n=strlen(token);while(n&&token[n-1]==' ')token[--n]=0;
  for(char *p=token;*p;p++)*p=(char)tolower((unsigned char)*p);
  if(strstr(token,"stick:")){
   int right=strstr(token,"rstick")!=NULL,horizontal=strstr(token,"left")||strstr(token,"right");
   int axis=(right?2:0)+(horizontal?0:1),negative=strstr(token,"left")||strstr(token,"up");
   b->axis_dirs|=1u<<(axis*2+negative);
  }else if(!strcmp(token,"m:left")||!strcmp(token,"m:lb")||!strcmp(token,"touch"))b->touch=1;
  else b->mask|=token_bit(token);
  token=next;
 }
}
static double evaluate_binding(const Binding *b,int kind){
 unsigned buttons=kind==2?buttons_prev:buttons_now;
 double value=(buttons&b->mask&(kind==1?~buttons_prev:~0u))!=0;
 if(b->touch&&(kind==2?touch_prev:touched)&&(kind!=1||!touch_prev))value=1;
 for(int axis=0;axis<4;axis++)for(int negative=0;negative<2;negative++)if(b->axis_dirs&(1u<<(axis*2+negative))){
  int sign=negative?-1:1,a=((kind==2?axes_prev[axis]:axes[axis])-128)*sign;
  if(a>24&&(kind!=1||(axes_prev[axis]-128)*sign<=24)){double v=(a-24)/103.0;if(v>value)value=v;}
 }
 return value;
}
static double input_value(const char *id,int kind){
 sg_perf.input_calls++;Binding *b=NULL;Binding temporary;
 for(int i=0;i<binding_count;i++)if(!strcmp(id,bindings[i].id)){b=&bindings[i];break;}
 if(!b){
  if(strlen(id)<sizeof(raw_bindings[0].id)){
   for(int i=0;i<32;i++)if(raw_bindings[i].id[0]&&!strcmp(id,raw_bindings[i].id)){b=&raw_bindings[i];break;}
   if(!b){b=&raw_bindings[raw_next++%32];snprintf(b->id,sizeof(b->id),"%s",id);snprintf(b->tokens,sizeof(b->tokens),"%s",id);compile_binding(b);}
  }else{b=&temporary;snprintf(b->tokens,sizeof(b->tokens),"%s",id);compile_binding(b);}
 }
 if(b->epoch!=input_epoch){for(int i=0;i<3;i++)b->values[i]=evaluate_binding(b,i);b->epoch=input_epoch;sg_perf.input_evaluations++;}
 return b->values[kind];
}
static int btn(lua_State *L){lua_pushboolean(L,input_value(luaL_checkstring(L,1),0)>0);return 1;}
static int btnp(lua_State *L){lua_pushboolean(L,input_value(luaL_checkstring(L,1),1)>0);return 1;}
static int btnr(lua_State *L){const char *id=luaL_checkstring(L,1);double now=input_value(id,0),previous=input_value(id,2);lua_pushboolean(L,previous>0&&now==0);return 1;}
static int btnv(lua_State *L){lua_pushnumber(L,input_value(luaL_checkstring(L,1),0));return 1;}
static int controls(lua_State *L){if(lua_gettop(L)){const char *s=luaL_checkstring(L,1);snprintf(control_string,sizeof(control_string),"%s",s);char copy[8192];snprintf(copy,sizeof(copy),"%s",s);char *line=copy;while(*line){char *end=strchr(line,'\n');if(end)*end=0;char *sep=strchr(line,'>');if(sep){*sep=0;while(*line==' '||*line=='\t'||*line=='\r')line++;size_t n=strlen(line);while(n&&(line[n-1]==' '||line[n-1]=='\r'||line[n-1]=='\t'))line[--n]=0;bind(line,sep+1);}if(!end)break;line=end+1;}}lua_pushstring(L,control_string);return 1;}
static int mouse(lua_State *L){if(!lua_gettop(L)){lua_pushinteger(L,mx);lua_pushinteger(L,my);return 2;}const char *op=luaL_checkstring(L,1);if(!strcmp(op,"show"))return 0;if(!strcmp(op,"pos")){lua_pushinteger(L,mx);lua_pushinteger(L,my);return 2;}return luaL_error(L,"Unsupported mouse operation %s",op);}
static void limit(lua_State *L, lua_Debug *ar) {
    if(lua_getinfo(L,"Sl",ar))sg_diag_location(ar->short_src,ar->currentline);
    if(++instructions>2000) luaL_error(L,"Instruction limit exceeded during bootstrap");
}
static void logline(const char *text) { if(output){fprintf(output,"%s\n",text);fflush(output);} }
static void resolve(const char *name,char *path,size_t cap) {
    /* APK scripts refer to the shared library directory two levels above. */
    while(!strncmp(name,"../",3)) name+=3;
    if(name[0]=='/' || strchr(name,':') || strstr(name,"..")) {path[0]=0;return;}
    snprintf(path,cap,"%s/%s",rootpath,name);
}
static int load_vita_script(lua_State *L,const char *path,const char *name){
 if(strcmp(name,"code/menu.lua")&&strcmp(name,"code.lua"))return luaL_loadfile(L,path);
 FILE *f=fopen(path,"rb");if(!f){lua_pushstring(L,"Game script missing");return LUA_ERRFILE;}
 fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
 if(size<0||size>4*1024*1024){fclose(f);lua_pushstring(L,"Game script size invalid");return LUA_ERRFILE;}
 char *source=malloc((size_t)size+128);if(!source){fclose(f);lua_pushstring(L,"Game script allocation failed");return LUA_ERRMEM;}
 size_t got=fread(source,1,(size_t)size,f);fclose(f);source[got]=0;
 if(got!=(size_t)size){free(source);lua_pushstring(L,"Game script read incomplete");return LUA_ERRFILE;}
 if(!strcmp(name,"code/menu.lua")){
  char *a=strstr(source,"local str=\"Join our Discord!\""),*b=a?strstr(a,"-- CASTLE"):NULL;
  if(a&&b){for(char*q=a;q<b;q++)if(*q!='\n'&&*q!='\r')*q=' ';logline("vita_external_links=removed");}
  else logline("vita_external_links=pattern_not_found");
 }else{
  const char *prefix="thr_gre = ";char *a=strstr(source,"thr_gre = hero.special and hero.special==\"grenade\" and hero.grenade_ready");
  if(a){a+=strlen(prefix);size_t tail=got-(size_t)(a-source);memmove(a+9,a,tail+1);memcpy(a,"hero and ",9);got+=9;logline("vita_touch_hero_guard=applied");}
  else logline("vita_touch_hero_guard=pattern_not_found");
 }
 char chunk[2050];snprintf(chunk,sizeof(chunk),"@%s",path);int rc=luaL_loadbuffer(L,source,got,chunk);free(source);return rc;
}
static int require_file(lua_State *L) {
    const char *name=luaL_checkstring(L,1);char path[2048];resolve(name,path,sizeof(path));
    /* safe_require loads into a fresh thread environment. Re-execute there,
     * rather than returning a result cached in an earlier mode environment. */
    lua_getfield(L,LUA_REGISTRYINDEX,"sg.root_env");int root_env=lua_rawequal(L,-1,LUA_GLOBALSINDEX);lua_pop(L,1);
    if(!root_env){
     logline(path);sg_diag_event("require_scoped",path);
     if(load_vita_script(L,path,name))return lua_error(L);lua_call(L,0,1);
     if(lua_isnil(L,-1)){lua_pop(L,1);lua_pushboolean(L,1);}return 1;
    }
    lua_getfield(L,LUA_REGISTRYINDEX,"sg.loaded");
    lua_getfield(L,-1,path);
    if(!lua_isnil(L,-1))return 1;
    lua_pop(L,1);
    logline(path);
    sg_diag_event("require",path);
    if(load_vita_script(L,path,name))return lua_error(L);
    lua_call(L,0,1);
    if(lua_isnil(L,-1)){lua_pop(L,1);lua_pushboolean(L,1);}
    lua_pushvalue(L,-1);lua_setfield(L,-3,path);
    return 1;
}
static int local_notice(lua_State *L){sg_ui_notice(luaL_checkstring(L,1));return 0;}
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
static int request_quit(lua_State *L){(void)L;quit_requested=1;return 0;}
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
/* This APK's option sliders pass an OPTIONS descriptor, rather than bank
 * coordinates. Bridge those descriptors to its current SET save structure.
 * Numeric legacy banks remain unsupported instead of fabricating their data. */
static int option_access(lua_State *L,int write){
 if(strcmp(luaL_checkstring(L,1),"save")||!lua_istable(L,2)||luaL_checkint(L,3)!=0)
  return luaL_error(L,"Legacy numeric banks are not supported; option descriptors only");
 lua_getfield(L,2,"nid");const char *key=luaL_checkstring(L,-1);
 lua_getglobal(L,"SET");if(!lua_istable(L,-1))return luaL_error(L,"Options SET not initialized");
 if(write){lua_pushvalue(L,4);lua_setfield(L,-2,key);return 0;}
 lua_getfield(L,-1,key);if(lua_isnil(L,-1)){lua_pop(L,1);lua_getfield(L,2,"def");}
 return 1;
}
static int option_get(lua_State *L){return option_access(L,0);}
static int option_set(lua_State *L){luaL_checkany(L,4);return option_access(L,1);}
static void index_options(lua_State *L){
 /* The APK's options array is also addressed by id in the desktop pause UI.
  * Preserve array order and descriptor aliases; convert numeric id aliases
  * to the same descriptors expected by our SET bridge. */
 lua_getglobal(L,"OPTIONS");
 if(lua_istable(L,-1))for(int i=1;i<=(int)lua_objlen(L,-1);i++){
  lua_rawgeti(L,-1,i);
  if(lua_istable(L,-1)){
   lua_getfield(L,-1,"id");
   if(lua_isstring(L,-1)){
    lua_pushvalue(L,-1);lua_rawget(L,-4);
    int missing=lua_isnil(L,-1)||lua_isnumber(L,-1);lua_pop(L,1);
    if(missing){lua_pushvalue(L,-2);lua_rawset(L,-4);}else lua_pop(L,1);
   }else lua_pop(L,1);
  }
  lua_pop(L,1);
 }
 lua_pop(L,1);
}
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
static double field_num(lua_State *L,int index,const char *name){
 if(!lua_istable(L,index))return -1;lua_getfield(L,index,name);double v=lua_isnumber(L,-1)?lua_tonumber(L,-1):-1;lua_pop(L,1);return v;
}
static int action_call(lua_State *L){
 int kind=lua_tointeger(L,lua_upvalueindex(2)),n=lua_gettop(L),report=1;
 if(kind==1){lua_getglobal(L,"hero");report=lua_rawequal(L,1,-1);lua_pop(L,1);}
 if(report){char detail[160];
  if(kind==2)snprintf(detail,sizeof(detail),"reason=%s",lua_type(L,1)==LUA_TSTRING?lua_tostring(L,1):"no_message");
  else snprintf(detail,sizeof(detail),"target=%.0f/%.0f just_check=%d",field_num(L,kind==1?2:1,"px"),field_num(L,kind==1?2:1,"py"),kind==0&&lua_toboolean(L,5));
  sg_diag_event(kind==0?"move_request":kind==1?"hero_goto":"game_rejected_action",detail);
 }
 lua_pushvalue(L,lua_upvalueindex(1));lua_insert(L,1);lua_call(L,n,LUA_MULTRET);return lua_gettop(L);
}
static void instrument_action(const char *name,int kind){
 lua_getglobal(state,name);if(!lua_isfunction(state,-1)){lua_pop(state,1);return;}
 lua_pushinteger(state,kind);lua_pushcclosure(state,action_call,2);lua_setglobal(state,name);
}
static int entity_call(lua_State *L){
 int draw=lua_toboolean(L,lua_upvalueindex(2));if(draw)sg_perf.entity_draws++;else sg_perf.entity_updates++;
 int args=lua_gettop(L);lua_pushvalue(L,lua_upvalueindex(1));lua_insert(L,1);lua_call(L,args,LUA_MULTRET);return lua_gettop(L);
}
static void instrument_entity(const char *name,int draw){
 lua_getglobal(state,name);if(!lua_isfunction(state,-1)){lua_pop(state,1);return;}
 lua_pushboolean(state,draw);lua_pushcclosure(state,entity_call,2);lua_setglobal(state,name);
}
static int frame_call(lua_State *L){lua_getglobal(L,"ents");if(lua_istable(L,-1))sg_perf.entities=lua_objlen(L,-1);lua_pop(L,1);sg_diag_phase(SG_DIAG_UPDATE);uint64_t start=sg_now_us();lib_callbacks(L,"_lib_update");lua_getglobal(L,"_update");if(lua_isfunction(L,-1))lua_call(L,0,0);else lua_pop(L,1);sg_perf.update_us=sg_now_us()-start;sg_diag_phase(SG_DIAG_DRAW);start=sg_now_us();lua_getglobal(L,"_draw");if(lua_isfunction(L,-1))lua_call(L,0,0);else lua_pop(L,1);lib_callbacks(L,"_lib_draw");sg_perf.draw_us=sg_now_us()-start;return 0;}
static int traceback(lua_State *L){
    char message[8192];const char *error=lua_tostring(L,1);
    size_t used=(size_t)snprintf(message,sizeof(message),"%s\nstack traceback:",error?error:"Non-string Lua error");
    for(int level=1;level<=32&&used<sizeof(message)-1;level++){
        lua_Debug ar;if(!lua_getstack(L,level,&ar))break;
        if(lua_getinfo(L,"nSl",&ar)){
            int n=snprintf(message+used,sizeof(message)-used,"\n  %s:%d in %s",ar.short_src,ar.currentline,ar.name?ar.name:ar.what);
            if(n<0)break;if((size_t)n>=sizeof(message)-used){used=sizeof(message)-1;break;}used+=(size_t)n;
        }
    }
    sg_diag_phase(SG_DIAG_ERROR);sg_diag_event("lua_error",error?error:"Non-string Lua error");
    logline(message);lua_pushstring(L,message);return 1;
}
static int protected_call(lua_State *L,int arguments,int results){
    int index=lua_gettop(L)-arguments;lua_pushcfunction(L,traceback);lua_insert(L,index);
    int status=lua_pcall(L,arguments,results,index);lua_remove(L,index);return status;
}
static int panic_log(lua_State *L){const char *error=lua_tostring(L,-1);logline(error?error:"Lua panic (non-string error)");sg_diag_event("lua_panic",error);return 0;}
static int graphics_start(lua_State *L){sg_graphics_init(L,rootpath);return 0;}
static int list(lua_State *L){char path[2048];resolve(luaL_optstring(L,1,""),path,sizeof(path));DIR *d=opendir(path);lua_newtable(L);if(d){struct dirent *e;int i=1;while((e=readdir(d))){if(e->d_name[0]=='.')continue;lua_pushstring(L,e->d_name);lua_rawseti(L,-2,i++);}closedir(d);}return 1;}
static int gameplay(lua_State *L){lua_getglobal(L,"require");lua_pushvalue(L,1);lua_call(L,1,1);return 1;}
static void reg(const char *name,lua_CFunction fn){lua_pushcfunction(state,fn);lua_setglobal(state,name);}
static void boolean(const char *name,int value){lua_pushboolean(state,value);lua_setglobal(state,name);}
static void alias(const char *name,const char *table,const char *field){lua_getglobal(state,table);lua_getfield(state,-1,field);lua_setglobal(state,name);lua_pop(state,1);}
int sg_boot(const char *root,FILE *log) {
    quit_requested=0;binding_count=0;raw_next=0;input_epoch=1;frame_time=0;buttons_now=buttons_prev=0;touched=touch_prev=0;memset(axes,0,sizeof(axes));memset(axes_prev,0,sizeof(axes_prev));memset(raw_bindings,0,sizeof(raw_bindings));
    output=log;failure[0]=0;instructions=0;snprintf(rootpath,sizeof(rootpath),"%s",root);
    state=luaL_newstate();if(!state){snprintf(failure,sizeof(failure),"Lua allocation failed");return 0;}
    lua_atpanic(state,panic_log);
    luaL_openlibs(state);
    lua_pushnil(state);lua_setglobal(state,"io");lua_pushnil(state);lua_setglobal(state,"os");
    lua_pushnil(state);lua_setglobal(state,"debug");lua_pushnil(state);lua_setglobal(state,"dofile");
    lua_pushnil(state);lua_setglobal(state,"loadfile");lua_pushnil(state);lua_setglobal(state,"package");
    lua_newtable(state);lua_setfield(state,LUA_REGISTRYINDEX,"sg.loaded");
    lua_pushvalue(state,LUA_GLOBALSINDEX);lua_setfield(state,LUA_REGISTRYINDEX,"sg.root_env");
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
    reg("bget",option_get);reg("bset",option_set);
    reg("dt",delta);reg("delta",delta);reg("t",timestamp);reg("sugar_step",sugarstep);reg("lerp",lerp);
    reg("round",roundvalue);
    reg("sgn",signvalue);
    reg("rm",remove_save);
    reg("url",muted_audio);reg("discord",muted_audio);reg("twitch",muted_audio);
    reg("fb_ev",muted_audio);reg("quit",request_quit);reg("ismobile",false_value);reg("live",false_value);reg("psn",false_value);reg("steam",false_value);
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
    if(protected_call(state,0,0)){snprintf(failure,sizeof(failure),"%s",lua_tostring(state,-1));logline(failure);return 0;}
    reg("controls",controls);reg("btn",btn);reg("btnp",btnp);reg("btnr",btnr);reg("btnv",btnv);reg("mouse",mouse);
    reg("rumble",muted_audio);
    sg_audio_init(state,rootpath,output);sg_audio_start(output);
    lua_sethook(state,limit,LUA_MASKCOUNT,10000);
    char path[2048];resolve("code.lua",path,sizeof(path));
    logline("ShotgunK2Vita Lua 5.1 bootstrap; unsupported APIs stop execution");
    sg_diag_event("load",path);
    int status=load_vita_script(state,path,"code.lua");if(!status)status=protected_call(state,0,0);
    if(!status){logline("code.lua loaded; invoking _init");lua_getglobal(state,"_init");status=protected_call(state,0,0);}
    if(!status){index_options(state);lua_pushcfunction(state,init_libs);status=protected_call(state,0,0);}
    if(status){snprintf(failure,sizeof(failure),"%s",lua_tostring(state,-1));logline(failure);return 0;}
    sg_ui_reset();reg("ach_unlock",local_notice);
    sg_graphics_fast_api(state);instrument_entity("dre",1);instrument_entity("upe",0);
    instrument_action("move_hero",0);instrument_action("goto_sq",1);instrument_action("fx_wrong",2);
    logline("_init completed; achievement popup=native; external services=disabled");return 1;
}
const char *sg_error(void){return failure;}
int sg_should_quit(void){return quit_requested;}
#ifndef SG_VITA
/* Host regression harness: absent from the Vita executable. */
int sg_host_eval(const char *script){
 if(!state||failure[0])return 0;
 instructions=0;
 int rc=luaL_loadstring(state,script);
 if(!rc)rc=protected_call(state,0,0);
 if(rc){snprintf(failure,sizeof(failure),"%s",lua_tostring(state,-1));logline(failure);lua_pop(state,1);return 0;}
 return 1;
}
#endif
void sg_report_state(void){if(!state)return;const char *names[]={"ingame","playing","game_mode","ctrl_mode","selectedOption","MOUSE","turn",NULL};for(int i=0;names[i];i++){lua_getglobal(state,names[i]);char line[256];const char *s=lua_tostring(state,-1);snprintf(line,sizeof(line),"state.%s=%s",names[i],s?s:lua_isboolean(state,-1)?(lua_toboolean(state,-1)?"true":"false"):lua_typename(state,lua_type(state,-1)));logline(line);lua_pop(state,1);}}
int sg_step(unsigned buttons,int lx,int ly,int rx,int ry,int touch,int x,int y){if(!state||failure[0])return 0;buttons_prev=buttons_now;buttons_now=buttons;memcpy(axes_prev,axes,sizeof(axes));touch_prev=touched;touched=touch;mx=x;my=y;axes[0]=lx;axes[1]=ly;axes[2]=rx;axes[3]=ry;frame_time+=1.0/60;sg_ui_tick();instructions=0;input_epoch++;sg_perf_begin();lua_pushcfunction(state,frame_call);if(protected_call(state,0,0)){const char *error=lua_tostring(state,-1);snprintf(failure,sizeof(failure),"%s",error?error:"Lua protected call failed");logline(failure);lua_pop(state,1);return 0;}return 1;}
unsigned sg_lua_memory_kb(void){return state?(unsigned)lua_gc(state,LUA_GCCOUNT,0):0;}
void sg_report_movement(void){
 if(!state||!output)return;lua_State *L=state;int top=lua_gettop(L);char mode[64];
 lua_getglobal(L,"ctrl_mode");snprintf(mode,sizeof(mode),"%s",lua_tostring(L,-1)?lua_tostring(L,-1):"none");lua_pop(L,1);
 lua_getglobal(L,"GAMEPAD");int pad=lua_toboolean(L,-1);lua_pop(L,1);lua_getglobal(L,"TOUCH");int touch=lua_toboolean(L,-1);lua_pop(L,1);
 lua_getglobal(L,"hero");int hero=lua_gettop(L);double x=field_num(L,hero,"x"),y=field_num(L,hero,"y");int tween=0;
 if(lua_istable(L,hero)){lua_getfield(L,hero,"twc");tween=lua_toboolean(L,-1);lua_pop(L,1);lua_getfield(L,hero,"sq");}else lua_pushnil(L);
 double age=field_num(L,hero,"t"),progress=field_num(L,hero,"twc"),duration=field_num(L,hero,"tws"),ex=field_num(L,hero,"ex"),ey=field_num(L,hero,"ey");
 int sq=lua_gettop(L);double px=field_num(L,sq,"px"),py=field_num(L,sq,"py"),sx=field_num(L,sq,"x"),sy=field_num(L,sq,"y");
 lua_getglobal(L,"move_cur");int cur=lua_gettop(L);
 fprintf(output,"control_state mode=%s gamepad=%d touch=%d hero_sq=%.0f/%.0f sprite=%.3f/%.3f square_pos=%.3f/%.3f tween=%d move_target=%.0f/%.0f hero_t=%.0f twc=%.3f tws=%.3f tween_end=%.3f/%.3f\n",mode,pad,touch,px,py,x,y,sx,sy,tween,field_num(L,cur,"px"),field_num(L,cur,"py"),age,progress,duration,ex,ey);lua_settop(L,top);
}
void sg_close(void){sg_audio_close();if(state)lua_close(state);state=NULL;sg_graphics_close();}
