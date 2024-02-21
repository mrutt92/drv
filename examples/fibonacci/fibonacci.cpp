// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <cello.hpp>

using namespace DrvAPI;

long fibonacci(long n) {
    if (n <= 1) {
        return n;
    }

    DrvAPIVar<long> x, y;
    cello::parallel_invoke(
        [&x, n] () mutable {
            x = fibonacci(n - 1);
        },
        [&y, n] () mutable {
            y = fibonacci(n - 2);
        }
    );
    return (long)x + (long)y;
}

int CelloMain(int argc, char** argv) {
    long n = strtol(argv[1], NULL, 10);    
    printf("Thread %ld: Running fibonacci(%ld)\n", cello::tid(), n);
    long r = 0;
    r = fibonacci(n);
    printf("Thread %ld: fibonacci(%ld) = %ld\n", cello::tid(), n, r);
    return 0;
}

