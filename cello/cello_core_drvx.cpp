#include <DrvAPI.hpp>
#include <cello.hpp>
#include <cello_drvx_internal.hpp>
#include <deque>
#include <memory>
#include <ostream>
#include <fstream>
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

//////////////////////////////////////////////////
// clock handler for profiling task queue sizes //
//////////////////////////////////////////////////
class task_queue_profiler {
public:
    /**
     * constructor
     */
    task_queue_profiler() {
        file().open("task_queue_profiler.csv");
        file() << csv_header() << std::endl;
        system_ = DrvAPI::DrvAPIThread::current()->getSystem();
        for (int i = 0; i < CORE_THREADS; i++) {
            task_queue_vaddr_[i] = DrvAPIVAddress{thread_task_queue[i].address()};
        }
    }

    /**
     * copy constructor
     */
    task_queue_profiler(const task_queue_profiler &) = default;

    /**
     * move constructor
     */
    task_queue_profiler(task_queue_profiler &&) = default;

    /**
     * copy assignment
     */
    task_queue_profiler &operator=(const task_queue_profiler &) = default;

    /**
     * move assignment
     */
    task_queue_profiler &operator=(task_queue_profiler &&) = default;

    /**
     * destructor
     */
    ~task_queue_profiler() = default;

    /**
     * thread id iterator
     */
    class system_thread_iterator {
    public:
        system_thread_iterator() : id_() {}
        system_thread_iterator(const thread_id_t &id) : id_(id) {}
        system_thread_iterator(const system_thread_iterator &) = default;
        system_thread_iterator(system_thread_iterator &&) = default;
        system_thread_iterator &operator=(const system_thread_iterator &) = default;
        system_thread_iterator &operator=(system_thread_iterator &&) = default;
        ~system_thread_iterator() = default;

        /**
         *post increment
         */
        system_thread_iterator &operator++() {
            id().thread++;
            if (id().thread >= DrvAPI::numCoreThreads()) {
                id().thread = 0;
                id().core++;
                if (id().core >= DrvAPI::numPodCores()) {
                    id().core = 0;
                    id().pod++;
                    if (id().pod >= DrvAPI::numPXNPods()) {
                        id().pod = 0;
                        id().pxn++;
                    }
                }
            }            
            return *this;
        }

        /**
         * return the thread id
         */
        thread_id_t operator*() const {
            return id_;
        }

        bool operator==(const system_thread_iterator &rhs) const {
            return id() == rhs.id();
        }

        bool operator!=(const system_thread_iterator &rhs) const {
            return id() != rhs.id();
        }

        /**
         * return the thread id
         */
        thread_id_t& id() {
            return id_;
        }

        /**
         * return the thread id
         */
        const thread_id_t& id() const {
            return id_;
        }

    private:
        thread_id_t id_;
    };

    struct system_thread_range {
        system_thread_iterator begin_ = system_thread_iterator{thread_id_t{0, 0, 0, 0}};        
        system_thread_iterator end_ = system_thread_iterator{
            thread_id_t{DrvAPI::numPXNs(),
                        DrvAPI::numPXNPods()-1,
                        DrvAPI::numPodCores()-1,
                        DrvAPI::numCoreThreads()-1}
        };
        system_thread_range() = default;
        system_thread_iterator begin() const {
            return begin_;
        }
        system_thread_iterator end() const {
            return end_;
        }
    };
    
    /**
     * get system thread ids
     */
    system_thread_range system_thread_ids() {
        return system_thread_range{};
    }

    /**
     * get the task queue of a specific thread
     */
    task_queue *task_queue_pointer_of(const thread_id_t &tid) {
        DrvAPIVAddress vaddr = task_queue_vaddr_[tid.thread];
        vaddr.pxn() = tid.pxn;
        vaddr.pod() = tid.pod;
        vaddr.core_x() = coreXFromId(tid.core);
        vaddr.core_y() = coreYFromId(tid.core);
        void *p; size_t _;
        DrvAPIAddressToNative(vaddr.encode(), &p, &_);
        return (task_queue *)p;
    }
    
    /**
     * run the clock handler
     */
    void run() {
        for (thread_id_t tid : system_thread_ids()) {
            task_queue * tq = task_queue_pointer_of(tid);
            auto queue = tq->queue_;
            file() << (uint64_t)(system().getSeconds() * 1e12) << ","
                   << tid.pxn << ","
                   << tid.pod << ","
                   << tid.core << ","
                   << tid.thread << ","
                   << queue->size() << "\n";
        }
    }

    /**
     * get the output file
     */
    std::ofstream &file() {
        return file_;
    }

    /**
     * get the output file
     */
    const std::ofstream &file() const {
        return file_;
    }

    /**
     * get the system
     */
    DrvAPI::DrvAPISystem &system() {
        return *system_;
    }

    /**
     * get the system
     */
    const DrvAPI::DrvAPISystem &system() const {
        return *system_;
    }

    /**
     * get the csv header
     */
    const std::string csv_header() const {
        return "time,pxn,pod,core,thread,size";
    }

private:
    std::shared_ptr<DrvAPI::DrvAPISystem> system_; //!< system
    std::ofstream       file_; //!< output file
    DrvAPIVAddress task_queue_vaddr_[CORE_THREADS]; //!< task queue addresses (local addresses)
};

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
#ifdef CELLO_ENABLE_TASK_QUEUE_PROFILER
            std::shared_ptr<task_queue_profiler> profiler
                = std::make_shared<task_queue_profiler>();
            DrvAPI::registerUserClock("25MHz", [profiler](){
                profiler->run();
                return false;
            });
#endif
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
