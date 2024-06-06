#ifndef MATRIX_HPP
#define MATRIX_HPP
#include <cmath>
#include <utility>
#ifndef RISCV
#include <random>
#endif
#include <array>
#include <tuple>
#include <foreach.hpp>
#include <pointer.hpp>
#include <field.hpp>
#include <array.hpp>

namespace common {
template <typename Idx, typename Value>
struct dynamic_matrix {
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
    AT(pointer<dynamic_matrix> m, Idx row, Idx column) {
        return m->data()[row * m->columns() + column];
    }

    static const_reference<value_type>
    AT(const_pointer<dynamic_matrix> m, Idx row, Idx column) {
        return m->data()[row * m->columns() + column];
    }

    std::pair<idx_type, idx_type>
    static UPPER_TRIANGLE_INDEX(const_pointer<dynamic_matrix> m, idx_type k) {
        idx_type n = m->rows();
        idx_type i = n - 2 - std::floor(std::sqrt(-8 * k + 4 * n * (n - 1) - 7) / 2.0 - 0.5);
        idx_type j = k + i + 1 - n * (n - 1) / 2 + (n - i) * ((n - i) - 1) / 2;
        return std::make_pair(i, j);
    }
    
    template <typename Body>
    static
    void FOREACH_UPPER_TRIANGLE(pointer<dynamic_matrix> m, Body &&body, bool parallel = true) {
        // error: this only works for a square dynamic_matrix
        idx_type n = m->rows();
        idx_type iters = n * (n - 1) / 2;
        common::foreach{parallel}((Idx)0, iters, (Idx)1, [m, body](Idx k) mutable {
            idx_type i, j;
            std::tie(i, j) = UPPER_TRIANGLE_INDEX(m, k);
            body(i, j);
        });
    }

    template <typename Body>
    static
    void FOREACH(pointer<dynamic_matrix>m, Body &&body, bool parallel = true) {
        idx_type iters = m->rows() * m->columns();
        common::foreach{parallel}((Idx)0, iters, (Idx)1, [m, body](Idx k) mutable {
            Idx i = k / m->rows();
            Idx j = k % m->rows();
            body(i, j);
        });
    }

    template <typename Body>
    static void FOREACH_ROW(pointer<dynamic_matrix> m, Body &&body, bool parallel = true) {
        common::foreach{parallel}((Idx)0, m->rows(), (Idx)1, [m, body](Idx i) mutable {
            body(i);
        });
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

/**
 * @brief A vector with dynamic size.
 */
template <typename Idx, typename Value, bool COLUMN=true>
struct dynamic_vector {
    typedef Idx idx_type;
    typedef Value value_type;
    typedef dynamic_matrix<idx_type, value_type> matrix_type;

    FIELD(matrix_type, matrix, _matrix);
    
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.matrix() = src.matrix();
    }

    /**
     * @brief Get the size of the vector.
     */
    static reference<idx_type> SIZE(pointer<dynamic_vector> v) {
        return COLUMN ? v->matrix().rows() : v->matrix().columns();
    }
    
    /**
     * @brief Get the size of the vector.
     */
    static const_reference<idx_type> SIZE(const_pointer<dynamic_vector> v) {
        return COLUMN ? v->matrix().rows() : v->matrix().columns();
    }

    /**
     * @brief Get value at an index
     */
    static reference<value_type> AT(pointer<dynamic_vector> v, idx_type i) {
        pointer<matrix_type> m = common::addressof(v->matrix());
        return COLUMN
            ? matrix_type::AT(m, i, 0)
            : matrix_type::AT(m, 0, i);
    }

    /**
     * @brief Get value at an index
     */
    static const_reference<value_type> AT(const_pointer<dynamic_vector> v, idx_type i) {
        const_pointer<matrix_type> m = common::addressof(v->matrix());
        return COLUMN
            ? matrix_type::AT(m, i, 0)
            : matrix_type::AT(m, 0, i);
    }


    /**
     * @brief Check if column vector
     */
    static bool IS_COLUMN(const_pointer<dynamic_vector> v) {
        return COLUMN;
    }

    /**
     * @brief Slice
     */
    static dynamic_vector SLICE(pointer<dynamic_vector> v, idx_type i, idx_type j) {
        // determine the size of the slice
        idx_type vsize = SIZE(v);
        if (i < 0) {
            i = 0;
        } else if (i > vsize) {
            i = vsize;
        }

        if (j < i) {
            j = i;
        }
        
        idx_type size = j-i;
        dynamic_vector ret;
        SIZE(common::addressof(ret)) = size;
        if (COLUMN) {
            ret.matrix().rows() = size;
            ret.matrix().columns() = 1;
        } else {
            ret.matrix().rows() = 1;
            ret.matrix().columns() = size;
        }
        ret.matrix().data() = v->matrix().data() + i;
        return ret;
    }
#ifdef RISCV

    /**
     * @brief initialize
     */
    void init(idx_type size) {
        if (COLUMN) {
            matrix().rows() = size;
            matrix().columns() = 1;
        } else {
            matrix().rows() = 1;
            matrix().columns() = size;
        }
        matrix().data() = (pointer<value_type>)allocate_dram(sizeof(value_type) * size);
    }
    
    /**
     * @brief Get the size of the vector.
     */
    reference<idx_type> size() {
        return SIZE(this);
    }
    
    /**
     * @brief Get the size of the vector.
     */
    const_reference<value_type> size() const {
        return SIZE(this);
    }

    /**
     * @brief Get value at an index
     */
    reference<value_type> at(idx_type i) {
        return AT(this, i);
    }

    /**
     * @brief Get value at an index
     */
    const_reference<value_type> at(idx_type i) const {
        return AT(this, i);
    }

    /**
     * @brief Get value at an index
     */
    reference<value_type> operator[](idx_type i) {
        return AT(this, i);
    }

    /**
     * @brief Get value at an index
     */
    const_reference<value_type> operator[](idx_type i) const {
        return AT(this, i);
    }

    /**
     * @brief Check if column vector
     */
    bool is_column() const {
        return IS_COLUMN(this);
    }

    /**
     * @brief Slice
     */
    dynamic_vector slice(idx_type i, idx_type j) {
        return SLICE(this, i, j);
    }
#endif       
};

template <size_t ROWS, size_t COLUMNS, typename Idx, typename Value>
struct static_matrix {
    typedef Idx idx_type;
    typedef Value value_type;
    typedef std::array<Value, ROWS*COLUMNS> array_type;
    FIELD(array_type, data, _data);

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        for (Idx i = 0; i < ROWS; i++) {
            for (Idx j = 0; j < COLUMNS; j++) {
                dst.data()[i * COLUMNS + j] = src.data()[i * COLUMNS + j];
            }
        }
    }

    idx_type rows() const {
        return ROWS;
    }

    idx_type columns() const {
        return COLUMNS;
    }

    static idx_type INDEX(const_pointer<static_matrix> matrix, idx_type i, idx_type j) {
        return i * COLUMNS + j;
    }

    static reference<value_type>
    AT(pointer<static_matrix> m, Idx row, Idx column) {
        return m->data()[row * COLUMNS + column];
    }

    template <typename Body>
    static
    void FOREACH(pointer<static_matrix> m, Body &&body, bool parallel = true) {
        idx_type iters = m->rows() * m->columns();
        common::foreach{parallel}((Idx)0, iters, (Idx)1, [m, body](Idx k) mutable {
            Idx i = k / m->rows();
            Idx j = k % m->rows();
            body(i, j);
        });
    }

#ifdef RISCV
    reference<value_type>
    at(Idx i, Idx j) {
        return AT(this, i, j);
    }

    reference<value_type>
    operator()(Idx i, Idx j) {
        return AT(this, i, j);
    }

    const_reference<value_type>
    at(Idx i, Idx j) const {
        return AT(this, i, j);
    }

    const_reference<value_type>
    operator()(Idx i, Idx j) const {
        return AT(this, i, j);
    }

    template <typename Body>
    void foreach(Body &&body, bool parallel = true) {
        FOREACH(this, body, parallel);
    }
#endif
};

} // namespace common

#ifndef RISCV
namespace DrvAPI {
template <typename Idx, typename Value>
class value_handle<common::dynamic_matrix<Idx, Value>> {
    typedef Idx idx_type;
    typedef Value value_type;
    typedef common::dynamic_matrix<Idx, Value> dynamic_matrix_type;

    VH_DEFAULTS(dynamic_matrix_type);
    VH_FIELD(dynamic_matrix_type, rows, _rows);
    VH_FIELD(dynamic_matrix_type, columns, _columns);
    VH_FIELD(dynamic_matrix_type, data, _data);

    reference<value_type>
    operator()(idx_type i, idx_type j) {
        return dynamic_matrix_type::AT(this, i, j);
    }

    const_reference<value_type>
    operator()(idx_type i, idx_type j) const {
        return dynamic_matrix_type::AT(address(), i, j);
    }

    reference<value_type>
    at(idx_type i, idx_type j) {
        pointer<dynamic_matrix_type> m = address();
        return dynamic_matrix_type::AT(m, i, j);
    }

    const_reference<value_type>
    at(idx_type i, idx_type j) const {
        const_pointer<dynamic_matrix_type> m = address();
        return dynamic_matrix_type::AT(m, i, j);
    }
    
    void init(idx_type rows, idx_type columns) {
        this->rows() = rows;
        this->columns() = columns;
        this->data() = (pointer<value_type>)DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, sizeof(value_type) * rows * columns);
    }

    /**
     * @brief Populates the dynamic_matrix with random values
     */
    struct RandomPopulator {
        value_type operator()(idx_type i, idx_type j) {
            return (value_type)rand();
        }
    };
    /**
     * @brief Populates the dynamic_matrix with values from a populator function
     */
    template <typename Populator>
    void populate(idx_type rows, idx_type columns, Populator &&populator) {
        init(rows, columns);
        for (idx_type i = 0; i < rows; i++) {
            for (idx_type j = 0; j < columns; j++) {
                at(i, j) = populator(i, j);
            }
        }
    }

    /**
     * @brief Populates the dynamic_matrix with random values
     */
    void populate(idx_type rows, idx_type columns) {
        populate(rows, columns, RandomPopulator{});
    }
};

template <typename Idx, typename Value, bool COLUMN>
class value_handle<common::dynamic_vector<Idx, Value, COLUMN>> {
    typedef Idx idx_type;
    typedef Value value_type;
    typedef common::dynamic_vector<Idx, Value, COLUMN> vector_type;

    VH_DEFAULTS(vector_type);
    VH_FIELD(vector_type, matrix, _matrix);

    /**
     * @brief Get the size of the vector
     */
    reference<idx_type> size() {
        return vector_type::SIZE(address());
    }

    /**
     * @brief Get the size of the vector
     */
    const_reference<idx_type> size() const {
        return vector_type::SIZE(address());
    }

    /**
     * @brief Get the value at index i
     */
    reference<value_type>
    operator[](idx_type i) {
        pointer<vector_type> v = address();
        return vector_type::AT(v, i);
    }

    /**
     * @brief Get the value at index i
     */
    const_reference<value_type>
    operator[](idx_type i) const {
        const_pointer<vector_type> v = address();
        return vector_type::AT(v, i);
    }

    /**
     * @brief Get the value at index i
     */
    reference<value_type>
    at(idx_type i) {
        pointer<vector_type> v = address();
        return vector_type::AT(v, i);
    }

    /**
     * @brief Get the value at index i
     */
    const_reference<value_type>
    at(idx_type i) const {
        const_pointer<vector_type> v = address();
        return vector_type::AT(v, i);
    }

    /**
     * @brief Check if the vector is column
     */
    bool is_column() const {
        return COLUMN;
    }

    /**
     * @brief Slice
     */
    vector_type slice(idx_type start, idx_type end) {
        return vector_type::SLICE(address(), start, end);
    }
    
    /**
     * @brief Initialize the vector with a size
     */
    void init(idx_type size) {
        if (is_column()) {
            matrix().init(size, 1);
        } else {
            matrix().init(1, size);
        }        
    }

    /**
     * @brief Populates the vector with values from a populator function
     */
    struct RandomPopulator {
        value_type operator()(idx_type i) {
            return (value_type)rand();
        }
    };

    /**
     * @brief Populates the vector with values from a populator function
     */
    template <typename Populator>
    void populate(idx_type size, Populator &&populator) {
        init(size);
        for (idx_type i = 0; i < size; i++) {
            at(i) = populator(i);
        }
    }
    
    /**
     * @brief Populates the vector with random values
     */
    void populate(idx_type size) {
        populate(size, RandomPopulator{});
    }
};

template <typename Idx, typename Value, size_t ROWS, size_t COLUMNS>
class value_handle<common::static_matrix<ROWS, COLUMNS, Idx, Value>> {
    typedef Idx idx_type;
    typedef Value value_type;
    typedef common::static_matrix<ROWS, COLUMNS, Idx, Value> static_matrix_type;

    VH_DEFAULTS(static_matrix_type);
    VH_FIELD(static_matrix_type, data, _data);

    idx_type rows() const {
        return ROWS;
    }

    idx_type columns() const {
        return COLUMNS;
    }

    reference<value_type>
    operator()(idx_type i, idx_type j) {
        return static_matrix_type::AT(address(), i, j);
    }

    const_reference<value_type>
    operator()(idx_type i, idx_type j) const {
        return static_matrix_type::AT(address(), i, j);
    }

    reference<value_type>
    at(idx_type i, idx_type j) {
        pointer<static_matrix_type> m = address();
        return static_matrix_type::AT(m, i, j);
    }

    const_reference<value_type>
    at(idx_type i, idx_type j) const {
        const_pointer<static_matrix_type> m = address();
        return static_matrix_type::AT(m, i, j);
    }


    template <typename Body>
    void foreach(Body &&body, bool parallel = true) {
        static_matrix_type::FOREACH(address(), body, parallel);
    }

    /**
     * @brief Populates the static_matrix with random values
     */
    struct RandomPopulator {
        value_type operator()(idx_type i, idx_type j) {
            return distribution(gen);
        }
        std::uniform_real_distribution<value_type> distribution{0, 1};
        std::mt19937 gen{};
    };
    
    /**
     * @brief Populates the static_matrix with values from a populator function
     */
    template <typename Populator>
    void populate(idx_type rows, idx_type columns, Populator &&populator) {
        for (idx_type i = 0; i < rows; i++) {
            for (idx_type j = 0; j < columns; j++) {
                at(i, j) = populator(i, j);
            }
        }
    }

    /**
     * @brief Populates the static_matrix with random values
     */
    void populate(idx_type rows, idx_type columns) {
        populate(rows, columns, RandomPopulator{});
    }    
};
}
#endif
#endif
