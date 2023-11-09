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
#include <atomic>
#include <array>
#include "common.hpp"

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

#define pr_info(fmt, ...)                                               \
    thread_safe_printf("PH: Core %d, Thread %d: " fmt                   \
                       ,myCoreId()                                      \
                       ,myThreadId()                                    \
                       ,##__VA_ARGS__)

#define DEBUG
#ifdef  DEBUG
#define pr_dbg(fmt,...)                         \
    pr_info(fmt, ##__VA_ARGS__)
#else
#define pr_dbg(fmt,...)
#endif

__l2sp__ std::atomic<int64_t> ph_ready;
__l2sp__ std::atomic<int64_t> cp_ready;

// graph data
__l2sp__ vertex_t g_V;
__l2sp__ vertex_t g_E;
__l2sp__ vertex_pointer_t g_fwd_offsets;
__l2sp__ vertex_pointer_t g_fwd_edges;
__l2sp__ vertex_pointer_t g_rev_offsets;
__l2sp__ vertex_pointer_t g_rev_edges;
__l2sp__ vertex_pointer_t g_distance;

__l2sp__ bool g_rev_not_fwd;
__l2sp__ int  g_mf;
__l2sp__ int  g_mu;

__l2sp__ frontier_data        frontier[3];

/**
 * @brief Wait for the CP to complete initialization
 */
int wait_for_cp()
{
    pr_dbg("Telling CP we're ready\n");
    // let ph know we're ready
    ph_ready.fetch_add(1, std::memory_order_relaxed);
    
    // command_processor_ready.store(-1, std::memory_order_relaxed);
    int64_t ready = cp_ready.load(std::memory_order_relaxed);
    int x = 0;
    while (ready != 1) {
        // wait for command processor to be ready
        wait(numPodCores()*myCoreThreads());
        ready = cp_ready.load(std::memory_order_relaxed);
        x++;
    }
    pr_dbg("Command processor ready\n");

}

int main()
{
    for (int i = 0; i < 3; i++) {
        frontier_ref f = &frontier[i];
        pr_dbg("frontier[%d].vertices = %llx\n", i, f.vertices());
    }
    return 0;
}
