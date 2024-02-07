// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include "read_graph.hpp"
#include "transpose_graph.hpp"
#include <fstream>

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

struct timer {
    timer(const std::string &name) : name(name) {
        start = DrvAPI::seconds();
    }

    ~timer() {
        stop = DrvAPI::seconds();
        printf("%20s: Elapsed time: %2.9lf seconds\n", name.c_str(), stop - start);
    }

    std::string name;
    double start;
    double stop;
};

vertex intersection(pointer<vertex> a, pointer<vertex> b, vertex a_size, vertex b_size, vertex exclude) {
    vertex count = 0;
    vertex i = 0;
    vertex j = 0;
    while (i < a_size && j < b_size) {
        if (a[i] < b[j]) {
            i++;
        } else if (a[i] > b[j]) {
            j++;
        } else if (a[i] != exclude) {
            count++;
            i++;
            j++;
        } else {
            i++;
            j++;
        }
    }
    return count;
}

int CelloMain(int argc, char *argv[]) {
    // Read the graph
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
        timer _("graph init");
        g.init(V, E, fwd_offsets, fwd_edges);
    }

    // triangles init
    pointer<vertex> triangles = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(vertex)*V);
    {
        timer _("triangles init");
        cello::parallel_for(0, V, 1, [=](vertex v) {
            triangles[v] = 0;
        });
    }

    // triangle counting
    {
        timer _("triangle counting");
        cello::parallel_for(0, V, 1, [=](vertex src) {
            vertex src_start = g.offsets[src];
            vertex src_end = g.offsets[src+1];
            for (vertex src_e = src_start; src_e < src_end; src_e++) {
                vertex dst = g.edges[src_e];
                vertex t = 0;
                if (dst < src) {
                    vertex dst_start = g.offsets[dst];
                    vertex dst_end = g.offsets[dst+1];
                    // find the intersection of the two adjacency lists
                    pointer<vertex> src_neighbors = &g.edges[src_start];
                    pointer<vertex> dst_neighbors = &g.edges[dst_start];
                    t += intersection(src_neighbors, dst_neighbors, src_end - src_start, dst_end - dst_start, dst);
                }
                triangles[src] = t;
            }
        });
    }

    // sum the triangles for each vertex
    DrvAPI::DrvAPIVar<vertex> total = 0;
    {
        timer _("triangle sum");
        cello::parallel_for(0, V, 1, [&](vertex v) {
            DrvAPI::atomic_add<vertex>(total.address(), triangles[v]);
        });
    }

    printf("Total triangles: %d\n", (vertex)total);
    return 0;
}

