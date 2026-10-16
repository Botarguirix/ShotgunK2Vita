#include "local_ui.h"
#include "perf.h"
#include "diagnostics.h"
#include <string.h>
static char notices[8][128];static unsigned head,count;static uint64_t start;
void sg_ui_reset(void){head=count=0;start=0;memset(notices,0,sizeof(notices));}
static char ascii(unsigned c){
 if(c>=224&&c<=229)return 'A';if(c>=232&&c<=235)return 'E';if(c>=236&&c<=239)return 'I';
 if(c>=242&&c<=246)return 'O';if(c>=249&&c<=252)return 'U';if(c==241)return 'N';
 if(c>=192&&c<=197)return 'A';if(c>=200&&c<=203)return 'E';if(c>=204&&c<=207)return 'I';
 if(c>=210&&c<=214)return 'O';if(c>=217&&c<=220)return 'U';if(c==209)return 'N';
 return c<128?(char)c:'?';
}
void sg_ui_notice(const char *message){
 if(count==8){sg_diag_event("achievement_popup","queue_full");return;}
 char *out=notices[(head+count)%8];unsigned n=0;const unsigned char*p=(const unsigned char*)message;
 while(*p&&n<127){unsigned c=*p++;if(c>=192&&c<224&&*p)c=((c&31)<<6)|(*p++&63);else if(c>=224&&c<240&&p[0]&&p[1]){c=((c&15)<<12)|((p[0]&63)<<6)|(p[1]&63);p+=2;}out[n++]=ascii(c);}
 out[n]=0;if(!count)start=sg_now_us();count++;sg_diag_event("achievement_popup",out);
}
void sg_ui_tick(void){if(count&&sg_now_us()-start>=4000000){head=(head+1)%8;count--;start=sg_now_us();}}
const char *sg_ui_current_notice(void){return count?notices[head]:NULL;}
static const unsigned char alphabet[26][7]={
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
 {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},
 {14,4,4,4,4,4,14},{7,2,2,2,2,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
 {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
 {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31}};
static const unsigned char digits[10][7]={
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},{2,6,10,18,31,2,2},
 {31,16,16,30,1,1,30},{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},{14,17,17,15,1,1,14}};

static int glyph(const char *s,int x,int y){
 if(x<0||y<0||y>=7)return 0;unsigned i=x/6;if(i>=strlen(s))return 0;unsigned c=(unsigned char)s[i];if(c>='a'&&c<='z')c-=32;
 int col=x%6;if(col>=5)return 0;const unsigned char*g=c>='A'&&c<='Z'?alphabet[c-'A']:c>='0'&&c<='9'?digits[c-'0']:NULL;
 return g?((g[y]>>(4-col))&1):0;
}
void sg_ui_row(uint32_t *row,int width,int height,int y){
 if(!count)return;int scale=width/320;if(height/180<scale)scale=height/180;if(scale<1)scale=1;
 int small=width<320||height<180;
 int ox=small?0:(width-320*scale)/2,oy=small?0:(height-180*scale)/2,yy=small?y*180/height:(y-oy)/scale;
 if(y<oy||yy<4||yy>=39)return;
 char first[47]={0},second[47]={0};const char *s=notices[head];size_t n=strlen(s),a=n<46?n:46;memcpy(first,s,a);if(n>46){size_t b=n-46;if(b>46)b=46;memcpy(second,s+46,b);}
 for(int x=0;x<width;x++){int xx=small?x*320/width:(x-ox)/scale;if(x<ox||xx<16||xx>=304)continue;
  uint32_t color=yy==4||yy==38||xx==16||xx==303?0xffc09070u:0xff201018u;
  if(glyph("LOGRO DESBLOQUEADO",xx-24,yy-8)||glyph(first,xx-24,yy-20)||glyph(second,xx-24,yy-29))color=0xffffffffu;row[x]=color;
 }
}
