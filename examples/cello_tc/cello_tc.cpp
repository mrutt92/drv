// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <util/timer.hpp>
#include "read_graph.hpp"
#include "transpose_graph.hpp"
#include "triangle_counting.hpp"
#include <fstream>
#include <set>
#include <tuple>
#include <atomic>
#define pr_info(fmt, ...)                                               \
    do {                                                                \
        printf("INFO:  " fmt ""                                         \
               ,##__VA_ARGS__);                                         \
        fflush(stdout);                                                 \
    } while (0)

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

#if defined(INTERSECTION_ALGORITHM_LINEAR_SEARCH)
vertex linear_search(pointer<vertex> v, vertex size, vertex key) {
    for (vertex i = 0; i < size; i++) {
        if (v[i] >= key) {
            return i;
        }
    }
    return size;
}

const char intersection_algorithm_name [] = "linear_search";
vertex intersection(pointer<vertex> a, pointer<vertex> b, vertex a_size, vertex b_size, [[maybe_unused]] vertex a_src, vertex b_src) {
    vertex count = 0;
    vertex i = linear_search(a, a_size, b_src+1);
    vertex j = 0;

    vertex a_i = 0, b_j = 0;
    if (i < a_size && j < b_size) {
        a_i = a[i];
        b_j = b[j];
    }

    while (i < a_size && j < b_size) {
        if (a_i < b_j) {
            a_i = a[++i];
        } else if (a_i > b_j) {
            b_j = b[++j];
        } else {
            if (b_src < a_i) {
                count++;
            }
            a_i = a[++i];
            b_j = b[++j];
        }
    }
    return count;
}
#elif defined(INTERSECTION_ALGORITHM_BINARY_SEARCH)
vertex binary_search(pointer<vertex> v, vertex size, vertex key) {
    vertex low = 0;
    vertex high = size;
    while (low < high) {
        vertex mid = low + (high - low) / 2;
        if (v[mid] < key) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

const char intersection_algorithm_name [] = "binary_search";
vertex intersection(pointer<vertex> a, pointer<vertex> b, vertex a_size, vertex b_size, [[maybe_unused]] vertex a_src, vertex b_src) {
    vertex count = 0;
    vertex i = binary_search(a, a_size, b_src+1);
    vertex j = binary_search(b, b_size, a_src+1);

    vertex a_i = 0, b_j = 0;
    if (i < a_size && j < b_size) {
        a_i = a[i];
        b_j = b[j];
    }

    while (i < a_size && j < b_size) {
        if (a_i < b_j) {
            a_i = a[++i];
        } else if (a_i > b_j) {
            b_j = b[++j];
        } else {
            if (b_src < a_i) {
                count++;
            }
            a_i = a[++i];
            b_j = b[++j];
        }
    }
    return count;
}
#else
const char intersection_algorithm_name [] = "baseline";
vertex intersection(pointer<vertex> a, pointer<vertex> b, vertex a_size, vertex b_size, [[maybe_unused]] vertex a_src, vertex b_src) {
    vertex count = 0;
    vertex i = 0;
    vertex j = 0;

    vertex a_i = 0, b_j = 0;
    if (i < a_size && j < b_size) {
        a_i = a[i];
        b_j = b[j];
    }

    while (i < a_size && j < b_size) {
        if (a_i < b_j) {
            a_i = a[++i];
        } else if (a_i > b_j) {
            b_j = b[++j];
        } else {
            if (b_src < a_i) {
                count++;
            }
            a_i = a[++i];
            b_j = b[++j];
        }
    }
    return count;
}
#endif

using namespace util;

int CelloMain(int argc, char *argv[]) {
    // Read the graph
    std::string graph_path = argv[1];
    std::vector<vertex> fwd_offsets, rev_offsets;
    std::vector<edge> fwd_edges, rev_edges;
    vertex V, E;
    pr_info("reading graph '%s'\n", graph_path.c_str());
    read_graph(graph_path, &V, &E, fwd_offsets, fwd_edges);
    transpose_graph (V, E, fwd_offsets, fwd_edges, rev_offsets, rev_edges);

    // assert that the graph is undirected
    if (fwd_offsets != rev_offsets || fwd_edges != rev_edges) {
        std::string msg = "Graph '" + graph_path + "' is not undirected";
        throw std::runtime_error(msg.c_str());
    }

    pr_info("running triangle counting on graph '%s'\n", graph_path.c_str());
    std::set<tc::triangle> triangles_reference;
    tc::triangle_counting(V, E, fwd_offsets, fwd_edges, triangles_reference);    
    pr_info("%s: V: %d, E: %d, triangles from reference = %zu\n", graph_path.c_str(), V, E, triangles_reference.size());
    pr_info("using intersection algorithm = '%s'\n", intersection_algorithm_name);

    std::vector<vertex> relabeled_offsets;
    std::vector<edge> relabeled_edges;
    tc::relabel_by_ascending_degree(V, E, fwd_offsets, fwd_edges, relabeled_offsets, relabeled_edges);

#ifdef USE_RELABELING
    pr_info("reference using relabeling\n");
    std::set<tc::triangle> relabeled_triangles_reference;
    tc::triangle_counting(V, E, relabeled_offsets, relabeled_edges, relabeled_triangles_reference);
    pr_info("triangles from reference using relabeling = %zu\n", relabeled_triangles_reference.size());
#endif
    
    graph g;
    // csr construction
    {
        timer _("graph init");
#ifdef USE_RELABELING
        g.init(V, E, relabeled_offsets, relabeled_edges);
#else
        g.init(V, E, fwd_offsets, fwd_edges);
#endif
    }

    // triangles init
    pointer<vertex> triangles = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(vertex)*V);
    {
        timer _("triangles init");
        cello::parallel_for(0, V, 1, [=](vertex v) mutable {
            triangles[v] = 0;
        });
    }

    // triangle counting
    {
        timer _("triangle counting");
        std::atomic<vertex> count(0);
        cello::parallel_for(0, V, 1, [=, &count](vertex src) {
            vertex src_start = g.offsets[src];
            vertex src_end = g.offsets[src+1];
            vertex step = 1;            
            cello::parallel_for(src_start, src_end, step, [=](vertex e) mutable {
                vertex start = e;
                vertex end = std::min(src_end, start + step);
                vertex c = 0;
                for (auto src_e = start; src_e < end; src_e++) {
                    vertex dst = g.edges[src_e];
                    if (src < dst) {
                        vertex dst_start = g.offsets[dst];
                        vertex dst_end = g.offsets[dst+1];
                        // find the intersection of the two adjacency lists
                        pointer<vertex> src_neighbors = g.edges[src_start].address();
                        pointer<vertex> dst_neighbors = g.edges[dst_start].address();
                        c += intersection(src_neighbors, dst_neighbors, src_end - src_start, dst_end - dst_start, src, dst);
                    }
                }
                DrvAPI::atomic_add<vertex>(triangles[src].address(), c);
            });
            if (++count % 1000 == 0) {
                pr_info("processed %d vertices\n", count.load());
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

    pr_info("Found triangles:     %9d\n", (vertex)total);
    pr_info("Reference triangles: %9zu\n", triangles_reference.size());
    return 0;
}

