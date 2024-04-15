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
l1sp_storage(int) wait_until_not_zero;

static int my_printf(const char*fmt, ...)
{
#if 1
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    write(STDOUT_FILENO, buffer, ret);
    return ret;
#else
    return 0;
#endif
}

int CelloMain(int argc, char *argv[])
{
    cello::parallel_invoke(
        []() {
            printf("Hello from task 0: core %d\n", myCoreId());
        },
        []() {
            printf("Hello from task 1: core %d\n", myCoreId());
        }
    );
    return 0;
}
