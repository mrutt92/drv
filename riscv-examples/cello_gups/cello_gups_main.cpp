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
#include "cello_gups.hpp"

int CelloMain(int argc, char *argv[])
{
    CelloGups(GUPS_TABLE_SIZE, GUPS_UPDATES);
    return 0;
}
