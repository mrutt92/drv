#ifndef CELLO_PR_HPP
#define CELLO_PR_HPP
#include <cstdint>
#ifndef RISCV
#include <DrvAPI.hpp>
#endif

typedef int64_t vertex;
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
};
}
#endif

/**
 * pagerank configuration
 */
struct pagerank_config {
    FIELD(pointer<graph>, g, _g); // pointer to the graph
};
#endif
