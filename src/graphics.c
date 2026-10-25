#include "graphics.h"
#include "local_ui.h"
#include "lauxlib.h"
#include "perf.h"
#include "diagnostics.h"
#include "runtime.h"
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
static int output_divisor=1;
void sg_graphics_set_output_divisor(int divisor){output_divisor=divisor==2?2:1;}
static int screen_pixel(Surface*s,int x,int y){if(s==surfaces&&output_divisor==2){x&=~1;y&=~1;}return s->p[y*s->w+x];}
unsigned sg_graphics_memory_kb(void){return (unsigned)(allocated/1024);}
static int clipx,clipy,clipw=4096,cliph=4096,pattern,pattern_transparent;
static unsigned short sprite_pattern[256],sprite_pair[256];
static unsigned char blend_table[256][256];
static int blend_identity=1,blend_changed;
typedef struct{unsigned cp;unsigned char *p;int valid,w,h,ox,oy,advance;} Glyph;
typedef struct{char key[80];unsigned char *data;stbtt_fontinfo info;float scale;int surface,ttf,dy,h,space;unsigned cp[256];int gx[256],gw[256],glyph_index[256],glyphs;Glyph cache[256];} Font;
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
static void dot(int x,int y,int c){Surface *s=&surfaces[dst];x-=camx;y-=camy;if(dst==0&&output_divisor==2&&((x|y)&1))return;if(x<clipx||y<clipy||x>=clipx+clipw||y>=clipy+cliph)return;int alt=(pattern>>((y&3)*4+(x&3)))&1;if(alt&&pattern_transparent)return;if(x>=0&&y>=0&&x<s->w&&y<s->h){int value=mapping[(alt?c>>8:c)&255];s->p[y*s->w+x]=blend_table[value][s->p[y*s->w+x]];}}
/* SUGAR: newsrf(w,h[,name]) or newsrf(data[,name]); data ending in ".png" is
 * an image (served here as its prepared .sgimg), anything else would be an
 * expsrf export. Those are not supported; like SUGAR, a bad data string must
 * not stop the game (a shipped mod calls newsrf("title","file.png")), so it
 * yields an empty 1x1 surface under that name and a console warning. */
static int ends_png(const char *s){size_t n=strlen(s);return n>=4&&(s[n-4]=='.')&&(s[n-3]|32)=='p'&&(s[n-2]|32)=='n'&&(s[n-1]|32)=='g';}
static int newsrf(lua_State *L){int id;if(lua_isnumber(L,1)){char key[64];snprintf(key,sizeof(key),"surface%d",count);id=create(L,luaL_optstring(L,3,key),luaL_checkint(L,1),luaL_checkint(L,2));}
 else{const char *data=luaL_checkstring(L,1),*name=luaL_optstring(L,2,data);
  if(ends_png(data))id=load(L,data,name,0);
  else{fprintf(stdout,"newsrf: unsupported surface data '%.40s' for '%s', using empty 1x1\n",data,name);id=create(L,name,1,1);}}
 lua_pushstring(L,surfaces[id].key);return 1;}
static int palette(lua_State *L){if(lua_gettop(L)){if(lua_istable(L,1)){int n=(int)lua_objlen(L,1);for(int i=0;i<n&&i<256;i++){lua_rawgeti(L,1,i+1);unsigned c=(unsigned)lua_tointeger(L,-1);lua_pop(L,1);colors[i]=0xff000000u|((c>>16)&255)|(c&0xff00)|((c&255)<<16);}}else load(L,luaL_checkstring(L,1),"_palette",1);}lua_newtable(L);for(int i=0;i<256;i++){unsigned c=colors[i];lua_pushinteger(L,((c&255)<<16)|(c&0xff00)|((c>>16)&255));lua_rawseti(L,-2,i+1);}return 1;}
static int target(lua_State *L){dst=lua_gettop(L)?locate(L,luaL_checkstring(L,1)):0;return 0;}
static int spritesheet(lua_State *L){
 if(lua_gettop(L)){
  const char *key=luaL_checkstring(L,1);
  if(strcmp(surfaces[sheet].key,key))sheet=locate(L,key);
 }
 lua_pushstring(L,surfaces[sheet].key);return 1;
}
static int sprgrid(lua_State *L){int w=luaL_optint(L,1,16),h=luaL_optint(L,2,w);if(w<1||h<1||w>4096||h>4096)return luaL_error(L,"Invalid sprite grid");gridw=w;gridh=h;return 0;}
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
/* Triangle coordinates are transformed before conversion to integer pixels.
 * Clip to the destination first, so far-offscreen vertices never create huge
 * loops or overflow the line rasterizer. Coverage includes integer edge pixels. */
static void triangle_pixel(int x,int y,int c){
 if(dst==0&&output_divisor==2&&((x|y)&1))return;
 int alt=(pattern>>((y&3)*4+(x&3)))&1;if(alt&&pattern_transparent)return;
 Surface *s=&surfaces[dst];unsigned char *p=s->p+(size_t)y*s->w+x;
 *p=blend_table[mapping[(alt?c>>8:c)&255]][*p];
}
static void triangle_segment(double ax,double ay,double bx,double by,int c,int left,int top,int right,int bottom){
 double dx=bx-ax,dy=by-ay,lo=0,hi=1;
 double p[4]={-dx,dx,-dy,dy},q[4]={ax-left,right-ax,ay-top,bottom-ay};
 for(int i=0;i<4;i++){
  if(p[i]==0){if(q[i]<0)return;continue;}
  double t=q[i]/p[i];if(p[i]<0){if(t>lo)lo=t;}else if(t<hi)hi=t;
  if(lo>hi)return;
 }
 int x=(int)fmax(left,fmin(right,floor(ax+lo*dx+.5))),y=(int)fmax(top,fmin(bottom,floor(ay+lo*dy+.5)));
 int x2=(int)fmax(left,fmin(right,floor(ax+hi*dx+.5))),y2=(int)fmax(top,fmin(bottom,floor(ay+hi*dy+.5)));
 int ix=abs(x2-x),sx=x<x2?1:-1,iy=-abs(y2-y),sy=y<y2?1:-1,err=ix+iy;
 for(;;){triangle_pixel(x,y,c);if(x==x2&&y==y2)break;int twice=err*2;if(twice>=iy){err+=iy;x+=sx;}if(twice<=ix){err+=ix;y+=sy;}}
}
static int triangle(lua_State *L,int filled){
 double x[3],y[3];
 for(int i=0;i<3;i++){
  double a=luaL_checknumber(L,1+i*2),b=luaL_checknumber(L,2+i*2);
  if(!isfinite(a)||!isfinite(b)||fabs(a)>1e9||fabs(b)>1e9)return luaL_error(L,"Invalid triangle coordinates");
  x[i]=a-camx;y[i]=b-camy;
 }
 int c=luaL_optint(L,7,ink);Surface *s=&surfaces[dst];
 int left=clipx>0?clipx:0,top=clipy>0?clipy:0;
 int right=(int)fmin(s->w-1,(double)clipx+clipw-1),bottom=(int)fmin(s->h-1,(double)clipy+cliph-1);
 if(left>right||top>bottom)return 0;
 double area=(x[1]-x[0])*(y[2]-y[0])-(y[1]-y[0])*(x[2]-x[0]);
 if(!filled){for(int i=0;i<3;i++)triangle_segment(x[i],y[i],x[(i+1)%3],y[(i+1)%3],c,left,top,right,bottom);return 0;}
 if(area==0){
  int a=0,b=0;double longest=-1;
  for(int i=0;i<3;i++)for(int j=i+1;j<3;j++){double dx=x[j]-x[i],dy=y[j]-y[i],length=dx*dx+dy*dy;if(length>longest){longest=length;a=i;b=j;}}
  triangle_segment(x[a],y[a],x[b],y[b],c,left,top,right,bottom);return 0;
 }
 double first=fmax(top,ceil(fmin(y[0],fmin(y[1],y[2])))),last=fmin(bottom,floor(fmax(y[0],fmax(y[1],y[2]))));
 if(first>last)return 0;
 for(int row=(int)first;row<=(int)last;row++){
  double low=1e30,high=-1e30;
  for(int i=0;i<3;i++){
   int j=(i+1)%3;if(row<fmin(y[i],y[j])||row>fmax(y[i],y[j]))continue;
   if(y[i]==y[j]){low=fmin(low,fmin(x[i],x[j]));high=fmax(high,fmax(x[i],x[j]));}
   else{double at=x[i]+(x[j]-x[i])*(row-y[i])/(y[j]-y[i]);low=fmin(low,at);high=fmax(high,at);}
  }
  double start=fmax(left,ceil(low-1e-9)),end=fmin(right,floor(high+1e-9));if(start>end)continue;
  if(!pattern&&blend_identity)memset(s->p+(size_t)row*s->w+(int)start,mapping[c&255],(size_t)(end-start+1));
  else for(int column=(int)start;column<=(int)end;column++)triangle_pixel(column,row,c);
 }
 return 0;
}
static int trifill(lua_State *L){return triangle(L,1);}
static int tri(lua_State *L){return triangle(L,0);}
static int circle(lua_State *L){double x=luaL_checknumber(L,1),y=luaL_checknumber(L,2),r=luaL_checknumber(L,3);int c=luaL_optint(L,4,ink);for(int yy=(int)(y-r);yy<=y+r;yy++)for(int xx=(int)(x-r);xx<=x+r;xx++)if((xx-x)*(xx-x)+(yy-y)*(yy-y)<=r*r)dot(xx,yy,c);return 0;}
static void blit(int sx,int sy,int sw,int sh,int dx,int dy,int dw,int dh,int fx,int fy){
 sg_perf.sprite_calls++;
 Surface *s=&surfaces[sheet],*d=&surfaces[dst];
 if(dw<0){dx+=dw;dw=-dw;fx=!fx;}if(dh<0){dy+=dh;dh=-dh;fy=!fy;}
 if(dw<=0||dh<=0||sw<=0||sh<=0)return;
 int x0=0,y0=0,x1=dw,y1=dh;
 int left=camx+(clipx>0?clipx:0),top=camy+(clipy>0?clipy:0);
 int right=camx+(clipx+clipw<d->w?clipx+clipw:d->w),bottom=camy+(clipy+cliph<d->h?clipy+cliph:d->h);
 if(dx+x0<left)x0=left-dx;if(dy+y0<top)y0=top-dy;if(dx+x1>right)x1=right-dx;if(dy+y1>bottom)y1=bottom-dy;
 if(x0>=x1||y0>=y1)return;
 if(dst==0&&output_divisor==2){
  x0+=(dx+x0-camx)&1;y0+=(dy+y0-camy)&1;
  for(int y=y0;y<y1;y+=2){
   int yy=sy+(fy?sh-1-y*sh/dh:y*sh/dh);if(yy<0||yy>=s->h)continue;
   int ty=dy+y-camy;unsigned char *row=d->p+ty*d->w;
   for(int x=x0;x<x1;x+=2){
    int xx=sx+(fx?sw-1-x*sw/dw:x*sw/dw);if(xx<0||xx>=s->w)continue;
    int c=screen_pixel(s,xx,yy),tx=dx+x-camx;sg_perf.blit_pixels++;
    if(!transparent[c]){int alt=(sprite_pattern[c]>>((ty&3)*4+(tx&3)))&1;c=mapping[(sprite_pair[c]>>(alt?8:0))&255];row[tx]=blend_identity?c:blend_table[c][row[tx]];}
   }
  }return;
 }
 /* Fast path for unscaled copies, mirrored or not: a per-call colour table
  * folds transparent/sprite_pair/mapping, so the inner loop is one lookup.
  * Same output as the scaled path below (screen_pixel only rounds when the
  * screen is the sheet at divisor 2). */
 if(dw==sw&&dh==sh&&(output_divisor==1||sheet!=0)){
  short lut[2][256];int patterned=0;
  for(int c=0;c<256;c++){
   if(transparent[c]){lut[0][c]=lut[1][c]=-1;continue;}
   lut[0][c]=mapping[sprite_pair[c]&255];lut[1][c]=mapping[(sprite_pair[c]>>8)&255];
   patterned|=sprite_pattern[c];
  }
  int xa=x0,xb=x1;
  if(fx){if(sx+sw-1-xa>=s->w)xa=sx+sw-s->w;if(sx+sw-1-(xb-1)<0)xb=sx+sw;}
  else{if(sx+xa<0)xa=-sx;if(sx+xb>s->w)xb=s->w-sx;}
  if(xa>=xb)return;
  for(int y=y0;y<y1;y++){
   int syy=sy+(fy?sh-1-y:y);if(syy<0||syy>=s->h)continue;
   int ty=dy+y-camy;const unsigned char *src=s->p+(size_t)syy*s->w+sx;
   unsigned char *row=d->p+(size_t)ty*d->w+(dx-camx);
   if(!patterned&&blend_identity&&!fx){for(int x=xa;x<xb;x++){int v=lut[0][src[x]];if(v>=0)row[x]=(unsigned char)v;}continue;}
   for(int x=xa;x<xb;x++){
    int c=src[fx?sw-1-x:x],alt=patterned?(sprite_pattern[c]>>((ty&3)*4+((dx+x-camx)&3)))&1:0,v=lut[alt][c];
    if(v>=0)row[x]=blend_identity?(unsigned char)v:blend_table[v][row[x]];
   }
  }
  sg_perf.blit_pixels+=(uint64_t)(xb-xa)*(y1-y0);return;
 }
 sg_perf.blit_pixels+=(uint64_t)(x1-x0)*(y1-y0);
 int step=sw/dw,extra=sw%dw,start=x0*sw/dw,startrem=x0*sw%dw;
 int yq=y0*sh/dh,yrem=y0*sh%dh;
 for(int y=y0;y<y1;y++){
  int yy=sy+(fy?sh-1-y*sh/dh:yq);if(yy<0||yy>=s->h){if(!fy){yrem+=sh;if(yrem>=dh){yrem-=dh;yq++;}}continue;}
  int q=start,rem=startrem,ty=dy+y-camy;unsigned char *row=d->p+ty*d->w;
  for(int x=x0;x<x1;x++){
   int xx=sx+(fx?sw-1-q:q);q+=step;rem+=extra;if(rem>=dw){rem-=dw;q++;}
   if(xx<0||xx>=s->w)continue;
   int c=screen_pixel(s,xx,yy),tx=dx+x-camx;
   if(!transparent[c]){int alt=(sprite_pattern[c]>>((ty&3)*4+(tx&3)))&1;c=mapping[(sprite_pair[c]>>(alt?8:0))&255];row[tx]=blend_identity?c:blend_table[c][row[tx]];}
  }
  if(!fy){yrem+=sh;if(yrem>=dh){yrem-=dh;yq++;} }
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
static int aspr(lua_State *L){
 int n=luaL_checkint(L,1),cols=surfaces[sheet].w/gridw;if(!cols)return 0;
 double x=luaL_checknumber(L,2),y=luaL_checknumber(L,3),angle=luaL_checknumber(L,4);
 int w=(int)(luaL_optnumber(L,5,1)*gridw),h=(int)(luaL_optnumber(L,6,1)*gridh);
 double zx=luaL_optnumber(L,7,1),zy=luaL_optnumber(L,8,1),ax=luaL_optnumber(L,9,w*.5),ay=luaL_optnumber(L,10,h*.5);
 lua_settop(L,0);lua_pushinteger(L,(n%cols)*gridw);lua_pushinteger(L,(n/cols)*gridh);
 lua_pushinteger(L,w);lua_pushinteger(L,h);lua_pushnumber(L,x);lua_pushnumber(L,y);lua_pushnumber(L,angle);
 lua_pushnumber(L,zx);lua_pushnumber(L,zy);lua_pushnumber(L,ax);lua_pushnumber(L,ay);
 return asspr(L);
}
static int circ(lua_State *L){
 double x=luaL_checknumber(L,1),y=luaL_checknumber(L,2),r=luaL_checknumber(L,3);int c=luaL_optint(L,4,ink);
 if(!isfinite(x)||!isfinite(y)||!isfinite(r)||fabs(x)>1e9||fabs(y)>1e9||r>1e6)return luaL_error(L,"Invalid circle coordinates");
 if(r<0)return 0;
 Surface *s=&surfaces[dst];double cx=x-camx,cy=y-camy;
 double l=fmax(0,fmax(clipx,ceil(cx-r-.5))),rr=fmin(s->w-1,fmin((double)clipx+clipw-1,floor(cx+r+.5)));
 double tt=fmax(0,fmax(clipy,ceil(cy-r-.5))),bb=fmin(s->h-1,fmin((double)clipy+cliph-1,floor(cy+r+.5)));
 if(l>rr||tt>bb)return 0;int left=(int)l,right=(int)rr,top=(int)tt,bottom=(int)bb;
 double inner=fmax(0,r-.5),outer=r+.5;
 for(int yy=top;yy<=bottom;yy++)for(int xx=left;xx<=right;xx++){double d=(xx-cx)*(xx-cx)+(yy-cy)*(yy-cy);if(d>=inner*inner&&d<=outer*outer)triangle_pixel(xx,yy,c);}
 return 0;
}
static int spr(lua_State *L){int n=luaL_checkint(L,1),cols=surfaces[sheet].w/gridw;if(!cols)return 0;int w=luaL_optint(L,4,1)*gridw,h=luaL_optint(L,5,1)*gridh;blit((n%cols)*gridw,(n/cols)*gridh,w,h,luaL_checkint(L,2),luaL_checkint(L,3),w,h,lua_toboolean(L,6),lua_toboolean(L,7));return 0;}
static int sget(lua_State *L){Surface *s=&surfaces[sheet];int x=luaL_checkint(L,1),y=luaL_checkint(L,2);lua_pushinteger(L,x>=0&&y>=0&&x<s->w&&y<s->h?screen_pixel(s,x,y):0);return 1;}
static int size(lua_State *L){int id=lua_gettop(L)?locate(L,luaL_checkstring(L,1)):dst;lua_pushinteger(L,surfaces[id].w);lua_pushinteger(L,surfaces[id].h);return 2;}
static int pal(lua_State *L){int n=lua_gettop(L);unsigned char *map=lua_toboolean(L,3)?displaymap:mapping;if(!n){for(int i=0;i<256;i++){mapping[i]=i;displaymap[i]=i;}return 0;}if(lua_isnil(L,1))return 0;if(lua_istable(L,1)){map=lua_toboolean(L,2)?displaymap:mapping;lua_pushnil(L);while(lua_next(L,1)){map[lua_tointeger(L,-2)&255]=lua_tointeger(L,-1)&255;lua_pop(L,1);}}else map[luaL_checkint(L,1)&255]=luaL_checkint(L,2)&255;return 0;}
static int palette_reset(lua_State *L){
 sg_perf.palette_resets++;
 /* The game temporarily replaces pal with a no-op while drawing pause layers. */
 lua_getglobal(L,"pal");int enabled=lua_iscfunction(L,-1)&&lua_tocfunction(L,-1)==pal;lua_pop(L,1);
 if(enabled){for(int i=0;i<10;i++)mapping[i]=displaymap[i]=i;}
 else {
  sg_perf.palette_suppressed++;
  /* Preserve custom/pause Lua replacements instead of assuming they are no-ops. */
  for(int i=0;i<10;i++){
   lua_getglobal(L,"pal");lua_pushinteger(L,i);lua_pushinteger(L,i);lua_call(L,2,0);
   lua_getglobal(L,"pal");lua_pushinteger(L,i);lua_pushinteger(L,i);lua_pushinteger(L,1);lua_call(L,3,0);
  }
 }
 for(int i=1;i<10;i++)transparent[i]=0;transparent[0]=1;
 sprite_pattern[1]=0;sprite_pair[1]=1;
 return 0;
}
static int palt(lua_State *L){if(!lua_gettop(L)){memset(transparent,0,256);transparent[0]=1;}else transparent[luaL_checkint(L,1)&255]=lua_toboolean(L,2);return 0;}
static int newwin(lua_State *L){create(L,"screen",luaL_checkint(L,2),luaL_checkint(L,3));dst=0;return 0;}
static int delsrf(lua_State *L){int id=locate(L,luaL_checkstring(L,1));if(id==0)return luaL_error(L,"Cannot delete screen");allocated-=(size_t)surfaces[id].w*surfaces[id].h;free(surfaces[id].p);surfaces[id].p=NULL;surfaces[id].w=surfaces[id].h=0;return 0;}
static int noop(lua_State *L){(void)L;return 0;}
static int fpslimit(lua_State *L){if(lua_gettop(L))sg_set_fps_limit(luaL_checkint(L,1));lua_pushinteger(L,sg_get_fps_limit());return 1;}
static int fillp(lua_State *L){pattern=luaL_optint(L,1,0);pattern_transparent=lua_toboolean(L,2);return 0;}
static int sfillp(lua_State *L){if(!lua_gettop(L)){for(int i=0;i<256;i++){sprite_pattern[i]=0;sprite_pair[i]=i|(i<<8);}}else{int c=luaL_checkint(L,1)&255;sprite_pattern[c]=luaL_checkint(L,2);sprite_pair[c]=luaL_checkint(L,3);}return 0;}
static int clip(lua_State *L){clipx=luaL_optint(L,1,0);clipy=luaL_optint(L,2,0);clipw=luaL_optint(L,3,4096);cliph=luaL_optint(L,4,4096);if(lua_toboolean(L,5)){clipx-=camx;clipy-=camy;}return 0;}
static int rect(lua_State *L){int x=luaL_checkint(L,1),y=luaL_checkint(L,2),x2=luaL_checkint(L,3),y2=luaL_checkint(L,4),c=luaL_optint(L,5,ink);for(int xx=x;xx<=x2;xx++){dot(xx,y,c);dot(xx,y2,c);}for(int yy=y;yy<=y2;yy++){dot(x,yy,c);dot(x2,yy,c);}return 0;}
static int pset(lua_State *L){dot(luaL_checkint(L,1),luaL_checkint(L,2),luaL_optint(L,3,ink));return 0;}
static int pget(lua_State *L){Surface *s=&surfaces[dst];int x=luaL_checkint(L,1)-camx,y=luaL_checkint(L,2)-camy;lua_pushinteger(L,x>=0&&y>=0&&x<s->w&&y<s->h?screen_pixel(s,x,y):0);return 1;}
static int shadow_batch(lua_State *L){
 sg_perf.shadow_calls++;
 lua_getglobal(L,"bright");int compatible=lua_rawequal(L,-1,lua_upvalueindex(2));lua_pop(L,1);
 const char *names[]={"sget","pget","pset"};lua_CFunction funcs[]={sget,pget,pset};
 for(int i=0;i<3&&compatible;i++){lua_getglobal(L,names[i]);compatible=lua_iscfunction(L,-1)&&lua_tocfunction(L,-1)==funcs[i];lua_pop(L,1);}
 if(!compatible){sg_perf.shadow_fallbacks++;int n=lua_gettop(L);lua_pushvalue(L,lua_upvalueindex(1));lua_insert(L,1);lua_call(L,n,LUA_MULTRET);return lua_gettop(L);}
 double a[6];for(int i=0;i<6;i++){a[i]=luaL_checknumber(L,i+1);if(!isfinite(a[i])||fabs(a[i])>1e9)return luaL_error(L,"Invalid shadow coordinates");}
 if(a[2]<1||a[3]<1)return 0;
 if(a[2]>4096||a[3]>4096)return luaL_error(L,"Shadow region too large");
 double inc=lua_isnoneornil(L,7)||(lua_isboolean(L,7)&&!lua_toboolean(L,7))?-1:luaL_checknumber(L,7);
 if(!isfinite(inc))return luaL_error(L,"Invalid shadow shade");
 int w=(int)a[2],h=(int)a[3],shade=(int)fmin(8,fmax(1,4+inc));
 Surface *s=&surfaces[sheet],*d=&surfaces[dst];
 /* Clip the shadow rectangle before sampling the mask. dot() would discard
    these pixels later, but doing so here avoids source reads and palette work. */
 int x0=(int)ceil(camx-a[4]),y0=(int)ceil(camy-a[5]);
 int x1=(int)floor(camx+d->w-a[4]),y1=(int)floor(camy+d->h-a[5]);
 if(x0<0)x0=0;if(y0<0)y0=0;if(x1>w)x1=w;if(y1>h)y1=h;
 for(int dx=x0;dx<x1;dx++)for(int dy=y0;dy<y1;dy++){
  int sx=(int)(a[0]+dx),sy=(int)(a[1]+dy);if(sx<0||sy<0||sx>=s->w||sy>=s->h||!screen_pixel(s,sx,sy))continue;
  int wx=(int)(a[4]+dx),wy=(int)(a[5]+dy),x=wx-camx,y=wy-camy;
  int color=x>=0&&y>=0&&x<d->w&&y<d->h?screen_pixel(d,x,y):0,bx=16+color;
  int out=bx<s->w&&shade<s->h?screen_pixel(s,bx,shade):0;
  dot(wx,wy,out);sg_perf.shadow_pixels++;
 }
 return 0;
}
/* The game "restores" entries with blend(a,b,a) instead of blend(), so count
 * non-identity entries; otherwise every later blit stays on the slow path. */
static int blend(lua_State *L){if(!lua_gettop(L)){blend_identity=1;blend_changed=0;for(int i=0;i<256;i++)for(int j=0;j<256;j++)blend_table[i][j]=i;}else {int a=luaL_checkint(L,1)&255,b=luaL_checkint(L,2)&255,v=luaL_checkint(L,3)&255;blend_changed+=(v!=a)-(blend_table[a][b]!=a);blend_table[a][b]=v;blend_identity=!blend_changed;}return 0;}
static int exe(lua_State *L){int d=dst,s=sheet,cx=camx,cy=camy,p=pattern,tr=pattern_transparent,ic=ink,cx0=clipx,cy0=clipy,cw=clipw,ch=cliph,f=current_font;unsigned char map[256];memcpy(map,mapping,256);lua_pushvalue(L,1);int status=lua_pcall(L,0,0,0);dst=d;sheet=s;camx=cx;camy=cy;pattern=p;pattern_transparent=tr;ink=ic;clipx=cx0;clipy=cy0;clipw=cw;cliph=ch;current_font=f;memcpy(mapping,map,256);if(status)return lua_error(L);return 0;}
static int patterns(lua_State *L){lua_newtable(L);int x=luaL_checkint(L,1),y=luaL_checkint(L,2),n=luaL_checkint(L,3);for(int i=0;i<n;i++){Surface *s=&surfaces[sheet];int value=0;for(int yy=0;yy<4;yy++)for(int xx=0;xx<4;xx++)if(s->p[(y+yy)*s->w+x+i*4+xx])value|=1<<(yy*4+xx);lua_pushinteger(L,value);lua_rawseti(L,-2,i+1);}return 1;}
static int newfont(lua_State *L){const char *path=luaL_checkstring(L,1),*key=luaL_optstring(L,3,"default");int id;for(id=0;id<fontcount;id++)if(!strcmp(fonts[id].key,key))break;if(id==fontcount){if(fontcount==16)return luaL_error(L,"Font limit");fontcount++;}Font *f=&fonts[id];free_glyphs(f);free(f->data);memset(f,0,sizeof(*f));for(int i=0;i<256;i++)f->glyph_index[i]=-1;snprintf(f->key,sizeof(f->key),"%s",key);f->space=4;f->h=8;
 if(strstr(path,".ttf")){char full[2048];snprintf(full,sizeof(full),"%s/%s",data_root,path);FILE *file=fopen(full,"rb");if(!file)return luaL_error(L,"Missing font %s",full);fseek(file,0,SEEK_END);long n=ftell(file);rewind(file);if(n<1||n>8*1024*1024){fclose(file);return luaL_error(L,"Font size limit");}f->data=malloc(n);if(!f->data){fclose(file);return luaL_error(L,"Font allocation failed");}size_t got=fread(f->data,1,n,file);fclose(file);if(got!=(size_t)n||!stbtt_InitFont(&f->info,f->data,0))return luaL_error(L,"Invalid font");f->ttf=1;f->h=luaL_checkint(L,2);f->scale=stbtt_ScaleForPixelHeight(&f->info,(float)f->h);
 }else{char name[100];snprintf(name,sizeof(name),"font_%s",key);f->surface=load(L,path,name,0);Surface *s=&surfaces[f->surface];const unsigned char *chars=(const unsigned char *)luaL_checkstring(L,2);int x=0;while(*chars&&x<s->w&&f->glyphs<256){while(x<s->w){int used=0;for(int y=0;y<s->h;y++)used|=s->p[y*s->w+x];if(used)break;x++;}int start=x;while(x<s->w){int used=0;for(int y=0;y<s->h;y++)used|=s->p[y*s->w+x];if(!used)break;x++;}int g=f->glyphs++;f->cp[g]=codepoint(&chars);f->gx[g]=start;f->gw[g]=x-start;if(f->cp[g]<256)f->glyph_index[f->cp[g]]=g;}f->h=s->h+1;}
 lua_pushstring(L,key);return 1;}
static int selectfont(lua_State *L){const char *key=luaL_optstring(L,1,"default");for(int i=0;i<fontcount;i++)if(!strcmp(fonts[i].key,key)){current_font=i;return 0;}return luaL_error(L,"Unknown font %s",key);}
static unsigned codepoint(const unsigned char **text){const unsigned char *p=*text;unsigned c=*p++;if(c>=0xc0&&c<0xe0&&*p)c=((c&31)<<6)|(*p++&63);else if(c>=0xe0&&c<0xf0&&p[0]&&p[1]){c=((c&15)<<12)|((p[0]&63)<<6)|(p[1]&63);p+=2;}*text=p;return c;}
static int advance(Font *f,unsigned c){if(c==' ')return f->space;if(f->ttf)return cached_glyph(f,c)->advance;if(c<256&&f->glyph_index[c]>=0)return f->gw[f->glyph_index[c]]+1;return 4;}
static void stamp_mask(const unsigned char *mask,int stride,int w,int h,int x,int y,int threshold,int c){
 Surface *s=&surfaces[dst];x-=camx;y-=camy;
 int x0=0,y0=0,x1=w,y1=h;
 int left=clipx>0?clipx:0,top=clipy>0?clipy:0;
 int right=clipx+clipw<s->w?clipx+clipw:s->w,bottom=clipy+cliph<s->h?clipy+cliph:s->h;
 if(x<left)x0=left-x;if(y<top)y0=top-y;if(x+x1>right)x1=right-x;if(y+y1>bottom)y1=bottom-y;
 if(x0>=x1||y0>=y1)return;
 int value=mapping[c&255],fast=!pattern&&blend_identity;
 int step=dst==0?output_divisor:1;if(step==2){x0+=(x+x0)&1;y0+=(y+y0)&1;}
 for(int yy=y0;yy<y1;yy+=step){
  const unsigned char *source=mask+(size_t)yy*stride;unsigned char *row=s->p+(size_t)(y+yy)*s->w+(x+x0);
  for(int xx=x0;xx<x1;xx+=step,row+=step)if(source[xx]>=threshold){if(fast)*row=value;else triangle_pixel(x+xx,y+yy,c);}
 }
}
static int drawtext(const char *str,int x,int y,int c,int draw){Font *f=&fonts[current_font];int start=x;const unsigned char *p=(const unsigned char *)str;while(*p){unsigned cp=codepoint(&p);if(cp=='\n'){x=start;y+=f->h;continue;}if(draw&&cp!=' '){if(f->ttf){Glyph *g=cached_glyph(f,cp);if(g->p)stamp_mask(g->p,g->w,g->w,g->h,x+g->ox,y+f->h+g->oy+f->dy,100,c);}else if(cp<256&&f->glyph_index[cp]>=0){int g=f->glyph_index[cp];Surface *s=&surfaces[f->surface];stamp_mask(s->p+f->gx[g],s->w,f->gw[g],s->h-1,x,y,1,c);}}x+=advance(f,cp);}return x;}
static int text(lua_State *L){const char *str=luaL_checkstring(L,1);if(lua_gettop(L)<3){fprintf(stdout,"%s\n",str);lua_pushinteger(L,0);return 1;}int x=luaL_checkint(L,2),y=luaL_checkint(L,3),c=luaL_optint(L,4,ink);lua_pushinteger(L,drawtext(str,x,y,c,1));return 1;}
static int width(lua_State *L){lua_pushinteger(L,drawtext(luaL_checkstring(L,1),0,0,ink,0));return 1;}
static int bprint(lua_State *L){const char *str=luaL_checkstring(L,1);int start=luaL_checkint(L,2),x=start,y=luaL_checkint(L,3),w=luaL_checkint(L,4),c=luaL_optint(L,5,ink);const unsigned char *p=(const unsigned char *)str;while(*p){const unsigned char *begin=p;unsigned cp=codepoint(&p);int a=advance(&fonts[current_font],cp);if(cp=='\n'||x+a>start+w){x=start;y+=fonts[current_font].h;if(cp=='\n')continue;}char glyph[8]={0};size_t n=(size_t)(p-begin);if(n<sizeof(glyph))memcpy(glyph,begin,n);drawtext(glyph,x,y,c,1);x+=a;}lua_pushinteger(L,y+fonts[current_font].h);return 1;}
static int fntspec(lua_State *L){Font *f=&fonts[current_font];const char *op=luaL_checkstring(L,1);if(!strcmp(op,"name")){lua_pushstring(L,f->key);return 1;}int *n=!strcmp(op,"dy")?&f->dy:!strcmp(op,"space")?&f->space:&f->h;if(lua_gettop(L)>1)*n=luaL_checkint(L,2);lua_pushinteger(L,*n);return 1;}
static int measured_graphics(lua_State*L){
 lua_CFunction f=lua_tocfunction(L,lua_upvalueindex(1));int group=lua_tointeger(L,lua_upvalueindex(2));
 sg_perf.gfx_calls[group]++;if(!sg_perf.gfx_sample)return f(L);
 uint64_t start=sg_now_us();int result=f(L);sg_perf.gfx_us[group]+=sg_now_us()-start;return result;
}
static void reg(lua_State *L,const char *name,lua_CFunction f){
 int group=-1;
 if(f==sspr||f==spr)group=0;else if(f==asspr||f==aspr)group=1;
 else if(f==rectfill||f==circle||f==circ||f==tri||f==trifill||f==line||f==rect)group=2;
 else if(f==text||f==bprint)group=3;
 lua_pushcfunction(L,f);if(group>=0){lua_pushinteger(L,group);lua_pushcclosure(L,measured_graphics,2);}lua_setglobal(L,name);
}
void sg_graphics_init(lua_State *L,const char *root){snprintf(data_root,sizeof(data_root),"%s",root);create(L,"screen",320,180);for(int i=0;i<256;i++){mapping[i]=i;displaymap[i]=i;colors[i]=0xff000000u|(i*0x010101u);}transparent[0]=1;
 for(int i=0;i<256;i++){sprite_pattern[i]=0;sprite_pair[i]=i|(i<<8);}
 for(int i=0;i<256;i++)for(int j=0;j<256;j++)blend_table[i][j]=i;
 reg(L,"newsrf",newsrf);reg(L,"palette",palette);reg(L,"target",target);reg(L,"spritesheet",spritesheet);reg(L,"sprgrid",sprgrid);reg(L,"camera",camera);reg(L,"tcamera",tcamera);reg(L,"cls",cls);reg(L,"color",color);reg(L,"rectfill",rectfill);reg(L,"line",line);reg(L,"circfill",circle);reg(L,"sspr",sspr);reg(L,"asspr",asspr);reg(L,"spr",spr);reg(L,"sget",sget);reg(L,"srfsize",size);reg(L,"pal",pal);reg(L,"palt",palt);reg(L,"newwin",newwin);reg(L,"get_patterns",patterns);reg(L,"winspec",noop);reg(L,"fpslimit",fpslimit);reg(L,"shdrf",noop);
 reg(L,"newfnt",newfont);reg(L,"font",selectfont);reg(L,"strwidth",width);reg(L,"fntspec",fntspec);reg(L,"print",text);
 reg(L,"clip",clip);reg(L,"delsrf",delsrf);reg(L,"fillp",fillp);reg(L,"rect",rect);reg(L,"pset",pset);reg(L,"exe",exe);
 reg(L,"sfillp",sfillp);reg(L,"bprint",bprint);
 reg(L,"pget",pget);reg(L,"blend",blend);
 reg(L,"trifill",trifill);reg(L,"tri",tri);
 reg(L,"aspr",aspr);reg(L,"circ",circ);
 /* Default font uses the actual shipped TTF before the game selects its font. */
 lua_pushcfunction(L,newfont);lua_pushstring(L,"assets/fonts/Terminus.ttf");lua_pushinteger(L,12);lua_pushstring(L,"default");lua_call(L,3,0);
}
void sg_present_rows(uint32_t *p,int width,int height,int first,int last){
 Surface *s=&surfaces[0];if(first<0)first=0;if(last>height)last=height;if(first>=last)return;
 if(!s->p||!s->w||!s->h){memset(p+(size_t)first*width,0,(size_t)(last-first)*width*4);return;}
 int rw=(s->w+output_divisor-1)/output_divisor,rh=(s->h+output_divisor-1)/output_divisor;
 int scale=width/rw;if(height/rh<scale)scale=height/rh;if(scale<1)scale=1;
 int ox=(width-rw*scale)/2,oy=(height-rh*scale)/2,overlay=-1,previous=-1;
 uint32_t palette[256];for(int i=0;i<256;i++)palette[i]=colors[displaymap[i]];
 for(int i=0;i<count;i++)if(!strcmp(surfaces[i].key,"hdtext")&&surfaces[i].p)overlay=i;
 /* Logical-size GPU path: precompute overlay coordinates instead of dividing per pixel. */
 if(width==rw&&height==rh){
  Surface *o=overlay>=0?&surfaces[overlay]:NULL;int xs[4096];
  if(o)for(int x=0;x<rw;x++)xs[x]=x*output_divisor*o->w/s->w;
  for(int y=first;y<last;y++){
   const unsigned char *base=s->p+(size_t)(y*output_divisor)*s->w;
   const unsigned char *ov=o?o->p+(size_t)(y*output_divisor*o->h/s->h)*o->w:NULL;
   uint32_t *row=p+(size_t)y*width;
   if(ov)for(int x=0;x<width;x++){int v=ov[xs[x]];row[x]=palette[v==255?base[x*output_divisor]:v];}
   else for(int x=0;x<width;x++)row[x]=palette[base[x*output_divisor]];
   sg_ui_row(row,width,height,y);
  }
  return;
 }
 for(int y=first;y<last;y++){
  uint32_t *row=p+(size_t)y*width;int yy=(y-oy)/scale;
  if(y<oy||yy>=rh){for(int x=0;x<width;x++)row[x]=palette[0];previous=-1;continue;}
  if(yy==previous){memcpy(row,row-width,(size_t)width*4);sg_ui_row(row,width,height,y);continue;}
  previous=yy;for(int x=0;x<width;x++)row[x]=palette[0];
  Surface *o=overlay>=0?&surfaces[overlay]:NULL;int py=o?yy*output_divisor*o->h/s->h:0;
  for(int x=0;x<rw;x++){
   int sx=x*output_divisor,sy=yy*output_divisor;
   int c=s->p[sy*s->w+sx];if(o){int v=o->p[py*o->w+sx*o->w/s->w];if(v!=255)c=v;}
   int start=ox+x*scale,end=start+scale;if(start<0)start=0;if(end>width)end=width;
   uint32_t value=palette[c];for(int xx=start;xx<end;xx++)row[xx]=value;
  }
  sg_ui_row(row,width,height,y);
 }
}
void sg_present(uint32_t *p,int width,int height){sg_present_rows(p,width,height,0,height);}
uint32_t sg_present_background(void){return colors[displaymap[0]];}

void sg_graphics_close(void){for(int i=0;i<count;i++)free(surfaces[i].p);for(int i=0;i<fontcount;i++){free_glyphs(&fonts[i]);free(fonts[i].data);}memset(fonts,0,sizeof(fonts));fontcount=current_font=0;memset(surfaces,0,sizeof(surfaces));count=0;allocated=0;dst=sheet=0;}

/* toolkit.lua apal(n) is `for i=0,#palette() do pal(i,n) end`: a fresh
 * 256-entry table plus 257 pal calls each time. Same result in one memset,
 * unless the game has swapped pal/palette (pause layers do), then defer. */
static int apal_batch(lua_State *L){
 lua_getglobal(L,"pal");int native=lua_iscfunction(L,-1)&&lua_tocfunction(L,-1)==pal;lua_pop(L,1);
 lua_getglobal(L,"palette");native=native&&lua_iscfunction(L,-1)&&lua_tocfunction(L,-1)==palette;lua_pop(L,1);
 if(!native||lua_gettop(L)<1){int n=lua_gettop(L);lua_pushvalue(L,lua_upvalueindex(1));lua_insert(L,1);lua_call(L,n,LUA_MULTRET);return lua_gettop(L);}
 memset(mapping,luaL_checkint(L,1)&255,256);return 0;
}
/* code.lua upe() counter pass: every "c_*" field moves one step toward 0
 * (or up with rev_c) and is cleared when it reaches 0. Walking the table in C
 * avoids interpreted pairs() over every field of every entity each frame. */
static int tick_counters(lua_State *L){
 luaL_checktype(L,1,LUA_TTABLE);lua_Number dir=lua_toboolean(L,2)?1:-1;
 lua_pushnil(L);
 while(lua_next(L,1)){
  if(lua_type(L,-2)==LUA_TSTRING){
   size_t n;const char *k=lua_tolstring(L,-2,&n);
   if(n>=2&&k[0]=='c'&&k[1]=='_'){
    if(!lua_isnumber(L,-1))return luaL_error(L,"attempt to perform arithmetic on field '%s'",k);
    lua_Number v=lua_tonumber(L,-1)+dir;
    lua_pushvalue(L,-2);if(v>0)lua_pushnumber(L,v);else lua_pushnil(L);lua_rawset(L,1);
   }
  }
  lua_pop(L,1);
 }
 return 0;
}
/* toolkit.lua split(str,sep) rebuilt with sub() per byte and re-cut the rest of
 * the string after each piece, quadratic in the text length; it showed up in
 * the turn-processing spikes. Matches its edge cases exactly: "" gives {},
 * sep "" gives one byte per item, a trailing sep adds a final "". */
static int split_exact(lua_State *L){
 size_t n,m;const char *s=luaL_checklstring(L,1,&n),*sep=luaL_checklstring(L,2,&m);
 lua_newtable(L);int k=1;
 if(!m){for(size_t i=0;i<n;i++){lua_pushlstring(L,s+i,1);lua_rawseti(L,-2,k++);}if(n){lua_pushliteral(L,"");lua_rawseti(L,-2,k);}return 1;}
 const char *p=s,*end=s+n;
 while(p<end){
  const char *q=p;while(q+m<=end&&memcmp(q,sep,m))q++;
  if(q+m>end){lua_pushlstring(L,p,(size_t)(end-p));lua_rawseti(L,-2,k++);break;}
  lua_pushlstring(L,p,(size_t)(q-p));lua_rawseti(L,-2,k++);p=q+m;
  if(p==end){lua_pushliteral(L,"");lua_rawseti(L,-2,k++);}
 }
 return 1;
}
/* Board distance BFS for code.lua trace_hdist/trace_piece_dist. Unit-cost BFS
 * gives shortest distances whatever the visit order, so only the neighbour
 * rules of each Lua gnx() need to match. Interpreted, each call cost ~10 ms
 * on Vita and a turn runs dozens of them.
 * sg_trace_dist(key|nil, starts, mode, self|nil, dgr|nil)
 * mode: {"walk"} {"free4"} {"diag8"} {"knight"} {"lines",a,b,dmin,dmax}
 * Writes sq.dist and sq[key], or dgr[sq] when dgr is given; unreached = 99. */
#define DIST_MAX 4096
static int dist_value[DIST_MAX],dist_queue[DIST_MAX*8],dist_px[DIST_MAX],dist_py[DIST_MAX];
static unsigned char dist_exists[DIST_MAX],dist_free[DIST_MAX],dist_walk[DIST_MAX],dist_moat[DIST_MAX];
static int dist_int(lua_State *L,int idx,int i,int def){lua_rawgeti(L,idx,i);int v=lua_isnumber(L,-1)?(int)lua_tointeger(L,-1):def;lua_pop(L,1);return v;}
static int trace_dist(lua_State *L){
 int has_key=lua_type(L,1)==LUA_TSTRING,has_self=!lua_isnoneornil(L,4),has_dgr=lua_istable(L,5);
 luaL_checktype(L,2,LUA_TTABLE);luaL_checktype(L,3,LUA_TTABLE);
 if(!has_key&&!has_dgr)return luaL_error(L,"sg_trace_dist needs a key or dgr");
 lua_getglobal(L,"xmax");int xm=(int)lua_tointeger(L,-1);lua_getglobal(L,"ymax");int ym=(int)lua_tointeger(L,-1);lua_pop(L,2);
 if(xm<=0||ym<=0||xm*ym>DIST_MAX)return luaL_error(L,"sg_trace_dist board %dx%d",xm,ym);
 int n=xm*ym;
 lua_getglobal(L,"squares");int squares=lua_gettop(L);if(!lua_istable(L,squares))return luaL_error(L,"sg_trace_dist: squares");
 lua_getglobal(L,"fly");int fly=lua_toboolean(L,-1);lua_pop(L,1);
 int dirs[16],knight[16];
 lua_getglobal(L,"DIRS");for(int i=0;i<16;i++)dirs[i]=lua_istable(L,-1)?dist_int(L,lua_gettop(L),i+1,0):0;lua_pop(L,1);
 lua_getglobal(L,"KNIGHT_MOVES");for(int i=0;i<16;i++)knight[i]=lua_istable(L,-1)?dist_int(L,lua_gettop(L),i+1,0):0;lua_pop(L,1);
 lua_rawgeti(L,3,1);const char *mode=lua_tostring(L,-1);lua_pop(L,1);if(!mode)return luaL_error(L,"sg_trace_dist: mode");
 int la=dist_int(L,3,2,0),lb=dist_int(L,3,3,0),dmin=dist_int(L,3,4,1),dmax=dist_int(L,3,5,12);
 for(int i=0;i<n;i++){
  lua_rawgeti(L,squares,i+1);dist_exists[i]=lua_istable(L,-1);dist_free[i]=dist_walk[i]=dist_moat[i]=0;dist_value[i]=99;
  if(dist_exists[i]){
   int sq=lua_gettop(L);
   lua_getfield(L,sq,"px");dist_px[i]=(int)lua_tointeger(L,-1);lua_getfield(L,sq,"py");dist_py[i]=(int)lua_tointeger(L,-1);lua_pop(L,2);
   lua_getfield(L,sq,"p");int p=lua_gettop(L),hasp=lua_toboolean(L,p),type6=0,self=0;
   if(hasp){if(lua_istable(L,p)){lua_getfield(L,p,"type");type6=lua_type(L,-1)==LUA_TNUMBER&&lua_tonumber(L,-1)==6;lua_pop(L,1);}self=has_self&&lua_rawequal(L,p,4);}
   lua_getfield(L,sq,"reserved");int reserved=lua_toboolean(L,-1);lua_getfield(L,sq,"moat");dist_moat[i]=(unsigned char)lua_toboolean(L,-1);lua_pop(L,3);
   int is_free=!hasp&&!reserved;
   dist_free[i]=(unsigned char)(is_free||self);dist_walk[i]=(unsigned char)(!hasp||type6);
  }
  lua_pop(L,1);
 }
 /* gsq(x,y): squares[1+x*ymax+y] inside the board, else nil. */
 #define DIST_AT(x,y) (((x)>=0&&(x)<xm&&(y)>=0&&(y)<ym&&dist_exists[(x)*ym+(y)])?(x)*ym+(y):-1)
 int head=0,tail=0;
 lua_pushnil(L);
 while(lua_next(L,2)){
  if(lua_istable(L,-1)){lua_getfield(L,-1,"px");int x=(int)lua_tointeger(L,-1);lua_getfield(L,-2,"py");int y=(int)lua_tointeger(L,-1);lua_pop(L,2);int j=DIST_AT(x,y);if(j>=0&&tail<DIST_MAX*8){dist_value[j]=0;dist_queue[tail++]=j;}}
  lua_pop(L,1);
 }
 while(head<tail){
  int i=dist_queue[head++],d=dist_value[i]+1,x=dist_px[i],y=dist_py[i];
  int cand[200],nc=0;
  if(!strcmp(mode,"walk")||!strcmp(mode,"free4")){int walk=mode[0]=='w';for(int di=0;di<4;di++){int j=DIST_AT(x+dirs[di*2],y+dirs[di*2+1]);if(j>=0&&(walk?dist_walk[j]:dist_free[j]))cand[nc++]=j;}}
  else if(!strcmp(mode,"diag8")){for(int di=0;di<8;di++){int j=DIST_AT(x+dirs[di*2],y+dirs[di*2+1]);if(j>=0)cand[nc++]=j;}}
  else if(!strcmp(mode,"knight")){for(int di=0;di<8;di++){int j=DIST_AT(x+knight[di*2],y+knight[di*2+1]);if(j>=0&&dist_free[j])cand[nc++]=j;}}
  else if(!strcmp(mode,"lines")){
   for(int di=la;di<=lb&&di<8;di++)for(int k=dmin;k<=dmax;k++){
    int j=DIST_AT(x+dirs[di*2]*k,y+dirs[di*2+1]*k);
    if(j>=0&&dist_free[j]){if(nc<200)cand[nc++]=j;if(dist_moat[j])break;}
    else if(!(j>=0&&fly))break;
   }
  }
  else return luaL_error(L,"sg_trace_dist: unknown mode %s",mode);
  for(int c=0;c<nc;c++){int j=cand[c];if(dist_value[j]>d&&tail<DIST_MAX*8){dist_value[j]=d;dist_queue[tail++]=j;}}
 }
 #undef DIST_AT
 for(int i=0;i<n;i++){
  if(!dist_exists[i])continue;
  lua_rawgeti(L,squares,i+1);int sq=lua_gettop(L);
  if(has_dgr){lua_pushvalue(L,sq);lua_pushinteger(L,dist_value[i]);lua_settable(L,5);}
  else{lua_pushinteger(L,dist_value[i]);lua_setfield(L,sq,"dist");lua_pushinteger(L,dist_value[i]);lua_setfield(L,sq,lua_tostring(L,1));}
  lua_pop(L,1);
 }
 return 0;
}
/* Combat-frame helpers, chosen from the time-weighted line profile. Each one
 * reproduces the Lua it replaces; code.lua keeps the Lua path as fallback. */

/* code.lua sfillp_rst(): for i=0,5 do sfillp(i,0,i) end. sfillp_dissolve(c):
 * for i=0,5 do sfillp(i,pat[1+flr(mid(0,c,1)*15)],i) end. Called around most
 * piece draws. Falls back to the original when sfillp is no longer native. */
static int sfillp_native(lua_State *L){lua_getglobal(L,"sfillp");int ok=lua_tocfunction(L,-1)==sfillp;lua_pop(L,1);return ok;}
static int sfillp_rst_fast(lua_State *L){
 if(!sfillp_native(L)){lua_pushvalue(L,lua_upvalueindex(1));lua_insert(L,1);lua_call(L,lua_gettop(L)-1,LUA_MULTRET);return lua_gettop(L);}
 for(int i=0;i<=5;i++){sprite_pattern[i]=0;sprite_pair[i]=(unsigned short)i;}
 return 0;
}
static int sfillp_dissolve_fast(lua_State *L){
 if(!sfillp_native(L)){lua_pushvalue(L,lua_upvalueindex(1));lua_insert(L,1);lua_call(L,lua_gettop(L)-1,LUA_MULTRET);return lua_gettop(L);}
 double c=luaL_checknumber(L,1),m=fmax(fmin(0,c),fmin(fmax(0,c),1));
 lua_getglobal(L,"pat");if(!lua_istable(L,-1))return luaL_error(L,"attempt to index global 'pat' (a nil value)");
 lua_rawgeti(L,-1,1+(int)floor(m*15));int v=luaL_checkint(L,-1);lua_pop(L,2);
 for(int i=0;i<=5;i++){sprite_pattern[i]=(unsigned short)v;sprite_pair[i]=(unsigned short)i;}
 return 0;
}
/* code.lua _draw depth list: a={} for i=0,15 do add(a,{}) end
 * for e in all(ents) do add(a[e.dp+1],e) end */
static int depth_lists(lua_State *L){
 luaL_checktype(L,1,LUA_TTABLE);
 lua_createtable(L,16,0);int a=lua_gettop(L);int len[16]={0};
 for(int i=1;i<=16;i++){lua_newtable(L);lua_rawseti(L,a,i);}
 for(int i=1;;i++){
  lua_rawgeti(L,1,i);if(lua_isnil(L,-1)){lua_pop(L,1);break;}
  int e=lua_gettop(L);lua_getfield(L,e,"dp");
  if(!lua_isnumber(L,-1))return luaL_error(L,"attempt to perform arithmetic on field 'dp'");
  lua_Number dp=lua_tonumber(L,-1)+1;lua_pop(L,1);
  int slot=(int)dp;if((lua_Number)slot!=dp||slot<1||slot>16)return luaL_error(L,"bad argument #1 to 'add' (table expected, got nil)");
  lua_rawgeti(L,a,slot);lua_pushvalue(L,e);lua_rawseti(L,-2,++len[slot-1]);lua_pop(L,2);
 }
 return 1;
}
/* code.lua upe() physics block (velocity, friction, gravity, shake decay). */
static lua_Number upe_num(lua_State *L,const char *k){
 lua_getfield(L,1,k);if(!lua_isnumber(L,-1))luaL_error(L,"attempt to perform arithmetic on field '%s'",k);
 lua_Number v=lua_tonumber(L,-1);lua_pop(L,1);return v;
}
static void upe_set(lua_State *L,const char *k,lua_Number v){lua_pushnumber(L,v);lua_setfield(L,1,k);}
static int upe_phys(lua_State *L){
 luaL_checktype(L,1,LUA_TTABLE);
 upe_set(L,"t",upe_num(L,"t")+1);
 lua_Number frict=upe_num(L,"frict"),vx=upe_num(L,"vx")*frict,vy;
 upe_set(L,"vx",vx);vy=upe_num(L,"vy")*frict;upe_set(L,"vy",vy);
 upe_set(L,"x",upe_num(L,"x")+vx);upe_set(L,"y",upe_num(L,"y")+vy);
 lua_getfield(L,1,"vz");int has_vz=lua_toboolean(L,-1);lua_pop(L,1);
 if(has_vz){
  lua_Number vz=upe_num(L,"vz")*frict;upe_set(L,"vz",vz);
  lua_getfield(L,1,"z");lua_Number z=0;if(lua_toboolean(L,-1)){if(!lua_isnumber(L,-1))return luaL_error(L,"attempt to perform arithmetic on field 'z'");z=lua_tonumber(L,-1);}lua_pop(L,1);
  upe_set(L,"z",z+vz);upe_set(L,"vz",vz+upe_num(L,"we"));
 }else upe_set(L,"vy",vy+upe_num(L,"we"));
 lua_Number dcx=upe_num(L,"dcx")*.9;upe_set(L,"dcx",dcx);
 lua_Number dcy=upe_num(L,"dcy")*.9;upe_set(L,"dcy",dcy);
 if(fabs(dcx)<1)upe_set(L,"dcx",0);
 if(fabs(dcy)<1)upe_set(L,"dcy",0);
 return 0;
}
static void wrap_lua_global(lua_State *L,const char *name,lua_CFunction f){
 lua_getglobal(L,name);if(lua_isfunction(L,-1)&&!lua_iscfunction(L,-1)){lua_pushcclosure(L,f,1);lua_setglobal(L,name);}else lua_pop(L,1);
}
void sg_graphics_fast_api(lua_State *L){
 reg(L,"pal_rst",palette_reset);
 reg(L,"sg_trace_dist",trace_dist);
 reg(L,"sg_depth_lists",depth_lists);reg(L,"sg_upe_phys",upe_phys);
 wrap_lua_global(L,"sfillp_rst",sfillp_rst_fast);wrap_lua_global(L,"sfillp_dissolve",sfillp_dissolve_fast);
 reg(L,"sg_tick_counters",tick_counters);
 lua_getglobal(L,"split");if(lua_isfunction(L,-1)&&!lua_iscfunction(L,-1)){lua_pop(L,1);reg(L,"split",split_exact);}else lua_pop(L,1);
 lua_getglobal(L,"apal");
 if(lua_isfunction(L,-1)){lua_pushcclosure(L,apal_batch,1);lua_setglobal(L,"apal");}
 else lua_pop(L,1);
 lua_getglobal(L,"shpr");lua_getglobal(L,"bright");
 if(lua_isfunction(L,-2)&&lua_isfunction(L,-1)){lua_pushcclosure(L,shadow_batch,2);lua_setglobal(L,"shpr");}
 else lua_pop(L,2);
}
