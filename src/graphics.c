#include "graphics.h"
#include "lauxlib.h"
#include "perf.h"
#include "diagnostics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define MAX_SURFACES 80
typedef struct {char key[96];int w,h;unsigned char *p;} Surface;
static Surface surfaces[MAX_SURFACES];static int count, dst, sheet, gridw=16,gridh=16,camx,camy,ink=4;
static uint32_t colors[256];static unsigned char mapping[256],displaymap[256],transparent[256];static char data_root[1024];
static size_t allocated;
unsigned sg_graphics_memory_kb(void){return (unsigned)(allocated/1024);}
static int clipx,clipy,clipw=4096,cliph=4096,pattern,pattern_transparent;
static unsigned short sprite_pattern[256],sprite_pair[256];
static unsigned char blend_table[256][256];
static int blend_identity=1;
typedef struct{unsigned cp;unsigned char *p;int valid,w,h,ox,oy,advance;} Glyph;
typedef struct{char key[80];unsigned char *data;stbtt_fontinfo info;float scale;int surface,ttf,dy,h,space;unsigned cp[256];int gx[256],gw[256],glyphs;Glyph cache[256];} Font;
static Font fonts[16];static int fontcount,current_font;
static void free_glyphs(Font *f){for(int i=0;i<256;i++){stbtt_FreeBitmap(f->cache[i].p,NULL);memset(&f->cache[i],0,sizeof(Glyph));}}
static Glyph *cached_glyph(Font *f,unsigned cp){
 Glyph *g=&f->cache[cp&255];
 if(g->valid&&g->cp==cp){sg_perf.glyph_hits++;return g;}
 uint64_t start=sg_now_us();stbtt_FreeBitmap(g->p,NULL);memset(g,0,sizeof(*g));
 g->cp=cp;int a,b;stbtt_GetCodepointHMetrics(&f->info,cp,&a,&b);g->advance=(int)ceil(a*f->scale);
 g->p=stbtt_GetCodepointBitmap(&f->info,0,f->scale,cp,&g->w,&g->h,&g->ox,&g->oy);g->valid=1;
 sg_perf.glyph_misses++;sg_perf.font_build_us+=sg_now_us()-start;return g;
}
static unsigned codepoint(const unsigned char **text);
static int locate(lua_State *L,const char *key){for(int i=0;i<count;i++)if(!strcmp(surfaces[i].key,key))return i;return luaL_error(L,"Unknown surface: %s",key);}
static int create(lua_State *L,const char *key,int w,int h){
 if(w<1||h<1||w>4096||h>4096||allocated+(size_t)w*h>32*1024*1024)return luaL_error(L,"Surface memory limit: %s",key);
 int i;for(i=0;i<count;i++)if(!strcmp(surfaces[i].key,key))break;
 if(i==count){if(count==MAX_SURFACES)return luaL_error(L,"Too many surfaces");count++;}
 if(surfaces[i].p){allocated-=(size_t)surfaces[i].w*surfaces[i].h;free(surfaces[i].p);}
 surfaces[i].p=calloc((size_t)w*h,1);if(!surfaces[i].p)return luaL_error(L,"Surface allocation failed");
 surfaces[i].w=w;surfaces[i].h=h;snprintf(surfaces[i].key,sizeof(surfaces[i].key),"%s",key);allocated+=(size_t)w*h;return i;
}
static int load(lua_State *L,const char *filename,const char *key,int use_palette){
 while(!strncmp(filename,"../",3))filename+=3;
 if(filename[0]=='/'||strchr(filename,':')||strstr(filename,".."))return luaL_error(L,"Invalid image path");
 char path[2048];snprintf(path,sizeof(path),"%s/%s",data_root,filename);char *ext=strrchr(path,'.');if(ext)strcpy(ext,".sgimg");
 sg_diag_event("image_load",path);
 FILE *f=fopen(path,"rb");if(!f)return luaL_error(L,"Missing prepared image: %s",path);
 unsigned char header[12],pal[1024];if(fread(header,1,12,f)!=12||memcmp(header,"SGI1",4)||fread(pal,1,1024,f)!=1024){fclose(f);return luaL_error(L,"Invalid SGI image");}
 unsigned w=header[4]|header[5]<<8|header[6]<<16|header[7]<<24,h=header[8]|header[9]<<8|header[10]<<16|header[11]<<24;
 if(w>4096||h>4096||!w||!h){fclose(f);return luaL_error(L,"Invalid image dimensions");}
 int id=create(L,key,(int)w,(int)h);size_t got=fread(surfaces[id].p,1,(size_t)w*h,f);fclose(f);if(got!=(size_t)w*h)return luaL_error(L,"Truncated image");
 if(use_palette)for(int j=0;j<256;j++)colors[j]=0xff000000u|pal[j*4]|pal[j*4+1]<<8|pal[j*4+2]<<16;
 return id;
}
static void dot(int x,int y,int c){Surface *s=&surfaces[dst];x-=camx;y-=camy;if(x<clipx||y<clipy||x>=clipx+clipw||y>=clipy+cliph)return;int alt=(pattern>>((y&3)*4+(x&3)))&1;if(alt&&pattern_transparent)return;if(x>=0&&y>=0&&x<s->w&&y<s->h){int value=mapping[(alt?c>>8:c)&255];s->p[y*s->w+x]=blend_table[value][s->p[y*s->w+x]];}}
static int newsrf(lua_State *L){int id;if(lua_isnumber(L,1)){char key[64];snprintf(key,sizeof(key),"surface%d",count);id=create(L,luaL_optstring(L,3,key),luaL_checkint(L,1),luaL_checkint(L,2));}else{id=load(L,luaL_checkstring(L,1),luaL_optstring(L,2,lua_tostring(L,1)),0);}lua_pushstring(L,surfaces[id].key);return 1;}
static int palette(lua_State *L){if(lua_gettop(L)){if(lua_istable(L,1)){int n=(int)lua_objlen(L,1);for(int i=0;i<n&&i<256;i++){lua_rawgeti(L,1,i+1);unsigned c=(unsigned)lua_tointeger(L,-1);lua_pop(L,1);colors[i]=0xff000000u|((c>>16)&255)|(c&0xff00)|((c&255)<<16);}}else load(L,luaL_checkstring(L,1),"_palette",1);}lua_newtable(L);for(int i=0;i<256;i++){unsigned c=colors[i];lua_pushinteger(L,((c&255)<<16)|(c&0xff00)|((c>>16)&255));lua_rawseti(L,-2,i+1);}return 1;}
static int target(lua_State *L){dst=lua_gettop(L)?locate(L,luaL_checkstring(L,1)):0;return 0;}
static int spritesheet(lua_State *L){if(lua_gettop(L))sheet=locate(L,luaL_checkstring(L,1));lua_pushstring(L,surfaces[sheet].key);return 1;}
static int sprgrid(lua_State *L){gridw=luaL_optint(L,1,16);gridh=luaL_optint(L,2,gridw);return 0;}
static int camera(lua_State *L){camx=luaL_optint(L,1,0);camy=luaL_optint(L,2,0);return 0;}
static int tcamera(lua_State *L){camx+=luaL_checkint(L,1);camy+=luaL_checkint(L,2);return 0;}
static int cls(lua_State *L){memset(surfaces[dst].p,luaL_optint(L,1,0)&255,(size_t)surfaces[dst].w*surfaces[dst].h);return 0;}
static int color(lua_State *L){ink=luaL_optint(L,1,0);return 0;}
static int rectfill(lua_State *L){
 int x1=luaL_checkint(L,1),y1=luaL_checkint(L,2),x2=luaL_checkint(L,3),y2=luaL_checkint(L,4),c=luaL_optint(L,5,ink);
 if(x1>x2){int t=x1;x1=x2;x2=t;}if(y1>y2){int t=y1;y1=y2;y2=t;}
 Surface *s=&surfaces[dst];x1-=camx;x2-=camx;y1-=camy;y2-=camy;
 if(x1<0)x1=0;if(y1<0)y1=0;if(x1<clipx)x1=clipx;if(y1<clipy)y1=clipy;
 if(x2>=s->w)x2=s->w-1;if(y2>=s->h)y2=s->h-1;if(x2>=clipx+clipw)x2=clipx+clipw-1;if(y2>=clipy+cliph)y2=clipy+cliph-1;
 if(x1>x2||y1>y2)return 0;
 if(!pattern&&blend_identity){for(int y=y1;y<=y2;y++)memset(s->p+y*s->w+x1,mapping[c&255],(size_t)(x2-x1+1));}
 else for(int y=y1;y<=y2;y++)for(int x=x1;x<=x2;x++)dot(x+camx,y+camy,c);
 return 0;
}
static int line(lua_State *L){int x=luaL_checkint(L,1),y=luaL_checkint(L,2),x2=luaL_checkint(L,3),y2=luaL_checkint(L,4),c=luaL_optint(L,5,ink);int dx=abs(x2-x),sx=x<x2?1:-1,dy=-abs(y2-y),sy=y<y2?1:-1,err=dx+dy;while(1){dot(x,y,c);if(x==x2&&y==y2)break;int e=err*2;if(e>=dy){err+=dy;x+=sx;}if(e<=dx){err+=dx;y+=sy;}}return 0;}
static int circle(lua_State *L){double x=luaL_checknumber(L,1),y=luaL_checknumber(L,2),r=luaL_checknumber(L,3);int c=luaL_optint(L,4,ink);for(int yy=(int)(y-r);yy<=y+r;yy++)for(int xx=(int)(x-r);xx<=x+r;xx++)if((xx-x)*(xx-x)+(yy-y)*(yy-y)<=r*r)dot(xx,yy,c);return 0;}
static void blit(int sx,int sy,int sw,int sh,int dx,int dy,int dw,int dh,int fx,int fy){
 Surface *s=&surfaces[sheet],*d=&surfaces[dst];
 if(dw<0){dx+=dw;dw=-dw;fx=!fx;}if(dh<0){dy+=dh;dh=-dh;fy=!fy;}
 if(dw<=0||dh<=0||sw<=0||sh<=0)return;
 int x0=0,y0=0,x1=dw,y1=dh;
 int left=camx+(clipx>0?clipx:0),top=camy+(clipy>0?clipy:0);
 int right=camx+(clipx+clipw<d->w?clipx+clipw:d->w),bottom=camy+(clipy+cliph<d->h?clipy+cliph:d->h);
 if(dx+x0<left)x0=left-dx;if(dy+y0<top)y0=top-dy;if(dx+x1>right)x1=right-dx;if(dy+y1>bottom)y1=bottom-dy;
 if(x0>=x1||y0>=y1)return;
 sg_perf.blit_pixels+=(uint64_t)(x1-x0)*(y1-y0);
 int step=sw/dw,extra=sw%dw,start=x0*sw/dw,startrem=x0*sw%dw;
 for(int y=y0;y<y1;y++){
  int yy=sy+(fy?sh-1-y*sh/dh:y*sh/dh);if(yy<0||yy>=s->h)continue;
  int q=start,rem=startrem,ty=dy+y-camy;unsigned char *row=d->p+ty*d->w;
  for(int x=x0;x<x1;x++){
   int xx=sx+(fx?sw-1-q:q);q+=step;rem+=extra;if(rem>=dw){rem-=dw;q++;}
   if(xx<0||xx>=s->w)continue;
   int c=s->p[yy*s->w+xx],tx=dx+x-camx;
   if(!transparent[c]){int alt=(sprite_pattern[c]>>((ty&3)*4+(tx&3)))&1;c=mapping[(sprite_pair[c]>>(alt?8:0))&255];row[tx]=blend_identity?c:blend_table[c][row[tx]];}
  }
 }
}
static int asspr(lua_State *L){
 int sx=luaL_checkint(L,1),sy=luaL_checkint(L,2),sw=luaL_checkint(L,3),sh=luaL_checkint(L,4);
 double dx=luaL_checknumber(L,5),dy=luaL_checknumber(L,6),a=luaL_checknumber(L,7)*6.283185307179586;
 double zx=luaL_optnumber(L,8,1),zy=luaL_optnumber(L,9,1),ax=luaL_optnumber(L,10,sw*.5),ay=luaL_optnumber(L,11,sh*.5);
 if(!zx||!zy||sw<=0||sh<=0)return 0;
 double cs=cos(a),sn=sin(a);Surface *s=&surfaces[sheet],*d=&surfaces[dst];
 double xlo=1e30,xhi=-1e30,ylo=1e30,yhi=-1e30;
 for(int j=0;j<4;j++){double x=((j&1)?sw-ax:-ax)*zx,y=((j&2)?sh-ay:-ay)*zy;double xx=dx+x*cs-y*sn,yy=dy+x*sn+y*cs;if(xx<xlo)xlo=xx;if(xx>xhi)xhi=xx;if(yy<ylo)ylo=yy;if(yy>yhi)yhi=yy;}
 int left=(int)fmax(floor(xlo),camx),right=(int)fmin(ceil(xhi),camx+d->w-1),top=(int)fmax(floor(ylo),camy),bottom=(int)fmin(ceil(yhi),camy+d->h-1),saved=pattern;pattern=0;
 for(int y=top;y<=bottom;y++)for(int x=left;x<=right;x++){double xx=x+.5-dx,yy=y+.5-dy;int u=(int)floor((xx*cs+yy*sn)/zx+ax),v=(int)floor((-xx*sn+yy*cs)/zy+ay);if(u<0||v<0||u>=sw||v>=sh||sx+u<0||sy+v<0||sx+u>=s->w||sy+v>=s->h)continue;int c=s->p[(sy+v)*s->w+sx+u];if(!transparent[c]){int alt=(sprite_pattern[c]>>(((y-camy)&3)*4+((x-camx)&3)))&1;dot(x,y,(sprite_pair[c]>>(alt?8:0))&255);}}
 pattern=saved;return 0;
}
static int sspr(lua_State *L){int sw=luaL_checkint(L,3),sh=luaL_checkint(L,4);blit(luaL_checkint(L,1),luaL_checkint(L,2),sw,sh,luaL_checkint(L,5),luaL_checkint(L,6),luaL_optint(L,7,sw),luaL_optint(L,8,sh),lua_toboolean(L,9),lua_toboolean(L,10));return 0;}
static int spr(lua_State *L){int n=luaL_checkint(L,1),cols=surfaces[sheet].w/gridw;if(!cols)return 0;int w=luaL_optint(L,4,1)*gridw,h=luaL_optint(L,5,1)*gridh;blit((n%cols)*gridw,(n/cols)*gridh,w,h,luaL_checkint(L,2),luaL_checkint(L,3),w,h,lua_toboolean(L,6),lua_toboolean(L,7));return 0;}
static int sget(lua_State *L){Surface *s=&surfaces[sheet];int x=luaL_checkint(L,1),y=luaL_checkint(L,2);lua_pushinteger(L,x>=0&&y>=0&&x<s->w&&y<s->h?s->p[y*s->w+x]:0);return 1;}
static int size(lua_State *L){int id=lua_gettop(L)?locate(L,luaL_checkstring(L,1)):dst;lua_pushinteger(L,surfaces[id].w);lua_pushinteger(L,surfaces[id].h);return 2;}
static int pal(lua_State *L){int n=lua_gettop(L);unsigned char *map=lua_toboolean(L,3)?displaymap:mapping;if(!n){for(int i=0;i<256;i++){mapping[i]=i;displaymap[i]=i;}return 0;}if(lua_isnil(L,1))return 0;if(lua_istable(L,1)){map=lua_toboolean(L,2)?displaymap:mapping;lua_pushnil(L);while(lua_next(L,1)){map[lua_tointeger(L,-2)&255]=lua_tointeger(L,-1)&255;lua_pop(L,1);}}else map[luaL_checkint(L,1)&255]=luaL_checkint(L,2)&255;return 0;}
static int palt(lua_State *L){if(!lua_gettop(L)){memset(transparent,0,256);transparent[0]=1;}else transparent[luaL_checkint(L,1)&255]=lua_toboolean(L,2);return 0;}
static int newwin(lua_State *L){create(L,"screen",luaL_checkint(L,2),luaL_checkint(L,3));dst=0;return 0;}
static int delsrf(lua_State *L){int id=locate(L,luaL_checkstring(L,1));if(id==0)return luaL_error(L,"Cannot delete screen");allocated-=(size_t)surfaces[id].w*surfaces[id].h;free(surfaces[id].p);surfaces[id].p=NULL;surfaces[id].w=surfaces[id].h=0;return 0;}
static int noop(lua_State *L){(void)L;return 0;}
static int fillp(lua_State *L){pattern=luaL_optint(L,1,0);pattern_transparent=lua_toboolean(L,2);return 0;}
static int sfillp(lua_State *L){if(!lua_gettop(L)){for(int i=0;i<256;i++){sprite_pattern[i]=0;sprite_pair[i]=i|(i<<8);}}else{int c=luaL_checkint(L,1)&255;sprite_pattern[c]=luaL_checkint(L,2);sprite_pair[c]=luaL_checkint(L,3);}return 0;}
static int clip(lua_State *L){clipx=luaL_optint(L,1,0);clipy=luaL_optint(L,2,0);clipw=luaL_optint(L,3,4096);cliph=luaL_optint(L,4,4096);if(lua_toboolean(L,5)){clipx-=camx;clipy-=camy;}return 0;}
static int rect(lua_State *L){int x=luaL_checkint(L,1),y=luaL_checkint(L,2),x2=luaL_checkint(L,3),y2=luaL_checkint(L,4),c=luaL_optint(L,5,ink);for(int xx=x;xx<=x2;xx++){dot(xx,y,c);dot(xx,y2,c);}for(int yy=y;yy<=y2;yy++){dot(x,yy,c);dot(x2,yy,c);}return 0;}
static int pset(lua_State *L){dot(luaL_checkint(L,1),luaL_checkint(L,2),luaL_optint(L,3,ink));return 0;}
static int pget(lua_State *L){Surface *s=&surfaces[dst];int x=luaL_checkint(L,1)-camx,y=luaL_checkint(L,2)-camy;lua_pushinteger(L,x>=0&&y>=0&&x<s->w&&y<s->h?s->p[y*s->w+x]:0);return 1;}
static int blend(lua_State *L){if(!lua_gettop(L)){blend_identity=1;for(int i=0;i<256;i++)for(int j=0;j<256;j++)blend_table[i][j]=i;}else {blend_identity=0;blend_table[luaL_checkint(L,1)&255][luaL_checkint(L,2)&255]=luaL_checkint(L,3)&255;}return 0;}
static int exe(lua_State *L){int d=dst,s=sheet,cx=camx,cy=camy,p=pattern,tr=pattern_transparent,ic=ink,cx0=clipx,cy0=clipy,cw=clipw,ch=cliph,f=current_font;unsigned char map[256];memcpy(map,mapping,256);lua_pushvalue(L,1);int status=lua_pcall(L,0,0,0);dst=d;sheet=s;camx=cx;camy=cy;pattern=p;pattern_transparent=tr;ink=ic;clipx=cx0;clipy=cy0;clipw=cw;cliph=ch;current_font=f;memcpy(mapping,map,256);if(status)return lua_error(L);return 0;}
static int patterns(lua_State *L){lua_newtable(L);int x=luaL_checkint(L,1),y=luaL_checkint(L,2),n=luaL_checkint(L,3);for(int i=0;i<n;i++){Surface *s=&surfaces[sheet];int value=0;for(int yy=0;yy<4;yy++)for(int xx=0;xx<4;xx++)if(s->p[(y+yy)*s->w+x+i*4+xx])value|=1<<(yy*4+xx);lua_pushinteger(L,value);lua_rawseti(L,-2,i+1);}return 1;}
static int newfont(lua_State *L){const char *path=luaL_checkstring(L,1),*key=luaL_optstring(L,3,"default");int id;for(id=0;id<fontcount;id++)if(!strcmp(fonts[id].key,key))break;if(id==fontcount){if(fontcount==16)return luaL_error(L,"Font limit");fontcount++;}Font *f=&fonts[id];free_glyphs(f);free(f->data);memset(f,0,sizeof(*f));snprintf(f->key,sizeof(f->key),"%s",key);f->space=4;f->h=8;
 if(strstr(path,".ttf")){char full[2048];snprintf(full,sizeof(full),"%s/%s",data_root,path);FILE *file=fopen(full,"rb");if(!file)return luaL_error(L,"Missing font %s",full);fseek(file,0,SEEK_END);long n=ftell(file);rewind(file);if(n<1||n>8*1024*1024){fclose(file);return luaL_error(L,"Font size limit");}f->data=malloc(n);if(!f->data){fclose(file);return luaL_error(L,"Font allocation failed");}size_t got=fread(f->data,1,n,file);fclose(file);if(got!=(size_t)n||!stbtt_InitFont(&f->info,f->data,0))return luaL_error(L,"Invalid font");f->ttf=1;f->h=luaL_checkint(L,2);f->scale=stbtt_ScaleForPixelHeight(&f->info,(float)f->h);
 }else{char name[100];snprintf(name,sizeof(name),"font_%s",key);f->surface=load(L,path,name,0);Surface *s=&surfaces[f->surface];const unsigned char *chars=(const unsigned char *)luaL_checkstring(L,2);int x=0;while(*chars&&x<s->w&&f->glyphs<256){while(x<s->w){int used=0;for(int y=0;y<s->h;y++)used|=s->p[y*s->w+x];if(used)break;x++;}int start=x;while(x<s->w){int used=0;for(int y=0;y<s->h;y++)used|=s->p[y*s->w+x];if(!used)break;x++;}int g=f->glyphs++;f->cp[g]=codepoint(&chars);f->gx[g]=start;f->gw[g]=x-start;}f->h=s->h+1;}
 lua_pushstring(L,key);return 1;}
static int selectfont(lua_State *L){const char *key=luaL_optstring(L,1,"default");for(int i=0;i<fontcount;i++)if(!strcmp(fonts[i].key,key)){current_font=i;return 0;}return luaL_error(L,"Unknown font %s",key);}
static unsigned codepoint(const unsigned char **text){const unsigned char *p=*text;unsigned c=*p++;if(c>=0xc0&&c<0xe0&&*p)c=((c&31)<<6)|(*p++&63);else if(c>=0xe0&&c<0xf0&&p[0]&&p[1]){c=((c&15)<<12)|((p[0]&63)<<6)|(p[1]&63);p+=2;}*text=p;return c;}
static int advance(Font *f,unsigned c){if(c==' ')return f->space;if(f->ttf)return cached_glyph(f,c)->advance;for(int i=0;i<f->glyphs;i++)if(f->cp[i]==c)return f->gw[i]+1;return 4;}
static int drawtext(const char *str,int x,int y,int c,int draw){Font *f=&fonts[current_font];int start=x;const unsigned char *p=(const unsigned char *)str;while(*p){unsigned cp=codepoint(&p);if(cp=='\n'){x=start;y+=f->h;continue;}if(draw&&cp!=' '){if(f->ttf){Glyph *g=cached_glyph(f,cp);if(g->p)for(int yy=0;yy<g->h;yy++)for(int xx=0;xx<g->w;xx++)if(g->p[yy*g->w+xx]>=100)dot(x+g->ox+xx,y+f->h+g->oy+yy+f->dy,c);}else{Surface *s=&surfaces[f->surface];for(int g=0;g<f->glyphs;g++)if(f->cp[g]==cp){for(int yy=0;yy<s->h-1;yy++)for(int xx=0;xx<f->gw[g];xx++)if(s->p[yy*s->w+f->gx[g]+xx])dot(x+xx,y+yy,c);break;}}}x+=advance(f,cp);}return x;}
static int text(lua_State *L){const char *str=luaL_checkstring(L,1);if(lua_gettop(L)<3){fprintf(stdout,"%s\n",str);lua_pushinteger(L,0);return 1;}int x=luaL_checkint(L,2),y=luaL_checkint(L,3),c=luaL_optint(L,4,ink);lua_pushinteger(L,drawtext(str,x,y,c,1));return 1;}
static int width(lua_State *L){lua_pushinteger(L,drawtext(luaL_checkstring(L,1),0,0,ink,0));return 1;}
static int bprint(lua_State *L){const char *str=luaL_checkstring(L,1);int start=luaL_checkint(L,2),x=start,y=luaL_checkint(L,3),w=luaL_checkint(L,4),c=luaL_optint(L,5,ink);const unsigned char *p=(const unsigned char *)str;while(*p){const unsigned char *begin=p;unsigned cp=codepoint(&p);int a=advance(&fonts[current_font],cp);if(cp=='\n'||x+a>start+w){x=start;y+=fonts[current_font].h;if(cp=='\n')continue;}char glyph[8]={0};size_t n=(size_t)(p-begin);if(n<sizeof(glyph))memcpy(glyph,begin,n);drawtext(glyph,x,y,c,1);x+=a;}lua_pushinteger(L,y+fonts[current_font].h);return 1;}
static int fntspec(lua_State *L){Font *f=&fonts[current_font];const char *op=luaL_checkstring(L,1);if(!strcmp(op,"name")){lua_pushstring(L,f->key);return 1;}int *n=!strcmp(op,"dy")?&f->dy:!strcmp(op,"space")?&f->space:&f->h;if(lua_gettop(L)>1)*n=luaL_checkint(L,2);lua_pushinteger(L,*n);return 1;}
static void reg(lua_State *L,const char *name,lua_CFunction f){lua_pushcfunction(L,f);lua_setglobal(L,name);}
void sg_graphics_init(lua_State *L,const char *root){snprintf(data_root,sizeof(data_root),"%s",root);create(L,"screen",320,180);for(int i=0;i<256;i++){mapping[i]=i;displaymap[i]=i;colors[i]=0xff000000u|(i*0x010101u);}transparent[0]=1;
 for(int i=0;i<256;i++){sprite_pattern[i]=0;sprite_pair[i]=i|(i<<8);}
 for(int i=0;i<256;i++)for(int j=0;j<256;j++)blend_table[i][j]=i;
 reg(L,"newsrf",newsrf);reg(L,"palette",palette);reg(L,"target",target);reg(L,"spritesheet",spritesheet);reg(L,"sprgrid",sprgrid);reg(L,"camera",camera);reg(L,"tcamera",tcamera);reg(L,"cls",cls);reg(L,"color",color);reg(L,"rectfill",rectfill);reg(L,"line",line);reg(L,"circfill",circle);reg(L,"sspr",sspr);reg(L,"asspr",asspr);reg(L,"spr",spr);reg(L,"sget",sget);reg(L,"srfsize",size);reg(L,"pal",pal);reg(L,"palt",palt);reg(L,"newwin",newwin);reg(L,"get_patterns",patterns);reg(L,"winspec",noop);reg(L,"fpslimit",noop);reg(L,"shdrf",noop);
 reg(L,"newfnt",newfont);reg(L,"font",selectfont);reg(L,"strwidth",width);reg(L,"fntspec",fntspec);reg(L,"print",text);
 reg(L,"clip",clip);reg(L,"delsrf",delsrf);reg(L,"fillp",fillp);reg(L,"rect",rect);reg(L,"pset",pset);reg(L,"exe",exe);
 reg(L,"sfillp",sfillp);reg(L,"bprint",bprint);
 reg(L,"pget",pget);reg(L,"blend",blend);
 /* Default font uses the actual shipped TTF before the game selects its font. */
 lua_pushcfunction(L,newfont);lua_pushstring(L,"assets/fonts/Terminus.ttf");lua_pushinteger(L,12);lua_pushstring(L,"default");lua_call(L,3,0);
}
void sg_present_rows(uint32_t *p,int width,int height,int first,int last){
 Surface *s=&surfaces[0];if(first<0)first=0;if(last>height)last=height;if(first>=last)return;
 if(!s->p||!s->w||!s->h){memset(p+(size_t)first*width,0,(size_t)(last-first)*width*4);return;}
 int scale=width/s->w;if(height/s->h<scale)scale=height/s->h;if(scale<1)scale=1;
 int ox=(width-s->w*scale)/2,oy=(height-s->h*scale)/2,overlay=-1,previous=-1;
 uint32_t palette[256];for(int i=0;i<256;i++)palette[i]=colors[displaymap[i]];
 for(int i=0;i<count;i++)if(!strcmp(surfaces[i].key,"hdtext")&&surfaces[i].p)overlay=i;
 for(int y=first;y<last;y++){
  uint32_t *row=p+(size_t)y*width;int yy=(y-oy)/scale;
  if(y<oy||yy>=s->h){for(int x=0;x<width;x++)row[x]=palette[0];previous=-1;continue;}
  if(yy==previous){memcpy(row,row-width,(size_t)width*4);continue;}
  previous=yy;for(int x=0;x<width;x++)row[x]=palette[0];
  Surface *o=overlay>=0?&surfaces[overlay]:NULL;int py=o?yy*o->h/s->h:0;
  for(int x=0;x<s->w;x++){
   int c=s->p[yy*s->w+x];if(o){int v=o->p[py*o->w+x*o->w/s->w];if(v!=255)c=v;}
   int start=ox+x*scale,end=start+scale;if(start<0)start=0;if(end>width)end=width;
   uint32_t value=palette[c];for(int xx=start;xx<end;xx++)row[xx]=value;
  }
 }
}
void sg_present(uint32_t *p,int width,int height){sg_present_rows(p,width,height,0,height);}

void sg_graphics_close(void){for(int i=0;i<count;i++)free(surfaces[i].p);for(int i=0;i<fontcount;i++){free_glyphs(&fonts[i]);free(fonts[i].data);}memset(fonts,0,sizeof(fonts));fontcount=current_font=0;memset(surfaces,0,sizeof(surfaces));count=0;allocated=0;dst=sheet=0;}
