#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP
#ifndef RISCV
#include <DrvAPI.hpp>
#else
#include "pandohammer/allocator.h"
#endif
namespace common
{
struct Allocator
{
    pointer<void> allocate(size_t size) {
#ifndef RISCV
        return DrvAPI::DrvAPIMemoryAlloc
            (DrvAPI::DrvAPIMemoryDRAM, size);
#else
        return allocate_dram(size);
#endif        
    }

    void deallocate(pointer<void> ptr, size_t size) {
#ifndef RISCV
#else
#endif
    }
};
}
#endif
