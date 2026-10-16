#include "gpu_presenter.h"
#include "runtime.h"
#include "perf.h"
#include <vita2d.h>
#include <string.h>
static vita2d_texture *scene,*error_image,*labels[2];
static int initialized;
static int scene_width=320,scene_height=180;
static vita2d_texture *label(int right){
 static const unsigned char letters[2][7]={{16,16,16,16,16,16,31},{30,17,17,30,20,18,17}};
 static const unsigned char one[7]={4,12,4,4,4,4,14};
 vita2d_texture *t=vita2d_create_empty_texture(32,16);if(!t)return NULL;
 uint32_t *p=vita2d_texture_get_datap(t);int stride=vita2d_texture_get_stride(t)/4;
 for(int c=0;c<2;c++)for(int y=0;y<7;y++)for(int x=0;x<5;x++)if((c?one[y]:letters[right][y])&(1<<(4-x)))
  for(int dy=0;dy<2;dy++)for(int dx=0;dx<2;dx++)p[(y*2+dy)*stride+c*12+x*2+dx]=0xffffffffu;
 vita2d_texture_set_filters(t,SCE_GXM_TEXTURE_FILTER_POINT,SCE_GXM_TEXTURE_FILTER_POINT);return t;
}
int sg_gpu_init(FILE *log,int divisor){
 int rc=vita2d_init_advanced(256*1024);
 if(rc<=0||!vita2d_get_context()){if(log)fprintf(log,"gpu_init_failed=%d fallback=cpu\n",rc);return 0;}
 initialized=1;vita2d_set_vblank_wait(1);
 /* Uncached mapped textures avoid the cache coherency failure seen with DMA. */
 vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE);
 scene_width=divisor==2?160:320;scene_height=divisor==2?90:180;
 scene=vita2d_create_empty_texture(scene_width,scene_height);labels[0]=label(0);labels[1]=label(1);
 if(!scene||!labels[0]||!labels[1]){if(log)fprintf(log,"gpu_texture_alloc_failed fallback=cpu\n");sg_gpu_close();return 0;}
 vita2d_texture_set_filters(scene,SCE_GXM_TEXTURE_FILTER_POINT,SCE_GXM_TEXTURE_FILTER_POINT);
 if(log)fprintf(log,"renderer=gpu scene=%dx%d logical=320x180 scale=%d y=2 upload_bytes=%d texture_memory=uncached display_buffers=3 queue_drained_each_frame=1\n",scene_width,scene_height,960/scene_width,scene_width*scene_height*4);
 return 1;
}
int sg_gpu_present(const uint32_t *pixels,int width,int height,unsigned buttons,int controls,uint32_t background,SgGpuStats *stats){
 memset(stats,0,sizeof(*stats));vita2d_texture *t=scene;
 if(width==960&&height==544){
  if(!error_image){error_image=vita2d_create_empty_texture(width,height);if(!error_image)return 0;}
  t=error_image;
 }else if(width!=scene_width||height!=scene_height)return 0;
 /* Last submission has drained before CPU updates this texture. */
 uint64_t start=sg_now_us();int stride=vita2d_texture_get_stride(t);
 unsigned char *dst=vita2d_texture_get_datap(t);
 for(int y=0;y<height;y++)memcpy(dst+y*stride,pixels+y*width,width*4);
 stats->upload_us=sg_now_us()-start;start=sg_now_us();
 vita2d_start_drawing();vita2d_set_clear_color(background);vita2d_clear_screen();
 vita2d_draw_texture_scale(t,0,t==scene?2:0,t==scene?960.0f/width:1,t==scene?540.0f/height:1);
 (void)controls;(void)buttons;
 vita2d_end_drawing();vita2d_swap_buffers();stats->submit_us=sg_now_us()-start;
 start=sg_now_us();int rc=sceGxmDisplayQueueFinish();stats->wait_us=sg_now_us()-start;
 return rc>=0;
}
void sg_gpu_close(void){
 if(!initialized)return;
 vita2d_wait_rendering_done();sceGxmDisplayQueueFinish();
 if(scene)vita2d_free_texture(scene);
 if(error_image)vita2d_free_texture(error_image);
 for(int i=0;i<2;i++)if(labels[i])vita2d_free_texture(labels[i]);
 scene=error_image=labels[0]=labels[1]=NULL;vita2d_fini();initialized=0;
}
