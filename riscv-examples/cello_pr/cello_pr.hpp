#ifndef CELLO_PR_HPP
#define CELLO_PR_HPP
#include <cstdint>
#include <vertex.hpp>
#include <edge.hpp>
#include <field.hpp>
#include <csr.hpp>
#ifndef RISCV
#include <DrvAPI.hpp>
#endif


#ifdef RISCV
template <typename T>
using pointer = T*;
#else
template <typename T>
using pointer = DrvAPI::pointer<T>;
#endif

using graph = common::csr_graph<vertex>;


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
    VH_DEFAULTS(pagerank_config);
    VH_FIELD(pagerank_config, g, _g);
    VH_FIELD(pagerank_config, old_rank, _old_rank);
    VH_FIELD(pagerank_config, new_rank, _new_rank);
    VH_FIELD(pagerank_config, out_degree, _out_degree);
    VH_FIELD(pagerank_config, contrib, _contrib);
};
}
#endif


#endif
