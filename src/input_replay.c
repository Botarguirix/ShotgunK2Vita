#include "input_replay.h"
#include "runtime.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define REPLAY_FILE SG_REPLAY_DIR "/input.sgr"
#define REPLAY_SNAPSHOT SG_REPLAY_DIR "/snapshot"
#define REPLAY_SAVE SG_REPLAY_DIR "/save"
#define RUN_BYTES 10
/* L+R+SELECT held on the physical pad hands control back during a replay. */
#define REPLAY_ABORT (SG_L|SG_R|SG_SELECT)

static void put16(unsigned char *p,unsigned v){p[0]=(unsigned char)v;p[1]=(unsigned char)(v>>8);}
static void put32(unsigned char *p,uint32_t v){put16(p,v&0xffff);put16(p+2,v>>16);}
static unsigned get16(const unsigned char *p){return p[0]|(unsigned)p[1]<<8;}
static uint32_t get32(const unsigned char *p){return get16(p)|(uint32_t)get16(p+2)<<16;}

static int is_dir(const char *path){struct stat st;return !stat(path,&st)&&S_ISDIR(st.st_mode);}
/* Copies regular files and subdirectories; returns the number of files. */
static int copy_tree(const char *src,const char *dst){
 DIR *d=opendir(src);if(!d)return 0;mkdir(dst,0777);int files=0;struct dirent *e;
 while((e=readdir(d))){
  if(!strcmp(e->d_name,".")||!strcmp(e->d_name,".."))continue;
  char a[1024],b[1024];snprintf(a,sizeof(a),"%s/%s",src,e->d_name);snprintf(b,sizeof(b),"%s/%s",dst,e->d_name);
  if(is_dir(a)){files+=copy_tree(a,b);continue;}
  FILE *in=fopen(a,"rb");if(!in)continue;FILE *out=fopen(b,"wb");if(!out){fclose(in);continue;}
  char buf[8192];size_t n;while((n=fread(buf,1,sizeof(buf),in))>0)fwrite(buf,1,n,out);
  fclose(in);fclose(out);files++;
 }
 closedir(d);return files;
}
/* Only ever called on paths under SG_REPLAY_DIR. */
static void clear_tree(const char *path){
 DIR *d=opendir(path);if(!d)return;struct dirent *e;
 while((e=readdir(d))){
  if(!strcmp(e->d_name,".")||!strcmp(e->d_name,".."))continue;
  char a[1024];snprintf(a,sizeof(a),"%s/%s",path,e->d_name);
  if(is_dir(a)){clear_tree(a);rmdir(a);}else remove(a);
 }
 closedir(d);
}

static void write_run(SgReplay *r){
 if(!r->out||!r->ticks)return;
 unsigned char b[RUN_BYTES];put32(b,r->buttons);memcpy(b+4,r->axes,4);put16(b+8,r->ticks);
 fwrite(b,1,RUN_BYTES,r->out);r->runs++;r->ticks=0;
}
static int load_run(SgReplay *r){
 if(r->pos+RUN_BYTES>r->size)return 0;
 const unsigned char *b=r->data+r->pos;r->pos+=RUN_BYTES;
 r->buttons=get32(b);memcpy(r->axes,b+4,4);r->left=get16(b+8);
 return r->left>0;
}

int sg_replay_start(SgReplay *r,int mode,const char *root,FILE *log){
 memset(r,0,sizeof(*r));if(mode!=1&&mode!=2)return 0;
 char save[1024];snprintf(save,sizeof(save),"%s/save",root);mkdir(SG_REPLAY_DIR,0777);
 if(mode==1){
  clear_tree(REPLAY_SNAPSHOT);int files=copy_tree(save,REPLAY_SNAPSHOT);
  r->out=fopen(REPLAY_FILE,"wb");
  if(!r->out){if(log)fprintf(log,"replay mode=record error=cannot_open file=%s\n",REPLAY_FILE);return 0;}
  setvbuf(r->out,NULL,_IOFBF,16384);
  unsigned char h[8]={'S','G','R','1'};put16(h+4,1);put16(h+6,0);fwrite(h,1,8,r->out);
  r->mode=1;if(log)fprintf(log,"replay mode=record file=%s snapshot_files=%d abort=none\n",REPLAY_FILE,files);
  return 1;
 }
 FILE *f=fopen(REPLAY_FILE,"rb");
 if(!f){if(log)fprintf(log,"replay mode=replay error=missing file=%s\n",REPLAY_FILE);return 0;}
 fseek(f,0,SEEK_END);long n=ftell(f);rewind(f);
 if(n<8+RUN_BYTES||n>64*1024*1024){fclose(f);if(log)fprintf(log,"replay mode=replay error=bad_size bytes=%ld\n",n);return 0;}
 r->data=malloc((size_t)n);r->size=(size_t)n;
 if(!r->data||fread(r->data,1,r->size,f)!=r->size||memcmp(r->data,"SGR1",4)||get16(r->data+4)!=1){
  fclose(f);free(r->data);r->data=NULL;if(log)fprintf(log,"replay mode=replay error=invalid_file\n");return 0;
 }
 fclose(f);r->pos=8;
 if(!is_dir(REPLAY_SNAPSHOT)){free(r->data);r->data=NULL;if(log)fprintf(log,"replay mode=replay error=missing_snapshot dir=%s\n",REPLAY_SNAPSHOT);return 0;}
 clear_tree(REPLAY_SAVE);int files=copy_tree(REPLAY_SNAPSHOT,REPLAY_SAVE);
 sg_set_save_redirect(REPLAY_SAVE);
 unsigned total=0;for(size_t p=8;p+RUN_BYTES<=r->size;p+=RUN_BYTES)total+=get16(r->data+p+8);
 r->mode=2;load_run(r);
 if(log)fprintf(log,"replay mode=replay file=%s runs=%u frames=%u snapshot_files=%d saves=%s abort=L+R+SELECT\n",REPLAY_FILE,(unsigned)((r->size-8)/RUN_BYTES),total,files,REPLAY_SAVE);
 return 1;
}

void sg_replay_frame(SgReplay *r,unsigned *buttons,unsigned char *lx,unsigned char *ly,unsigned char *rx,unsigned char *ry,unsigned physical,FILE *log){
 if(r->mode==1){
  unsigned char axes[4]={*lx,*ly,*rx,*ry};
  if(r->ticks&&(r->buttons!=*buttons||memcmp(r->axes,axes,4)||r->ticks==0xffff))write_run(r);
  if(!r->ticks){r->buttons=*buttons;memcpy(r->axes,axes,4);}
  r->ticks++;r->frames++;
  if(!(r->frames%60))fflush(r->out);
  return;
 }
 if(r->mode!=2||r->done)return;
 if((physical&REPLAY_ABORT)==REPLAY_ABORT){
  r->aborted=1;r->done=1;if(log)fprintf(log,"replay_aborted frame=%u\n",r->frames);return;
 }
 if(!r->left&&!load_run(r)){r->done=1;if(log)fprintf(log,"replay_done frames=%u\n",r->frames);return;}
 *buttons=r->buttons;*lx=r->axes[0];*ly=r->axes[1];*rx=r->axes[2];*ry=r->axes[3];
 r->left--;r->frames++;
}

void sg_replay_stop(SgReplay *r,FILE *log){
 if(r->mode==1){write_run(r);if(r->out)fclose(r->out);if(log)fprintf(log,"replay_recorded frames=%u runs=%u\n",r->frames,r->runs);}
 if(r->mode==2&&!r->done&&log)fprintf(log,"replay_stopped frame=%u\n",r->frames);
 free(r->data);memset(r,0,sizeof(*r));
}
