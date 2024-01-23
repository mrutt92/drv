#ifndef CELLO_HPP
#define CELLO_HPP
#include <DrvAPI.hpp>

namespace cello
{

/**
 * task base class
 */
struct __task {
public:
    __task() {}
    virtual void execute() = 0;
};

/**
 * task implementation
 */
template <typename F>
struct __task_impl : public __task {
public:
    __task_impl(F f) : f_(f) {}
    void execute() override { f_(); }
private:
    F f_;
};

/**
 * @brief spawn a task
 * 
 * @param task 
 */
void spawn(__task *task);

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

}

extern "C" int CelloMain(int argc, char *argv[]);

#endif // CELLO_HPP
