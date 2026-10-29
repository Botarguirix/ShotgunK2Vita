#include "steam_install.h"
#include "miniz.h"
#include "steam_compat_lua.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <dirent.h>

/* Same layout as tools/extract_sgr.py and tools/prepare_steam_data.py. */
#define MARKER "steam-installed.txt"
#define MAX_FILES 1024
#define MAX_ENTRY (64u*1024*1024)
#define CHUNK (256*1024)

typedef struct {uint32_t x,y,z,w,v;} Xor;
/* One xorshift step; the key byte mixes the outgoing z with the new value. */
static uint32_t xor_step(Xor *s){
 uint32_t t=s->x^(s->x>>7),nv=s->v^(s->v<<6)^t^(t<<13),key=(2*s->z+1)*nv;
 s->x=s->y;s->y=s->z;s->z=s->w;s->w=s->v;s->v=nv;return key;
}
static void xor_init(Xor *s,const unsigned char *trailer){
 uint32_t seed=((uint32_t)trailer[0]<<24|(uint32_t)trailer[1]<<16|(uint32_t)trailer[2]<<8|trailer[3])^0x550F0F55u;
 *s=(Xor){seed,(seed>>1)+0x159A55E5u,(seed>>3)+0x1F123BB5u,(seed>>5)+0x05491333u,(seed>>7)+0x34DAD465u};
 unsigned skip=xor_step(s)&15;while(skip--)xor_step(s);
}
/* Undo the even-byte layer of a stored entry in place; returns its length. */
static uint32_t untransform(unsigned char *p,uint32_t n){
 Xor s;xor_init(&s,p+n-4);n-=4;for(uint32_t i=0;i<n;i+=2)p[i]^=xor_step(&s)&255;return n;
}

typedef struct {
 const char *root;FILE *log;
 char *names[MAX_FILES];int count,pngs;
 /* Container push parser: fill `need` bytes into buf, then act on state. */
 int state,meta;unsigned char head[1024],*buf;uint32_t need,got,name_len,size;
 char name[1024];int flag,failed;
} Install;
enum {NAME_LEN,NAME,FLAG,SIZE,DATA};

static void mkdirs(char *path){for(char *p=path+1;*p;p++)if(*p=='/'){*p=0;mkdir(path,0777);*p='/';}}
static int write_file(const char *root,const char *name,const void *data,size_t n){
 char path[1600];snprintf(path,sizeof(path),"%s/%s",root,name);mkdirs(path);
 FILE *f=fopen(path,"wb");if(!f)return 0;int ok=fwrite(data,1,n,f)==n;return fclose(f)==0&&ok;
}
/* SUGAR exports some sources as ../../libs/x.lua; the game loads libs/x.lua. */
static const char *safe_name(const char *name){
 while(!strncmp(name,"../",3))name+=3;
 if(!*name||*name=='/'||strstr(name,"..")||strchr(name,':')||strchr(name,'\\'))return NULL;
 return name;
}
static int ends_with(const char *s,const char *tail){size_t a=strlen(s),b=strlen(tail);return a>=b&&!strcasecmp(s+a-b,tail);}

static void entry(Install *in){
 if(in->flag)return; /* flag 1 entries are wildcard lists, not files */
 const char *name=safe_name(in->name);
 if(!name){if(in->log)fprintf(in->log,"steam_install_skip_unsafe=%s\n",in->name);return;}
 for(int i=0;i<in->count;i++)if(!strcasecmp(in->names[i],name))return;
 if(in->count==MAX_FILES){in->failed=1;return;}
 uint32_t n=untransform(in->buf,in->size);
 if(!write_file(in->root,name,in->buf,n)){if(in->log)fprintf(in->log,"steam_install_write_failed=%s\n",name);in->failed=1;return;}
 in->names[in->count++]=strdup(name);if(ends_with(name,".png"))in->pngs++;
}
static void expect(Install *in,int state,uint32_t n,unsigned char *buf){in->state=state;in->need=n;in->got=0;in->buf=buf;}
static void feed(Install *in,const unsigned char *p,size_t n){
 while(n&&!in->failed){
  uint32_t take=in->need-in->got;if(take>n)take=(uint32_t)n;
  memcpy(in->buf+in->got,p,take);in->got+=take;p+=take;n-=take;
  if(in->got<in->need)continue;
  if(in->state==NAME_LEN){in->name_len=in->head[0]<<8|in->head[1];if(in->name_len>=sizeof(in->name)){in->failed=1;break;}
   if(in->name_len)expect(in,NAME,in->name_len,(unsigned char *)in->name);else{in->name[0]=0;goto named;}}
  else if(in->state==NAME){in->name[in->name_len]=0;named:
   if(in->meta<3){in->meta++;expect(in,NAME_LEN,2,in->head);}else expect(in,FLAG,1,in->head);}
  else if(in->state==FLAG){in->flag=in->head[0];if(in->flag>1){in->failed=1;break;}expect(in,SIZE,4,in->head);}
  else if(in->state==SIZE){in->size=(uint32_t)in->head[0]<<24|in->head[1]<<16|in->head[2]<<8|in->head[3];
   if(in->size<4||in->size>MAX_ENTRY||!(in->buf=malloc(in->size))){in->failed=1;break;}expect(in,DATA,in->size,in->buf);}
  else {entry(in);free(in->buf);expect(in,NAME_LEN,2,in->head);}
 }
}

/* Decrypt and inflate data.sgr while streaming into the container parser. */
static int unpack(Install *in,FILE *f,long total,SgInstallProgress progress,void *user){
 unsigned char trailer[4];if(total<10||fseek(f,total-4,SEEK_SET)||fread(trailer,1,4,f)!=4||fseek(f,0,SEEK_SET))return 0;
 Xor s;xor_init(&s,trailer);
 unsigned char *chunk=malloc(CHUNK),*out=malloc(CHUNK);mz_stream z;memset(&z,0,sizeof(z));
 if(!chunk||!out||mz_inflateInit(&z)!=MZ_OK){free(chunk);free(out);return 0;}
 long pos=0,body=total-4;unsigned char size_be[4];int header=0,status=MZ_OK;uint32_t expected=0;unsigned last=101;
 expect(in,NAME_LEN,2,in->head);
 while(pos<body&&status==MZ_OK&&!in->failed){
  size_t n=(size_t)(body-pos<CHUNK?body-pos:CHUNK);if(fread(chunk,1,n,f)!=n)break;
  for(size_t i=(pos&1);i<n;i+=2)chunk[i]^=xor_step(&s)&255;
  unsigned char *p=chunk;size_t left=n;pos+=(long)n;
  while(header<4&&left){size_be[header++]=*p++;left--;if(header==4)expected=(uint32_t)size_be[0]<<24|size_be[1]<<16|size_be[2]<<8|size_be[3];}
  z.next_in=p;z.avail_in=(unsigned)left;
  while((z.avail_in||status==MZ_OK)&&status==MZ_OK&&!in->failed){
   z.next_out=out;z.avail_out=CHUNK;status=mz_inflate(&z,MZ_NO_FLUSH);
   if(status==MZ_BUF_ERROR)status=MZ_OK;
   feed(in,out,CHUNK-z.avail_out);
   if(!z.avail_in&&z.avail_out)break;
  }
  unsigned pct=(unsigned)(pos*90/body);if(progress&&pct!=last){progress(pct,user);last=pct;}
 }
 int ok=status==MZ_STREAM_END&&!in->failed&&z.total_out==expected&&in->state==NAME_LEN&&in->got==0;
 if(in->log)fprintf(in->log,"steam_install_container status=%d inflated=%lu expected=%u files=%d ok=%d\n",status,(unsigned long)z.total_out,expected,in->count,ok);
 mz_inflateEnd(&z);free(chunk);free(out);if(in->state==DATA&&in->buf)free(in->buf);return ok;
}

/* Minimal PNG reader for the shipped and Workshop formats: indexed (1-8 bit),
 * RGB or RGBA (8 bit), not interlaced. Rows are unfiltered in place in the
 * inflated buffer (row y at pixels+y*pitch+1) so a 1888x6544 RGBA mod sheet
 * needs ~49 MB instead of twice that on the Vita's 96 MB heap. */
typedef struct {uint32_t w,h;int type,depth,bpp,colors;size_t pitch;unsigned char palette[768],*pixels;} Png;
static const unsigned char *png_row(const Png *png,uint32_t y){return png->pixels+y*png->pitch+1;}
static int png_index(const unsigned char *row,int depth,uint32_t x){
 if(depth==8)return row[x];
 size_t bit=(size_t)x*depth;return row[bit/8]>>(8-depth-bit%8)&((1<<depth)-1);
}
static uint32_t be32(const unsigned char *p){return (uint32_t)p[0]<<24|p[1]<<16|p[2]<<8|p[3];}
static int paeth(int a,int b,int c){int p=a+b-c,pa=abs(p-a),pb=abs(p-b),pc=abs(p-c);return pa<=pb&&pa<=pc?a:pb<=pc?b:c;}
static int png_read(const char *path,Png *png){
 memset(png,0,sizeof(*png));FILE *f=fopen(path,"rb");if(!f)return 0;
 fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);unsigned char *d=n>8?malloc(n):NULL;
 int ok=d&&fread(d,1,n,f)==(size_t)n&&!memcmp(d,"\x89PNG\r\n\x1a\n",8);fclose(f);if(!ok){free(d);return 0;}
 unsigned char *idat=malloc(n);size_t idat_n=0;ok=0;
 for(long p=8;idat&&p+12<=n;){uint32_t len=be32(d+p);const unsigned char *t=d+p+4,*c=d+p+8;if(len>(uint32_t)(n-p-12))break;
  if(!memcmp(t,"IHDR",4)&&len>=13){png->w=be32(c);png->h=be32(c+4);png->depth=c[8];png->type=c[9];
   ok=c[12]==0&&(png->type==3?c[8]==1||c[8]==2||c[8]==4||c[8]==8:c[8]==8&&(png->type==6||png->type==2));}
  else if(!memcmp(t,"PLTE",4)&&len<=768){memcpy(png->palette,c,len);png->colors=(int)len/3;}  else if(!memcmp(t,"IDAT",4)){memcpy(idat+idat_n,c,len);idat_n+=len;}
  else if(!memcmp(t,"IEND",4))break;
  p+=12+len;}
 free(d);
 if(!ok||!png->w||!png->h||png->w>8192||png->h>8192){free(idat);return 0;}
 png->bpp=png->type==3?1:png->type==2?3:4;
 size_t stride=png->type==3?((size_t)png->w*png->depth+7)/8:png->w*png->bpp;png->pitch=stride+1;
 mz_ulong raw_n=(mz_ulong)(png->pitch*png->h);unsigned char *raw=png->pixels=malloc(raw_n);
 ok=raw&&mz_uncompress(raw,&raw_n,idat,(mz_ulong)idat_n)==MZ_OK&&raw_n==png->pitch*png->h;free(idat);
 for(uint32_t y=0;ok&&y<png->h;y++){
  unsigned char *row=raw+y*png->pitch+1;int filter=row[-1];const unsigned char *up=y?row-png->pitch:NULL;
  for(size_t x=0;x<stride;x++){int a=x>=(size_t)png->bpp?row[x-png->bpp]:0,b=up?up[x]:0,c=up&&x>=(size_t)png->bpp?up[x-png->bpp]:0,v=row[x];
   if(filter==1)v+=a;else if(filter==2)v+=b;else if(filter==3)v+=(a+b)/2;else if(filter==4)v+=paeth(a,b,c);else if(filter)ok=0;
   row[x]=(unsigned char)v;}
 }
 if(!ok){free(png->pixels);png->pixels=NULL;}return ok;
}
static int nearest(const Png *ref,const unsigned char *c){
 int best=0,dist=1<<30;
 for(int k=0;k<ref->colors;k++){const unsigned char *p=ref->palette+3*k;int dr=p[0]-c[0],dg=p[1]-c[1],db=p[2]-c[2],e=dr*dr+dg*dg+db*db;if(e<dist){dist=e;best=k;}}
 return best;
}
/* Same .sgimg as prepare_images(): gfx.png's palette, alpha<128 -> index 0,
 * other RGBA colors mapped to the first nearest palette entry. Indexed PNGs
 * keep their raw indices, as SUGAR does: the console prompt sheets rely on
 * it, and glac terminal's 13-color gfx.png uses index 6 for the blue it
 * appends to the palette at runtime. */
static int write_sgimg_png(const char *path,Png *png,const Png *ref){
 size_t count=(size_t)png->w*png->h;unsigned char *out=malloc(12+1024+count);if(!out)return 0;
 memcpy(out,"SGI1",4);for(int i=0;i<4;i++){out[4+i]=(png->w>>(8*i))&255;out[8+i]=(png->h>>(8*i))&255;}
 memset(out+12,0,1024);for(int i=0;i<ref->colors;i++){memcpy(out+12+4*i,ref->palette+3*i,3);out[15+4*i]=255;}
 unsigned char *idx=out+12+1024;
 if(png->type==3){
  for(uint32_t y=0;y<png->h;y++){const unsigned char *row=png_row(png,y);for(uint32_t x=0;x<png->w;x++)*idx++=(unsigned char)png_index(row,png->depth,x);}
 }
 else {uint32_t last_rgb=0xffffffffu;int last=0;
  for(uint32_t y=0;y<png->h;y++)for(uint32_t x=0;x<png->w;x++,idx++){const unsigned char *c=png_row(png,y)+(size_t)png->bpp*x;if(png->bpp==4&&c[3]<128){*idx=0;continue;}
   uint32_t rgb=(uint32_t)c[0]<<16|c[1]<<8|c[2];
   if(rgb!=last_rgb){last=nearest(ref,c);last_rgb=rgb;}
   *idx=(unsigned char)last;}
 }
 char out_path[1600];snprintf(out_path,sizeof(out_path),"%s",path);strcpy(strrchr(out_path,'.'),".sgimg");
 FILE *f=fopen(out_path,"wb");int ok=f&&fwrite(out,1,12+1024+count,f)==12+1024+count;if(f&&fclose(f))ok=0;
 free(out);return ok;
}
static int write_sgimg(const char *root,const char *name,const Png *ref){
 char path[1600];snprintf(path,sizeof(path),"%s/%s",root,name);Png png;if(!png_read(path,&png))return 0;
 int ok=write_sgimg_png(path,&png,ref);free(png.pixels);return ok;
}

static int by_name(const void *a,const void *b){return strcmp(*(char *const *)a,*(char *const *)b);}
static int write_profile(Install *in){
 char path[1600];snprintf(path,sizeof(path),"%s/sg_steam_profile.lua",in->root);FILE *f=fopen(path,"w");if(!f)return 0;
 qsort(in->names,in->count,sizeof(char *),by_name);
 /* Music stays .ogg: the runtime plays Vorbis directly, so no mp3 map. */
 fputs("return {files={",f);
 for(int i=0;i<in->count;i++){fputs(i?",\"":"\"",f);for(const char *c=in->names[i];*c;c++){if(*c=='"'||*c=='\\')fputc('\\',f);fputc(*c,f);}fputc('"',f);}
 fputs("},music={}}\n",f);return fclose(f)==0;
}
static int marker_matches(const char *root,const char *expected){
 char path[1600],line[256]={0};snprintf(path,sizeof(path),"%s/" MARKER,root);FILE *f=fopen(path,"r");if(!f)return 0;
 int same=fgets(line,sizeof(line),f)&&!strcmp(line,expected);fclose(f);return same;
}

/* Mods are copied as shipped (Workshop folders); give every PNG in mods/ a
 * .sgimg built with the game palette, unless it already has one. */
/* Bump when the conversion changes so mods/ images are redone once. */
#define MOD_IMAGE_FORMAT "mod-images=1\n"
static int force_mod_images;
static void convert_dir(const char *root,const char *rel,const Png *ref,FILE *log,int *count){
 char path[1600];snprintf(path,sizeof(path),"%s/%s",root,rel);DIR *d=opendir(path);if(!d)return;
 struct dirent *e;
 while((e=readdir(d))){
  if(e->d_name[0]=='.')continue;
  char child[1024];snprintf(child,sizeof(child),"%s/%s",rel,e->d_name);
  snprintf(path,sizeof(path),"%s/%s",root,child);struct stat st;if(stat(path,&st))continue;
  if(S_ISDIR(st.st_mode)){convert_dir(root,child,ref,log,count);continue;}
  if(!ends_with(child,".png"))continue;
  time_t png_time=st.st_mtime;strcpy(strrchr(path,'.'),".sgimg");
  if(!force_mod_images&&!stat(path,&st)&&st.st_mtime>=png_time)continue;
  if(write_sgimg(root,child,ref))(*count)++;else if(log)fprintf(log,"steam_mod_png_failed=%s\n",child);
 }
 closedir(d);
}
/* Mods find each other by folder: info.lua says name="glac terminal" -- make
 * this match your mod folder. Workshop downloads are numbered folders, so
 * give each one its info.lua name and carry its modlist.lua entry along. */
static int info_name(const char *path,char *out,size_t size){
 FILE *f=fopen(path,"r");if(!f)return 0;char line[512];int found=0;
 while(!found&&fgets(line,sizeof(line),f)){
  char *p=line;while(*p==' '||*p=='\t')p++;
  if(strncmp(p,"name",4))continue;p+=4;while(*p==' '||*p=='\t')p++;if(*p++!='=')continue;while(*p==' '||*p=='\t')p++;
  char quote=*p++;if(quote!='"'&&quote!='\'')continue;char *end=strchr(p,quote);
  if(end&&end>p&&(size_t)(end-p)<size){memcpy(out,p,end-p);out[end-p]=0;found=1;}
 }
 fclose(f);
 /* A folder name: nothing that leaves mods/ or breaks modlist.lua quoting. */
 for(char *c=out;found&&*c;c++)if(*c=='/'||*c=='\\'||*c=='\''||*c=='"'||*c==':')found=0;
 return found&&strcmp(out,".")&&strcmp(out,"..");
}
static void rename_mod_folders(const char *root,FILE *log){
 char path[1600],from[1600],to[1600],name[256];snprintf(path,sizeof(path),"%s/mods",root);
 DIR *d=opendir(path);if(!d)return;
 char *list=NULL;size_t list_n=0;
 snprintf(path,sizeof(path),"%s/mods/modlist.lua",root);
 FILE *f=fopen(path,"rb");if(f){fseek(f,0,SEEK_END);long n=ftell(f);fseek(f,0,SEEK_SET);if(n>0&&(list=malloc(n+1))&&fread(list,1,n,f)==(size_t)n){list[n]=0;list_n=n;}else{free(list);list=NULL;}fclose(f);}
 int renamed=0;struct dirent *e;struct stat st;
 while((e=readdir(d))){
  if(e->d_name[0]=='.')continue;
  snprintf(from,sizeof(from),"%s/mods/%s",root,e->d_name);if(stat(from,&st)||!S_ISDIR(st.st_mode))continue;
  snprintf(path,sizeof(path),"%s/info.lua",from);if(!info_name(path,name,sizeof(name))||!strcmp(name,e->d_name))continue;
  snprintf(to,sizeof(to),"%s/mods/%s",root,name);
  if(!stat(to,&st)){if(log)fprintf(log,"steam_mod_folder_kept=%s (mods/%s exists)\n",e->d_name,name);continue;}
  if(rename(from,to)){if(log)fprintf(log,"steam_mod_folder_rename_failed=%s\n",e->d_name);continue;}
  if(log)fprintf(log,"steam_mod_folder=%s -> %s\n",e->d_name,name);renamed++;
  /* modlist.lua keeps { 'folder', active } entries: rename the quoted one. */
  char old_key[300],new_key[300];snprintf(old_key,sizeof(old_key),"'%s'",e->d_name);snprintf(new_key,sizeof(new_key),"'%s'",name);
  char *hit=list?strstr(list,old_key):NULL;
  if(hit){size_t a=hit-list,ol=strlen(old_key),nl=strlen(new_key);char *grown=malloc(list_n-ol+nl+1);
   if(grown){memcpy(grown,list,a);memcpy(grown+a,new_key,nl);memcpy(grown+a+nl,hit+ol,list_n-a-ol+1);free(list);list=grown;list_n=list_n-ol+nl;}}
 }
 closedir(d);
 if(renamed&&list){snprintf(path,sizeof(path),"%s/mods/modlist.lua",root);f=fopen(path,"wb");if(f){fwrite(list,1,list_n,f);fclose(f);}}
 free(list);
}
static void convert_mods(const char *root,FILE *log){
 rename_mod_folders(root,log);
 char path[1600];snprintf(path,sizeof(path),"%s/assets/gfx/gfx.png",root);Png ref;
 if(!png_read(path,&ref)||ref.type!=3){free(ref.pixels);return;}free(ref.pixels);
 char marker[1600],line[64]="";snprintf(marker,sizeof(marker),"%s/mods/.sg_mod_images",root);
 FILE *m=fopen(marker,"r");if(m){if(!fgets(line,sizeof(line),m))line[0]=0;fclose(m);}
 force_mod_images=strcmp(line,MOD_IMAGE_FORMAT)!=0;
 int count=0;convert_dir(root,"mods",&ref,log,&count);
 if(log)fprintf(log,"steam_mod_images_converted=%d forced=%d\n",count,force_mod_images);
 m=fopen(marker,"w");if(m){fputs(MOD_IMAGE_FORMAT,m);fclose(m);}
}

unsigned *sg_steam_loader_image(const char *path,int w,int h){
 FILE *f=fopen(path,"rb");if(!f)return NULL;
 unsigned char head[8],pal[1024];unsigned *out=NULL;unsigned char *packed=NULL,*index=NULL;
 if(fread(head,1,8,f)!=8||memcmp(head,"SGL1",4)||(head[4]|head[5]<<8)!=w||(head[6]|head[7]<<8)!=h||fread(pal,1,1024,f)!=1024)goto done;
 long start=ftell(f);fseek(f,0,SEEK_END);long size=ftell(f)-start;fseek(f,start,SEEK_SET);
 mz_ulong count=(mz_ulong)w*h;
 if(size<=0||!(packed=malloc(size))||!(index=malloc(count))||fread(packed,1,size,f)!=(size_t)size)goto done;
 if(mz_uncompress(index,&count,packed,size)!=MZ_OK||count!=(mz_ulong)w*h||!(out=malloc(count*4)))goto done;
 /* RGBA bytes are the display's 0xAABBGGRR words on this little-endian CPU. */
 for(mz_ulong i=0;i<count;i++)memcpy(out+i,pal+index[i]*4,4);
done:
 fclose(f);free(packed);free(index);return out;
}
int sg_steam_game_version(const char *root,char *out,size_t size){
 char path[1024],line[256];snprintf(path,sizeof(path),"%s/code/data.lua",root);out[0]=0;
 FILE *f=fopen(path,"r");if(!f)return 0;
 for(int n=0;n<16&&fgets(line,sizeof(line),f);n++){
  char *v=strstr(line,"VERSION=\"");if(!v)continue;v+=9;char *end=strchr(v,'"');
  if(end&&(size_t)(end-v)<size){memcpy(out,v,end-v);out[end-v]=0;}break;
 }
 fclose(f);return out[0]!=0;
}

static int steam_install(const char *root,FILE *log,SgInstallProgress progress,void *user);
int sg_steam_install(const char *root,FILE *log,SgInstallProgress progress,void *user){
 int ok=steam_install(root,log,progress,user);if(ok)convert_mods(root,log);return ok;
}
static int steam_install(const char *root,FILE *log,SgInstallProgress progress,void *user){
 /* The bridge ships with the app, so refresh it on every boot. */
 if(!write_file(root,"sg_steam_compat.lua",sg_steam_compat_lua,sizeof(sg_steam_compat_lua))){if(log)fprintf(log,"steam_install_bridge_failed\n");return 0;}
 char path[1600];snprintf(path,sizeof(path),"%s/data.sgr",root);FILE *f=fopen(path,"rb");
 if(!f){snprintf(path,sizeof(path),"%s/sg_steam_profile.lua",root);struct stat st;int prepared=!stat(path,&st);
  if(log)fprintf(log,"steam_install=no_data_sgr prepared_profile=%d\n",prepared);return prepared;}
 fseek(f,0,SEEK_END);long total=ftell(f);unsigned char trailer[4]={0};
 if(total>4){fseek(f,total-4,SEEK_SET);if(fread(trailer,1,4,f)!=4)total=0;}
 char identity[128];snprintf(identity,sizeof(identity),"format=1 sgr_bytes=%ld trailer=%02x%02x%02x%02x\n",total,trailer[0],trailer[1],trailer[2],trailer[3]);
 if(marker_matches(root,identity)){fclose(f);if(log)fprintf(log,"steam_install=current %s",identity);return 1;}
 if(log){fprintf(log,"steam_install=start %s",identity);fflush(log);}
 snprintf(path,sizeof(path),"%s/" MARKER,root);remove(path);
 Install *in=calloc(1,sizeof(*in));if(!in){fclose(f);return 0;}in->root=root;in->log=log;
 int ok=unpack(in,f,total,progress,user);fclose(f);
 Png ref={0};
 if(ok){snprintf(path,sizeof(path),"%s/assets/gfx/gfx.png",root);ok=png_read(path,&ref)&&ref.type==3&&ref.colors>0;free(ref.pixels);
  if(!ok&&log)fprintf(log,"steam_install_palette_failed\n");}
 int converted=0;
 for(int i=0;ok&&i<in->count;i++)if(ends_with(in->names[i],".png")){
  if(!write_sgimg(root,in->names[i],&ref)){if(log)fprintf(log,"steam_install_png_failed=%s\n",in->names[i]);ok=0;}
  else if(progress)progress(90+9*++converted/(in->pngs?in->pngs:1),user);}
 if(ok){snprintf(path,sizeof(path),"%s/save",root);mkdir(path,0777);snprintf(path,sizeof(path),"%s/mods",root);mkdir(path,0777);ok=write_profile(in);}
 if(ok)ok=write_file(root,MARKER,identity,strlen(identity));
 if(log){fprintf(log,"steam_install=%s files=%d images=%d\n",ok?"done":"failed",in->count,converted);fflush(log);}
 if(progress&&ok)progress(100,user);
 for(int i=0;i<in->count;i++)free(in->names[i]);free(in);return ok;
}
