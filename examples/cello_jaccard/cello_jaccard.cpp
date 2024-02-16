// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <util/timer.hpp>
#include <vector>
#include <read_graph.hpp>
#include <transpose_graph.hpp>
using namespace DrvAPI;

#ifdef DEBUG
#define pr_dbg(fmt, ...)                                                \
    do {                                                                \
        printf("DEBUG: " fmt ""                                         \
               ,##__VA_ARGS__);                                         \
    } while (0)
#else
#define pr_dbg(fmt, ...)                                                \
    do {                                                                \
    } while (0)
#endif

using namespace util;

typedef int32_t idx;
typedef idx  vertex;
typedef idx  edge;
typedef float val;

template <typename T>
using pointer = DrvAPI::DrvAPIPointer<T>;

struct graph {
    vertex V = 0;
    vertex E = 0;
    pointer<vertex> offsets = 0;    
    pointer<edge> edges = 0;
};

DRV_API_REF_CLASS_BEGIN(graph)
    DRV_API_REF_CLASS_DATA_MEMBER(graph, V)
    DRV_API_REF_CLASS_DATA_MEMBER(graph, E)
    DRV_API_REF_CLASS_DATA_MEMBER(graph, offsets)
    DRV_API_REF_CLASS_DATA_MEMBER(graph, edges)

    pointer<edge>::value_handle edges(idx i) {
        pointer<edge> e = this->edges();
        return e[i];
    }

    pointer<vertex>::value_handle offsets(idx i) {
        pointer<vertex> o = this->offsets();
        return o[i];
    }

    void init(vertex V, vertex E, const std::vector<vertex> &offsets, const std::vector<edge> &edges) {
        this->V() = V;
        this->E() = E;
        this->offsets() = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(vertex)*(V + 1));
        this->edges() = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(edge)*E);
        cello::parallel_invoke(
             [this, &offsets](){
                 cello::parallel_for(0, this->V()+1, 1, [=](vertex v) {
                     this->offsets(v) = offsets[v];
                 });
             },
             [this, &edges](){
                 cello::parallel_for(0, (int)this->E(), 1, [=](vertex e) {
                     this->edges(e) = edges[e];
                 });
             }
        );
    }

    std::tuple<vertex, pointer<vertex>> neighbors(vertex v) {
        vertex start = this->offsets(v);
        vertex end = this->offsets(v + 1);
        vertex size = end - start;
        pointer<edge> neih = &edges(start);
        return std::make_tuple(size, neih);
    }

DRV_API_REF_CLASS_END(graph)

struct matrix {
    idx rows;
    idx cols;
    pointer<val> data;
};

struct matrix_val {
    matrix_val(matrix &m) : m(m) {}
    matrix &m;
    idx & rows() { return m.rows; }
    idx & cols() { return m.cols; }
    pointer<val> & data() { return m.data; }
    void init(idx rows, idx cols) {
        this->rows() = rows;
        this->cols() = cols;
        this->data() = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(val)*rows*cols);
    }
    pointer<val>::value_handle operator()(idx i, idx j) {
        pointer<val> d = this->data();
        return d[i*this->cols() + j];
    }
};

DRV_API_REF_CLASS_BEGIN(matrix)
    DRV_API_REF_CLASS_DATA_MEMBER(matrix, rows)
    DRV_API_REF_CLASS_DATA_MEMBER(matrix, cols)
    DRV_API_REF_CLASS_DATA_MEMBER(matrix, data)
    void init(idx rows, idx cols) {
        this->rows() = rows;
        this->cols() = cols;
        this->data() = DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(val)*rows*cols);
    }
    pointer<val>::value_handle operator()(idx i, idx j) {
        pointer<val> d = this->data();
        return d[i*this->cols() + j];
    }
DRV_API_REF_CLASS_END(matrix)

vertex intersection(pointer<vertex> a, pointer<vertex> b, vertex a_size, vertex b_size) {
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
            a_i = a[++i];
            b_j = b[++j];
            count++;
        }
    }
    return count;
}

int CelloMain(int argc, char** argv) {
    std::string graph_path = argv[1];
    std::vector<vertex> fwd_offsets, rev_offsets;
    std::vector<edge> fwd_edges, rev_edges;
    vertex V, E;
    read_graph(graph_path, &V, &E, fwd_offsets, fwd_edges);
    transpose_graph (V, E, fwd_offsets, fwd_edges, rev_offsets, rev_edges);
    printf("%s: V: %d, E: %d\n", graph_path.c_str(), V, E);

    graph _g;
    graph_ref g(&_g);

    // csr construction
    {
        timer t("graph init");
        g.init(V, E, fwd_offsets, fwd_edges);
    }

    matrix _m;
    matrix_val m(_m);
    {
        timer t("matrix init");
        m.init(V, V);
    }

    // jaccard similarity
    {
        timer t("jaccard");
        cello::parallel_for<vertex>(0, g.V(), 1, [&m, g](vertex v) mutable {
            m(v, v) = 1;
            cello::parallel_for<vertex>(v+1, g.V(), 1, [&m, g, v](vertex u) mutable {
                pointer<vertex> v_neih, u_neih;
                vertex v_size, u_size;
                std::tie(v_size, v_neih) = g.neighbors(v);
                std::tie(u_size, u_neih) = g.neighbors(u);
                vertex common = intersection(v_neih, u_neih, v_size, u_size);
                pr_dbg("v: %d, u: %d, common: %d\n", v, u, common);
                float j = (float)common / (v_size + u_size - common);
                m(v, u) = j;
                m(u, v) = j;                
            });
        });
    }

    return 0;
}
