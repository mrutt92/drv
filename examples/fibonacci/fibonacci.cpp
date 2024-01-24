// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <cello.hpp>

using namespace DrvAPI;

long fibonacci(long n) {
    if (n <= 1) {
        return n;
    }
    
    size_t _;
    
    cello::joiner joiner;
    cello::joiner_ref jref(&joiner);
    jref.add(2);

    DrvAPIVar<long> x, y;
    cello::task_impl fib_task([n, &x, jref] () mutable {
        x = fibonacci(n - 1);
        jref.join();
    });

    cello::spawn(&fib_task);
    
    y = fibonacci(n - 2);

    jref.sync();

    return (long)x + (long)y;
}

int CelloMain(int argc, char** argv) {
    long n = strtol(argv[1], NULL, 10);    
    printf("Thread %ld: Running fibonacci(%ld)\n", cello::tid(), n);
    long r = fibonacci(n);
    printf("Thread %ld: fibonacci(%ld) = %ld\n", cello::tid(), n, r);
    return 0;
}

