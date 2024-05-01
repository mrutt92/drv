#ifndef MATRIX_HPP
#define MATRIX_HPP
#include <cmath>
#include <utility>
#include <tuple>
#include <foreach.hpp>
#include <pointer.hpp>
#include <field.hpp>

namespace common {
template <typename Idx, typename Value>
struct matrix {
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
    AT(const_pointer<matrix> m, Idx row, Idx column) {
        return m->data()[row * m->columns() + column];
    }

    std::pair<idx_type, idx_type>
    static UPPER_TRIANGLE_INDEX(const_pointer<matrix> m, idx_type k) {
        idx_type n = m->rows();
        idx_type i = n - 2 - std::floor(std::sqrt(-8 * k + 4 * n * (n - 1) - 7) / 2.0 - 0.5);
        idx_type j = k + i + 1 - n * (n - 1) / 2 + (n - i) * ((n - i) - 1) / 2;
        return std::make_pair(i, j);
    }
    
    template <typename Body>
    static
    void FOREACH_UPPER_TRIANGLE(pointer<matrix> m, Body &&body, bool parallel = true) {
        // error: this only works for a square matrix
        idx_type n = m->rows();
        idx_type iters = n * (n - 1) / 2;
        if (parallel) {
            common::parallel_foreach{}((Idx)0, iters, (Idx)1, [m, body](Idx k) mutable {
                idx_type i, j;
                std::tie(i, j) = UPPER_TRIANGLE_INDEX(m, k);
                body(i, j);
            });
        } else {
            common::serial_foreach{}((Idx)0, iters, (Idx)1, [m, body](Idx k) mutable {
                idx_type i, j;
                std::tie(i, j) = UPPER_TRIANGLE_INDEX(m, k);
                body(i, j);
            });
        }
    }

    template <typename Body>
    void FOREACH(pointer<matrix>m, Body &&body, bool parallel = true) {
        idx_type iters = m->rows() * m->columns();
        if (parallel) {
            common::parallel_foreach{}((Idx)0, iters, (Idx)1, [m, body](Idx k) mutable {
                Idx i = k / m->rows();
                Idx j = k % m->rows();
                body(i, j);
            });
        } else {
            common::serial_foreach{}((Idx)0, iters, (Idx)1, [m, body](Idx k) mutable {
                Idx i = k / m->rows();
                Idx j = k % m->rows();
                body(i, j);
            });
        }
    }

    template <typename Body>
    static void FOREACH_ROW(pointer<matrix> m, Body &&body, bool parallel = true) {
        if (parallel) {
            common::parallel_foreach{}((Idx)0, m->rows(), (Idx)1, [m, body](Idx i) mutable {
                body(i);
            });
        } else {
            common::serial_foreach{}((Idx)0, m->rows(), (Idx)1, [m, body](Idx i) mutable {
                body(i);
            });
        }
    }

#ifdef RISCV
    reference<value_type>
    at(Idx i, Idx j) {
        return AT(this, i, j);
    }

    const_reference<value_type>
    at(Idx i, Idx j) const {
        return AT(this, i, j);
    }


    std::pair<idx_type, idx_type>
    upper_triangle_index(idx_type k) const {
        return UPPER_TRIANGLE_INDEX(this, k);
    }

    template <typename Body>
    void foreach_upper_triangle(Body &&body, bool parallel = true) {
        FOREACH_UPPER_TRIANGLE(this, body, parallel);
    }

    template <typename Body>
    void foreach(Body &&body, bool parallel = true) {
        FOREACH(this, body, parallel);
    }

    template <typename Body>
    void foreach_row(Body &&body, bool parallel = true) {
        FOREACH_ROW(this, body, parallel);
    }
    
    reference<value_type>
    operator()(Idx i, Idx j) {
        return AT(this, i, j);
    }

    const_reference<value_type>
    operator()(Idx i, Idx j) const {
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
    VH_FIELD(matrix_type, rows, _rows);
    VH_FIELD(matrix_type, columns, _columns);
    VH_FIELD(matrix_type, data, _data);

    reference<value_type>
    operator()(idx_type i, idx_type j) {
        return matrix_type::AT(this, i, j);
    }

    const_reference<value_type>
    operator()(idx_type i, idx_type j) const {
        return matrix_type::AT(this, i, j);
    }

    reference<value_type>
    at(idx_type i, idx_type j) {
        return matrix_type::AT(this, i, j);
    }

    const_reference<value_type>
    at(idx_type i, idx_type j) const {
        return matrix_type::AT(this, i, j);
    }
    
    void init(idx_type rows, idx_type columns) {
        this->rows() = rows;
        this->columns() = columns;
        this->data() = (pointer<value_type>)DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(value_type) * rows * columns);
    }
};
}
#endif

#endif
