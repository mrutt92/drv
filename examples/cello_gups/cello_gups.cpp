// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>

using namespace DrvAPI;

template <typename T>
using pointer = DrvAPIPointer<T>;

int CelloMain(int argc, char *argv[])
{
    int64_t gups_table_size, updates;
    gups_table_size = std::stoll(argv[1]);
    updates = std::stoll(argv[2]);
    printf("GUPS: table-size = %" PRId64 ", updates = %" PRId64 "\n", gups_table_size, updates);
    pointer<int64_t> table = DrvAPI::DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, gups_table_size * sizeof(int64_t));
    cello::parallel_for(0l, updates, 1l, [table, gups_table_size](int64_t i) {
        int64_t index = rand() % gups_table_size;
        int64_t value = table[index];
        table[index] = value ^ i;
    });
    return 0;
}
