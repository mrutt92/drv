#ifndef CELLO_PR_HPP
#define CELLO_PR_HPP
#include <cstdint>
#ifndef RISCV
#include <DrvAPI.hpp>
#include <vector>
#endif

typedef int32_t vertex;
typedef vertex edge;

#ifdef RISCV
template <typename T>
using pointer = T*;
#else
template <typename T>
using pointer = DrvAPI::pointer<T>;
#endif

#ifndef FIELD
#define FIELD(type, field, field_data)          \
    type field_data;                            \
    type & field() { return field_data; }       \
    const type & field() const { return field_data; }
#endif

/**
 * csr graph
 */
struct graph {
    FIELD(vertex, V, _V);
    FIELD(edge, E, _E);
    FIELD(pointer<vertex>, offsets, _offsets);
    FIELD(pointer<edge>, edges, _edges);

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.V() = src.V();
        dst.E() = src.E();
        dst.offsets() = src.offsets();
        dst.edges() = src.edges();
    }
};

#ifndef RISCV
namespace DrvAPI
{
template <>
class value_handle<graph> {
    DRV_API_VALUE_HANDLE_DEFAULTS(graph);
    DRV_API_VALUE_HANDLE_FIELD(graph, V, vertex, _V);
    DRV_API_VALUE_HANDLE_FIELD(graph, E, edge, _E);
    DRV_API_VALUE_HANDLE_FIELD(graph, offsets, pointer<vertex>, _offsets);
    DRV_API_VALUE_HANDLE_FIELD(graph, edges, pointer<edge>, _edges);
    void init(int V, int E,
              const std::vector<int> &offsets,const std::vector<int> &edges) {
        this->V() = (vertex)V;
        this->E() = (edge)E;
        constexpr auto memtype = DrvAPI::DrvAPIMemoryDRAM;
        this->offsets() = (pointer<vertex>)
            DrvAPI::DrvAPIMemoryAlloc(memtype, (V+1)*sizeof(vertex));
        this->edges() = (pointer<edge>)
            DrvAPI::DrvAPIMemoryAlloc(memtype, E*sizeof(edge));
    }
};
}
#endif

/**
 * pagerank configuration
 */
struct pagerank_config {
    FIELD(pointer<graph>, g, _g); // pointer to the graph
    FIELD(pointer<float>, old_rank, _old_rank); // pointer to the old rank
    FIELD(pointer<float>, new_rank, _new_rank); // pointer to the new rank
    FIELD(pointer<vertex>, out_degree, _out_degree); // pointer to the out degree
    FIELD(pointer<float>, contrib, _contrib); // pointer to the contribution
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.g() = src.g();
        dst.old_rank() = src.old_rank();
        dst.new_rank() = src.new_rank();
        dst.out_degree() = src.out_degree();
        dst.contrib() = src.contrib();        
    }
};

#ifndef RISCV
namespace DrvAPI
{
template <>
class value_handle<pagerank_config> {
    DRV_API_VALUE_HANDLE_DEFAULTS(pagerank_config);
    DRV_API_VALUE_HANDLE_FIELD(pagerank_config, g, pointer<graph>, _g);
    DRV_API_VALUE_HANDLE_FIELD(pagerank_config, old_rank, pointer<float>, _old_rank);
    DRV_API_VALUE_HANDLE_FIELD(pagerank_config, new_rank, pointer<float>, _new_rank);
    DRV_API_VALUE_HANDLE_FIELD(pagerank_config, out_degree, pointer<vertex>, _out_degree);
    DRV_API_VALUE_HANDLE_FIELD(pagerank_config, contrib, pointer<float>, _contrib);
};
}
#endif


#endif
