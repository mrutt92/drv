// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <util/timer.hpp>

using namespace DrvAPI;
using namespace util;

using DrvAPI::pointer;

int CelloMain(int argc, char *argv[])
{
    int64_t gups_table_size, updates;
    gups_table_size = std::stoll(argv[1]);
    updates = std::stoll(argv[2]);
    printf("GUPS: table-size = %" PRId64 ", updates = %" PRId64 "\n", gups_table_size, updates);
    pointer<int64_t> table=0;
    {
        timer _("init");
        table = DrvAPI::DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, gups_table_size * sizeof(int64_t));
    }
    printf("GUPS: table = %s (%" PRIx64 ")\n",
	   DrvAPI::DrvAPIVAddress{table}.to_string().c_str(),
	   (uint64_t)table);
    {
        timer _("gups");
        cello::parallel_for(0l, updates, 1l, [table, gups_table_size](int64_t i) mutable {
            int64_t index = rand() % gups_table_size;
            int64_t value = table[index];
            table[index] = value ^ i;
        });
    }
    return 0;
}
