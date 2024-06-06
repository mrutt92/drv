#include <cello.hpp>
#include "cello_vector_add.hpp"
#include <cstdio>

int CelloMain(int argc, char *argv[])
{
    printf("hello, from cello vector add\n");
    printf("vector add on size %d\n", vsize);
    pointer<float> a = (pointer<float>)allocate_dram(sizeof(float)*vsize);
    pointer<float> b = (pointer<float>)allocate_dram(sizeof(float)*vsize);
    pointer<float> c = (pointer<float>)allocate_dram(sizeof(float)*vsize);
    common::foreach_block{true}(0, vsize, 8, [=](idx_type i, idx_type j) mutable {
        common::foreach{false}(i, j, 1, [=](idx_type k) mutable {
            c[k] = a[k] + b[k];
        });
    });
    return 0;
}
