#ifndef ATOMIC_HPP
#define ATOMIC_HPP
#ifndef RISCV
#include <DrvAPI.hpp>
#else
#include <atomic>
#endif
#include <pointer.hpp>

namespace common
{
template <typename T>
static inline T atomic_add(pointer<T> ptr, T val) {
#ifndef RISCV
    return DrvAPI::atomic_add(ptr, val);
#else
    std::atomic<T>*ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return ap->fetch_add(val, std::memory_order_relaxed);
#endif
}

template <typename T>
static inline T atomic_or(pointer<T> ptr, T val) {
#ifndef RISCV
    return DrvAPI::atomic_or(ptr, val);
#else
    std::atomic<T>*ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return ap->fetch_or(val, std::memory_order_relaxed);
#endif
}

}
#endif
