#include <iostream>
#include <stdio.h>
#include "pandohammer/storage.h"
#include "pandohammer/mmio.h"
#include "cello.hpp"
#include "cello_pr.hpp"

#ifndef ITERATIONS
#define ITERATIONS 10
#endif

l2sp_storage(pagerank_config) pagerank_configure;
l1sp_storage(int32_t) count[CORE_THREADS];

void pagerank(pagerank_config cfg)
{
    graph g = *cfg.g();
    vertex V = g.V();
    float damp = 0.85f;    
    float beta_score = (1.0f - damp)/V;
    pointer<float> contrib = cfg.contrib();
    pointer<float> old_rank = cfg.old_rank();
    pointer<float> new_rank = cfg.new_rank();
    pointer<vertex> out_degree = cfg.out_degree();
    for (int i = 0; i < ITERATIONS; i++) {
        cello::parallel_for(0, V, 1, [=](vertex v) mutable {
            contrib[v] = old_rank[v] / out_degree[v];
        });
        cello::parallel_for(0, V, 1, [=](vertex dst) mutable {
            float rank = 0.0f;
            vertex start = g.offsets()[dst];
            vertex end = g.offsets()[dst+1];
            for (vertex e = start; e < end; e++) {
                vertex src = g.edges()[e];
                rank += contrib[src];
            }
            new_rank[dst] = damp * new_rank[dst] + beta_score;
            old_rank[dst] = new_rank[dst];
        });
        ph_print_int(-i);
    }
}


int CelloMain(int argc, char *argv[])
{
    graph g = *pagerank_configure.g();
    printf("V = %d, E = %d, offsets = %p, edges = %p\n"
           , g.V(), g.E(), g.offsets(), g.edges());
    pagerank(pagerank_configure);
    return 0;
}
