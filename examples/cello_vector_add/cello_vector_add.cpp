// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <util/timer.hpp>
#include <vector>
using namespace DrvAPI;

typedef DrvAPI::float_type float_type;

int CelloMain(int argc, char *argv[])
{
    int n = strtol(argv[1], nullptr, 10);
    printf("n = %d\n", n);
    pointer<float_type> a = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, n*sizeof(float_type));
    pointer<float_type> b = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, n*sizeof(float_type));
    pointer<float_type> c = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, n*sizeof(float_type));
    {
        util::timer _("init");
        cello::parallel_for(0, n, 1, [a,b](int i) mutable {
            a[i] = i;
            b[i] = i;
        });
    }
    {
        util::timer _("vadd");
        cello::parallel_for(0, n, 1, [&v, &x, a, b, c](int i) mutable {
            c[i] = a[i] + b[i];
            v[i] = true;
        });
    }
    return 0;    
}
