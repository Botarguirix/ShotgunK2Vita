#include "audio.h"
#include "lauxlib.h"
#include "lualib.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int script(lua_State *L,const char *s){if(luaL_dostring(L,s)){fprintf(stderr,"%s\n",lua_tostring(L,-1));return 0;}return 1;}
static void le32(FILE *f,uint32_t n){unsigned char p[4]={n,n>>8,n>>16,n>>24};fwrite(p,1,4,f);}
int main(int argc,char **argv){
 if(argc<3)return 2;FILE *f=fopen(argv[2],"wb");if(!f)return 2;
 lua_State *L=luaL_newstate();luaL_openlibs(L);sg_audio_init(L,argv[1],stdout);
 int ok=script(L,"newsfx('assets/sfx/shoot.wav','shot');newmus('assets/music/title_A.mp3','A');newmus('assets/music/title_B.mp3','B');music('A',0,false,0);nxtmusic('B',-1,true)");
 const unsigned saved_frames=44100*8;fwrite("RIFF",1,4,f);le32(f,36+saved_frames*4);fwrite("WAVEfmt ",1,8,f);le32(f,16);
 unsigned char fmt[4]={1,0,2,0};fwrite(fmt,1,4,f);le32(f,44100);le32(f,44100*4);unsigned char align[4]={4,0,16,0};fwrite(align,1,4,f);fwrite("data",1,4,f);le32(f,saved_frames*4);
 int16_t block[2048*2];uint64_t nonzero=0,total=0;unsigned remaining=saved_frames;
 /* Run 300 simulated seconds to check A->B and looping, but save only 8. */
 for(unsigned pos=0;ok&&pos<44100u*300;pos+=2048){
  if(pos%44100<2048&&pos<44100u*5)ok=script(L,"sfx('shot',-1,.25,0,1)");
  unsigned frames=2048;if(44100u*300-pos<frames)frames=44100u*300-pos;
  sg_audio_mix(block,frames);
  for(unsigned i=0;i<frames*2;i++){nonzero+=block[i]!=0;total++;}
  unsigned save=remaining<frames?remaining:frames;if(save){fwrite(block,4,save,f);remaining-=save;}
 }
 SgAudioStats stats=sg_audio_stats();
 printf("Audio: starts=%u loops=%u failures=%u fx=%u drops=%u nonzero=%llu/%llu\n",stats.music_starts,stats.music_loops,stats.music_failures,stats.fx_starts,stats.queue_drops,(unsigned long long)nonzero,(unsigned long long)total);
 ok=ok&&stats.music_starts>=2&&stats.music_loops>=1&&!stats.music_failures&&!stats.queue_drops&&stats.fx_starts>=5&&nonzero>0&&!remaining;
 /* Mute and clear FX must produce silence without retaining stale blocks. */
 ok=ok&&script(L,"music();sfx(nil,-1);musvol(true);sfxvol(true)");sg_audio_mix(block,2048);
 for(unsigned i=0;i<4096;i++)if(block[i])ok=0;
 fclose(f);sg_audio_close();lua_close(L);return ok?0:1;
}
