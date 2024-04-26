#ifndef MATRIX_HPP
#define MATRIX_HPP
#include <pointer.hpp>
#include <field.hpp>

namespace common {
template <typename Idx, typename Value>
class matrix {
    typedef Idx idx_type;
    typedef Value value_type;

    FIELD(Idx, rows, _rows);
    FIELD(Idx, columns, _columns);
    FIELD(pointer<Value>, data, _data);

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.rows() = src.rows();
        dst.columns() = src.columns();
        pointer<Value> _;
        _ = src.data();
        dst.data() = _;
    }

    static reference<value_type>
    AT(pointer<matrix> m, Idx row, Idx column) {
        return m->data()[row * m->columns() + column];
    }

    static const_reference<value_type>
    AT(pointer<matrix> m, Idx row, Idx column) {
        return m->data()[row * m->columns() + column];
    }

#ifdef RISCV
    reference<value_type>
    operator()(Idx i, idx j) {
        return AT(this, i, j);
    }

    const_reference<value_type>
    operator()(Idx i, idx j) const {
        return AT(this, i, j);
    }
#endif
    
};
} // namespace common

#ifndef RISCV
namespace DrvAPI {
template <typename Idx, typename Value>
class value_handle<common::matrix<Idx, Value>> {
    typedef Idx idx_type;
    typedef Value value_type;
    typedef common::matrix<Idx, Value> matrix_type;

    VH_DEFAULTS(matrix_type);
    VH_FIELD(idx_type, rows, _rows);
    VH_FIELD(idx_type, columns, _columns);
    VH_FIELD(pointer<value_type>, data, _data);
    
    reference<value_type>
    operator()(idx_type i, idx_type j) {
        return matrix_type::AT(this, i, j);
    }

    const_reference<value_type>
    operator()(idx_type i, idx_type j) const {
        return matrix_type::AT(this, i, j);
    }
};
}
#endif

#endif
