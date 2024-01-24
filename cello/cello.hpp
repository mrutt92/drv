#ifndef CELLO_HPP
#define CELLO_HPP
#include <DrvAPI.hpp>
#include <functional>
#include <inttypes.h>
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
        yield();
    }
}
DRV_API_REF_CLASS_DATA_MEMBER(joiner, count)
DRV_API_REF_CLASS_DATA_MEMBER(joiner, joined)
DRV_API_REF_CLASS_END(joiner)

/////////////////////
// Parallel invoke //
/////////////////////

/**
 * @brief parallel invoke two functors
 * returns when all functors have completed
 */
template <typename F1, typename F2>
void parallel_invoke(F1 && f1, F2 && f2) {
    // create a joiner
    cello::joiner joiner;
    cello::joiner_ref jref(&joiner);
    jref.add(1);

    // define child task that executes f1
    struct child_func : task {
        child_func(cello::joiner_ref jref, F1 &&f) : _jref(jref), _f(f) {}
        void execute() override {
            _f();
            _jref.join();
        }
        cello::joiner_ref _jref;
        F1 _f;
    };

    // spawn the child task
    child_func child(jref, std::forward<F1>(f1));
    spawn(&child);

    // execute f2 directly
    f2();
    jref.sync();
}

#if 0
template <typename F>
void parallel_invoke_impl_async(F && f) {
    f();
}

template <typename F, typename ...Fs>
void parallel_invoke_impl_async(F && f, Fs && ...fs) {
    struct child_func : task {
        child_func(F &&f) : _f(f) {}
        void execute() override {
            _f();
        }
        F _f;
    };
    child_func child(std::forward<F>(f));
    spawn(&child);
    parallel_invoke_impl_async(std::forward<Fs>(fs)...);
}

/**
 * @brief parallel invoke multiple functors
 * returns immediately, functors may still be running
 */
template <typename ...Fs>
void parallel_invoke_async(Fs && ...fs) {
    parallel_invoke_impl_async(std::forward<Fs>(fs)...);
}


//////////////////
// Parallel for //
//////////////////

template <typename Idx>
struct loop_info {
    loop_info(Idx start, Idx stop, Idx step, Idx grain) :
        start(start), stop(stop), step(step), grain(grain) {
        if (step != 0)
            iters = ((stop-start)+(step-1))/step;

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
    Idx start;
    Idx stop;
    Idx step;
    Idx grain;
    Idx iters;
    Idx mid;
};
#endif

// template <typename Idx, typename F>
// void parallel_for(Idx start, Idx stop, Idx step, F && f) {
//     cello::joiner joiner;
//     cello::joiner_ref jref(&joiner);
//     for (Idx i = start; i < stop; i += step) {
//         jref.add(1);
//         auto child_func = [jref, f, i] () mutable {
//             f(i);
//             jref.join();
//         };
//         cello::task_impl<decltype(child_func)> child(child_func);
//         spawn(&child);
//     }
//     jref.sync();
// }
}

extern "C" int CelloMain(int argc, char *argv[]);

#endif // CELLO_HPP
