#include <cello.hpp>
#include <cstdio>
#include <pandohammer/storage.h>
#include "cello_gemm.hpp"

l2sp_storage(gemm_config) gemm_cfg;

l1sp_storage(block_type) A_blocks [CORE_THREADS];
l1sp_storage(block_type) B_blocks [CORE_THREADS];
l1sp_storage(block_type) C_blocks [CORE_THREADS];

reference<block_type> my_A_block() {
    return A_blocks[myThreadId()];
}

reference<block_type> my_B_block() {
    return B_blocks[myThreadId()];
}

reference<block_type> my_C_block() {
    return C_blocks[myThreadId()];
}

int CelloMain(int argc, char *argv[])
{
    printf("hello, from cello gemm\n");
    printf("%d x %d x %d\n"
           ,gemm_cfg.A()->rows()
           ,gemm_cfg.A()->columns()
           ,gemm_cfg.B()->columns());

    reference<matrix_type_A> A = *gemm_cfg.A();
    printf("A(0,0) = %f\n", A(0,0));
    return 0;
}
