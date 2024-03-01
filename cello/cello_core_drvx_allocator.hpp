#ifndef CELLO_CORE_DRVX_ALLOCATOR_HPP
#define CELLO_CORE_DRVX_ALLOCATOR_HPP
#include <DrvAPI.hpp>
#include <cello_core_drvx.hpp>
namespace cello
{

/**
 * @brief initialize the allocator
 */
void allocator_init();

/**
 * @brief allocate memory
 */
DrvAPI::pointer<void> allocate(uint64_t size);

/**
 * @brief deallocate memory
 */
void deallocate(DrvAPI::pointer<void> ptr, uint64_t size);

}
#endif
