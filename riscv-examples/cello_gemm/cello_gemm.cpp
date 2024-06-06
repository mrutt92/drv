#include <cello.hpp>
#include <cmath>
#include <cstdio>
#include <pandohammer/storage.h>
#include "cello_gemm.hpp"

l2sp_storage(gemm_config) gemm_cfg;

l1sp_storage(block_type_A) A_blocks [CORE_THREADS];
l1sp_storage(block_type_B) B_blocks [CORE_THREADS];
l1sp_storage(block_type_C) C_blocks [CORE_THREADS];

template <typename matrix_type>
void clear(reference<matrix_type> block) {
    for (idx_type i = 0; i < block.rows(); i++) {
        for (idx_type j = 0; j < block.columns(); j++) {
            block(i, j) = 0.0;
        }
    }
}


template <typename small_matrix_type, typename big_matrix_type>
void load_block(idx_type block_bm, idx_type block_bk, reference<small_matrix_type> block, reference<big_matrix_type> matrix) {
    idx_type bm_start = block_bm * block.rows();
    idx_type bk_start = block_bk * block.columns();
    for (idx_type i = 0; i < block.rows(); i++) {
        for (idx_type j = 0; j < block.columns(); j++) {
            block(i, j) = matrix(bm_start + i, bk_start + j);
        }
    }
}

template <typename small_matrix_type, typename big_matrix_type>
void store_block(idx_type block_bm, idx_type block_bk, reference<small_matrix_type> block, reference<big_matrix_type> matrix) {
    idx_type bm_start = block_bm * block.rows();
    idx_type bk_start = block_bk * block.columns();
    for (idx_type i = 0; i < block.rows(); i++) {
        for (idx_type j = 0; j < block.columns(); j++) {
            matrix(bm_start + i, bk_start + j) = block(i, j);
        }
    }
}


void compute_block(reference<block_type_C> C_block, reference<block_type_A> A_block, reference<block_type_B> B_block) {
    static constexpr idx_type SUBBLOCK_D0 = SUBBLOCK_M;
    static constexpr idx_type SUBBLOCK_D2 = SUBBLOCK_K;
    static constexpr idx_type BLOCK_D0 = BLOCK_M;
    static constexpr idx_type BLOCK_D2 = BLOCK_K;
    static constexpr idx_type BLOCK_D1 = BLOCK_N;
    common::static_matrix <SUBBLOCK_D0, SUBBLOCK_W, idx_type, float> vec1;
    common::static_matrix <SUBBLOCK_W, SUBBLOCK_D2, idx_type, float> vec2;
    common::static_matrix <SUBBLOCK_D0, SUBBLOCK_D2, idx_type, float> psum;
    for (idx_type sy = 0; sy < BLOCK_D0; sy += SUBBLOCK_D0) {
        for (idx_type sx = 0; sx < BLOCK_D2; sx += SUBBLOCK_D2) {
            // read in psum
            idx_type d0_start = sy;
            idx_type d2_start = sx;
            for (idx_type py = 0; py < SUBBLOCK_D0; py++) {
                for (idx_type px = 0; px < SUBBLOCK_D2; px++) {
                    psum(py, px) = C_block(d0_start + py, d2_start + px);
                }
            }

            // compute
            for (idx_type sz = 0; sz < BLOCK_D1; sz += SUBBLOCK_W) {
                for (idx_type py  = 0; py < SUBBLOCK_D0; py++) {
                    for (idx_type w = 0; w < SUBBLOCK_W; w++) {
                        vec1(py, w) = A_block(d0_start + py, sz + w);
                    }
                }
                for (idx_type px = 0; px < SUBBLOCK_D2; px++) {
                    for (idx_type w = 0; w < SUBBLOCK_W; w++) {
                        vec2(w, px) = B_block(sz + w, d2_start + px);
                    }
                }
                for (idx_type px = 0; px < SUBBLOCK_D2; px++) {
                    for (idx_type py = 0; py < SUBBLOCK_D0; py++) {
                        for (idx_type w = 0; w < SUBBLOCK_W; w++) {
                            psum(py, px) = fmaf(vec1(py,w), vec2(w,px), psum(py,px));
                        }
                    }
                }
            }

            // write out psum
            for (idx_type py = 0; py < SUBBLOCK_D0; py++) {
                for (idx_type px = 0; px < SUBBLOCK_D2; px++) {
                    C_block(d0_start + py, d2_start + px) = psum(py, px);
                }
            }
        }
    }
}

reference<block_type_A> my_A_block() {
    return A_blocks[myThreadId()];
}

reference<block_type_B> my_B_block() {
    return B_blocks[myThreadId()];
}

reference<block_type_C> my_C_block() {
    return C_blocks[myThreadId()];
}

int CelloMain(int argc, char *argv[])
{
    printf("hello, from cello gemm\n");
    printf("%d x %d x %d\n"
           ,gemm_cfg.A()->rows()
           ,gemm_cfg.A()->columns()
           ,gemm_cfg.B()->columns());

    pointer<matrix_type_A> Ap = gemm_cfg.A();
    pointer<matrix_type_B> Bp = gemm_cfg.B();
    pointer<matrix_type_C> Cp = gemm_cfg.C();

    cello::parallel_for(0, M_BLOCKS, 1, [=](idx_type bm) {
        cello::parallel_for(0, K_BLOCKS, 1, [=](idx_type bk) {
            reference<matrix_type_C> C = *Cp;
            reference<block_type_C> C_block = my_C_block();
            clear(C_block);
            for (idx_type bn = 0; bn < N_BLOCKS; bn++) {
                reference<matrix_type_A> A = *Ap;
                reference<matrix_type_B> B = *Bp;
                reference<block_type_A> A_block = my_A_block();
                reference<block_type_B> B_block = my_B_block();
                load_block(bm, bn, A_block, A);
                load_block(bn, bk, B_block, B);
                compute_block(C_block, A_block, B_block);
            }
            store_block(bm, bk, C_block, C);
        });
    });
    return 0;
}
