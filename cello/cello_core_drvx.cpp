#include <DrvAPI.hpp>
#include <cello.hpp>
#include <cello_drvx_internal.hpp>
#include <deque>
#include <memory>
#include <inttypes.h>

using namespace DrvAPI;
using namespace cello;

//#define CELLO_CORE_DRVX_DEBUG
#ifdef CELLO_CORE_DRVX_DEBUG
#define pr_dbg(fmt, ...)                                                \
    do {                                                                \
        printf("DEBUG: %s:%d: tid=%4ld: %s(): " fmt, __FILE__, __LINE__, cello::tid(), __PRETTY_FUNCTION__, ##__VA_ARGS__); } \
    while (0)
#else
#define pr_dbg(fmt, ...)                                                \
    do {                                                                \
    } while (0)
#endif
#define pr_warn(fmt, ...)                                               \
    do {                                                                \
        static int warned = 0;                                          \
        if (!warned++)                                                  \
            printf("WARNING: %s:%d: tid=%4ld: %s(): " fmt, __FILE__, __LINE__, cello::tid(), __PRETTY_FUNCTION__, ##__VA_ARGS__); \
    } while (0)

namespace cello
{
///////////////////////////////////
// each core has a copy of these //
///////////////////////////////////
StaticL1SP<task_queue> thread_task_queue[CORE_THREADS]; // an array of task queue by thread
task_queue_ref my_task_queue() {
  return thread_task_queue[myThreadId()];
}


/**
 * get the task queue of a specific thread
 */
task_queue_ref task_queue_of(const thread_id_t &tid) {
    DrvAPIVAddress vaddr = static_cast<DrvAPIAddress>(&thread_task_queue[tid.thread]);
    vaddr.global() = true;
    vaddr.l2_not_l1() = false;
    vaddr.pxn() = tid.pxn;
    vaddr.pod() = tid.pod;
    vaddr.core_x() = coreXFromId(tid.core);
    vaddr.core_y() = coreYFromId(tid.core);
    Pointer<task_queue> ptr = vaddr.encode();
    return *ptr;
}

//////////////////////////////////
// each pxn has a copy of these //
//////////////////////////////////

StaticMainMem<int64_t>  num_threads_ready; // how many threads have initialized

/* return the copy of num_threads_ready on pxn 0 */
DrvAPIPointer<int64_t> num_threads_ready_ptr() {
    DrvAPIVAddress vaddr = static_cast<DrvAPIAddress>(&num_threads_ready);
    vaddr.not_scratchpad() = true;
    vaddr.pxn() = 0;
    return vaddr.encode();
}

StaticMainMem<int64_t>  terminate; // should terminate

/* return the copy of terminate on pxn 0 */
DrvAPIPointer<int64_t> terminate_ptr() {
    DrvAPIVAddress vaddr = static_cast<DrvAPIAddress>(&terminate);
    vaddr.not_scratchpad() = true;
    vaddr.pxn() = 0;
    return vaddr.encode();
}

/**
 * @brief try to steal a task from another thread
 */
void steal() {
    // select a random victim
    pr_dbg("trying to steal\n");
    thread_id_t victim;
    victim.pxn    = random() % numPXNs();    
    victim.pod    = random() % numPXNPods();
    victim.core   = random() % numPodCores();
    victim.thread = random() % numCoreThreads();

    pr_dbg("trying to steal from tid=%4ld\n", tid(victim));
    task_queue_ref victim_queue = task_queue_of(victim);

    DrvAPI::DrvAPIVAddress vaddr{&victim_queue};
    // pop from the victim's back
    task *task = victim_queue.pop_back();

    pr_dbg("popped from victim's queue, task = %p\n", task);
    if (task != nullptr) {
        // execute the task
        task->execute();
    }
}

/**
 * @brief find work to do
 * 
 */
void find_work() {
    // first try to pop from your own queue
    task *task = my_task_queue().pop_front();
    if (task != nullptr) {
        pr_dbg("popped from my own queue\n");
        // execute the task
        task->execute();
        return;
    }
    pr_dbg("couldn't pop from my own queue\n");
    // if you can't find work, try to steal from others
    steal();
}

/**
 * @brief spawn a task
 * 
 * @param task 
 */
void spawn(task *task) {
    // new tasks are placed at front
    pr_dbg("spawning task onto queue @ 0x%016" PRIx64 "\n", (DrvAPIAddress)&my_task_queue());    
    my_task_queue().push_front(task);    
}

/**
 * @brief yield to another task
 * 
 */
void yield() {
    auto *thread = DrvAPI::DrvAPIThread::current();
    // make sure we have more than 4KB of stack left
    if (thread->getStackRemaining() <= 4096) {
        pr_warn("not enough stack remaining\n");
        nop(32);
        return;
    }
    pr_dbg("yielding\n");
    find_work();    
}

}

int cello_start(int argc, char *argv[])
{
    DrvAPI::DrvAPITagGuard guard(CELLO_TAG);
    DrvAPIMemoryAllocatorInit();

    // initialize your threads queue
    pr_dbg("initializing my task queue\n");
    my_task_queue().init();
    atomic_add(num_threads_ready_ptr(), 1);

    // poll until all threads are ready
    int64_t ready = *num_threads_ready_ptr();
    while (ready != num_threads()) {
        pr_dbg("%" PRId64 "/%" PRId64 " threads are ready\n"
               , ready
               , num_threads());
        nop(32);
        ready = *num_threads_ready_ptr();
    }

    pr_dbg("%" PRId64 "/%" PRId64 " threads are ready\n"
	   , ready
	   , num_threads());
    
    if (tid() == 0) {
        auto call_main = [argc, argv](){
            {
                DrvAPI::DrvAPITagGuard guard(DrvAPI::DEFAULT_TAG);
                CelloMain(argc, argv);
            }
            {
                DrvAPI::DrvAPITagGuard guard(CELLO_TAG);
                *terminate_ptr() = 1;
            }
        };
        task_impl <decltype(call_main)> main_task (call_main);
        
        spawn(&main_task);        
    }

    while (*terminate_ptr() != 1) {
        // if you are in this loop than you are idle
        find_work();
        // temp.
        nop(1024);
    }

    return 0;
}

declare_drv_api_main(cello_start);
