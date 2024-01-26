// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <read_graph.hpp>
#include <transpose_graph.hpp>
#include <breadth_first_search_graph.hpp>
#include <inttypes.h>
#define DEBUG
using namespace DrvAPI;

template <typename T>
using pointer = DrvAPI::DrvAPIPointer<T>;

template <typename T>
using handle = typename pointer<T>::value_handle;

struct frontier_data {
    int32_t size;
    int32_t dense;
    int32_t capacity;
    pointer<int32_t> vertices;
};

DRV_API_REF_CLASS_BEGIN(frontier_data)

void init(int32_t dense_, int32_t capacity_) {
    size() = 0;
    dense() = dense_;
    capacity() = capacity_;
    vertices() = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, capacity_ * sizeof(int32_t));
    clear();
}

void destroy() {
    DrvAPIMemoryFree(static_cast<pointer<int32_t>>(vertices()));
}

void clear() {
    if (sparse()) {
        size() = 0;
    } else {
        cello::parallel_for(0, capacity()/32, 1, [=](int32_t i) {
            vertices(i) = 0;
        });
    }
}

int32_t sparse() { return !dense(); }

handle<int32_t> vertices(int32_t i) {
    pointer<int32_t> vp = vertices();
    return vp[i];
}

void insert(int32_t v) {
    if (sparse()) {
        insert_sparse_fast(v);
    } else {
        if (insert_dense_fast(v))
            atomic_add(&size(), 1);
    }
}

/* should only be called on a sparse frontier */
void insert_sparse_fast(int32_t v) {
#ifdef DEBUG
    if (!sparse()) {
         throw std::runtime_error("insert_sparse_fast called on dense frontier");
    }
#endif
    int32_t i = atomic_add(&size(), 1);
    vertices(i) = v;
}

/* should only be called on a dense frontier */
/* does not update the size */
/* returns true if the vertex was not already in the frontier */
bool insert_dense_fast(int32_t v) {
#ifdef DEBUG
    if (!dense()) {
         throw std::runtime_error("insert_dense_fast called on sparse frontier");
    }
#endif
    int32_t i = v / 32;
    int32_t j = v % 32;
    int32_t o =atomic_or(&vertices(i), 1 << j);
    return (o & (1 << j)) == 0;
}

DRV_API_REF_CLASS_DATA_MEMBER(frontier_data, size)
DRV_API_REF_CLASS_DATA_MEMBER(frontier_data, dense)
DRV_API_REF_CLASS_DATA_MEMBER(frontier_data, capacity)
DRV_API_REF_CLASS_DATA_MEMBER(frontier_data, vertices)

DRV_API_REF_CLASS_END(frontier_data)

using frontier = frontier_data_ref;

/**
 * @brief swap two frontiers
 * 
 */
void swap(frontier& a, frontier& b) {
    frontier_data tmp_data;
    frontier tmp (&tmp_data);

    // printf("<=== before swap ===>\n");
    // printf("a: size=%d, dense=%d, capacity=%d, vertices=%lx\n",
    //        (int32_t)a.size(), (int32_t)a.dense(), (int32_t)a.capacity(), (DrvAPIAddress)(pointer<int>)a.vertices());
    // printf("b: size=%d, dense=%d, capacity=%d, vertices=%lx\n",
    //        (int32_t)b.size(), (int32_t)b.dense(), (int32_t)b.capacity(), (DrvAPIAddress)(pointer<int>)b.vertices());
    
    tmp.size() = (int32_t)a.size();
    tmp.dense() = (int32_t)a.dense();
    tmp.capacity() = (int32_t)a.capacity();
    tmp.vertices() = (pointer<int32_t>)a.vertices();

    a.size() = (int32_t)b.size();
    a.dense() = (int32_t)b.dense();
    a.capacity() = (int32_t)b.capacity();
    a.vertices() = (pointer<int32_t>)b.vertices();

    b.size() = (int32_t)tmp.size();
    b.dense() = (int32_t)tmp.dense();
    b.capacity() = (int32_t)tmp.capacity();
    b.vertices() = (pointer<int32_t>)tmp.vertices();

    // printf("<=== after swap ===>\n");
    // printf("a: size=%d, dense=%d, capacity=%d, vertices=%lx\n",
    //        (int32_t)a.size(), (int32_t)a.dense(), (int32_t)a.capacity(), (DrvAPIAddress)(pointer<int>)a.vertices());
    // printf("b: size=%d, dense=%d, capacity=%d, vertices=%lx\n",
    //        (int32_t)b.size(), (int32_t)b.dense(), (int32_t)b.capacity(), (DrvAPIAddress)(pointer<int>)b.vertices());
}

/**
 * @brief convert a dense frontier to a sparse frontier
 *
 * @param dst frontier that will be cleared and populated
 * @param src a dense frontier
 */
void to_sparse(frontier &dst, frontier &src) {
    if (src.sparse()) {
        swap(dst, src);
    } else {
        dst.dense() = 0;
        dst.clear();
        cello::parallel_for(0, src.capacity()/32, 1, [=](int32_t i) mutable {
            int32_t w = src.vertices(i);
            int32_t sz = 0;
            for (int32_t j = 0; j < 32; j++) {
                if (w & (1 << j)) {
                    dst.insert(i*32 + j);
                }
            }
        });
    }
}

/**
 * @brief convert a sparse frontier to a dense frontier
 *
 * @param dst frontier that will be cleared and populated
 * @param src a sparse frontier
 */
void to_dense(frontier &dst, frontier &src) {
#ifdef DEBUG
    if (!src.sparse()) {
        throw std::runtime_error("to_dense called on dense frontier");
    }
#endif
    if (src.dense()) {
        swap(dst, src);
    } else {
        dst.dense() = 1;
        dst.clear();
        cello::parallel_for(0, (int32_t) src.size(), 1, [=](int32_t i) mutable {
            dst.insert_dense_fast(src.vertices(i));
        });
        dst.size() = src.size();
    }
}

int CelloMain(int argc, char* argv[]) {
    std::string graph_file = argv[1];
    int32_t root = atoi(argv[2]);
    printf("BFS with root = %" PRId32 " on graph %s\n", root, graph_file.c_str());

    // read the inputs
    std::vector<int> ref_fwd_offsets, ref_fwd_edges, ref_distance, ref_rev_offsets, ref_rev_edges;
    int _v, _e;
    read_graph(graph_file, &_v, &_e, ref_fwd_offsets, ref_fwd_edges);
    transpose_graph(_v, _e, ref_fwd_offsets, ref_fwd_edges, ref_rev_offsets, ref_rev_edges);
    
    // run the baseline for correctness
    breadth_first_search_graph(root, _v, _e, ref_fwd_offsets, ref_fwd_edges, ref_distance);
    
    // phase 1. construct csr
    double csr_start_time = DrvAPI::seconds();
    
    int32_t v = _v;
    int32_t e = _e;
    pointer<int32_t> fwd_offsets = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, (v+1)*sizeof(int32_t));
    pointer<int32_t> rev_offsets = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, (v+1)*sizeof(int32_t));
    pointer<int32_t> fwd_edges = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, e*sizeof(int32_t));
    pointer<int32_t> rev_edges = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, e*sizeof(int32_t));
    pointer<int32_t> distance = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, v*sizeof(int32_t));

#ifdef PARALLEL_SETUP
    cello::parallel_invoke(
        [=](){
#endif
            cello::parallel_for(0, v+1, 1, [=] (int32_t i) {
                fwd_offsets[i] = ref_fwd_offsets[i];
                rev_offsets[i] = ref_rev_offsets[i];                                   
            });
#ifdef PARALLEL_SETUP
        },
        [=](){
#endif
            cello::parallel_for(0, e, 1, [=] (int32_t i) {
                fwd_edges[i] = ref_fwd_edges[i];
                rev_edges[i] = ref_rev_edges[i];                                   
            });
#ifdef PARALLEL_SETUP
        },
        [=](){
#endif
            cello::parallel_for(0, v, 1, [=] (int32_t i) {
                distance[i] = -1;
            });
#ifdef PARALLEL_SETUP
        }
    );
#endif
    // todo: fix parallel invoke (3)
    
    double csr_end_time = DrvAPI::seconds();

    printf("CSR CONSTRUCTION TIME: %2.9lf s\n", csr_end_time - csr_start_time);    
    DrvAPI::outputStatistics();

    double bfs_start_time = DrvAPI::seconds();
    // phase 2. run bfs
    frontier_data f_data [3];
    frontier curr(&f_data[0]), next(&f_data[1]), temp(&f_data[2]);
    curr.init(true, v);
    next.init(true, v);
    temp.init(true, v);

    curr.insert(root);
    
    while (curr.size() != 0) {
        next.clear();
        // set curr to sparse
        //printf("to_sparse\n");
        to_sparse(temp, curr);
        //printf("swap\n");
        swap(temp, curr);
        //printf("curr size = %d\n", (int)curr.size());
        cello::parallel_for(0, (int32_t)curr.size(), 1, [=] (int32_t i) mutable {
            int32_t s = curr.vertices(i);
            //printf("s = %d\n", s);
            int32_t s_start = fwd_offsets[s];
            int32_t s_stop = fwd_offsets[s+1];
            //printf("s = %d: s_start = %d, s_stop = %d\n", s, s_start, s_stop);
            for (int32_t d_i = s_start; d_i < s_stop; d_i++) {
                int32_t d = fwd_edges[d_i];
                //printf("distance[%d] = %d\n", d, (int32_t)distance[d]);
                if (distance[d] == -1) {
                    //printf("updating %d -> %d\n", s, d);
                    distance[d] = distance[s] + 1;
                    next.insert(d);
                }
            }
        });
        swap(curr, next);
        printf("next.size() = %d, curr.size() = %d\n", (int)next.size(), (int)curr.size());
        // bool do_clear = next.size() > 0;
        // std::swap(swap, next);
        // next.dense() = 1;        

        // if (do_clear)
        //     next.clear();
    }

    double bfs_end_time = DrvAPI::seconds();
    printf("BFS TIME: %2.9lf s\n", bfs_end_time - bfs_start_time);
    DrvAPI::outputStatistics();
    
    return 0;
}

