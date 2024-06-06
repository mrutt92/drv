#ifndef ARRAY_HPP
#define ARRAY_HPP
#include <array>
#include <pointer.hpp>
#ifndef RISCV
#include <DrvAPI.hpp>
namespace DrvAPI
{
template <typename T, size_t N>
class value_handle<std::array<T, N>> {
    using array_type = std::array<T, N>;
    VH_DEFAULTS(array_type);

    reference<T>
    operator[](size_t i) {
        pointer<T> p = address();
        return p[i];
    }

    const_reference<T>
    operator[](size_t i) const {
        const_pointer<T> p = address();
        return p[i];
    }
};
} // namespace DrvAPI
#endif
#endif
