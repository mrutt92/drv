#ifndef CELLO_BFS_HPP
#define CELLO_BFS_HPP
#include <csr.hpp>
#include <atomic.hpp>
#include <foreach.hpp>
#include <allocator.hpp>
#include <cstdint>
#include <array>

typedef common::csr_graph<int32_t> graph_type;

/**
 * baseclass for a set of vertices
 */
template <typename VERTEX_TYPE>
struct vertex_set_base {
    typedef VERTEX_TYPE vertex_type;
    typedef std::array<vertex_type, 1> array_type;
    FIELD(vertex_type, V, _V);
    FIELD(vertex_type, n, _n);
    FIELD(array_type, d, _d);

    /**
     * create a vertex set that can hold V vertices
     */
    template <typename Allocator=common::Allocator>
    static
    pointer<vertex_set_base> create(vertex_type V){
        pointer<vertex_set_base> set = (pointer<vertex_set_base>)Allocator{}
            .allocate(sizeof(vertex_set_base) + sizeof(vertex_type)*(V - 1));
        
        set->V() = V;
        set->n() = 0;
        return set;
    }

    /**
     * destroy a vertex set
     */
    template <typename Allocator=common::Allocator>
    static
    void destroy(pointer<vertex_set_base> set){
        Allocator{}.deallocate(set, sizeof(vertex_set_base)+(set->V()-1)*sizeof(vertex_type));
    }
};

/**
 * a sparse set of vertices
 */
template <typename VERTEX_TYPE>
struct sparse_vertex_set : public vertex_set_base<VERTEX_TYPE> {
    typedef VERTEX_TYPE vertex_type;

    /**
     * create a vertex set that can hold V vertices
     */
    template <typename Allocator=common::Allocator>
    static
    pointer<sparse_vertex_set> create(vertex_type V){
        pointer<sparse_vertex_set> set = (pointer<sparse_vertex_set>)Allocator{}
            .allocate(sizeof(sparse_vertex_set) + sizeof(vertex_type)*(V - 1));
        set->V() = V;
        set->n() = 0;
        return set;
    }

    /**
     * destroy a vertex set
     */
    template <typename Allocator=common::Allocator>
    static
    void destroy(pointer<sparse_vertex_set> sparse_set){
        vertex_set_base<vertex_type>::template destroy<Allocator>(sparse_set);
    }

    /**
     * clear a vertex set
     */
    static
    void CLEAR(pointer<sparse_vertex_set> sparse_set){
        sparse_set->n() = 0;
    }

    /**
     * insert a vertex into the set
     * does not check for duplicates
     */
    static
    void INSERT
    (pointer<sparse_vertex_set> sparse_set, vertex_type v){
        vertex_type i = common::atomic_add
            (common::addressof(sparse_set->n()), 1);
        sparse_set->d()[i] = v;
    }

    /**
     * call function for each member
     */
    template <typename Body>
    static
    void FOREACH(pointer<sparse_vertex_set> sparse_set, Body body){
        common::parallel_foreach{}(0, sparse_set->n(), 1, [sparse_set, body](vertex_type i) {
            body(sparse_set->d()[i]);
        });
    }

#ifdef RISCV
    void clear() {
        CLEAR(this);
    }
    void insert(vertex_type v) {
        INSERT(this, v);
    }
    template <typename Body>
    void foreach(Body &&body) {
        FOREACH(this, body);
    }
#endif
};

/**
 * a dense set of vertices
 */
template <typename VERTEX_TYPE>
struct dense_vertex_set : public vertex_set_base<VERTEX_TYPE> {
    typedef VERTEX_TYPE vertex_type;
    static constexpr vertex_type BITS = 8*sizeof(vertex_type);    
    /**
     * create a vertex set that can hold V vertices
     */
    template <typename Allocator=common::Allocator>
    static
    pointer<dense_vertex_set> create(vertex_type V){
        vertex_type words = (V + BITS - 1)/BITS;
        pointer<dense_vertex_set> dense_set = (pointer<dense_vertex_set>)Allocator{}
            .allocate(sizeof(dense_vertex_set) + sizeof(vertex_type)*(words - 1));
        dense_set->V() = V;
        dense_set->n() = 0;
        CLEAR(dense_set);
        return dense_set;
    }

    /**
     * destroy a vertex set
     */
    template <typename Allocator=common::Allocator>
    static
    void destroy(pointer<dense_vertex_set> dense_set){
        vertex_set_base<vertex_type>::template destroy<Allocator>(dense_set);
    }

    /**
     * number of words needed to store V vertices
     */
    vertex_type words() const {
        return (this->V() + BITS - 1)/BITS;
    }

    /**
     * insert a vertex into the set
     * return true if already present
     */
    static
    bool INSERT_FAST
    (pointer<dense_vertex_set> dense_set, vertex_type v){
        vertex_type i = v/BITS;
        vertex_type j = v%BITS;
        vertex_type o = common::atomic_or
            (common::addressof(dense_set->d()[i]), 1 << j);
        return (o & (1 << j)) == 0;
    }

    /**
     * insert a vertex into the set
     * checks for duplicates
     */
    static
    void INSERT
    (pointer<dense_vertex_set> dense_set, vertex_type v){
        if (INSERT_FAST(dense_set, v)) {
            common::atomic_add
                (common::addressof(dense_set->n()), 1);
        }
    }

    /**
     * check if a vertex is in the set
     */
    static
    bool CONTAINS
    (pointer<dense_vertex_set> dense_set, vertex_type v){
        vertex_type i = v/BITS;
        vertex_type j = v%BITS;
        return (dense_set->d()[i] & (1 << j)) != 0;
    }

    /**
     * clear a vertex set
     */
    static
    void CLEAR(pointer<dense_vertex_set> dense_set) {
        common::parallel_foreach{}(0, dense_set->words(), 1, [dense_set](vertex_type w) {
            dense_set->d()[w] = 0;
        });
        dense_set->n() = 0;
    }


    /**
     * call function for each member
     */
    template <typename Body>
    static
    void FOREACH(pointer<dense_vertex_set> dense_set, Body &&body) {
        common::parallel_foreach{}(0, dense_set->words(), 1, [dense_set, body](vertex_type w) {
            vertex_type x = dense_set->d()[w];
            for (vertex_type j=0; j<BITS; j++) {
                if (x & (1 << j)) {
                    body(w*BITS+j);
                }
            }
        });
    }
    
#ifdef RISCV
    bool insert_fast(vertex_type v) {
        return INSERT_FAST(this, v);
    }
    void insert(vertex_type v) {
        INSERT(this, v);
    }
    bool contains(vertex_type v) {
        return CONTAINS(this, v);
    }
    void clear() {
        CLEAR(this);
    }
    template <typename Body>
    void foreach(Body &&body) {
        FOREACH(this, body);
    }
#endif
};

/**
 * a vertex set
 */
template <typename VERTEX_TYPE>
struct vertex_set {
    typedef VERTEX_TYPE vertex_type;
    typedef dense_vertex_set<vertex_type> dense_set;
    typedef sparse_vertex_set<vertex_type> sparse_set;
    FIELD(pointer<dense_set>, dense, _dense);
    FIELD(pointer<sparse_set>, sparse, _sparse);
    FIELD(bool, is_dense, _is_dense);

    /**
     * create a vertex set that can hold V vertices
     */
    static void INIT(pointer<vertex_set> vertex_set, vertex_type V, bool is_dense) {
        vertex_set->dense() = dense_set::create(V);
        vertex_set->sparse() = sparse_set::create(V);
        vertex_set->is_dense() = is_dense;
    }

    /**
     * clear a vertex set
     */
    static void CLEAR(pointer<vertex_set> vertex_set) {
        if (vertex_set->is_dense()) {
            dense_set::CLEAR(vertex_set->dense());
        } else {
            sparse_set::CLEAR(vertex_set->sparse());
        }
    }

    /**
     * insert a vertex into the set
     */
    static void INSERT_FAST(pointer<vertex_set> vertex_set, vertex_type v) {
        if (vertex_set->is_dense()) {
            dense_set::INSERT_FAST(vertex_set->dense(), v);
        } else {
            sparse_set::INSERT(vertex_set->sparse(), v);
        }
    }

    /**
     * insert a vertex into the set
     */
    static void INSERT(pointer<vertex_set> vertex_set, vertex_type v) {
        if (vertex_set->is_dense()) {
            dense_set::INSERT(vertex_set->dense(), v);
        } else {
            sparse_set::INSERT(vertex_set->sparse(), v);
        }
    }

    /**
     * to dense
     */
    static void TO_DENSE(pointer<vertex_set> vertex_set) {
        if (vertex_set->is_dense()) {
            return;
        }
        auto dense = vertex_set->dense();
        dense_set::CLEAR(dense);
        sparse_set::FOREACH(vertex_set->sparse(), [dense](vertex_type v) {
            dense_set::INSERT(dense, v);
        });
        vertex_set->is_dense() = true;
    }

    /**
     * to sparse
     */
    static void TO_SPARSE(pointer<vertex_set> vertex_set) {
        if (!vertex_set->is_dense()) {
            return;
        }
        auto sparse = vertex_set->sparse();        
        sparse_set::CLEAR(sparse);
        dense_set::FOREACH(vertex_set->dense(), [sparse](vertex_type v) {
            sparse_set::INSERT(sparse, v);
        });
        vertex_set->is_dense() = false;
    }

    /**
     * call function for each member
     */
    template <typename Body>
    static void FOREACH(pointer<vertex_set> vertex_set, Body &&body) {
        if (vertex_set->is_dense()) {
            dense_set::FOREACH(vertex_set->dense(), body);
        } else {
            sparse_set::FOREACH(vertex_set->sparse(), body);
        }
    }

    /**
     * check if empty
     */
    static bool EMPTY(pointer<vertex_set> vertex_set) {
        if (vertex_set->is_dense()) {
            return vertex_set->dense()->n() == 0;
        } else {
            return vertex_set->sparse()->n() == 0;
        }
    }

#ifdef RISCV
    void init(vertex_type V, bool is_dense) {
        INIT(this, V, is_dense);
    }
    bool insert_fast(vertex_type v) {
        return INSERT_FAST(this, v);
    }
    void insert(vertex_type v) {
        INSERT(this, v);
    }
    void clear() {
        CLEAR(this);
    }
    void to_dense() {
        TO_DENSE(this);
    }
    void to_sparse() {
        TO_SPARSE(this);
    }
    template <typename Body>
    void foreach(Body &&body) {
        FOREACH(this, body);
    }
    bool empty() {
        return EMPTY(this);
    }
#endif
};

/**
 * a bidirectional graph
 */
struct bidirectional_graph {
    typedef graph_type::vertex_type vertex_type;
    FIELD(graph_type, fwd, _fwd);
    FIELD(graph_type, rev, _rev);
    vertex_type V() const { return fwd().V(); }
    vertex_type E() const { return fwd().E(); }
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.fwd() = src.fwd();
        dst.rev() = src.rev();
    }
#ifdef RISCV
    /**
     * call a function for each vertex
     */
    template <typename Body>
    void foreach_vertex(Body &&body) {
        fwd().foreach_vertex(body);
    }

    /**
     * call a function for each out edge
     */
    template <typename Body>
    void foreach_out_edge(vertex_type v, Body &&body) {
        fwd().foreach_edge(v, body);
    }

    /**
     * call a function for each in edge
     */
    template <typename Body>
    void foreach_in_edge(vertex_type v, Body &&body) {
        rev().foreach_edge(v, body);
    }
#endif
};

/**
 * DrvX handle for a bidirectional graph
 */
#ifndef RISCV
namespace DrvAPI
{
template <>
class value_handle<bidirectional_graph> {
    typedef bidirectional_graph::vertex_type vertex_type;
    VH_DEFAULTS(bidirectional_graph);
    VH_FIELD(bidirectional_graph, fwd, _fwd);
    VH_FIELD(bidirectional_graph, rev, _rev);
    void init(vertex_type V, vertex_type E,
              const std::vector<vertex_type> &fwd_offsets,
              const std::vector<vertex_type> &fwd_edges,
              const std::vector<vertex_type> &rev_offsets,
              const std::vector<vertex_type> &rev_edges) {
        fwd().init(V, E, fwd_offsets, fwd_edges);
        rev().init(V, E, rev_offsets, rev_edges);
    }
    /**
     * call a function for each vertex
     */
    template <typename Body>
    void foreach_vertex(Body &&body) {
        fwd().foreach_vertex(body);
    }


    /**
     * call a function for each outgoing edge
     */
    template <typename Body>
    void foreach_out_edge(vertex_type v, Body &&body) {
        fwd().foreach_edge(v, body);
    }

    /**
     * call a function for each incoming edge
     */
    template <typename Body>
    void foreach_in_edge(vertex_type v, Body &&body) {
        rev().foreach_edge(v, body);
    }
};
}
#endif

/**
 * configuration for BFS
 */
struct bfs_configuration {
    FIELD(pointer<bidirectional_graph>, g, _g);
    FIELD(graph_type::vertex_type, start, _start);
    FIELD(pointer<graph_type::vertex_type>, distance, _distance);
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.g() = src.g();
        dst.start() = src.start();
        dst.distance() = src.distance();
    }
};

/**
 * DrvX handle for a BFS configuration
 */
#ifndef RISCV
namespace DrvAPI
{
template <>
class value_handle<bfs_configuration> {
    VH_DEFAULTS(bfs_configuration);
    VH_FIELD(bfs_configuration, g, _g);
    VH_FIELD(bfs_configuration, start, _start);
    VH_FIELD(bfs_configuration, distance, _distance);
};
}
#endif

#endif
