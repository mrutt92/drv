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
template <typename F>
void parallel_invoke_impl(cello::joiner_ref jref, F && f) {
    jref.add(1);
    f();
    jref.join();
}

template <typename F, typename ...Fs>
void parallel_invoke_impl(cello::joiner_ref jref, F && f, Fs && ...fs) {
    jref.add(1);
    auto child_func = [jref, f] () mutable {
        f();
        jref.join();
    };
    cello::task_impl<decltype(child_func)> child(child_func);
    spawn(&child);
    parallel_invoke_impl(jref, std::forward<Fs>(fs)...);
}

template <typename ...Fs>
void parallel_invoke(Fs && ...fs) {
    cello::joiner joiner;
    cello::joiner_ref jref(&joiner);
    parallel_invoke_impl(jref, std::forward<Fs>(fs)...);
    jref.sync();
}

}

extern "C" int CelloMain(int argc, char *argv[]);

#endif // CELLO_HPP
