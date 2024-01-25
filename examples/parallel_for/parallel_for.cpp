// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
using namespace DrvAPI;

int CelloMain(int argc, char *argv[]) {
    int start = atoi(argv[1]);
    int stop = atoi(argv[2]);
    int step = atoi(argv[3]);
    int grain = atoi(argv[4]);
    if (grain != 0) {
        cello::parallel_for(start, stop, step, grain, [&](int i) {
            printf("%4d: Hello from thread %ld\n", i, cello::tid());
        });
    } else {
        cello::parallel_for(start, stop, step, [&](int i) {
            printf("%4d: Hello from thread %ld\n", i, cello::tid());
        });
    }
    return 0;
}


