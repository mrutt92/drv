// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include <pandohammer/mmio.h>
#include <pandohammer/cpuinfo.h>
#include <pandohammer/atomic.h>
#include <cstdint>
#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <unistd.h>
#include <cstdio>

#define __l1sp__ __attribute__((section(".dmem")))
#define __l2sp__ __attribute__((section(".dram"))) // TODO: this is actually l2sp; need fix in linker script

int thread_safe_printf(const char* fmt, ...)
{
    char buf[256];
    va_list va;
    va_start(va, fmt);
    int ret = vsnprintf(buf, sizeof(buf), fmt, va);
    write(STDOUT_FILENO, buf, strlen(buf));
    va_end(va);
    return ret;
}

/**
 * no-op for x cycles
 */
void wait(volatile int& x) {
    for(int i  = 0; i < x; i++) {
        asm volatile("nop");
    }    
}

#define THREADS                                 \
    (numPodCores()*myCoreThreads())

struct barrier {
    int count_;
    int signal_;
    int sense_;
    
    int& count() { return count_; }
    int& signal() { return signal_; }
    int& sense() { return sense_; }
    
    void sync() {
        sync([](){});
    }
    
    template <typename F>
    void sync(F f) {
        int signal_ = signal();
        int count_ = atomic_fetch_add(&count(), 1);
        if (count_ == THREADS-1) {
            count() = 0;
            f();
            signal() = !signal_;
        } else {
            static constexpr int backoff_limit = 1000;
            int backoff_counter = 8;
            while (signal() == signal_) {
                wait(backoff_counter);
                backoff_counter = std::min(backoff_counter*2, backoff_limit);
            }
        }    
    }

};



__l2sp__ barrier barrier;

__l2sp__ int counter = 0;

#define THREAD_SAFE
int main()
{
#ifdef THREAD_SAFE
    for (int i = 0; i < myCoreThreads(); i++) {
        barrier.sync();
        if (i == myThreadId())
            counter++;
        barrier.sync();
    }
#else
    counter++;
#endif
    thread_safe_printf("Core %d, Thread %d: counter = %d\n"
                       ,myCoreId()
                       ,myThreadId()
                       ,counter);
    return 0;
}
