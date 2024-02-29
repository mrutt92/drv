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

DrvAPIGlobalDRAM<float_type> contrib;
DrvAPIGlobalDRAM<float_type> old_rank;
DrvAPIGlobalDRAM<int>        out_degree;
DrvAPIGlobalDRAM<float_type> new_rank;

int FloatMain(int argc, char *argv[])
{
    int V = 128;
    g_a = 1.0/V;
    printf("g_a: %1.12f\n", (float)g_a);

    float_type damp = 0.85;
    float_type beta_score = (1.0 - damp) / V; // *
    printf("beta_score: %1.12f\n", (float)beta_score);

    contrib = 0.0;
    old_rank = 1.0/8;
    out_degree = 2;

    contrib = old_rank / out_degree;
    printf("contrib: %1.12f\n", (float)contrib);

    float_type rank = 0.0;
    rank += contrib;
    new_rank = rank;
    printf("new_rank: %1.12f\n", (float)new_rank);

    rank = damp * new_rank + beta_score;
    printf("damp: %1.12f, new_rank: %1.12f, beta_score: %1.12f\n", (float)damp, (float)new_rank, (float)beta_score);
    printf("rank: %1.12f\n", (float)rank);
    old_rank = rank;
    printf("old_rank: %1.12f\n", (float)old_rank);
    return 0;
}

declare_drv_api_main(FloatMain);
