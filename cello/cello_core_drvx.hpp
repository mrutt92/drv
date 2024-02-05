#ifndef CELLO_CORE_DRVX_HPP
#define CELLO_CORE_DRVX_HPP
#include <DrvAPI.hpp>
#include <functional>
#include <inttypes.h>
namespace cello
{

static constexpr int CELLO_TAG = 1;

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
    task_impl(F f) : f_(f) {}
    void execute() override { f_(); }
private:
    F f_;
};

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

inline long tid() {
    using namespace DrvAPI;
    return myPXNId()*numPXNPods()*numPodCores()*numCoreThreads()
        +  myPodId()*numPodCores()*numCoreThreads()
        +  myCoreId()*numCoreThreads()
        +  myThreadId();
}

inline long num_threads() {
    using namespace DrvAPI;
    return numPXNs()*numPXNPods()*numPodCores()*numCoreThreads();
}

/**
 * joiner
 */
struct joiner {
public:
    joiner() {}

    int64_t count = 0;
    int64_t joined = 0;
};

DRV_API_REF_CLASS_BEGIN(joiner)
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
    atomic_add(&count(), num);
}
/**
 * join the joiner
 */
void join() {
    atomic_add(&joined(), 1);
}
/**
 * join the joiner
 */
void sync() {
    while (joined() < count()) {
        // void *sp;
        // asm volatile("mov %%rsp, %0" : "=r"(sp));
        // printf("T %3ld: yielding: sp = %p\n", tid(), sp);
        yield();
    }
    // void *sp;
    // asm volatile("mov %%rsp, %0" : "=r"(sp));
    // printf("T %3ld: sync'd: sp = %p\n", tid(), sp);       
}
DRV_API_REF_CLASS_DATA_MEMBER(joiner, count)
DRV_API_REF_CLASS_DATA_MEMBER(joiner, joined)
DRV_API_REF_CLASS_END(joiner)

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
    cello::joiner_ref jref(&joiner);
    jref.add(1);

    // spawn the child task
    invoke_child<F1> child(jref, std::forward<F1>(f1));
    spawn(&child);

    // execute f2 directly
    f2();
    jref.sync();
}

/**
 * @brief parallel invoke two functors
 * returns when all functors have completed
 */
template <typename F1, typename F2>
void parallel_invoke(F1 && f1, F2 && f2) {
    DrvAPI::DrvAPITagGuard grd(CELLO_TAG);
    parallel_invoke_impl
        ([&](){ DrvAPI::DrvAPITagGuard grd(DrvAPI::DEFAULT_TAG); f1(); },
         [&](){ DrvAPI::DrvAPITagGuard grd(DrvAPI::DEFAULT_TAG); f2(); });
}

/**
 * @brief parallel invoke two functors
 * returns when all functors have completed
 */
template <typename F1, typename F2, typename F3>
void parallel_invoke_impl(F1 && f1, F2 && f2, F3 && f3) {
    // create a joiner
    cello::joiner joiner;
    cello::joiner_ref jref(&joiner);
    jref.add(2);
    
    // spawn the child task
    invoke_child<F1> child1(jref, std::forward<F1>(f1));
    invoke_child<F2> child2(jref, std::forward<F2>(f2));
    spawn(&child1);
    spawn(&child2);

    // execute f2 directly
    f3();
    jref.sync();
}

/**
 * @brief parallel invoke two functors
 * returns when all functors have completed
 */
template <typename F1, typename F2, typename F3>
void parallel_invoke(F1 && f1, F2 && f2, F3 && f3) {
    DrvAPI::DrvAPITagGuard grd(CELLO_TAG);
    parallel_invoke_impl
        ([&](){ DrvAPI::DrvAPITagGuard grd(DrvAPI::DEFAULT_TAG); f1(); },
         [&](){ DrvAPI::DrvAPITagGuard grd(DrvAPI::DEFAULT_TAG); f2(); },
         [&](){ DrvAPI::DrvAPITagGuard grd(DrvAPI::DEFAULT_TAG); f3(); });    
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

        if (grain > 2048)
            grain = 2048;
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

template <typename Idx, typename F>
void parallel_for(const cello::loop_info<Idx> &info, F && body) {
    DrvAPI::DrvAPITagGuard grd(CELLO_TAG);    
    parallel_for_impl
        (info,
         [&](Idx i){ DrvAPI::DrvAPITagGuard grd(DrvAPI::DEFAULT_TAG); body(i); });
}

template <typename Idx, typename F>
void parallel_for(Idx start, Idx stop, Idx step, F && body) {
    // create loop info
    loop_info<Idx> info(start, stop, step);
    parallel_for( info, std::forward<F>(body));
}

template <typename Idx, typename F>
void parallel_for(Idx start, Idx stop, Idx step, Idx grain, F && body) {
    // create loop info
    loop_info<Idx> info(start, stop, step, grain);
    parallel_for( info, std::forward<F>(body));
}

}

extern "C" int CelloMain(int argc, char *argv[]);

#endif // CELLO_HPP
