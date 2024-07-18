// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <pandohammer/atomic.h>
#include <pandohammer/cpuinfo.h>
#include <pandohammer/mmio.h>

#define decl_l1sp(type) __attribute__((section(".l1sp"))) type
#define decl_l2sp(type) __attribute__((section(".l2sp"))) type
#define decl_dram(type) __attribute__((section(".dram"))) type


static decl_l1sp (long) l1sp;
decl_l2sp (long) l2sp;
decl_dram (long) dram;

int main()
{
    ph_print_int((long)cycle());
    ph_print_hex((unsigned long)&l1sp);
    asm volatile("ld x0, %0" : : "m"(l1sp));
    ph_print_int((long)cycle());
    ph_print_hex((unsigned long)&l2sp);
    asm volatile("ld x0, %0" : : "m"(l2sp));
    ph_print_int((long)cycle());
    ph_print_hex((unsigned long)&dram);
    asm volatile("ld x0, %0" : : "m"(dram));
    ph_print_int((long)cycle());
    return 0;
}
