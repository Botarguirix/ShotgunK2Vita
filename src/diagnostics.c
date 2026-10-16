#include "diagnostics.h"
#include "perf.h"
#include <stdatomic.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#ifdef SG_VITA
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/cpu.h>
#include <psp2/io/fcntl.h>
static SceUID writer = -1;
#else
#include <pthread.h>
#include <unistd.h>
static pthread_t writer;
#endif

#define EVENTS 64
#define EVENT_SIZE 320
static char directory[1200];
static FILE *trace;
static int enabled, thread_started;
static atomic_uint quit, phase, frame, buttons, lua_kb, graphics_kb, audio_blocks;
static atomic_int audio_rc;
static atomic_uint workers[3], drops;
static atomic_flag guard = ATOMIC_FLAG_INIT;
static char location[192], events[EVENTS][EVENT_SIZE];
static int axes[4], line;
static unsigned read_event, write_event;
static unsigned long bytes_written;
static int last_sync_rc;
static atomic_int thread_ids[SG_THREAD_COUNT];
static uint64_t last_device_sync_us,last_thread_snapshot,last_run_clock[SG_THREAD_COUNT];
void sg_diag_thread(unsigned role,int id){if(role<SG_THREAD_COUNT)atomic_store(&thread_ids[role],id);}
static void thread_snapshot(void){
#ifdef SG_VITA
 static const char*roles[]={"main_lua","audio","compose1","compose2","diagnostics"};
 uint64_t now=sg_now_us();if(now-last_thread_snapshot<1000000)return;last_thread_snapshot=now;
 for(unsigned i=0;i<SG_THREAD_COUNT;i++){
  int id=atomic_load(&thread_ids[i]);if(id<0)continue;
  SceKernelThreadInfo info={0};info.size=sizeof(info);int rc=sceKernelGetThreadInfo(id,&info);
  uint64_t raw=0;if(rc>=0)memcpy(&raw,&info.runClocks,sizeof(raw));
  uint64_t delta=raw>=last_run_clock[i]?raw-last_run_clock[i]:0;last_run_clock[i]=raw;
  fprintf(trace,"thread role=%s id=%d rc=%d name=%.31s affinity=0x%X last_cpu=%d priority=%d status=0x%X wait_type=%u run_clock_raw=%llu delta_raw=%llu\n",roles[i],id,rc,rc>=0?info.name:"unavailable",info.currentCpuAffinityMask,info.lastExecutedCpuId,info.currentPriority,info.status,info.waitType,(unsigned long long)raw,(unsigned long long)delta);
 }
#endif
}

static int enter(void) { return !atomic_flag_test_and_set_explicit(&guard, memory_order_acquire); }
static void leave(void) { atomic_flag_clear_explicit(&guard, memory_order_release); }
static void pause_writer(void) {
#ifdef SG_VITA
    sceKernelDelayThread(250000);
#else
    struct timespec delay = {0, 250000000}; nanosleep(&delay, NULL);
#endif
}
static void sync_file(FILE *file) {
    fflush(file);
#ifndef SG_VITA
    fsync(fileno(file));
#endif
}
static void device_sync(void) {
#ifdef SG_VITA
    uint64_t start=sg_now_us();
    last_sync_rc=sceIoSync("ux0:", 0);
    last_device_sync_us=sg_now_us()-start;
#endif
}

static void checkpoint(void) {
    static const char *names[] = {"boot","input","lua_update","lua_draw","present","display_wait","idle","lua_error","shutdown"};
    char source[192], pending[EVENTS][EVENT_SIZE]; int current_line, sticks[4]; unsigned count = 0;
    if (!enter()) return;
    memcpy(source, location, sizeof(source)); current_line = line; memcpy(sticks, axes, sizeof(sticks));
    while (read_event != write_event && count < EVENTS) {
        memcpy(pending[count++], events[read_event % EVENTS], EVENT_SIZE); read_event++;
    }
    leave();
    unsigned stage = atomic_load(&phase);
    if (bytes_written < 16*1024*1024) {
        int written = fprintf(trace,"checkpoint_us=%llu frame=%u phase=%s buttons=0x%X sticks=%d/%d/%d/%d lua=%s:%d lua_kb=%u graphics_kb=%u audio_blocks=%u audio_rc=%d workers=%u/%u/%u dropped_events=%u sync_rc=%d\n",
            (unsigned long long)sg_now_us(), atomic_load(&frame), stage < 9 ? names[stage] : "unknown",
            atomic_load(&buttons), sticks[0], sticks[1], sticks[2], sticks[3], source, current_line,
            atomic_load(&lua_kb), atomic_load(&graphics_kb), atomic_load(&audio_blocks), atomic_load(&audio_rc),
            atomic_load(&workers[0]), atomic_load(&workers[1]), atomic_load(&workers[2]), atomic_load(&drops),last_sync_rc);
        if (written > 0) bytes_written += (unsigned)written;
        for (unsigned i=0;i<count;i++) { written = fprintf(trace,"%s\n",pending[i]); if(written>0)bytes_written+=(unsigned)written; }
        if (bytes_written >= 16*1024*1024) fprintf(trace,"trace_limit_reached=16MiB; session logs remain available\n");
        thread_snapshot();
        fprintf(trace,"previous_device_sync_us=%llu\n",(unsigned long long)last_device_sync_us);
        sync_file(trace);
    }
    device_sync();
}
#ifdef SG_VITA
static int entry(SceSize size, void *arg) {
    (void)size; (void)arg;
#else
static void *entry(void *arg) {
    (void)arg;
#endif
    while (!atomic_load_explicit(&quit,memory_order_acquire)) { checkpoint(); pause_writer(); }
    checkpoint();
#ifdef SG_VITA
    return 0;
#else
    return NULL;
#endif
}

int sg_diag_start(const char *root) {
    char base[1200], latest[1200];
    mkdir(root,0777); snprintf(base,sizeof(base),"%s/logs",root);
    if (mkdir(base,0777) && errno != EEXIST) return 0;
    int found=0;
    for(unsigned attempt=0;attempt<10000;attempt++) {
        snprintf(directory,sizeof(directory),"%s/logs/%s-%llu-%u",root,SG_BUILD_VERSION,(unsigned long long)time(NULL),attempt);
        if (!mkdir(directory,0777)) { found=1;break; }
        if (errno != EEXIST) break;
    }
    if(!found){directory[0]=0;return 0;}
    trace=sg_diag_open("crash-trace.log");
    if(!trace){directory[0]=0;return 0;}
    atomic_store(&quit,0); atomic_store(&phase,SG_DIAG_BOOT); atomic_store(&frame,0);
    atomic_store(&buttons,0); atomic_store(&lua_kb,0); atomic_store(&graphics_kb,0);
    atomic_store(&audio_blocks,0); atomic_store(&audio_rc,0); atomic_store(&drops,0);
    for(unsigned i=0;i<3;i++)atomic_store(&workers[i],0);
    read_event=write_event=0; line=0; location[0]=0; memset(axes,0,sizeof(axes)); bytes_written=0;
    for(unsigned i=0;i<SG_THREAD_COUNT;i++){atomic_store(&thread_ids[i],-1);last_run_clock[i]=0;}last_thread_snapshot=last_device_sync_us=0;
    fprintf(trace,"build=%s session_start=1 checkpoints_ms=250 clean_end_missing_means_unclean_or_forced_exit\n",SG_BUILD_VERSION);
    sync_file(trace);
    snprintf(latest,sizeof(latest),"%s/logs/latest.txt",root);
    FILE *pointer=fopen(latest,"w");if(pointer){fprintf(pointer,"%s\n",directory);sync_file(pointer);fclose(pointer);}
    device_sync(); enabled=1;
#ifdef SG_VITA
    writer=sceKernelCreateThread("SG diagnostic writer",entry,0x10000110,0x10000,0,SCE_KERNEL_CPU_MASK_USER_1,NULL);
    int rc=writer<0?writer:sceKernelStartThread(writer,0,NULL);
    thread_started=rc>=0;
    if(thread_started)sg_diag_thread(SG_THREAD_DIAGNOSTICS,writer);
    if(!thread_started&&writer>=0){sceKernelDeleteThread(writer);writer=-1;}
#else
    thread_started=pthread_create(&writer,NULL,entry,NULL)==0;
#endif
    sg_diag_event("writer",thread_started?"started":"failed; only explicit logs available");
    return thread_started;
}
const char *sg_diag_directory(void) { return directory; }
FILE *sg_diag_open(const char *name) {
    if(!directory[0]||strchr(name,'/')||strchr(name,':')||strstr(name,".."))return NULL;
    char path[1400];snprintf(path,sizeof(path),"%s/%s",directory,name);
    FILE *file=fopen(path,"w");if(file)setvbuf(file,NULL,_IOLBF,4096);return file;
}
void sg_diag_phase(unsigned value) { if(enabled)atomic_store(&phase,value); }
void sg_diag_frame(unsigned value,unsigned keys,int lx,int ly,int rx,int ry) {
    if(!enabled)return;atomic_store(&frame,value);atomic_store(&buttons,keys);
    if(enter()){axes[0]=lx;axes[1]=ly;axes[2]=rx;axes[3]=ry;leave();}
}
void sg_diag_location(const char *source,int current_line) {
    if(!enabled||!enter())return;snprintf(location,sizeof(location),"%s",source?source:"?");line=current_line;leave();
}
void sg_diag_event(const char *kind,const char *detail) {
    if(!enabled)return;
    if(!enter()){atomic_fetch_add(&drops,1);return;}
    if(write_event-read_event==EVENTS){atomic_fetch_add(&drops,1);leave();return;}
    snprintf(events[write_event%EVENTS],EVENT_SIZE,"event_us=%llu frame=%u kind=%s detail=%s",
        (unsigned long long)sg_now_us(),atomic_load(&frame),kind,detail?detail:"");write_event++;leave();
}
void sg_diag_memory(unsigned lua,unsigned graphics) { if(enabled){atomic_store(&lua_kb,lua);atomic_store(&graphics_kb,graphics);} }
void sg_diag_audio(unsigned blocks,int rc) { if(enabled){atomic_store(&audio_blocks,blocks);atomic_store(&audio_rc,rc);} }
void sg_diag_worker(unsigned worker,unsigned value) { if(enabled&&worker<3)atomic_store(&workers[worker],value); }
void sg_diag_stop(const char *reason) {
    if(!enabled)return;sg_diag_phase(SG_DIAG_EXIT);
    atomic_store_explicit(&quit,1,memory_order_release);
    if(thread_started){
#ifdef SG_VITA
        sceKernelWaitThreadEnd(writer,NULL,NULL);sceKernelDeleteThread(writer);writer=-1;
#else
        pthread_join(writer,NULL);
#endif
    }else checkpoint();
    fprintf(trace,"session_end=%s\n",reason?reason:"clean");sync_file(trace);device_sync();
    enabled=0;thread_started=0;fclose(trace);trace=NULL;
}
