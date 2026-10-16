#include "audio.h"
#include "diagnostics.h"
#include "lauxlib.h"
#include "perf.h"
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <math.h>
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#ifdef SG_VITA
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/cpu.h>
#include <malloc.h>
#endif

#define RATE 44100
#define ASSETS 256
#define CHANNELS 12
#define QUEUE 128
typedef struct {char key[96],path[1536];int16_t *pcm;uint32_t frames,rate;int channels,music;float volume;} Asset;
typedef struct {int asset,channel,type,loop;float volume,pan,pitch,fade;} Command;
typedef struct {int asset;uint64_t position;uint32_t step;float volume,pan;} Voice;
enum {PLAY_FX,PLAY_MUSIC,NEXT_MUSIC,FX_GAIN,MUSIC_GAIN,DELETE_FX};
static Asset assets[ASSETS];static int asset_count;
static Command queue[QUEUE];static _Atomic unsigned head,tail,dropped;
static Voice voices[CHANNELS];static float sfx_gain=1,music_gain=1,query_sfx=1,query_music=1;
static drmp3 decoder;static int decoder_open,music_id=-1,music_loop,next_id=-1,next_loop;
static int16_t music_pcm[4096*2];static uint64_t music_phase;static unsigned music_frames;
static float fade_gain=1,fade_target=1,fade_step;static int stop_after_fade;
static char data_root[1024];static FILE *audio_log,*setup_log;
static _Atomic int quit;
static _Atomic unsigned music_starts,music_loops,music_failures,fx_starts;
#ifdef SG_VITA
static SceUID audio_thread=-1;
#endif

static uint16_t u16(const unsigned char *p){return p[0]|(uint16_t)p[1]<<8;}
static uint32_t u32(const unsigned char *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static float clamp(float v,float lo,float hi){return v<lo?lo:v>hi?hi:v;}
static void push(Command c){
 unsigned h=atomic_load_explicit(&head,memory_order_relaxed),t=atomic_load_explicit(&tail,memory_order_acquire);
 if(h-t>=QUEUE){atomic_fetch_add(&dropped,1);return;}queue[h%QUEUE]=c;atomic_store_explicit(&head,h+1,memory_order_release);
}
static int find_asset(const char *key,int music){for(int i=0;i<asset_count;i++)if(assets[i].music==music&&!strcmp(assets[i].key,key))return i;return -1;}

static int load_wave(Asset *a){
 FILE *f=fopen(a->path,"rb");if(!f)return 0;
 unsigned char h[16];int good=0;long data=0;uint32_t bytes=0;int format=0,bits=0,align=0;
 if(fread(h,1,12,f)!=12||memcmp(h,"RIFF",4)||memcmp(h+8,"WAVE",4))goto done;
 while(fread(h,1,8,f)==8){uint32_t n=u32(h+4);long start=ftell(f);if(n>32*1024*1024)goto done;
  if(!memcmp(h,"fmt ",4)){if(n<16||fread(h,1,16,f)!=16)goto done;format=u16(h);a->channels=u16(h+2);a->rate=u32(h+4);align=u16(h+12);bits=u16(h+14);}
  else if(!memcmp(h,"data",4)){data=start;bytes=n;}
  if(fseek(f,start+n+(n&1),SEEK_SET))goto done;
 }
 if(format!=1||bits!=16||a->channels<1||a->channels>2||align!=2*a->channels||a->rate<8000||a->rate>96000||!data||!bytes||bytes>8*1024*1024||bytes%align)goto done;
 a->pcm=malloc(bytes);if(!a->pcm)goto done;
 if(fseek(f,data,SEEK_SET)||fread(a->pcm,1,bytes,f)!=bytes){free(a->pcm);a->pcm=NULL;goto done;}
 a->frames=bytes/align;good=1;
 done:fclose(f);return good;
}
static int register_asset(lua_State *L,int music){
 const char *name=luaL_checkstring(L,1),*key=luaL_optstring(L,2,name);
 if(asset_count==ASSETS)return luaL_error(L,"Audio asset limit");
 if(find_asset(key,music)>=0)return luaL_error(L,"Audio asset already registered: %s",key);
 while(!strncmp(name,"../",3))name+=3;
 if(name[0]=='/'||strchr(name,':')||strstr(name,".."))return luaL_error(L,"Invalid audio path");
 Asset *a=&assets[asset_count];snprintf(a->path,sizeof(a->path),"%s/%s",data_root,name);snprintf(a->key,sizeof(a->key),"%s",key);
 sg_diag_event(music?"music_register":"sfx_register",a->path);
 a->music=music;a->volume=clamp((float)luaL_optnumber(L,3,1),0,1);
 FILE *f=fopen(a->path,"rb");if(!f)return luaL_error(L,"Missing audio resource: %s",a->path);fclose(f);
 if(!music&&!load_wave(a)&&setup_log)fprintf(setup_log,"audio_fx_unsupported=%s (PCM16 WAV required)\n",name);
 asset_count++;lua_pushstring(L,key);return 1;
}
static int newfx(lua_State *L){return register_asset(L,0);}
static int newmusic(lua_State *L){return register_asset(L,1);}
static int playfx(lua_State *L){
 int id=lua_isnil(L,1)?-1:find_asset(luaL_checkstring(L,1),0);
 if(id<0&&!lua_isnil(L,1))return luaL_error(L,"Unknown sound: %s",lua_tostring(L,1));
 Command c={.type=id<0?DELETE_FX:PLAY_FX,.asset=id,.channel=luaL_optint(L,2,-1),.volume=clamp(luaL_optnumber(L,3,1),0,1),.pan=clamp(luaL_optnumber(L,4,0),-1,1),.pitch=clamp(luaL_optnumber(L,5,1),.125f,4)};
 push(c);lua_pushinteger(L,c.channel);return 1;
}
static int playmusic(lua_State *L){
 int id=lua_isnoneornil(L,1)?-1:find_asset(luaL_checkstring(L,1),1);
 if(id<0&&!lua_isnoneornil(L,1))return luaL_error(L,"Unknown music: %s",lua_tostring(L,1));
 push((Command){.type=PLAY_MUSIC,.asset=id,.loop=lua_toboolean(L,3),.fade=fmax(0,luaL_optnumber(L,4,0))});return 0;
}
static int nextmusic(lua_State *L){int id=find_asset(luaL_checkstring(L,1),1);if(id<0)return luaL_error(L,"Unknown queued music");push((Command){.type=NEXT_MUSIC,.asset=id,.loop=lua_toboolean(L,3)});return 0;}
static int volume(lua_State *L,int music){
 float *query=music?&query_music:&query_sfx;
 if(lua_gettop(L)){if(lua_gettop(L)>1&&!lua_isnil(L,2))return luaL_error(L,"Per-asset audio volume unsupported");
  *query=lua_isboolean(L,1)?(lua_toboolean(L,1)?0:1):clamp(luaL_checknumber(L,1),0,1);
  push((Command){.type=music?MUSIC_GAIN:FX_GAIN,.volume=*query});
 }lua_pushnumber(L,*query);return 1;
}
static int fxvol(lua_State *L){return volume(L,0);}
static int musvol(lua_State *L){return volume(L,1);}
static int delfx(lua_State *L){int id=find_asset(luaL_checkstring(L,1),0);push((Command){.type=DELETE_FX,.asset=id,.channel=-1});return 0;}

static void close_music(void){if(decoder_open)drmp3_uninit(&decoder);decoder_open=0;music_id=-1;music_frames=0;music_phase=0;}
static int open_music(int id,int loop){
 close_music();if(id<0)return 1;
 sg_diag_event("mp3_open",assets[id].path);
 if(!drmp3_init_file(&decoder,assets[id].path,NULL)){atomic_fetch_add(&music_failures,1);if(audio_log)fprintf(audio_log,"audio_mp3_open_failed=%s\n",assets[id].path);return 0;}
 decoder_open=1;if(decoder.channels<1||decoder.channels>2||decoder.sampleRate<8000||decoder.sampleRate>96000){close_music();return 0;}
 music_id=id;music_loop=loop;
 atomic_fetch_add(&music_starts,1);
 if(audio_log)fprintf(audio_log,"audio_music=%s hz=%u channels=%u\n",assets[id].key,decoder.sampleRate,decoder.channels);
 return 1;
}
static void commands(void){
 unsigned t=atomic_load_explicit(&tail,memory_order_relaxed),h=atomic_load_explicit(&head,memory_order_acquire);
 while(t<h){Command c=queue[t++%QUEUE];
  if(c.type==PLAY_FX){int channel=c.channel;
   if(channel<0){for(channel=0;channel<CHANNELS;channel++)if(voices[channel].asset<0)break;if(channel==CHANNELS)channel=CHANNELS-1;}
   if(channel<CHANNELS&&assets[c.asset].pcm){Asset *a=&assets[c.asset];voices[channel]=(Voice){c.asset,0,(uint32_t)(a->rate*65536.0*c.pitch/RATE),c.volume,c.pan};atomic_fetch_add(&fx_starts,1);}
  }else if(c.type==DELETE_FX){for(int i=0;i<CHANNELS;i++)if((c.channel<0||c.channel==i)&&(c.asset<0||voices[i].asset==c.asset))voices[i].asset=-1;
  }else if(c.type==PLAY_MUSIC){next_id=-1;
   if(c.asset<0){if(c.fade>0&&decoder_open){fade_target=0;fade_step=-fade_gain/(c.fade*RATE);stop_after_fade=1;}else close_music();}
   else {open_music(c.asset,c.loop);fade_gain=c.fade>0?0:1;fade_target=1;fade_step=c.fade>0?1/(c.fade*RATE):0;stop_after_fade=0;}
  }else if(c.type==NEXT_MUSIC){next_id=c.asset;next_loop=c.loop;}
  else if(c.type==FX_GAIN)sfx_gain=c.volume;
  else if(c.type==MUSIC_GAIN)music_gain=c.volume;
 }atomic_store_explicit(&tail,t,memory_order_release);
}

static int music_sample(int *left,int *right){
 if(!decoder_open)return 0;
 unsigned pos=(unsigned)(music_phase>>16);
 if(pos>=music_frames){
  if(music_frames)music_phase-=(uint64_t)music_frames<<16;
  music_frames=(unsigned)drmp3_read_pcm_frames_s16(&decoder,4096,music_pcm);
  if(!music_frames){
   if(next_id>=0){int id=next_id;next_id=-1;open_music(id,next_loop);}
   else if(music_loop){drmp3_seek_to_pcm_frame(&decoder,0);music_phase=0;atomic_fetch_add(&music_loops,1);}
   else {close_music();return 0;}
   if(!decoder_open)return 0;
   music_frames=(unsigned)drmp3_read_pcm_frames_s16(&decoder,4096,music_pcm);if(!music_frames){close_music();return 0;}
  }pos=(unsigned)(music_phase>>16);if(pos>=music_frames)pos=music_frames-1;
 }
 *left=music_pcm[pos*decoder.channels];*right=music_pcm[pos*decoder.channels+(decoder.channels==2)];
 music_phase+=((uint64_t)decoder.sampleRate<<16)/RATE;return 1;
}
void sg_audio_mix(int16_t *out,unsigned frames){
 commands();
 for(unsigned n=0;n<frames;n++){
  int left=0,right=0,l=0,r=0;
  if(music_sample(&l,&r)){float gain=music_gain*assets[music_id].volume*fade_gain;left=(int)(l*gain);right=(int)(r*gain);}
  if(fade_step){fade_gain+=fade_step;if((fade_step>0&&fade_gain>=fade_target)||(fade_step<0&&fade_gain<=fade_target)){fade_gain=fade_target;fade_step=0;if(stop_after_fade)close_music();}}
  for(int i=0;i<CHANNELS;i++){
   Voice *v=&voices[i];if(v->asset<0)continue;Asset *a=&assets[v->asset];uint32_t pos=v->position>>16;
   if(pos>=a->frames){v->asset=-1;continue;}
   unsigned next=pos+1<a->frames?pos+1:pos,frac=v->position&65535;
   int l0=a->pcm[pos*a->channels],l1=a->pcm[next*a->channels],r0=a->pcm[pos*a->channels+(a->channels==2)],r1=a->pcm[next*a->channels+(a->channels==2)];
   float gain=v->volume*a->volume*sfx_gain;
   left+=(int)((l0+(int64_t)(l1-l0)*frac/65536)*gain*(v->pan>0?1-v->pan:1));
   right+=(int)((r0+(int64_t)(r1-r0)*frac/65536)*gain*(v->pan<0?1+v->pan:1));v->position+=v->step;
  }
  out[2*n]=left<-32768?-32768:left>32767?32767:left;out[2*n+1]=right<-32768?-32768:right>32767?32767:right;
 }
}

#ifdef SG_VITA
static int audio_entry(SceSize size,void *arg){
 (void)size;(void)arg;const unsigned frames=2048;int16_t *buffers=memalign(64,2*frames*2*sizeof(int16_t));
 if(!buffers)return -1;
 int port=sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM,frames,RATE,SCE_AUDIO_OUT_MODE_STEREO);
 if(audio_log){fprintf(audio_log,"audio_port_rc=%d rate=%d block=%u\n",port,RATE,frames);fflush(audio_log);}
 if(port<0){free(buffers);return port;}
 int volume[2]={16384,16384};int rc=sceAudioOutSetVolume(port,SCE_AUDIO_VOLUME_FLAG_L_CH|SCE_AUDIO_VOLUME_FLAG_R_CH,volume);
 unsigned active=0;uint64_t blocks=0,max_fill=0,late=0;
 while(rc>=0&&!atomic_load_explicit(&quit,memory_order_acquire)){
  int16_t *p=buffers+active*frames*2;uint64_t start=sg_now_us();sg_audio_mix(p,frames);uint64_t fill=sg_now_us()-start;
  if(fill>max_fill)max_fill=fill;if(fill>(uint64_t)frames*1000000/RATE)late++;
  rc=sceAudioOutOutput(port,p);active^=1;blocks++;sg_diag_audio((unsigned)blocks,rc);
  if(audio_log&&blocks%120==0){fprintf(audio_log,"audio_blocks=%llu fill_max_us=%llu late_blocks=%llu queue_drops=%u rc=%d\n",(unsigned long long)blocks,(unsigned long long)max_fill,(unsigned long long)late,atomic_load(&dropped),rc);fflush(audio_log);}
 }
 sceAudioOutOutput(port,NULL);sceAudioOutReleasePort(port);free(buffers);
 if(audio_log){fprintf(audio_log,"audio_stop_rc=%d blocks=%llu\n",rc,(unsigned long long)blocks);fflush(audio_log);}return rc;
}
#endif

int sg_audio_start(FILE *log){
 (void)log;
#ifdef SG_VITA
 audio_log=sg_diag_open("audio.log");
 if(!audio_log){char path[1536];snprintf(path,sizeof(path),"%s/audio.log",data_root);audio_log=fopen(path,"a");}
 if(audio_log)setvbuf(audio_log,NULL,_IOLBF,4096);
 audio_thread=sceKernelCreateThread("SG native audio",audio_entry,0x100000E0,0x20000,0,SCE_KERNEL_CPU_MASK_USER_2,NULL);
 int rc=audio_thread<0?audio_thread:sceKernelStartThread(audio_thread,0,NULL);
 if(rc>=0)sg_diag_thread(SG_THREAD_AUDIO,audio_thread);
 if(log){fprintf(log,"audio_thread=%d start_rc=%d log=audio.log\n",audio_thread,rc);fflush(log);}
 if(rc<0&&audio_thread>=0){sceKernelDeleteThread(audio_thread);audio_thread=-1;}return rc>=0;
#else
 return 1; /* Host tests explicitly drive sg_audio_mix; no device output. */
#endif
}
static void reg(lua_State *L,const char *name,lua_CFunction fn){lua_pushcfunction(L,fn);lua_setglobal(L,name);}
SgAudioStats sg_audio_stats(void){return (SgAudioStats){atomic_load(&music_starts),atomic_load(&music_loops),atomic_load(&music_failures),atomic_load(&fx_starts),atomic_load(&dropped)};}
void sg_audio_init(lua_State *L,const char *root,FILE *log){
 snprintf(data_root,sizeof(data_root),"%s",root);setup_log=log;
 for(int i=0;i<CHANNELS;i++)voices[i].asset=-1;
 reg(L,"newsfx",newfx);reg(L,"newmus",newmusic);reg(L,"sfx",playfx);reg(L,"music",playmusic);reg(L,"mus",playmusic);
 reg(L,"musvol",musvol);reg(L,"sfxvol",fxvol);reg(L,"nxtmusic",nextmusic);reg(L,"delsfx",delfx);
}
void sg_audio_close(void){
 atomic_store_explicit(&quit,1,memory_order_release);
#ifdef SG_VITA
 if(audio_thread>=0){sceKernelWaitThreadEnd(audio_thread,NULL,NULL);sg_diag_thread(SG_THREAD_AUDIO,-1);sceKernelDeleteThread(audio_thread);audio_thread=-1;}
#endif
 close_music();if(audio_log)fclose(audio_log);audio_log=NULL;setup_log=NULL;for(int i=0;i<asset_count;i++)free(assets[i].pcm);memset(assets,0,sizeof(assets));asset_count=0;
 atomic_store(&head,0);atomic_store(&tail,0);atomic_store(&dropped,0);atomic_store(&quit,0);
 sfx_gain=music_gain=query_sfx=query_music=1;next_id=-1;fade_step=0;fade_gain=1;stop_after_fade=0;
 atomic_store(&music_starts,0);atomic_store(&music_loops,0);atomic_store(&music_failures,0);atomic_store(&fx_starts,0);
}
