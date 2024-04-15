#include "pandohammer/storage.h"
#include "pandohammer/addressmap.hpp"
#include "pandohammer/cpuinfo.h"
#include "cello_core_drvr.hpp"
#include <stdint.h>
#include <new>
namespace cello
{

l2sp_storage(int64_t) num_threads_ready = 0; // number of threads ready

/**
 * return an absolute pointer to the number of threads ready
 */
static inline int64_t *num_threads_ready_ptr() {
    return virtual_address{std::addressof(num_threads_ready)}
        .set_pod(0)
        .set_is_global(true);
}

l2sp_storage(int64_t) terminate = 0; // set to 1 to terminate

/**
 * return an absolute pointer to the terminate flag
 */
static inline int64_t *terminate_ptr() {
    return virtual_address{std::addressof(terminate)}
        .set_pod(0)
        .set_is_global(true);
}

// lock type
typedef int32_t lock_t;

/**
 * initialize a lock
 */
inline void lock_init(lock_t* lock_ptr) {
    *lock_ptr = 0;
}

/**
 * @brief lock a lock
 * 
 * @param lock_ptr 
 */
inline void lock(lock_t* lock_ptr) {
    int lock_val = 1;
    do {
        lock_val = atomic_swap_i32(lock_ptr, 1);
    } while (lock_val != 0);
    return;
}

/**
 * @brief unlock a lock
 * 
 * @param lock_ptr 
 */
inline void unlock(lock_t* lock_ptr) {
    atomic_swap_i32(lock_ptr, 0);
}

/**
 * @brief lock guard
 *
 * automatically releases lock when out of scope
 */
struct LockGuard {
public:
    LockGuard(lock_t* lock_ptr) : lock_ptr_(lock_ptr) {
        lock(lock_ptr_);
    }
    ~LockGuard() {
        unlock(lock_ptr_);
    }
    lock_t* lock_ptr_;
};

/**
 * @brief initialize memory
 */
static void init_mem()
{    
}

/**
 * @brief a Deque
 */
template <typename T, size_t QUEUE_SIZE>
class Deque {
public:
    Deque(){}
    ~Deque(){}
    void reset();
    void push_back(const T &t);
    T pop_back();
    T pop_front();
    bool unsafe_empty() const {
        return (m_tail_ptr - m_head_ptr) == 0;
    }

private:
    T* m_array_rp;
    T* m_head_ptr;
    T* m_tail_ptr;
    T* m_array_end;
    lock_t m_mutex;
    T m_array[QUEUE_SIZE];
};

using TaskDeque = Deque<task*, 16>;

template <typename T, size_t QUEUE_SIZE>
void Deque<T, QUEUE_SIZE>::reset()
{
    m_array_rp = virtual_address{std::addressof(m_array)}
        .set_core_x(myCoreX())
        .set_core_y(myCoreY())
        .set_is_global(true);

    m_head_ptr = m_array_rp;
    m_tail_ptr = m_array_rp;
    m_array_end = m_array_rp + QUEUE_SIZE;
    lock_init(&m_mutex);
}

template <typename T, size_t QUEUE_SIZE>
void Deque<T, QUEUE_SIZE>::push_back(const T &t)
{
    LockGuard lock_guard(&m_mutex);
    if (m_tail_ptr < m_array_end) {
        *m_tail_ptr = t;
        m_tail_ptr++;
    }
}

template <typename T,  size_t QUEUE_SIZE>
T Deque<T, QUEUE_SIZE>::pop_back()
{
    LockGuard lock_guard(&m_mutex);
    T ret_val = T{};
    if (m_tail_ptr - m_head_ptr > 0) {
        T*tmp = --m_tail_ptr;
        // reset pointer
        if (m_tail_ptr == m_head_ptr) {
            m_tail_ptr = m_head_ptr = m_array_rp;
        }
        ret_val = *tmp;
    }
    return ret_val;
}

template <typename T, size_t QUEUE_SIZE>
T Deque<T, QUEUE_SIZE>::pop_front()
{
    LockGuard lock_guard(&m_mutex);
    T ret_val = T{};
    if (m_tail_ptr - m_head_ptr > 0) {
        ret_val = *m_head_ptr++;
        // reset pointer
        if (m_tail_ptr == m_head_ptr) {
            m_tail_ptr = m_head_ptr = m_array_rp;
        }
    }
    return ret_val;
}

l1sp_storage(TaskDeque) thread_task_queue[CORE_THREADS];
/**
 * @brief get my task queue
 */
TaskDeque *my_task_queue()
{
    return &thread_task_queue[myThreadId()];
}

/**
 * @brief get task queue of a thread
 */
TaskDeque *task_queue_of(thread_id_t &tid) {
    TaskDeque *p = &thread_task_queue[tid.thread];
    // todo; set pod and pxn
    p = virtual_address{p}
        .set_core_x(coreXFromId(tid.core))
        .set_core_y(coreYFromId(tid.core))
        .set_is_global(true);
    return p;
}

/**
 * @brief initialize queues
 */
void init_queues()
{
    my_task_queue()->reset();
}


/**
 * @brief try to steal a task from another thread
 */
void steal() {
    // select a random victim
    thread_id_t victim;
    victim.pxn    = rand() % numPXNs();    
    victim.pod    = rand() % numPXNPods();
    victim.core   = rand() % numPodCores();
    victim.thread = rand() % numCoreThreads();

    auto *victim_queue = task_queue_of(victim);

    // pop from the victim's back
    task *task = victim_queue->pop_back();

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
    task *task = my_task_queue()->pop_front();
    if (task != nullptr) {
        // execute the task
        task->execute();
        return;
    }
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
    my_task_queue()->push_back(task);
}

/**
 * @brief yield to another task
 * 
 */
void yield() {
    find_work();    
}


/**
 * call main struct
 */
struct call_main {
    call_main(int argc, char *argv[]) : argc_(argc), argv_(argv) {}
    void operator()() {
        CelloMain(argc_, argv_);
        *terminate_ptr() = true;
    }
    int argc_;
    char **argv_;
};

}

using namespace cello;

int main(int argc, char *argv[])
{
    // set stack pointer to a global address
    void *p;
    asm volatile ("mv %0, sp" : "=r" (p));
    p = virtual_address{p}
        .set_pxn(myPXNId())
        .set_pod(myPodId())
        .set_core_x(myCoreX())
        .set_core_y(myCoreY())
        .set_is_global(true);
    asm volatile ("mv sp, %0" : : "r" (p));
    // initialize memory allocators
    init_mem();
    // initialize task queues
    init_queues();

    // indicate ready
    atomic_fetch_add_i64(num_threads_ready_ptr(), 1);
    int64_t ready = atomic_load_i64(num_threads_ready_ptr());
    while (ready < num_threads()) {
        // wait for all threads to be ready
        ready = atomic_load_i64(num_threads_ready_ptr());
        // todo; deschedule
    }
    if (tid() == 0) {
        // call main
        auto *call_main_p = new_task<task_impl<call_main>>(argc, argv);
        spawn(call_main_p);
    };
    
    while (*terminate_ptr() == 0) {
        // wait for termination
        find_work();
        // todo; deschedule
    }
    return 0;
}    


