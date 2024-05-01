#ifndef CELLO_TC_HPP
#define CELLO_TC_HPP

#include "csr.hpp"
#include "matrix.hpp"
#include "field.hpp"

using idx_type = int32_t;
using sparse_type = common::csr<idx_type>;
using matrix_type = common::matrix<idx_type, float>;
using sparse_vector_type = sparse_type::sparse_vector_type;

struct tc_configure {
    FIELD(pointer<sparse_type>, csr, _csr);
    FIELD(idx_type, triangles, _triangles);
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        {
            pointer<sparse_type> _;
            _ = src.csr();
            dst.csr() = _;
        }
        dst.triangles() = src.triangles();
    }
};

#ifndef RISCV
namespace DrvAPI
{
template <>
class value_handle<tc_configure> {
    VH_DEFAULTS(tc_configure);
    VH_FIELD(tc_configure, csr, _csr);
    VH_FIELD(tc_configure, triangles, _triangles);
};
} // namespace DrvAPI

#endif
#endif
