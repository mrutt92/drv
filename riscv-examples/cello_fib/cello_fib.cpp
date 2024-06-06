#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "cello.hpp"
#include "pandohammer/storage.h"
#include "pandohammer/addressmap.hpp"
#include "pandohammer/cpuinfo.h"
#include "pandohammer/atomic.h"
#include "pandohammer/mmio.h"

#ifndef FIB_N
#define FIB_N 8
#endif

int fib(int n)
{
    if (n <= 1) {
        return n;
    }
    int x, y;
    cello::parallel_invoke(
        [&] { x = fib(n - 1); },
        [&] { y = fib(n - 2); }
    );
    return x + y;
}

int CelloMain(int argc, char *argv[])
{
    printf("fib(%d) = %d\n", FIB_N, fib(FIB_N));
    return 0;
}
