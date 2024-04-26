#ifndef POINTER_HPP
#define POINTER_HPP

#ifndef RISCV
#include <DrvAPI.hpp>
#endif // RISCV

/**
 * pointer type
 */
#ifdef RISCV
template <typename T>
using pointer = T*;
#else
template <typename T>
using pointer = DrvAPI::pointer<T>;
#endif

/**
 * volatile pointer type
 */
#ifdef RISCV
template <typename T>
using volatile_pointer = volatile T*;
#else
template <typename T>
using volatile_pointer = DrvAPI::pointer<T>;
#endif

namespace common
{
/**
 * addressof
 */
#ifdef RISCV
template <typename T>
pointer<T> addressof(T&v) {
    return &v;
}
#else
template <typename T>
pointer<T> addressof(DrvAPI::value_handle<T>v) {
    return v.address();
}
#endif
}
#endif
