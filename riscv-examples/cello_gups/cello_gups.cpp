#include <stdio.h>
#include "cello.hpp"
#include "cello_gups.hpp"
#include "pandohammer/addressmap.hpp"
#include "pandohammer/storage.h"
#include "pandohammer/allocator.h"
#include "pandohammer/mmio.h"

uint64_t rand(uint64_t *seed)
{
    // xorshift
    int64_t x = *seed;
    if (x == 0) {
        x = cello::tid()+1;
    }
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *seed = x;
    return x;
}

int CelloGups(int64_t gups_table_size, int64_t updates)
{
    int64_t *dram_base = reinterpret_cast<int64_t*>
        (allocate_dram(gups_table_size * sizeof(int64_t)));    
    printf("DRAM base address: %p\n", dram_base);

    static l1sp_storage(uint64_t) thread_seeds[CORE_THREADS];
    ph_print_time();
    cello::parallel_for(0l, updates, 1l, [dram_base, gups_table_size](int64_t i) {
        int64_t index = rand(&thread_seeds[cello::tid()]) % gups_table_size;
        dram_base[index] ^= dram_base[index];
    });
    ph_print_time();
    return 0;
}
