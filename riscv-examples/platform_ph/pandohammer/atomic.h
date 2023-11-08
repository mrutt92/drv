// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#ifndef PANDOHAMMER_ATOMIC_H
#define PANDOHAMMER_ATOMIC_H
#include <stdint.h>
inline int atomic_fetch_add(volatile int *ptr, int val)
{
    int ret;
    asm volatile("amoadd.w %0, %2, 0(%1)" : "=r"(ret): "r"(ptr) , "r"(val));
    return ret;
}
#endif
