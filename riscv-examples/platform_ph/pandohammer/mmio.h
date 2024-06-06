// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington
/* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved. */

#ifndef PANDOHAMMER_MMIO_H
#define PANDOHAMMER_MMIO_H
#include <string.h>
#include <pandohammer/mmio_reg.h>
#ifdef __cplusplus
extern "C" {
#endif

static inline void ph_print_float(float x)
{
    *(volatile float*)PH_PRINT_FLOAT = x;
}

static inline void ph_print_int(long x)
{
    *(volatile long*)PH_PRINT_INT = x;
}

static inline void ph_print_hex(unsigned long x)
{
    *(volatile unsigned long*)PH_PRINT_HEX = x;
}

static inline void ph_print_char(char x)
{
    *(volatile char*)PH_PRINT_CHAR = x;
}

static inline void ph_puts(char *cstr)
{
    for (long i = 0; i < strlen(cstr); i++) {
        ph_print_char(cstr[i]);
    }
}

static inline void ph_print_time()
{
    *(volatile char*)PH_PRINT_TIME = 0;
}

#ifdef __cplusplus
}
#endif
#endif
