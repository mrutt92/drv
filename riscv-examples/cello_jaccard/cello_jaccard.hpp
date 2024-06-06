#ifndef CELLO_JACCARD_HPP
#define CELLO_JACCARD_HPP
#include "pointer.hpp"
#include "csr.hpp"
#include "matrix.hpp"
#include "field.hpp"

using idx_type = int32_t;
using sparse_type = common::csr<idx_type>;
using matrix_type = common::dynamic_matrix<idx_type, float>;
using sparse_vector_type = common::sparse_vector<idx_type>;

struct jaccard_config {
    FIELD(pointer<sparse_type>, csr, _csr);
    FIELD(pointer<matrix_type>, matrix, _matrix);

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        {
            pointer<sparse_type> _;
            _ = src.csr();
            dst.csr() = _;
        }
        {
            pointer<matrix_type> _;
            _ = src.matrix();
            dst.matrix() = _;
        }
    }
};

#ifndef RISCV
namespace DrvAPI
{
template <>
class value_handle<jaccard_config> {
    VH_DEFAULTS(jaccard_config);
    VH_FIELD(jaccard_config, csr, _csr);
    VH_FIELD(jaccard_config, matrix, _matrix);
};
} // namespace DrvAPI
#endif

#endif
