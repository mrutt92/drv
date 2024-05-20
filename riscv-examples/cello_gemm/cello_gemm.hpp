#ifndef CELLO_GEMM_HPP
#define CELLO_GEMM_HPP
#include <cstdint>
#include "pointer.hpp"
#include "field.hpp"
#include "matrix.hpp"

typedef int32_t idx_type;
static constexpr idx_type BLOCK_M = 16;
static constexpr idx_type BLOCK_N = 16;
static constexpr idx_type BLOCK_K = 16;
static constexpr idx_type M_BLOCKS  = GEMM_M / BLOCK_M;
static constexpr idx_type N_BLOCKS  = GEMM_N / BLOCK_N;
static constexpr idx_type K_BLOCKS = GEMM_K / BLOCK_K;

static constexpr idx_type SUBBLOCK_M = 4;
static constexpr idx_type SUBBLOCK_K = 4;
static constexpr idx_type SUBBLOCK_W  = 2; // width of subblock

static constexpr idx_type M_SUBBLOCKS = GEMM_M / SUBBLOCK_M;
static constexpr idx_type K_SUBBLOCKS = GEMM_K / SUBBLOCK_K;

template <int32_t M, int32_t N>
using matrix_type =  common::static_matrix<GEMM_M, GEMM_N, int32_t, float>;
using matrix_type_A = matrix_type<GEMM_M, GEMM_N>;
using matrix_type_B = matrix_type<GEMM_N, GEMM_K>;
using matrix_type_C = matrix_type<GEMM_M, GEMM_K>;

typedef common::static_matrix<BLOCK_M, BLOCK_N, int32_t, float> block_type_A;
typedef common::static_matrix<BLOCK_N, BLOCK_K, int32_t, float> block_type_B;
typedef common::static_matrix<BLOCK_M, BLOCK_K, int32_t, float> block_type_C;


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
