#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "cello.hpp"
#include "pandohammer/storage.h"
#include "pandohammer/addressmap.hpp"
#include "pandohammer/cpuinfo.h"
#include "pandohammer/atomic.h"
#include "pandohammer/mmio.h"
#include "cello_bfs.hpp"

typedef bidirectional_graph::vertex_type vertex;

using sparse_frontier = sparse_vertex_set<vertex>;
using dense_frontier = dense_vertex_set<vertex>;
using frontier = vertex_set<vertex>;

l2sp_storage(bfs_configuration) bfs_config;

int CelloMain(int argc, char *argv[])
{

    printf("hello, from bfs\n");
    
    bidirectional_graph g = *bfs_config.g();
    pointer<vertex> distance = bfs_config.distance();
    printf("bfs on graph with %d vertices and %d edges\n",
           g.V(), g.E());
    printf("distance array at %p\n", distance);

    
    vertex start = bfs_config.start();
    distance[start] = 0;

    frontier curr, next;
    curr.init(g.V(), false);
    next.init(g.V(), true);

    curr.insert(start);
    next.clear();
    ph_print_time();

    bool switched = false,
        rev_not_fwd = false;

    while (!curr.empty()) {
        // decide direction
        vertex mu = 0, mf =0;
        if (!rev_not_fwd && !switched) {
            curr.to_sparse();
            // find sum of in degree
            curr.foreach([&mu, &g](vertex v){
                common::atomic_add(&mu, g.out_degree(v));
            });
            // find sum degree unvisited
            g.foreach_vertex([&mf, &g, distance](vertex v){
                if (distance[v] == -1) {
                    common::atomic_add(&mf, g.out_degree(v));
                }
            });
            rev_not_fwd = (mf > (mu/20));
        } else {
            rev_not_fwd = (curr.size() >= g.V()/20);
            switched = true;
        }        
        if (rev_not_fwd) {
            // reverse direction
            curr.to_dense();
            g.foreach_vertex([&curr, &next, &g, distance](vertex v){
                if (distance[v] == -1) {
                    g.foreach_in_edge(v, [&next, &curr, v, distance](vertex u) {
                        if (curr.contains(u)) {
                            distance[v] = distance[u] + 1;
                            next.insert(v);
                        }
                    });
                }
            });            
        } else {
            // forward direction
            curr.to_sparse();
            curr.foreach([&next, &g, distance](vertex v){
                g.foreach_out_edge(v, [&next, v, distance](vertex u) {
                    if (distance[u] == -1) {
                        distance[u] = distance[v] + 1;
                        next.insert(u);
                    }
                });
            });
        }
        std::swap(curr, next);
        next.clear();
        next.to_dense();        
    }
    ph_print_time();

    return 0;
}
