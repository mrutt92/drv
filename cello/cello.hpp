#ifndef CELLO_HPP
#define CELLO_HPP
#include <DrvAPI.hpp>
#include <functional>
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
struct task_impl : public task {
public:
    template <typename F>
    task_impl(F f) : f_(f) {}
    void execute() override { f_(); }
private:
    std::function<void()> f_;
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

}

extern "C" int CelloMain(int argc, char *argv[]);

#endif // CELLO_HPP
