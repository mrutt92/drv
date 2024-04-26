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
 * const pointer type
 */
#ifdef RISCV
template <typename T>
using const_pointer = const T*;
#else
template <typename T>
using const_pointer = DrvAPI::pointer<const T>;
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

/**
 * reference type
 */
#ifdef RISCV
template <typename T>
using reference = T&;
#else
template <typename T>
using reference = DrvAPI::value_handle<T>;
#endif

/**
 * const reference type
 */
#ifdef RISCV
template <typename T>
using const_reference = const T&;
#else
template <typename T>
using const_reference = DrvAPI::value_handle<T>;
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
