// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include "DrvAPI.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <atomic>
#include <inttypes.h>
namespace DrvAPI
{

static constexpr int64_t STATUS_UNINIT = 0;
static constexpr int64_t STATUS_INIT = 1;
static constexpr int64_t STATUS_INIT_IN_PROCESS = 2;

// data struct for memory meta data
struct global_memory_data {
    DrvAPIAddress  base;
    int64_t status;

  static void copy(global_memory_data &dst,
		   const value_handle<global_memory_data> &src);

  static void copy(value_handle<global_memory_data> &dst,
		   const global_memory_data & src);
};

// reference wrapper class for global memory
DRV_API_VALUE_HANDLE_BEGIN(global_memory_data)
    DRV_API_VALUE_HANDLE_FIELD(global_memory_data, base, DrvAPIAddress, base)
    DRV_API_VALUE_HANDLE_FIELD(global_memory_data, status, int64_t, status)
    void init(DrvAPIMemoryType type) {
        // 1. check if initialized
       int64_t s = status();
       if (s == STATUS_INIT) {
             return;
       }
       // 2. check if I should initialize
       s = DrvAPI::atomic_cas(&status(), STATUS_UNINIT, STATUS_INIT_IN_PROCESS);
       if (s == STATUS_UNINIT) {
             DrvAPISection &section = DrvAPI::DrvAPISection::GetSection(type);
             DrvAPIAddress sz = static_cast<DrvAPIAddress>(section.getSize());
             // align to 16-byte boundary
             sz = (sz + 15) & ~15;
             // make a global address
             DrvAPIAddress localBase = section.getBase(myPXNId(), myPodId(), myCoreId());
             DrvAPIAddress globalBase = toGlobalAddress(localBase, myPXNId(), myPodId(), myCoreY(), myCoreX());
             base() = globalBase + sz;
             // TODO: FENCE
             status() = STATUS_INIT;
             return;
       }
       // 3. wait until initialized
       while (s == STATUS_INIT_IN_PROCESS) {
           DrvAPI::wait(32);
           s = status();
       }
    }
    DrvAPIPointer<void> allocate(size_t size) {
        // size should be 8-byte aligned
        size = (size + 7) & ~7;
        uint64_t addr = DrvAPI::atomic_add<uint64_t>(&base(), size);
        return DrvAPIPointer<void>(addr);
    }
DRV_API_VALUE_HANDLE_END(global_memory_data)

void global_memory_data::copy(global_memory_data &dst,
			      const value_handle<global_memory_data> &src) {
  dst.base = src.base();
  dst.status = src.status();
}

void global_memory_data::copy(value_handle<global_memory_data> &dst,
			      const global_memory_data & src) {
  dst.base() = src.base;
  dst.status() = src.status;
}

using global_memory_ref = value_handle<global_memory_data>;

namespace allocator
{
DrvAPIGlobalL1SP<global_memory_data> l1sp_memory; //!< L1SP memory allocator
DrvAPIGlobalL2SP<global_memory_data> l2sp_memory; //!< L2SP memory allocator
DrvAPIGlobalDRAM<global_memory_data> dram_memory; //!< DRAM memory allocator
}

void DrvAPIMemoryAllocatorInit() {
    using namespace allocator;
    if (!isCommandProcessor()) {
        // 1. init l1sp
        l1sp_memory.init(DrvAPIMemoryType::DrvAPIMemoryL1SP);
        // 2. init l2sp
        l2sp_memory.init(DrvAPIMemoryType::DrvAPIMemoryL2SP);
    }
    // 3. init dram
    dram_memory.init(DrvAPIMemoryType::DrvAPIMemoryDRAM);
}

DrvAPIPointer<void> DrvAPIMemoryAlloc(DrvAPIMemoryType type, size_t size) {
    // if we use l1sp for stack, disallow allocation of l1sp
    if (type == DrvAPIMemoryType::DrvAPIMemoryL1SP
        && DrvAPIThread::current()->stackInL1SP()) {
        std::cerr << "ERROR: cannot allocate L1SP memory for stack" << std::endl;
        exit(1);
    }

    // size should be 8-byte aligned
    global_memory_ref mem(0);
    switch (type) {
    case DrvAPIMemoryType::DrvAPIMemoryL1SP:
        return allocator::l1sp_memory.allocate(size);
    case DrvAPIMemoryType::DrvAPIMemoryL2SP:
        return allocator::l2sp_memory.allocate(size);
    case DrvAPIMemoryType::DrvAPIMemoryDRAM:
        return allocator::dram_memory.allocate(size);
    default:
        std::cerr << "ERROR: invalid memory type: " << static_cast<int>(type) << std::endl;
        exit(1);
    }
    return {0};
}

void DrvAPIMemoryFree(const DrvAPIPointer<void> &ptr) {
}


}
