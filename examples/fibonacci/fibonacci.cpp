// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <cello.hpp>

using namespace DrvAPI;

long fibonacci(long n) {
    if (n <= 1) {
        return n;
    }
    long _x, _y;
    long _sync = 2;
    DrvAPIPointer<long> x_ptr, y_ptr, sync_ptr;
    DrvAPIAddress x_addr, y_addr, sync_addr;
    size_t _;

    DrvAPINativeToAddress(&_x, &x_addr, &_);
    DrvAPINativeToAddress(&_y, &y_addr, &_);
    DrvAPINativeToAddress(&_sync, &sync_addr, &_);

    x_ptr = x_addr;
    y_ptr = y_addr;
    sync_ptr = sync_addr;

    cello::task_impl fib_task([n, x_ptr, sync_ptr]() {
        *x_ptr = fibonacci(n - 1);
        atomic_add(sync_ptr, -1);
    });

    cello::spawn(&fib_task);
    
    *y_ptr = fibonacci(n - 2);
    atomic_add(sync_ptr, -1);

    while (*sync_ptr != 0) {
        cello::yield();
    }

    return *x_ptr + *y_ptr;
}

int CelloMain(int argc, char** argv) {
    long n = strtol(argv[1], NULL, 10);    
    printf("Thread %ld: Running fibonacci(%ld)\n", cello::tid(), n);
    long r = fibonacci(n);
    printf("Thread %ld: fibonacci(%ld) = %ld\n", cello::tid(), n, r);
    return 0;
}

