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


__l2sp__ std::atomic<int64_t> ph_ready;
__l2sp__ std::atomic<int64_t> cp_ready;

__l2sp__ frontier_data        frontier[2];

int main()
{
    thread_safe_printf("PH: Telling CP we're ready\n");
    // let ph know we're ready
    ph_ready.store(1, std::memory_order_relaxed);
    
    // command_processor_ready.store(-1, std::memory_order_relaxed);
    int64_t ready = cp_ready.load(std::memory_order_relaxed);
    ph_print_hex((unsigned long)&cp_ready);
    int x = 0;
    while (ready != 1) {
        // wait for command processor to be ready
        ready = cp_ready.load(std::memory_order_relaxed);
        x++;
    }
    thread_safe_printf("PH: Command processor ready\n");
    return 0;
}
