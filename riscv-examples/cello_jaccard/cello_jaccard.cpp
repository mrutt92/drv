#include <cello.hpp>
#include <stdio.h>
#include "pandohammer/mmio.h"
#include "cello_jaccard.hpp"
#include "pandohammer/storage.h"

l2sp_storage(jaccard_config) jaccard_cfg;
l2sp_storage(pointer<sparse_type>) sparse;
l2sp_storage(pointer<matrix_type>) m;

idx_type intersection(sparse_vector_type &a, sparse_vector_type &b)    
{
    idx_type count = 0;
    idx_type i = 0;
    idx_type j = 0;
    idx_type a_i = 0, b_j = 0;
    if (i < a.NNZ() && j < b.NNZ()) {
        a_i = a.at(i);
        b_j = b.at(j);
    }
    while (i < a.NNZ() && j < b.NNZ()) {
        if (a_i == b_j) {
            a_i = a.at(++i);
        } else if (a_i < b_j) {
            b_j = b.at(++j);
        } else {
            a_i = a.at(++i);
            b_j = b.at(++j);
            count++;
        }
    }
    return count;
}

int CelloMain(int argc, char *argv[])
{
    printf("hello, from jaccard\n");
    sparse = jaccard_cfg.csr();
    m = jaccard_cfg.matrix();
    sparse->foreach_row([=](idx_type i) {
        m->at(i, i) = 1.0;
        sparse->foreach_row(i, m->rows(), [=](idx_type j) {
            //ph_print_int(2000000 + j);
            sparse_vector_type ri = sparse->row(i);
            sparse_vector_type rj = sparse->row(j);
            float common = intersection(ri, rj);
            float jc
                = common
                / (sparse->num_nonzeros(i) + sparse->num_nonzeros(j) - common);
            m->at(i, j) = jc;
            m->at(j, i) = jc;
        }, false);
    });
    return 0;
}
