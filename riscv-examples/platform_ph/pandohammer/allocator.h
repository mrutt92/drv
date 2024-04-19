#ifndef PANDOHAMMER_ALLOCATOR_H
#define PANDOHAMMER_ALLOCATOR_H
#include <stddef.h>
#include <pandohammer/addressmap.hpp>
#ifdef __cplusplus
extern "C" {
#endif
    
/**
 * @brief initialize the dram allocator
 */
void dram_allocator_init(intptr_t dram_base, size_t dram_size);

/**
 * @brief Allocate a block of memory from the DRAM
 * @param size The size of the block to allocate
 * @return A pointer to the allocated block
 */
void  *allocate_dram(size_t size);

/**
 * @brief Deallocate a block of memory from the DRAM
 * @param ptr A pointer to the block to deallocate
 * @param size The size of the block to deallocate
 */
void deallocate_dram(void *ptr, size_t size);
#ifdef __cplusplus
}
#endif
#endif

