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

int CelloMain(int argc, char *argv[])
{
    cello::parallel_for(0, 32, 1, [](int i) {
        ph_print_int(i);
    });
    return 0;
}
