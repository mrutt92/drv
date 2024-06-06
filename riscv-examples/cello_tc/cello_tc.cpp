#include <cello.hpp>
#include <cstdio>
#include "cello_tc.hpp"
#include "atomic.hpp"
#include "pandohammer/storage.h"
#include "pandohammer/allocator.h"
l2sp_storage(tc_configure) tc_cfg;

idx_type intersection
(sparse_vector_type &a, sparse_vector_type &b, idx_type lower_bound)
{
    idx_type count = 0;
    idx_type a_i = 0, b_j = 0;
    idx_type i = 0, j = 0;
    if (i < a.NNZ() && j < b.NNZ()) {
        a_i = a.at(i);
        b_j = b.at(j);
    }
    while (i < a.NNZ() && j < b.NNZ()) {
        if (a_i < b_j) {
            a_i = a.at(++i);
        } else if (a_i > b_j) {
            b_j = b.at(++j);
        } else {
            if (a_i > lower_bound)
                count++;
            a_i = a.at(++i);
            b_j = b.at(++j);
        }
    }
    return count;
}

int CelloMain(int argc, char *argv[])
{
    printf("hello, from cello tc\n");
    pointer<sparse_type> g = tc_cfg.csr();
    pointer<idx_type> triangles = (pointer<idx_type>)allocate_dram(sizeof(idx_type)*g->M());
    g->foreach_row([=](idx_type i) mutable {
        idx_type count = 0;
        g->foreach_nonzero(i, [=, &count](idx_type j) mutable {
            if (i < j) {
                sparse_vector_type ri = g->row(i);
                sparse_vector_type rj = g->row(j);
                common::atomic_add(common::addressof(count), intersection(ri, rj, j));
            }
        }, false);
        triangles[i] = count;
    });
    g->foreach_row([=](idx_type i) mutable {
        common::atomic_add(common::addressof(tc_cfg.triangles()), triangles[i]);
    });
    return 0;
}
