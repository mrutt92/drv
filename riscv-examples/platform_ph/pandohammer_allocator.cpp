#include <new>
#include <stdio.h>
#include <stdint.h>
#include "pandohammer/allocator.h"
#include "pandohammer/atomic.h"
#include "pandohammer/cpuinfo.h"
#include "pandohammer/addressmap.hpp"

typedef int64_t status_t;
static constexpr status_t STATUS_UNINIT = 0;
static constexpr status_t STATUS_INIT = 1;
static constexpr status_t STATUS_INIT_IN_PROCESS = 2;

/**
 * @brief status guard for doing once
 */
template <typename F>
void do_once(status_t *status, F && f) {
    status_t s = atomic_compare_and_swap_i64
        (status, STATUS_UNINIT, STATUS_INIT_IN_PROCESS);
    // do init; hasn't happened yet
    if (s == STATUS_UNINIT) {
        f();
        atomic_fence();
        *status = s = STATUS_INIT;
    }
    // wait for init to complete
    while (s != STATUS_INIT) {
        // todo: wait
        s = *status;
    }
}

#define FIELD(type, field, field_data)                  \
    type field_data;                                    \
    type & field() { return field_data; }               \
    const type & field() const { return field_data; }

class bump_allocator {
public:
    FIELD(intptr_t, base, _base);
    FIELD(size_t, end, _end);

    bump_allocator(intptr_t base, size_t size)
        : _base(base), _end(base+size) {
    }

    void *allocate(size_t sz) {
        sz = (sz + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t)-1);
        if (base() + sz > end()) {
            return nullptr;
        }
        intptr_t ptr = atomic_fetch_add_i64(&base(), sz);
        return reinterpret_cast<void *>(ptr);
    }
};

class allocator {
public:
    FIELD(bump_allocator, bump, _bump);

    allocator(uintptr_t bump_base, size_t bump_size)
        : _bump(bump_base, bump_size) {
    }

    void *allocate(size_t sz) {
        return bump().allocate(sz);
    }
};

#define dram_status_offset \
    0x00
#define dram_allocator_offset \
    (dram_status_offset + sizeof(status_t))

#define dram_allocator_data_offset              \
    (dram_allocator_offset + sizeof(allocator))

/**
 * hardcode the address of the dram status
 */
#define dram_status \
    (*((status_t*)(DRAM_BASE_ADDR(myPXNId())+dram_status_offset)))
/**
 * hardcode the address of the dram allocator
 */
#define dram_allocator_ptr                                      \
    ((allocator*)(DRAM_BASE_ADDR(myPXNId())+dram_allocator_offset))

#define dram_allocator                                          \
    (*dram_allocator_ptr)

/**
 * hardcode the address of the dram data 
 */
#define dram_allocator_data                     \
    ((intptr_t)DRAM_BASE_ADDR(myPXNId())+dram_allocator_data_offset)

/**
 * @brief initialize the dram allocator
 */
void dram_allocator_init() {
    do_once(&dram_status, []() {
         new (&dram_allocator) allocator
             (dram_allocator_data, DRAM_SIZE - dram_allocator_data_offset);
    });
}

/**
 * @brief Allocate a block of memory from the DRAM
 * @param size The size of the block to allocate
 * @return A pointer to the allocated block
 */
void  *allocate_dram(size_t size) {
    return dram_allocator.allocate(size);
}

/**
 * @brief Deallocate a block of memory from the DRAM
 * @param ptr A pointer to the block to deallocate
 * @param size The size of the block to deallocate
 */
void deallocate_dram(void *ptr, size_t size) {
}
