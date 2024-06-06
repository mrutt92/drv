#ifndef CELLO_SORT_HPP
#define CELLO_SORT_HPP
#include "matrix.hpp"

/**
 * value type
 */
typedef int32_t value_type;

/**
 * index type
 */
typedef int32_t idx_type;

/**
 * vector type
 */
typedef common::dynamic_vector<idx_type, value_type> vector_type;

/**
 * sort config
 */
struct sort_config {
    FIELD(pointer<vector_type>, vec, _vec);

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        {
            pointer<vector_type> _;
            _ = src.vec();
            dst.vec() = _;
        }
    }
};

#ifndef RISCV
namespace DrvAPI
{
template <>
class value_handle<sort_config> {
    VH_DEFAULTS(sort_config);
    VH_FIELD(sort_config, vec, _vec);
};
} // namespace DrvAPI

#endif
#endif
