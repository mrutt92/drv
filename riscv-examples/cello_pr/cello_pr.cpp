#include <iostream>
#include <stdio.h>
#include "pandohammer/storage.h"
#include "pandohammer/mmio.h"
#include "cello.hpp"
#include "cello_pr.hpp"
#include "atomic.hpp"

#ifndef ITERATIONS
#define ITERATIONS 10
#endif

l2sp_storage(pagerank_config) pagerank_configure;

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
    ph_print_time();
    for (int i = 0; i < ITERATIONS; i++) {
        g.foreach_vertex([contrib, old_rank, out_degree](vertex v) {
            contrib[v] = old_rank[v] / out_degree[v];
        });
        g.foreach_vertex([=](vertex dst) mutable {
            float rank = 0.0f;
            g.foreach_edge(dst, [=, &rank](vertex src) mutable {
                rank += contrib[src];
            }, false); // serial
            new_rank[dst] = damp * rank + beta_score;
            old_rank[dst] = new_rank[dst];
        });
        ph_print_time();
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
