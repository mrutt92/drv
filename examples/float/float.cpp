// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <string>
#include <cstdio>
#include <inttypes.h>
using namespace DrvAPI;

DrvAPIGlobalDRAM<float_type> g_a;
DrvAPIGlobalDRAM<float_type> g_b;
DrvAPIGlobalDRAM<float_type> g_c;


int FloatMain(int argc, char *argv[])
{
    float_type a = 1.0;
    float_type b = 2.0;
    float_type c;
    c = a * b;

    g_a = a;
    g_b = b;
    g_c = c;

    g_c = g_a * g_b;
    g_c = muladd(g_a, g_b, g_c);
    printf("c   = %f\n", (float)c);
    printf("g_c = %f\n", (float)g_c);
    printf("float_type::stats = %s\n", float_type::Stats().to_string().c_str());
    return 0;
}

declare_drv_api_main(FloatMain);
