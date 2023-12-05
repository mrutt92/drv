// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington
#include <DrvAPI.hpp>
#include <cstdio>
#include <inttypes.h>
#include <vector>

#define pr_info(fmt, ...)                                               \
    do {                                                                \
        printf("PXN %3d: POD: %3d: CORE %3d: " fmt ""                   \
               ,myPXNId()                                               \
               ,myPodId()                                               \
               ,myCoreId()                                              \
               ,##__VA_ARGS__);                                         \
    } while (0)


int ToAddressMain(int argc, char *argv[])
{
    using namespace DrvAPI;
    uint64_t x = 0;
    DrvAPIAddress addr = 0;
    std::size_t size = 0;
    DrvAPINativeToAddress(&x, &addr, &size);

    DrvAPIPointer<uint64_t> as_sim_pointer = addr;
    uint64_t wval = 0xdeadbeef;
    pr_info("Writing %010" PRIx64 " to Simulator Address %" PRIx64"\n"
            ,wval
            ,addr
            );
    *as_sim_pointer = 0xdeadbeef;
    pr_info("Reading %010" PRIx64 " from Native Address %p\n"
            ,x
            ,&x
            );
    return 0;
}

declare_drv_api_main(ToAddressMain);
