#ifndef CELLO_GEMM_HPP
#define CELLO_GEMM_HPP
#include <cstdint>
#include "pointer.hpp"
#include "field.hpp"
#include "matrix.hpp"

typedef int32_t idx_type;

template <int32_t M, int32_t N>
using matrix_type =  common::static_matrix<GEMM_M, GEMM_N, int32_t, float>;
using matrix_type_A = matrix_type<GEMM_M, GEMM_N>;
using matrix_type_B = matrix_type<GEMM_N, GEMM_K>;
using matrix_type_C = matrix_type<GEMM_M, GEMM_K>;

typedef common::static_matrix<16, 16, int32_t, float> block_type;

struct gemm_config {
    FIELD(pointer<matrix_type_A>, A, _A);
    FIELD(pointer<matrix_type_B>, B, _B);
    FIELD(pointer<matrix_type_B>, C, _C);
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        {
            pointer<matrix_type_A> _;
            _ = src.A();
            dst.A() = _;
        }
        {
            pointer<matrix_type_B> _;
            _ = src.B();
            dst.B() = _;
        }
        {
            pointer<matrix_type_B> _;
            _ = src.C();
            dst.C() = _;
        }
    }
};

#ifndef RISCV
namespace DrvAPI
{
template <>
class value_handle<gemm_config> {
    VH_DEFAULTS(gemm_config);
    VH_FIELD(gemm_config, A, _A);
    VH_FIELD(gemm_config, B, _B);
    VH_FIELD(gemm_config, C, _C);
};
} // namespace DrvAPI
#endif
#endif
