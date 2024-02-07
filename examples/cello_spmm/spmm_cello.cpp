// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <read_graph.hpp>

using namespace DrvAPI;

struct native_sparse_matrix {
    int rows = 0;
    int cols = 0;
    int nnz = 0;
    std::vector<int> rowptr;
    std::vector<std::pair<int, float>> nonzeros;

    /**
     * @brief native_sparse_matrix
     *
     */
    native_sparse_matrix() = default;
    native_sparse_matrix(const native_sparse_matrix &) = delete;
    native_sparse_matrix & operator=(const native_sparse_matrix &) = delete;
    native_sparse_matrix(native_sparse_matrix &&) = default;
    native_sparse_matrix & operator=(native_sparse_matrix &&) = default;
    ~native_sparse_matrix() = default;

    /**
     * @brief init
     *
     * @param filename
     */
    static native_sparse_matrix FromFile(const std::string &filename) {
        native_sparse_matrix m;
        read_sparse_matrix(
            filename,
            &m.rows,
            &m.nnz,
            m.rowptr,
            m.nonzeros
        );
        m.cols = m.rows;
        return m;
    }
};

// idx type
using idx_t = int32_t;

// value type
using value_t = float;

struct nonzero {
    idx_t   idx;
    value_t val;
};

DRV_API_REF_CLASS_BEGIN(nonzero)
    DRV_API_REF_CLASS_DATA_MEMBER(nonzero, idx)
    DRV_API_REF_CLASS_DATA_MEMBER(nonzero, val)
DRV_API_REF_CLASS_END(nonzero)

// pointer type
template <typename T>
using pointer_t = DrvAPI::DrvAPIPointer<T>;

template <typename T>
using handle_t = typename pointer_t<T>::value_handle;

struct sparse_matrix {
    int rows;
    int cols;
    int nnz;
    pointer_t<idx_t> rowptr;
    pointer_t<nonzero> nonzeros;
};

DRV_API_REF_CLASS_BEGIN(sparse_matrix)
    DRV_API_REF_CLASS_DATA_MEMBER(sparse_matrix, rows)
    DRV_API_REF_CLASS_DATA_MEMBER(sparse_matrix, cols)
    DRV_API_REF_CLASS_DATA_MEMBER(sparse_matrix, nnz)
    DRV_API_REF_CLASS_DATA_MEMBER(sparse_matrix, rowptr)

    idx_t get_rows() const  {
        return rows();
    }

    idx_t get_cols() const {
        return cols();
    }

    idx_t get_nnz() const {
        return nnz();
    }

    handle_t<idx_t> rowptr(idx_t i) {
        pointer_t<idx_t> p = rowptr();
        return p[i];
    }

    DRV_API_REF_CLASS_DATA_MEMBER(sparse_matrix, nonzeros)

    nonzero_ref nonzeros(idx_t i) {
        pointer_t<nonzero> p = nonzeros();
        nonzero_ref ref(&p[i]);;
        return ref;
    }

    void init(native_sparse_matrix &native) {
        rows() = native.rows;
        cols() = native.cols;
        nnz() = native.nnz;
        rowptr() = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, (rows()+1)*sizeof(idx_t));
        nonzeros() = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, nnz()*sizeof(nonzero));
        cello::parallel_invoke(
            [this, &native]() {
                cello::parallel_for(0, get_rows()+1, 1, [this, &native](idx_t i) {
                    rowptr(i) = native.rowptr[i];
                });
            },
            [this, &native]() {
                cello::parallel_for(0, get_nnz(), 1, [this, &native](idx_t i) {
                    nonzeros(i).idx() = native.nonzeros[i].first;
                    nonzeros(i).val() = native.nonzeros[i].second;
                });
            }
        );
    }

DRV_API_REF_CLASS_END(sparse_matrix)

int CelloMain(int argc, char** argv) {
    std::string m0 = argv[1];
    std::string m1 = argv[2];

    native_sparse_matrix M0_ = native_sparse_matrix::FromFile(m0);
    native_sparse_matrix M1_ = native_sparse_matrix::FromFile(m1);

    printf("CelloMain: M0.rows = %d, nnz = %d\n", M0_.rows, M0_.nnz);
    printf("CelloMain: M1.rows = %d, nnz = %d\n", M1_.rows, M1_.nnz);

    sparse_matrix M0_data, M1_data;
    sparse_matrix_ref M0(&M0_data), M1(&M1_data);
    M0.init(M0_);
    M1.init(M1_);

    return 0;
}

