#include "render_pool.h"
#include "graphics.h"
#include "perf.h"
#include "diagnostics.h"
#include <string.h>
#ifdef SG_VITA
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/cpu.h>
#else
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#endif

typedef struct {
    uint32_t *pixels;
    int width, height, first, last, quit, cpu;
    uint64_t time_us;
#ifdef SG_VITA
    SceUID thread, ready, done;
#else
    pthread_t thread;
    sem_t ready, done;
#endif
} Worker;
static Worker jobs[2];
static int started;
static SgRenderStats last_stats;

/* Semaphore handoffs publish the job and fence all surface/palette reads.
 * The caller joins every job before Lua can mutate surfaces again. */
#ifdef SG_VITA
static int entry(SceSize size, void *arg) {
    Worker *w=NULL;
    if(size!=sizeof(w))return -1;
    memcpy(&w,arg,sizeof(w));
    w->cpu=sceKernelGetCpuId();
    for(;;){
        if(sceKernelWaitSema(w->ready,1,NULL)<0)break;
        if(w->quit)break;
        sg_diag_worker((unsigned)(w-jobs)+1,1);
        uint64_t start=sg_now_us();sg_present_rows(w->pixels,w->width,w->height,w->first,w->last);w->time_us=sg_now_us()-start;
        sg_diag_worker((unsigned)(w-jobs)+1,0);
        sceKernelSignalSema(w->done,1);
    }
    return 0;
}
#else
static void wait_sem(sem_t *s){while(sem_wait(s)<0&&errno==EINTR){}}
static void *entry(void *arg){
    Worker *w=arg;
    w->cpu=-1;
    for(;;){wait_sem(&w->ready);if(w->quit)break;
        sg_diag_worker((unsigned)(w-jobs)+1,1);
        uint64_t start=sg_now_us();sg_present_rows(w->pixels,w->width,w->height,w->first,w->last);w->time_us=sg_now_us()-start;
        sg_diag_worker((unsigned)(w-jobs)+1,0);
        sem_post(&w->done);
    }
    return NULL;
}
#endif

int sg_render_pool_start(int workers,FILE *log){
    memset(&last_stats,0,sizeof(last_stats));for(int i=0;i<3;i++)last_stats.cpu[i]=-1;
    if(workers<1)workers=1;if(workers>3)workers=3;
    for(int i=0;i<workers-1;i++){
        Worker *w=&jobs[i];memset(w,0,sizeof(*w));
#ifdef SG_VITA
        w->ready=sceKernelCreateSema("SG render ready",0,0,1,NULL);
        w->done=sceKernelCreateSema("SG render done",0,0,1,NULL);
        w->thread=sceKernelCreateThread("SG present worker",entry,0x10000100,0x10000,0,
            i==0?SCE_KERNEL_CPU_MASK_USER_1:SCE_KERNEL_CPU_MASK_USER_2,NULL);
        int rc=(w->ready<0||w->done<0||w->thread<0)?-1:sceKernelStartThread(w->thread,sizeof(w),&w);
        if(rc>=0)sg_diag_thread(i==0?SG_THREAD_RENDER1:SG_THREAD_RENDER2,w->thread);
        if(log)fprintf(log,"render_worker=%d thread=%d ready=%d done=%d start_rc=%d\n",i+1,w->thread,w->ready,w->done,rc);
        if(rc<0){if(w->thread>=0)sceKernelDeleteThread(w->thread);if(w->ready>=0)sceKernelDeleteSema(w->ready);if(w->done>=0)sceKernelDeleteSema(w->done);break;}
#else
        if(sem_init(&w->ready,0,0))break;
        if(sem_init(&w->done,0,0)){sem_destroy(&w->ready);break;}
        if(pthread_create(&w->thread,NULL,entry,w)){sem_destroy(&w->ready);sem_destroy(&w->done);break;}
#endif
        started++;
    }
    return started+1;
}

void sg_render_pool_present(uint32_t *pixels,int width,int height){
    int total=started+1;
    for(int i=0;i<started;i++){
        Worker *w=&jobs[i];w->pixels=pixels;w->width=width;w->height=height;
        w->first=height*(i+1)/total;w->last=height*(i+2)/total;
#ifdef SG_VITA
        sceKernelSignalSema(w->ready,1);
#else
        sem_post(&w->ready);
#endif
    }
    sg_diag_worker(0,1);
    uint64_t start=sg_now_us();sg_present_rows(pixels,width,height,0,height/total);last_stats.worker_us[0]=sg_now_us()-start;
    sg_diag_worker(0,0);
#ifdef SG_VITA
    last_stats.cpu[0]=sceKernelGetCpuId();
#else
    last_stats.cpu[0]=-1;
#endif
    for(int i=0;i<started;i++){
#ifdef SG_VITA
        sceKernelWaitSema(jobs[i].done,1,NULL);
#else
        wait_sem(&jobs[i].done);
#endif
        last_stats.worker_us[i+1]=jobs[i].time_us;last_stats.cpu[i+1]=jobs[i].cpu;
    }
}
SgRenderStats sg_render_pool_stats(void){return last_stats;}

void sg_render_pool_stop(void){
    for(int i=0;i<started;i++){
        Worker *w=&jobs[i];w->quit=1;
#ifdef SG_VITA
        sceKernelSignalSema(w->ready,1);sceKernelWaitThreadEnd(w->thread,NULL,NULL);
        sg_diag_thread(i==0?SG_THREAD_RENDER1:SG_THREAD_RENDER2,-1);
        sceKernelDeleteThread(w->thread);sceKernelDeleteSema(w->ready);sceKernelDeleteSema(w->done);
#else
        sem_post(&w->ready);pthread_join(w->thread,NULL);sem_destroy(&w->ready);sem_destroy(&w->done);
#endif
    }
    started=0;
}
