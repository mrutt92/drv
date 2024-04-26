#ifndef CSR_HPP
#define CSR_HPP
#include <cstdint>
#include <utility>
#include <pointer.hpp>
#include <field.hpp>
#include <foreach.hpp>
#include <vector>
#ifndef RISCV
#include <DrvAPI.hpp>
#endif

namespace common
{
/**
 * CSR sparse matrix
 */
template <typename idx_type=int32_t>
struct csr {
    FIELD(idx_type, M, _M);
    FIELD(idx_type, NNZ, _NNZ);
    FIELD(pointer<idx_type>, offsets, _offsets);
    FIELD(pointer<idx_type>, nonzeros,_nonzeros);
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src&src) {
        dst.M() = src.M();
        dst.NNZ() = src.NNZ();
        pointer<idx_type> _;
        _ = src.offsets();
        dst.offsets() = _;
        _ = src.nonzeros();
        dst.nonzeros() = _;
    }

    template <typename Body>
    static void FOREACH_ROW(pointer<csr> csr, Body &&body) {
        common::parallel_foreach{}((idx_type)0, (idx_type)csr->M(), (idx_type)1, body);
    }

    template <typename Body>
    static void FOREACH_NONZERO(pointer<csr> csr, idx_type row, Body &&body) {
        common::parallel_foreach_block{}(csr->offsets()[row], csr->offsets()[row+1], (idx_type)16, [csr, body](idx_type start, idx_type end) {
            for (idx_type nz = start; nz < end; nz++) {
                body(csr->nonzeros()[nz]);
            }
        });
    }

    static idx_type NUM_NONZEROS(pointer<csr> csr, idx_type row) {
        return csr->offsets()[row+1] - csr->offsets()[row];
    }
    
#ifdef RISCV
    template <typename Body>
    void foreach_row(Body &&body) {
        FOREACH_ROW(this, body);
    }

    template <typename Body>
    void foreach_nonzero(idx_type row, Body &&body) {
        FOREACH_NONZERO(this, row, body);
    }

    idx_type num_nonzeros(idx_type row) {
        return NUM_NONZEROS(this, row);
    }
#endif
};

/**
 * CSR graph
 */
template <typename VERTEX_TYPE=int32_t>
struct csr_graph {
    typedef VERTEX_TYPE vertex_type;
    typedef csr<vertex_type> csr_type;
    FIELD(csr_type, CSR, _CSR);
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.CSR() = src.CSR();
    }

    vertex_type& V() { return CSR().M(); }
    vertex_type& E() { return CSR().NNZ(); }
    pointer<vertex_type> &offsets() { return CSR().offsets(); }
    pointer<vertex_type> &edges() { return CSR().nonzeros(); }

    const vertex_type &V() const { return CSR().M(); }    
    const vertex_type &E() const { return CSR().NNZ(); }
    const pointer<vertex_type> &offsets() const { return CSR().offsets(); }
    const pointer<vertex_type> &edges() const { return CSR().nonzeros(); }


    static
    vertex_type DEGREE(pointer<csr_graph> graph, vertex_type v) {
        return csr_type::NUM_NONZEROS(common::addressof(graph->CSR()), v);
    }
    
    template <typename Body>
    static
    void FOREACH_VERTEX(pointer<csr_graph> graph, Body&&body) {
        csr_type::FOREACH_ROW(common::addressof(graph->CSR()), body);
    }

    template <typename Body>
    static void FOREACH_EDGE(pointer<csr_graph> graph, vertex_type v, Body &&body) {
        csr_type::FOREACH_NONZERO(common::addressof(graph->CSR()), v, body);
    }

#ifdef RISCV
    template <typename Body>
    void foreach_vertex(Body &&body) {
        FOREACH_VERTEX(this, body);
    }

    template <typename Body>
    void foreach_edge(vertex_type v, Body &&body) {
        FOREACH_EDGE(this, v, body);
    }
    vertex_type degree(vertex_type v) {
        return DEGREE(this, v);
    }
#endif
    
};
}
#ifndef RISCV
namespace DrvAPI
{
/**
 * DrvX handle for CSR sparse matrix
 */
template<typename idx_type>
class value_handle<common::csr<idx_type>> {
    typedef common::csr<idx_type> csr_type;
    VH_DEFAULTS(csr_type);
    VH_FIELD(csr_type, M, _M);
    VH_FIELD(csr_type, NNZ, _NNZ);
    VH_FIELD(csr_type, offsets, _offsets);
    VH_FIELD(csr_type, nonzeros, _nonzeros);
    void init(idx_type M, idx_type NNZ, const std::vector<idx_type> &offsets, const std::vector<idx_type> &nonzeros) {
        this->M() = M;
        this->NNZ() = NNZ;
        auto memtype = DrvAPI::DrvAPIMemoryDRAM;
        this->offsets() = (pointer<idx_type>)
            DrvAPI::DrvAPIMemoryAlloc(memtype, (M+1) * sizeof(idx_type));
        this->nonzeros() = (pointer<idx_type>)
            DrvAPI::DrvAPIMemoryAlloc(memtype, NNZ * sizeof(idx_type));
        for (size_t i = 0; i < offsets.size(); i++)
            this->offsets()[i] = offsets[i];
        for (size_t i = 0; i < nonzeros.size(); i++) {
            this->nonzeros()[i] = nonzeros[i];
        }
    }

    template <typename Body>
    void foreach_row(Body &&body) {
        FOREACH_ROW(this->address(), body);
    }

    template <typename Body>
    void foreach_nonzero(idx_type row, Body &&body) {
        FOREACH_NONZERO(this->address(), row, body);
    }

    idx_type num_nonzeros(idx_type row) {
        return NUM_NONZEROS(this->address(), row);
    }
};
/**
 * DrvX handle for CSR graph
 */
template <typename VERTEX_TYPE>
class value_handle<common::csr_graph<VERTEX_TYPE>> {
    typedef common::csr_graph<VERTEX_TYPE> graph_type;    
    typedef typename graph_type::vertex_type vertex_type;
    
    VH_DEFAULTS(graph_type);
    VH_FIELD(graph_type, CSR, _CSR);

    value_handle<vertex_type>
    V() { return CSR().M(); }
    value_handle<vertex_type>
    E() { return CSR().NNZ(); }
    value_handle<pointer<vertex_type>>
    offsets() { return CSR().offsets(); }
    value_handle<pointer<vertex_type>>
    edges() { return CSR().nonzeros(); }

    const value_handle<vertex_type>
    V() const { return CSR().M(); }
    const value_handle<vertex_type>
    E() const { return CSR().NNZ(); }
    const value_handle<pointer<vertex_type>>
    offsets() const { return CSR().offsets(); }
    const value_handle<pointer<vertex_type>>
    edges() const { return CSR().nonzeros(); }

    void init(vertex_type V, vertex_type E, const std::vector<vertex_type> &offsets, const std::vector<vertex_type> &nonzeros) {
        return CSR().init(V, E, offsets, nonzeros);
    }

    template <typename Body>
    void foreach_vertex(Body &&body) {
        graph_type::FOREACH_VERTEX(this->address(), body);
    }

    template <typename Body>
    void foreach_edge(vertex_type v, Body &&body) {
        graph_type::FOREACH_EDGE(this->address(), v, body);
    }

    vertex_type degree(vertex_type v) {
        return graph_type::DEGREE(this->address(), v);
    }
};
}
#endif
#endif // CSR_HPP
