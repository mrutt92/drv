// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <util/timer.hpp>
#include <random>
#include <Eigen/Core>
using namespace DrvAPI;

typedef float_type val;
typedef int32_t idx;

#define FIELD(type, pub, priv) \
    type priv;                        \
    const type& pub() const { return priv; } \
    type& pub() { return priv; }


struct dynamic_matrix {
    FIELD(idx, rows, rows_);
    FIELD(idx, cols, cols_);
    FIELD(pointer<val>, data, data_);
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.rows() = src.rows();
        dst.cols() = src.cols();
        dst.data() = src.data();
    }
};

template <>
class DrvAPI::value_handle<dynamic_matrix> {
    DRV_API_VALUE_HANDLE_DEFAULTS(dynamic_matrix)
    DRV_API_VALUE_HANDLE_FIELD(dynamic_matrix, rows, idx, rows_)
    DRV_API_VALUE_HANDLE_FIELD(dynamic_matrix, cols, idx, cols_)
    DRV_API_VALUE_HANDLE_FIELD(dynamic_matrix, data, pointer<val>, data_)
    const value_handle<val> operator()(idx i, idx j) const {
        return data()[i * cols() + j];
    }
    value_handle<val> operator()(idx i, idx j) {
        return data()[i * cols() + j];
    }    
};

template <idx ROWS, idx COLUMNS>
struct matrix {
    val data[ROWS * COLUMNS];
    val &operator()(idx i, idx j) { return data[i * COLUMNS + j]; }
    const val &operator()(idx i, idx j) const { return data[i * COLUMNS + j]; }
    idx rows() const { return ROWS; }
    idx cols() const { return COLUMNS; }
};

template <idx ROWS, idx COLUMNS>
class DrvAPI::value_handle<matrix<ROWS, COLUMNS>> {
    DRV_API_VALUE_HANDLE_CONSTRUCTORS(matrix)
    DRV_API_VALUE_HANDLE_INTERNAL(matrix)
    pointer<val> data() { return address(); }
    const pointer<val> data() const { return address(); }
    idx rows() const { return ROWS; }
    idx cols() const { return COLUMNS; }
    value_handle<val> operator()(idx i, idx j) {
        return data()[i * cols() + j];
    }
    const value_handle<val> operator()(idx i, idx j) const {
        return data()[i * cols() + j];
    }
};

dram_static<matrix<GEMM_D0, GEMM_D1>> A;
dram_static<matrix<GEMM_D1, GEMM_D2>> B;
dram_static<matrix<GEMM_D0, GEMM_D2>> C;

float to_float(val v) {
    return static_cast<float>(v);
}

static constexpr idx BLOCK_D0 = 4;
static constexpr idx BLOCK_D2 = 4;
static constexpr idx D0_BLOCKS = GEMM_D0 / BLOCK_D0;
static constexpr idx D2_BLOCKS = GEMM_D2 / BLOCK_D2;
void compute_block(idx block_d0, idx block_d1)
{
    idx d0_start = block_d0 * BLOCK_D0;
    idx d0_stop  = d0_start + BLOCK_D0;
    idx d2_start = block_d1 * BLOCK_D2;
    idx d2_stop  = d2_start + BLOCK_D2;
    // we are modeling these fitting in registers
    // we know this is possible from a hammerblade implementation
    matrix <BLOCK_D0, 1> vec1;
    matrix <1, BLOCK_D2> vec2;
    matrix <BLOCK_D0, BLOCK_D2> psum;
    for (idx py = 0; py < BLOCK_D0; py++) {
        for (idx px = 0; px < BLOCK_D2; px++) {
            psum(py, px) = C(py + d0_start, px + d2_start);
        }
    }
    for (idx sz = 0; sz < GEMM_D1; sz++) {
        for (idx sy = 0; sy < BLOCK_D0; sy++) {
            vec1(sy, 0) = A(d0_start+sy, sz);
        }
        for (idx sx = 0; sx < BLOCK_D2; sx++) {
            vec2(0, sx) = B(sz, d2_start+sx);
        }
        for (idx sx = 0; sx < BLOCK_D2; sx++) {
            for (idx sy = 0; sy < BLOCK_D0; sy++) {
                psum(sy, sx) += vec1(sy, 0) * vec2(0, sx);
            }
        }
    }
    for (idx py = 0; py < BLOCK_D0; py++) {
        for (idx px = 0; px < BLOCK_D2; px++) {
            C(py + d0_start, px + d2_start) = psum(py, px);
        }
    }
    
}

int CelloMain(int argc, char** argv) {
    Eigen::MatrixXf A_ref = Eigen::MatrixXf::Random(GEMM_D0, GEMM_D1);
    Eigen::MatrixXf B_ref = Eigen::MatrixXf::Random(GEMM_D1, GEMM_D2);
    Eigen::MatrixXf C_ref = A_ref * B_ref;
    {
        util::timer _("init A");
        for (idx i = 0; i < GEMM_D0; i++) {
            for (idx j = 0; j < GEMM_D1; j++) {
                A(i, j) = (float)A_ref(i, j);
            }
        }
    }
    {
        util::timer _("init B");
        for (idx i = 0; i < GEMM_D1; i++) {
            for (idx j = 0; j < GEMM_D2; j++) {
                B(i, j) = (float)B_ref(i, j);
            }
        }
    }
    {
        util::timer _("init C");
        for (idx i = 0; i < GEMM_D0; i++) {
            for (idx j = 0; j < GEMM_D2; j++) {
                C(i, j) = 0;
            }
        }
    }
    {
        util::timer _("gemm");
#if 1
        cello::parallel_for(0, D0_BLOCKS, 1, [&](idx block_d0) {
            cello::parallel_for(0, D2_BLOCKS, 1, [&](idx block_d2) {
                idx d0_start = block_d0 * BLOCK_D0;
                idx d0_stop  = d0_start + BLOCK_D0;
                idx d2_start = block_d2 * BLOCK_D2;
                idx d2_stop  = d2_start + BLOCK_D2;
                compute_block(block_d0, block_d2);
                printf("computing block (%3d,%3d) = [%3d;%3d,%3d;%3d]\n",
                       block_d0,
                       block_d2,
                       d0_start,
                       d0_stop,
                       d2_start,
                       d2_stop);                
            });
        });
#else
        for (idx i = 0; i < GEMM_D0; i++) {
            for (idx j = 0; j < GEMM_D1; j++) {
                for (idx k = 0; k < GEMM_D2; k++) {
                    C(i, j) = A(i, k) * B(k, j) + C(i, j);
                }
            }
        }
#endif
    }

    {
        util::timer _("check");
        cello::parallel_for(0, GEMM_D0, 1, [&](idx i) {
            cello::parallel_for(0, GEMM_D2, 1, [&](idx j) {
                val _ = C(i, j);
                float res = (float)_;
                float ref = C_ref(i, j);
                if (res != ref) {
                    printf("C(%d,%d) = %+2.6f != %+2.6f\n", i, j, res, ref);
                }
            });
        });
    }
    return 0;
}

