#ifndef CELLO_CORE_DRVR_HPP
#define CELLO_CORE_DRVR_HPP
#include <utility>
#include <cstdlib>
#include <new>
#include "cello_core_common.hpp"
#include "cello_drvr_internal.hpp"
#include "pandohammer/cpuinfo.h"
#include "pandohammer/atomic.h"
#include "pandohammer/profile.h"

#ifndef CORE_THREADS
#error "CORE_THREADS not defined"
#endif

namespace cello
{
/**
 * task base class
 */
struct task {
public:
    task() {}
    virtual void execute() = 0;
};

/**
 * task implementation
 */
template <typename F>
struct task_impl : public task {
public:
    template <typename ...Args>
    task_impl(Args &&...args) : f_(std::forward<Args>(args)...) {}
    void execute() override { f_(); }
private:
    F f_;
};

/**
 * allocate a new task
 */
template <typename TaskType, typename ...Args>
TaskType *new_task(Args &&...args) {
    void *task = malloc(sizeof(TaskType));
    new (task) TaskType(std::forward<Args>(args)...);
    return reinterpret_cast<TaskType *>(task);
}

/**
 * free a task
 */
template <typename TaskType>
void free_task(TaskType *task) {
    task->~TaskType();
    free(task);
}

/**
 * @brief spawn a task
 * 
 * @param task 
 */
void spawn(task *task);

/**
 * @brief yield execution of this task
 */
void yield();


/**
 * @brief Thread tag guard - restores old tag when out of scope
 */
class TagGuard {
public:
    TagGuard(int tag) {
        //old_tag_ = DrvAPIThread::current()->setTag(tag);
    }
    TagGuard(const TagGuard &) = delete;
    TagGuard &operator=(const TagGuard &) = delete;
    TagGuard(TagGuard &&) = delete;
    TagGuard &operator=(TagGuard &&) = delete;

    ~TagGuard() {
        //DrvAPIThread::current()->setTag(old_tag_);
    }
    int old_tag_;
};


/**
 * encode an absolute thread id
 */
inline long tid(thread_id_t &thread_id) {
    return thread_id.pxn*numPXNPods()*numPodCores()*numCoreThreads()
        +  thread_id.pod*numPodCores()*numCoreThreads()
        +  thread_id.core*numCoreThreads()
        +  thread_id.thread;
}

/**
 * return the absolute thread id
 */
inline long tid() {
    thread_id_t thread_id = {myPXNId(), myPodId(), myCoreId(), myThreadId() };
    return tid(thread_id);
}

/**
 * return the number of threads
 */
inline long num_threads() {
    return numPXN()*numPXNPods()*numPodCores()*numCoreThreads();
}

/**
 * joiner
 */
struct joiner {
public:
    joiner() {}
    int64_t &count() { return count_; }
    int64_t &joined() { return joined_; }
    const int64_t &count() const { return count_; }
    const int64_t &joined() const { return joined_; }

    /**
     * initialize the joiner
     */
    void init() {
        count() = 0;
        joined() = 0;
    }

    /**
     * add to the joiner
     */
    void add(int64_t num) {
        atomic_fetch_add_i64(&count(), num);
    }

    /**
     * join the joiner
     */
    void join() {
        atomic_fetch_add_i64(&joined(), 1);
    }

    /**
     * join the joiner
     */
    void sync() {
        while (joined() < count()) {
            cello::yield();
        }
    }

    int64_t count_ = 0;
    int64_t joined_ = 0;
};

/**
 * joiner_ref
 */
using joiner_ref = joiner &;

/////////////////////
// Parallel invoke //
/////////////////////
// define child task that executes f1
template <typename F>
struct invoke_child : task {
    invoke_child(cello::joiner_ref jref, F &&f) : _jref(jref), _f(f) {}
    void execute() override {
        _f();
        _jref.join();
    }
    cello::joiner_ref _jref;
    F _f;
};

/**
 * @brief parallel invoke two functors
 * returns when all functors have completed
 */
template <typename F1, typename F2>
void parallel_invoke_impl(F1 && f1, F2 && f2) {
    // create a joiner
    cello::joiner joiner;
    joiner.init();
    joiner.add(1);

    // spawn the child task
    auto *child = new_task<invoke_child<F1>>(joiner, std::forward<F1>(f1));
    spawn(child);

    // execute f2 directly
    f2();
    joiner.sync();
    free_task(child);
}

/**
 * @brief parallel invoke two functors
 * returns when all functors have completed
 */
template <typename F1, typename F2>
void parallel_invoke(F1 && f1, F2 && f2) {
    TagGuard grd(CELLO_TAG);
    parallel_invoke_impl
        ([&](){ TagGuard grd(DEFAULT_TAG); f1(); },
         [&](){ TagGuard grd(DEFAULT_TAG); f2(); });
}

/**
 * @brief parallel invoke two functors
 * returns when all functors have completed
 */
template <typename F1, typename F2, typename F3>
void parallel_invoke_impl(F1 && f1, F2 && f2, F3 && f3) {
    // create a joiner
    cello::joiner joiner;
    joiner.init();
    joiner.add(2);
    
    // spawn the child task
    auto *child1 = new_task<invoke_child<F1>>(joiner, std::forward<F1>(f1));
    auto *child2 = new_task<invoke_child<F2>>(joiner, std::forward<F2>(f2));
    spawn(child1);
    spawn(child2);

    // execute f2 directly
    f3();
    joiner.sync();
    free_task(child1);
    free_task(child2);
}

/**
 * @brief parallel invoke two functors
 * returns when all functors have completed
 */
template <typename F1, typename F2, typename F3>
void parallel_invoke(F1 && f1, F2 && f2, F3 && f3) {
    TagGuard grd(CELLO_TAG);
    parallel_invoke_impl
        ([&](){ TagGuard grd(DEFAULT_TAG); f1(); },
         [&](){ TagGuard grd(DEFAULT_TAG); f2(); },
         [&](){ TagGuard grd(DEFAULT_TAG); f3(); });    
}

//////////////////
// Parallel for //
//////////////////

template <typename Idx>
struct loop_info {
    loop_info(Idx start, Idx stop, Idx step, Idx grain) :
        start(start), stop(stop), step(step), grain(grain) {
        if (step != 0) {
            iters = ((stop-start)+(step-1))/step;
        } else {
            iters = 0;
        }

        if (iters < 0)
            iters = 0;

        mid = start + step*(iters/2);
    }
    loop_info(Idx start, Idx stop, Idx step) :
        loop_info(start, stop, step, 1) {
        grain = iters / (num_threads() * 8);
        if (grain < 1)
            grain = 1;

        //if (grain > 2048)
        // grain = 2048;
    }

    Idx leafs() const  { return iters/grain; }

    loop_info<Idx> left() const {
        return loop_info<Idx>(start, mid, step, grain);
    }
    loop_info<Idx> right() const {
        return loop_info<Idx>(mid, stop, step, grain);
    }
    Idx start;
    Idx stop;
    Idx step;
    Idx grain;
    Idx iters;
    Idx mid;
};

/**
 * @brief parallel for loop
 */
template <typename Idx, typename F>
void parallel_for_impl(const cello::loop_info<Idx> &info, F && body) {
    if (info.leafs() == 0) {
        return;
    } else if (info.leafs() == 1) {
        for (Idx i = info.start; i < info.stop; i += info.step)
            body(i);
        return;
    } else {
        struct child_branch {
            child_branch(const cello::loop_info<Idx> &info, F && body) :
                info(info), body(body) {}
            void operator()() {
                cello::parallel_for_impl(info, body);
            }
            loop_info<Idx> info;
            F body;
        };
        parallel_invoke
            (child_branch(info.left(), std::forward<F>(body)),
             child_branch(info.right(), std::forward<F>(body))
             );
        return;
    }    
}

/**
 * @brief parallel for loop
 */
template <typename Idx, typename F>
void parallel_for(const cello::loop_info<Idx> &info, F && body) {
    TagGuard grd(CELLO_TAG);    
    parallel_for_impl
        (info,
         [&](Idx i){ TagGuard grd(DEFAULT_TAG); body(i); });
}

/**
 * @brief parallel for loop
 */
template <typename Idx, typename F>
void parallel_for(Idx start, Idx stop, Idx step, F && body) {
    // create loop info
    loop_info<Idx> info(start, stop, step);
    parallel_for( info, std::forward<F>(body));
}

/**
 * @brief parallel for loop
 */
template <typename Idx, typename F>
void parallel_for(Idx start, Idx stop, Idx step, Idx grain, F && body) {
    // create loop info
    loop_info<Idx> info(start, stop, step, grain);
    parallel_for( info, std::forward<F>(body));
}
}

// Cello main
extern "C" int CelloMain(int argc, char *argv[]);

#endif
