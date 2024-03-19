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

    void clear() {
        for (idx i = 0; i < rows(); i++) {
            for (idx j = 0; j < cols(); j++) {
                (*this)(i, j) = 0;
            }
        }
    }
};

dram_static<matrix<GEMM_D0, GEMM_D1>> A;
dram_static<matrix<GEMM_D1, GEMM_D2>> B;
dram_static<matrix<GEMM_D0, GEMM_D2>> C;

float to_float(val v) {
    return static_cast<float>(v);
}

static constexpr idx BLOCK_D0 = 16;
static constexpr idx BLOCK_D1 = 16;
static constexpr idx BLOCK_D2 = 16;
static constexpr idx D0_BLOCKS = GEMM_D0 / BLOCK_D0;
static constexpr idx D1_BLOCKS = GEMM_D1 / BLOCK_D1;
static constexpr idx D2_BLOCKS = GEMM_D2 / BLOCK_D2;

static constexpr idx SUBBLOCK_D0 = 4;
static constexpr idx SUBBLOCK_D2 = 4;
static constexpr idx SUBBLOCK_W  = 2; // width of subblock

static constexpr idx D0_SUBBLOCKS = GEMM_D0 / SUBBLOCK_D0;
static constexpr idx D2_SUBBLOCKS = GEMM_D2 / SUBBLOCK_D2;

template <typename small_matrix_handle, typename big_matrix_handle>
inline void load_block(idx block_d0, idx block_d1, small_matrix_handle &block, big_matrix_handle &whole) {
    idx d0_start = block_d0 * block.rows();
    idx d0_stop  = d0_start + block.rows();
    idx d1_start = block_d1 * block.cols();
    idx d1_stop  = d1_start + block.cols();

    for (idx d0 = 0; d0 < block.rows(); d0++) {
        for (idx d1 = 0; d1 < block.cols(); d1++) {
            block(d0, d1) = whole(d0_start + d0, d1_start + d1);
        }
    }
}

template <typename small_matrix_handle, typename big_matrix_handle>
inline void store_block(idx block_d0, idx block_d1, small_matrix_handle &block, big_matrix_handle &whole) {
    idx d0_start = block_d0 * block.rows();
    idx d0_stop  = d0_start + block.rows();
    idx d1_start = block_d1 * block.cols();
    idx d1_stop  = d1_start + block.cols();

    for (idx d0 = 0; d0 < block.rows(); d0++) {
        for (idx d1 = 0; d1 < block.cols(); d1++) {
            whole(d0_start + d0, d1_start + d1) = block(d0, d1);
        }
    }
}
 
void compute_block(idx block_d0, idx block_d2)
{
    idx d0_start = block_d0 * SUBBLOCK_D0;
    idx d0_stop  = d0_start + SUBBLOCK_D0;
    idx d2_start = block_d2 * SUBBLOCK_D2;
    idx d2_stop  = d2_start + SUBBLOCK_D2;
    // we are modeling these fitting in registers
    // we know this is possible from a hammerblade implementation
    matrix <SUBBLOCK_D0, 2> vec1;
    matrix <2, SUBBLOCK_D2> vec2;
    matrix <SUBBLOCK_D0, SUBBLOCK_D2> psum;
    for (idx py = 0; py < SUBBLOCK_D0; py++) {
        for (idx px = 0; px < SUBBLOCK_D2; px++) {
            psum(py, px) = C(py + d0_start, px + d2_start);
        }
    }
    for (idx sz = 0; sz < GEMM_D1; sz += 2) {
        for (idx sy = 0; sy < SUBBLOCK_D0; sy++) {
            vec1(sy, 0) = A(d0_start+sy, sz+0);
            vec1(sy, 1) = A(d0_start+sy, sz+1);
        }
        for (idx sx = 0; sx < SUBBLOCK_D2; sx++) {
            vec2(0, sx) = B(sz+0, d2_start+sx);
            vec2(1, sx) = B(sz+1, d2_start+sx);
        }
        for (idx sx = 0; sx < SUBBLOCK_D2; sx++) {
            for (idx sy = 0; sy < SUBBLOCK_D0; sy++) {
                psum(sy, sx) = muladd(vec1(sy, 0), vec2(0, sx), psum(sy, sx));
                psum(sy, sx) = muladd(vec1(sy, 1), vec2(1, sx), psum(sy, sx));
            }
        }
    }
    for (idx py = 0; py < SUBBLOCK_D0; py++) {
        for (idx px = 0; px < SUBBLOCK_D2; px++) {
            C(py + d0_start, px + d2_start) = psum(py, px);
        }
    }    
}

void compute_block(value_handle<matrix<BLOCK_D0,BLOCK_D2>> C_block, value_handle<matrix<BLOCK_D0,BLOCK_D1>> A_block, value_handle<matrix<BLOCK_D1,BLOCK_D2>> B_block)
{
    matrix <SUBBLOCK_D0, SUBBLOCK_W> vec1;
    matrix <SUBBLOCK_W, SUBBLOCK_D2> vec2;
    matrix <SUBBLOCK_D0, SUBBLOCK_D2> psum;
    for (idx sy = 0; sy < BLOCK_D0; sy += SUBBLOCK_D0) {
        for (idx sx = 0; sx < BLOCK_D2; sx += SUBBLOCK_D2) {
            // read in psum
            idx d0_start = sy;
            idx d2_start = sx;
            for (idx py = 0; py < SUBBLOCK_D0; py++) {
                for (idx px = 0; px < SUBBLOCK_D2; px++) {
                    psum(py, px) = C_block(d0_start + py, d2_start + px);
                }
            }

            // compute
            for (idx sz = 0; sz < BLOCK_D1; sz += SUBBLOCK_W) {
                for (idx py  = 0; py < SUBBLOCK_D0; py++) {
                    for (idx w = 0; w < SUBBLOCK_W; w++) {
                        vec1(py, w) = A_block(d0_start + py, sz + w);
                    }
                }
                for (idx px = 0; px < SUBBLOCK_D2; px++) {
                    for (idx w = 0; w < SUBBLOCK_W; w++) {
                        vec2(w, px) = B_block(sz + w, d2_start + px);
                    }
                }
                for (idx px = 0; px < SUBBLOCK_D2; px++) {
                    for (idx py = 0; py < SUBBLOCK_D0; py++) {
                        for (idx w = 0; w < SUBBLOCK_W; w++) {
                            psum(py, px) = muladd(vec1(py,w), vec2(w,px), psum(py,px));
                        }
                    }
                }
            }

            // write out psum
            for (idx py = 0; py < SUBBLOCK_D0; py++) {
                for (idx px = 0; px < SUBBLOCK_D2; px++) {
                    C_block(d0_start + py, d2_start + px) = psum(py, px);
                }
            }
        }
    }
}

std::array<l1sp_static<pointer<matrix<BLOCK_D0,BLOCK_D1>>>, 64> A_block_ptrs;
std::array<l1sp_static<pointer<matrix<BLOCK_D1,BLOCK_D2>>>, 64> B_block_ptrs;
std::array<l1sp_static<pointer<matrix<BLOCK_D0,BLOCK_D2>>>, 64> C_block_ptrs;

value_handle<matrix<BLOCK_D0,BLOCK_D1>> my_A_block() {
    pointer<matrix<BLOCK_D0,BLOCK_D1>> ptr = A_block_ptrs[DrvAPI::myThreadId()];
    if (ptr == 0) {
        ptr = DrvAPIMemoryAllocateType<matrix<BLOCK_D0,BLOCK_D1>>(DrvAPIMemoryL1SP);
        A_block_ptrs[DrvAPI::myThreadId()] = ptr;
    }
    return *ptr;
}

value_handle<matrix<BLOCK_D1,BLOCK_D2>> my_B_block() {
    pointer<matrix<BLOCK_D1,BLOCK_D2>> ptr = B_block_ptrs[DrvAPI::myThreadId()];
    if (ptr == 0) {
        
        ptr = DrvAPIMemoryAllocateType<matrix<BLOCK_D1,BLOCK_D2>>(DrvAPIMemoryL1SP);
        B_block_ptrs[DrvAPI::myThreadId()] = ptr;
    }
    return *ptr;
}

value_handle<matrix<BLOCK_D0,BLOCK_D2>> my_C_block() {
    pointer<matrix<BLOCK_D0,BLOCK_D2>> ptr = C_block_ptrs[DrvAPI::myThreadId()];
    if (ptr == 0) {
        ptr = DrvAPIMemoryAllocateType<matrix<BLOCK_D0,BLOCK_D2>>(DrvAPIMemoryL1SP);
        C_block_ptrs[DrvAPI::myThreadId()] = ptr;
    }
    return *ptr;
}

int CelloMain(int argc, char** argv) {
    Eigen::MatrixXf A_ref = Eigen::MatrixXf::Random(GEMM_D0, GEMM_D1);
    Eigen::MatrixXf B_ref = Eigen::MatrixXf::Random(GEMM_D1, GEMM_D2);
    Eigen::MatrixXf C_ref = Eigen::MatrixXf::Zero(GEMM_D0, GEMM_D2);
    for (idx i = 0; i < GEMM_D0; i++) {
        for (idx j = 0; j < GEMM_D2; j++) {
            for (idx k = 0; k < GEMM_D1; k++) {
                C_ref(i, j) += A_ref(i, k) * B_ref(k, j);
            }
        }
    }
#if 1
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
#endif
#if 1
    {
        util::timer _("gemm");
        cello::parallel_for(0, D0_BLOCKS, 1, [&](idx b0) {
            cello::parallel_for(0, D2_BLOCKS, 1, [&](idx b2) {
                auto C_block = my_C_block();
                C_block.clear();
                printf("Computing block (%3d,%3d) = [%3d;%3d,%3d;%3d]\n",
                       b0,
                       b2,
                       (b0+0)*C_block.rows(),
                       (b0+1)*C_block.rows(),
                       (b2+0)*C_block.cols(),
                       (b2+1)*C_block.cols());
                for (idx b1 = 0; b1 < D1_BLOCKS; b1++) {
                    auto A_block = my_A_block();
                    auto B_block = my_B_block();
                    load_block(b0, b1, A_block, A);
                    load_block(b1, b2, B_block, B);
                    compute_block(C_block, A_block, B_block);
                }
                store_block(b0, b2, C_block, C);
            });
        });
    }
#endif
#if 1
    {
        util::timer _("check");
        for (idx i = 0; i < GEMM_D0; i++) {
            for (idx j = 0; j < GEMM_D2; j++) {
                val _ = C(i, j);
                float res = (float)_;
                float ref = C_ref(i, j);
                if (res != ref) {
                    printf("C(%d,%d) = %+2.6f != %+2.6f\n", i, j, res, ref);
                }
            }
        }
    }
#endif
    return 0;
}

