#ifndef CELLO_DRVX_INTERNAL_HPP
#define CELLO_DRVX_INTERNAL_HPP
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <deque>
namespace cello
{


/* shorthand for l1sp */
template <typename T>
using StaticL1SP = DrvAPI::DrvAPIGlobalL1SP<T>;

/* shorthand for l2sp */
template <typename T>
using StaticL2SP = DrvAPI::DrvAPIGlobalL2SP<T>;

/* shorthand for main-mem */
template <typename T>
using StaticMainMem = DrvAPI::DrvAPIGlobalDRAM<T>;

template <typename T>
using Pointer = DrvAPI::DrvAPIPointer<T>;

#ifndef CORE_THREADS
#define CORE_THREADS 16
#endif


/**
 * acquire a lock
 * automatically release lock when out of scope
 */
struct lock_guard {
public:
    lock_guard(Pointer<int> lock) : lock_(lock) {
        int cycles = 16;
        while (DrvAPI::atomic_cas(lock, 0, 1) != 0) {
            DrvAPI::nop(cycles);
        }
    }
    ~lock_guard() {
        *lock_ = 0;
    }
private:
    Pointer<int> lock_;
};

/**
 * a threads task queue
 */
struct task_queue {
public:
    std::deque<__task*>*queue = nullptr;
    int32_t lock = 0;    
};

DRV_API_REF_CLASS_BEGIN(task_queue)
/**
 * initialize the task queue
 */
void init() {
    queue() = new std::deque<__task*>();
    lock() = 0;
}
void destroy() {
    delete queue();
}

void push_front(__task* task) {
    lock_guard guard(&lock());
    queue().get()->push_front(task);
}

void push_back(__task* task) {
    lock_guard guard(&lock());
    queue().get()->push_back(task);
}

__task* pop_front() {
    lock_guard guard(&lock());
    if (queue().get()->empty()) {
        return nullptr;
    }
    auto task = queue().get()->front();
    queue().get()->pop_front();
    return task;
}

__task* pop_back() {
    lock_guard guard(&lock());
    if (queue().get()->empty()) {
        return nullptr;
    }
    auto task = queue().get()->back();
    queue().get()->pop_back();
    return task;
}

DRV_API_REF_CLASS_DATA_MEMBER(task_queue, queue)
DRV_API_REF_CLASS_DATA_MEMBER(task_queue, lock)
DRV_API_REF_CLASS_END(task_queue)

/**
 * get the task queue for the current thread
 */
task_queue_ref my_task_queue();

/**
 * a thread id
 */
struct thread_id_t {
    long thread = 0;
    long core = 0;
    long pod = 0;
    long pxn = 0;
};
    
/**
 * get the task queue of a specific thread
 */
task_queue_ref task_queue_of(const thread_id_t &tid);
}

#endif
