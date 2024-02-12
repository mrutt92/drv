// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <util/timer.hpp>
#include "read_graph.hpp"
#include "transpose_graph.hpp"
#include <fstream>

#ifndef ITERATIONS
#define ITERATIONS 10
#endif

using vertex = int32_t;
using edge = vertex;

template <typename T>
using pointer = DrvAPI::DrvAPIPointer<T>;

struct graph {
    vertex V = 0;
    vertex E = 0;
    pointer<vertex> offsets = 0;
    pointer<edge> edges = 0;

    void init(vertex V, vertex E, const std::vector<vertex> &offsets, const std::vector<edge> &edges) {
        this->V = V;
        this->E = E;
        this->offsets = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(vertex)*(V + 1));
        this->edges = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(edge)*E);
        cello::parallel_invoke(
             [=](){
                 cello::parallel_for(0, V+1, 1, [=](vertex v) {
                     this->offsets[v] = offsets[v];
                 });
             },
             [=](){
                 cello::parallel_for(0, E, 1, [=](vertex e) {
                     this->edges[e] = edges[e];
                 });
             }
        );
    }    
};

using namespace util;
    
int CelloMain(int argc, char** argv) {
    std::string graph_path = argv[1];
    std::vector<vertex> fwd_offsets, rev_offsets;
    std::vector<edge> fwd_edges, rev_edges;
    vertex V, E;
    read_graph(graph_path, &V, &E, fwd_offsets, fwd_edges);
    transpose_graph (V, E, fwd_offsets, fwd_edges, rev_offsets, rev_edges);
    printf("%s: V: %d, E: %d\n", graph_path.c_str(), V, E);

    graph g;

    // csr construction
    {
        timer t("graph init");
        g.init(V, E, fwd_offsets, fwd_edges);
    }

    pointer<float> old_rank = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(float)*V);
    pointer<float> new_rank = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(float)*V);
    pointer<vertex> out_degree = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(vertex)*V);
    pointer<float> contrib = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(float)*V);

    // rank init
    {
        timer t("rank init");
        cello::parallel_for(0, V, 1, [=](vertex v) {
            old_rank[v] = 1.0f/V;
            new_rank[v] = 0.0f;
            out_degree[v] = fwd_offsets[v+1] - fwd_offsets[v];
        });
    }

    float damp = 0.85;
    float beta_score = (1.0 - damp)/V;

    // pagerank
    {
        timer t("pagerank");
        for (int i = 0; i < ITERATIONS; i++) {
            {
                timer t("pagerank iter " + std::to_string(i));
                cello::parallel_for(0, V, 1, [=](vertex v) {
                    contrib[v] = old_rank[v]/out_degree[v];
                });
        
                cello::parallel_for(0, V, 1, [=](vertex dst) {
                    float rank = 0.0f;
                    vertex start = g.offsets[dst];
                    vertex end = g.offsets[dst+1];
                    for (vertex e = start; e < end; e++) {
                        vertex src = g.edges[e];
                        rank += contrib[src];
                    }
                    new_rank[dst] = rank;
                });

                cello::parallel_for(0, V, 1, [=](vertex v) {
                    float rank = damp*new_rank[v] + beta_score;
                    old_rank[v] = rank;
                });
            }
        }
    }

    // output the ranks
    {
        timer t("output ranks");
        std::ofstream out("ranks.txt");
        float sum = 0.0;
        for (vertex v = 0; v < V; v++) {
            float rank = old_rank[v];
            sum += rank;
            out << v << " " << rank << std::endl;
        }
        printf("sum: %2.9lf\n", sum);
    }
    
    return 0;
}

