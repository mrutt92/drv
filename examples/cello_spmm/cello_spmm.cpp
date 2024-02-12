// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <read_graph.hpp>
#include <util/timer.hpp>
#include <Eigen/Sparse>
#include <set>
using namespace DrvAPI;

//#define DEBUG
#ifdef DEBUG
#define pr_dbg(fmt, ...)                                                \
    do {                                                                \
        printf("DEBUG: tid=%4ld: " fmt ""                               \
               ,cello::tid()                                            \
               ,##__VA_ARGS__);                                         \
    } while (0)
#else
#define pr_dbg(fmt, ...)                                                \
    do {                                                                \
    } while (0)
#endif

#define pr_error(fmt, ...)                                              \
    do {                                                                \
        printf("ERROR: tid=%4ld: " fmt ""                               \
               ,cello::tid()                                            \
               ,##__VA_ARGS__);                                         \
    } while (0)

template <typename T>
using EigenSparseMatrix = Eigen::SparseMatrix<T, Eigen::RowMajor>;

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

    /**
     * convert to and Eigen sparse matrix
     */
    EigenSparseMatrix<float> to_eigen() const {
        EigenSparseMatrix<float> m(rows, cols);
        std::vector<Eigen::Triplet<float>> triplets;
        for (int i = 0; i < rows; i++) {
            for (int j = rowptr[i]; j < rowptr[i + 1]; j++) {
                triplets.push_back({i, nonzeros[j].first, nonzeros[j].second});
            }
        }
        m.setFromTriplets(triplets.begin(), triplets.end());
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
    std::string to_string() const {
        return "(" + std::to_string(idx) + ":" + std::to_string(val) + ")";
    }
};

DRV_API_REF_CLASS_BEGIN(nonzero)
    DRV_API_REF_CLASS_DATA_MEMBER(nonzero, idx)
    DRV_API_REF_CLASS_DATA_MEMBER(nonzero, val)
    nonzero_ref & operator=(std::pair<idx_t, value_t> rhs) {
        idx() = rhs.first;
        val() = rhs.second;
        return *this;
    }
    nonzero_ref & operator=(nonzero rhs) {
        idx() = rhs.idx;
        val() = rhs.val;
        return *this;
    }
    operator nonzero() {
        return nonzero{idx(), val()};
    }
    std::string to_string() const {
        nonzero n;
        n.idx = idx();
        n.val = val();
        return n.to_string();
    }
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

    pointer_t<nonzero> nonzerosof(idx_t i) {
        pointer_t<nonzero> p = nonzeros();
        idx_t off = rowptr(i);
        return &p[off];
    }

    idx_t nnzof(idx_t i) {
        pointer_t<idx_t> p = rowptr();
        return p[i+1] - p[i];
    }

    DRV_API_REF_CLASS_DATA_MEMBER(sparse_matrix, nonzeros)

    nonzero_ref nonzero_at(idx_t i) {
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
                    nonzero_at(i) = native.nonzeros[i];
                });
            }
        );
    }

DRV_API_REF_CLASS_END(sparse_matrix)

/**
 * a vector class with fixed capacity
 */
struct vector {
    vector() = default;
    vector(idx_t size, pointer_t<nonzero> data, idx_t capacity) : size(size), data(data), capacity(capacity) {}
    idx_t size;
    pointer_t<nonzero> data;
    idx_t capacity;
    std::string to_string() const {
        std::string s;
        for (idx_t i = 0; i < size; i++) {
            nonzero_ref ref(&data[i]);
            s += std::to_string(ref.idx()) + ":" + std::to_string(ref.val()) + " ";
        }
        return s;
    }
};
DRV_API_REF_CLASS_BEGIN(vector)
DRV_API_REF_CLASS_DATA_MEMBER(vector, capacity)
DRV_API_REF_CLASS_DATA_MEMBER(vector, size)
DRV_API_REF_CLASS_DATA_MEMBER(vector, data)
void init(idx_t capacity) {
    this->capacity() = capacity;
    this->size() = 0;
    this->data() = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, capacity*sizeof(nonzero));
}
void clear() {
    size() = 0;
}
nonzero_ref at(idx_t i) {
    pointer_t<nonzero> p = data();
    return nonzero_ref(&p[i]);
}
nonzero_ref operator[](idx_t i) {
    return at(i);
}
void push_back(const nonzero& val) {
    idx_t i = DrvAPI::atomic_add(&size(), 1);
    //pr_dbg("push_back: inserting %s at %d\n", val.to_string().c_str(), i);
    at(i) = val;
    //pr_dbg("push_back: inserted %s at %d\n", ((nonzero)at(i)).to_string().c_str(), i);
    
}
operator vector() {
    return vector{size(), data(), capacity()};
}
vector_ref operator=(vector rhs) {
    size() = rhs.size;
    capacity() = rhs.capacity;
    data() = rhs.data;
    return *this;
}

std::string to_string() {
    vector v = *this;
    return v.to_string();
}
DRV_API_REF_CLASS_END(vector)

/**
 * swap two vectors of nonzeros
 */
void swap(vector_ref a, vector_ref b) {
    vector tmp = a;
    //pr_dbg("swap:before: a = %s, b = %s\n", a.to_string().c_str(), b.to_string().c_str());
    a = (vector)b;
    b = tmp;
    //pr_dbg("swap:after: a = %s, b = %s\n", a.to_string().c_str(), b.to_string().c_str());
}

/**
 * merge two sorted vectors of nonzeros
 */
template <typename merge_value>
void merge(vector_ref o, vector_ref i0, vector_ref i1, merge_value &&mergef) {
    idx_t i = 0, j = 0, k = 0;
    o.clear();
    //pr_dbg("merge [i0: %s], [i1: %s]\n", i0.to_string().c_str(), i1.to_string().c_str());
    if (i0.size() == 0 && i1.size() == 0) {
        pr_dbg("merge [o: %s]\n", o.to_string().c_str());
        return;
    } else if (i0.size() == 0) {
        swap(o, i1);
        pr_dbg("merge [o: %s]\n", o.to_string().c_str());
        return;
    } else if (i1.size() == 0) {
        swap(o, i0);
        pr_dbg("merge [o: %s]\n", o.to_string().c_str());
        return;
    }
    while (i < i0.size() && j < i1.size()) {
        if (i0[i].idx() < i1[j].idx()) {
            o[k++] = (nonzero)i0[i++];
        } else if (i0[i].idx() > i1[j].idx()) {
            o[k++] = (nonzero)i1[j++];
        } else {
            o[k].idx() = (idx_t)i0[i].idx();
            o[k].val() = mergef(i0[i].val(), i1[j].val());
            k++;
            i++;
            j++;
        }
    }
    while (i < i0.size()) {
        o[k++] = (nonzero)i0[i++];
    }
    while (j < i1.size()) {
        o[k++] = (nonzero)i1[j++];
    }
    i0.clear();
    i1.clear();
    o.size() = k;
    pr_dbg("merge [o: %s]\n", o.to_string().c_str());
}

/**
 * a product class with place holder data
 */
struct sparse_matrix_product {
    idx_t rows;
    idx_t cols;
    pointer_t<vector> row_data;
};

DRV_API_REF_CLASS_BEGIN(sparse_matrix_product)
DRV_API_REF_CLASS_DATA_MEMBER(sparse_matrix_product, rows)
DRV_API_REF_CLASS_DATA_MEMBER(sparse_matrix_product, cols)
DRV_API_REF_CLASS_DATA_MEMBER(sparse_matrix_product, row_data)

vector_ref row_data(idx_t i) {
    pointer_t<vector> p = row_data();
    vector_ref ref(&p[i]);
    return ref;
}

idx_t get_rows() const {
    return rows();
}

void init(sparse_matrix_ref I0, sparse_matrix_ref I1) {
    rows() = I0.get_rows();
    cols() = I1.get_cols();
    row_data() = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, (1+rows())*sizeof(vector));
    row_data(rows()).size() = 0;    
}

operator sparse_matrix() {
    auto ceil_log2 = [](idx_t x) -> idx_t {
        idx_t y = 0;
        while (x > 0) {
            x >>= 1;
            y++;
        }
        return y;
    };
    auto floor_log2 = [](idx_t x) -> idx_t {
        idx_t y = 0;
        while (x > 1) {
            x >>= 1;
            y++;
        }
        return y;
    };
    auto tree_lchild = [](idx_t root)  -> idx_t { return 2*root + 1; };
    auto tree_rchild = [](idx_t root)  -> idx_t { return 2*root + 2; };
    auto tree_levels = [ceil_log2](idx_t leafs) -> idx_t { return ceil_log2(leafs); };

    sparse_matrix O_data;
    sparse_matrix_ref O(&O_data);
    // 0. allocate row vector
    O.rows() = (idx_t)rows();
    O.cols() = (idx_t)cols();
    idx_t N = O.rows()+1;
    O.rowptr() = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, N*sizeof(idx_t));
    idx_t regions = std::min(1l<<floor_log2(8*cello::num_threads()),
                             1l<<floor_log2(N));
    idx_t tree_size = ceil_log2(regions);
    pointer_t<idx_t> tree = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, tree_size*sizeof(idx_t));
    cello::parallel_for(0, tree_size, 1, [tree](idx_t i) mutable {
        tree[i] = 0;
    });
    // 1. computer row vector with prefix sum
    // 1. i. compute local nnz in each region
    cello::parallel_for(0, regions, 1, [=](idx_t i) mutable {
        idx_t region_size = (N + regions - 1) / regions;
        idx_t start = i * region_size;
        idx_t end = std::min(start + region_size, N);
        idx_t nnz = 0;
        for (idx_t j = start; j < end; j++) {
            O.rowptr(j) = nnz;
            nnz += row_data(j).size();
        }
        idx_t r = 0;
        idx_t m = regions;
        idx_t L = tree_levels(m);
        for (idx_t l = 0; l < L; l++) {
            DrvAPI::atomic_add(&tree[r], nnz);
            m >>= 1;
            if (m & i) {
                r = tree_rchild(r);
            } else {
                r = tree_lchild(r);
            }
        }
    });
    // 1. ii. update region with nnz from its subtree
    cello::parallel_for(0, regions, 1, [=](idx_t i) mutable {
        idx_t region_size = (N + regions - 1) / regions;
        idx_t start = i * region_size;
        idx_t end = std::min(start + region_size, N);
        idx_t nnz = 0;
        idx_t r = 0;
        idx_t m = regions;
        idx_t L = tree_levels(m);
        for (idx_t l = 0; l < L; l++) {
            m >>= 1;
            if (i & m) {
                nnz += tree[tree_lchild(r)];
                r = tree_rchild(r);
            } else {
                r = tree_lchild(r);
            }
        }

        for (idx_t j = start; j < end; j++) {
            idx_t r = DrvAPI::atomic_add(&O.rowptr(j), nnz);
        }
    });
    // 2. allocate flat nonzero vector
    pr_dbg("allocating float nonzero vector (size = %d)\n", (idx_t)O.rowptr(O.rows()));
    O.nonzeros() = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, O.rowptr(O.rows()) * sizeof(nonzero));
    // 3. copy nonzeros into flat nonzero vector
    cello::parallel_for(0, (idx_t)O.rows(), 1, [O, this](idx_t i) mutable {
        idx_t nnz = O.nnzof(i);
        vector_ref src = row_data(i);
        pointer_t<nonzero> dst = O.nonzerosof(i);
        for (idx_t j = 0; j < src.size(); j++) {            
            nonzero_ref r(&src[j]);
            dst[j] = (nonzero)r;
        }
    });
    DrvAPIMemoryFree(tree);
    return O_data;
}

DRV_API_REF_CLASS_END(sparse_matrix_product)

/**
 * sparse matrix product
 */
sparse_matrix_product operator*(sparse_matrix_ref I0, sparse_matrix_ref I1)
{
    sparse_matrix_product O_data;
    sparse_matrix_product_ref O(&O_data);
    O.init(I0, I1);
    std::atomic<idx_t> rows_done(0);
    cello::parallel_for(0, O.get_rows(), 1, [I0, I1, O, &rows_done](idx_t i) mutable {
        idx_t nnz = 0;
        // initialize buffers
        vector nonzero_buffers[3];
        vector_ref
            merge_buffer(&nonzero_buffers[0]),
            fadd_buffer(&nonzero_buffers[1]),
            result_buffer(&nonzero_buffers[2]);
        merge_buffer.init(I1.cols());
        fadd_buffer.init(I1.cols());
        result_buffer.init(I1.cols());        
        // for each nonzero in I0[i]
        idx_t start_i = I0.rowptr(i);
        idx_t end_i = I0.rowptr(i+1);
        for (idx_t iter_i = start_i; iter_i < end_i; iter_i++) {
            nonzero nz = I0.nonzero_at(iter_i);
            idx_t j = nz.idx;
            float v = nz.val;
            // for each nonzero in I1[j]
            idx_t start_j = I1.rowptr(j);
            idx_t end_j = I1.rowptr(j+1);
            //pr_dbg("I0[%d,%d] * I1[%d;]\n", i, j, j);
            for (idx_t iter_j = start_j; iter_j < end_j; iter_j++) {
                nonzero nz = I1.nonzero_at(iter_j);
                idx_t k = nz.idx;
                float w = nz.val;
                // merge the nonzeros
                pr_dbg("I0[%4d,%4d] * I1[%4d,%4d] = %4.4f * %4.4f\n", i, j, j, k, v, w);
                fadd_buffer.push_back(nonzero{k, v*w});
            }
            // merge the fadd buffer with the result buffer into the merge buffer
            merge(merge_buffer, fadd_buffer, result_buffer, [] (value_t a, value_t b) -> value_t  { return a+b; });
            swap(merge_buffer, result_buffer);
            fadd_buffer.clear();            
        }
        pr_dbg("O[%4d;].size() = %4d\n", i, (idx_t)result_buffer.size());
        O.row_data(i) = (vector)result_buffer;
        pr_dbg("O[%4d;] = [%s]\n", i, O.row_data(i).to_string().c_str());
        idx_t done = rows_done++;
        if (done % 10 == 0) {
            printf("\trows_done = %d\n", done);
        }
    });
    return O_data;
}

int CelloMain(int argc, char** argv) {
    std::string i0 = argv[1];
    std::string i1 = argv[2];

    native_sparse_matrix I0_ = native_sparse_matrix::FromFile(i0);
    native_sparse_matrix I1_ = native_sparse_matrix::FromFile(i1);

    printf("CelloMain: I0.rows = %d, nnz = %d\n", I0_.rows, I0_.nnz);
    printf("CelloMain: I1.rows = %d, nnz = %d\n", I1_.rows, I1_.nnz);

    // compute a reference product
    EigenSparseMatrix<float> reference
        = I0_.to_eigen()
        * I1_.to_eigen();

    using namespace util;
    
    sparse_matrix
        I0_data,
        I1_data;

    sparse_matrix_ref
        I0(&I0_data),
        I1(&I1_data);

    // init the inputs
    {
        timer _("inputs init");
        I0.init(I0_);
        I1.init(I1_);
    }
    // find the product
    sparse_matrix_product O_data;
    {
        timer _("row-wise product");
        O_data = I0 * I1;
    }
    // compare result to reference output
    {
        timer _("check product");
        pr_dbg("reference{rows,nnz} = %4d, %4d\n", (idx_t)reference.rows(), (idx_t)reference.nonZeros());
        sparse_matrix_product_ref O(&O_data);
        pr_dbg("O.rows = %d\n", (idx_t)O.get_rows());
        for (idx_t i = 0; i < O.rows(); i++) {
            vector_ref o = O.row_data(i);
            Eigen::SparseVector<float> ref = reference.row(i);
            std::map<idx_t, float> ref_row, o_row;
            for (Eigen::SparseVector<float>::InnerIterator it(ref); it; ++it) {
                idx_t j = it.index();
                float v = it.value();
                ref_row.insert(std::pair<idx_t, float>(j, v));
            }
            for (idx_t j = 0; j < o.size(); j++) {
                nonzero nz = o[j];
                o_row.insert(std::pair<idx_t, float>(nz.idx, nz.val));
            }

            
            for (auto itr = ref_row.begin(); itr != ref_row.end(); itr++) {
                idx_t j = itr->first;
                float v = itr->second;
                auto ito = o_row.find(j);
                if (ito == o_row.end()) {
                    pr_error("O[%4d,%4d] = %4.4f, Ref[%4d,%4d] = %4.4f\n", i, j, 0.0f, i, j, v);
                } else if (v != ito->second) {
                    pr_error("O[%4d,%4d] = %4.4f, Ref[%4d,%4d] = %4.4f\n", i, j, ito->second, i, j, v);
                }
            }
            for (auto ito = o_row.begin(); ito != o_row.end(); ito++) {
                idx_t j = ito->first;
                float v = ito->second;
                auto itr = ref_row.find(j);
                if (itr == ref_row.end()) {
                    pr_error("O[%4d,%4d] = %4.4f, Ref[%4d,%4d] = %4.4f\n", i, j, v, i, j, 0.0f);
                }
            }
        }        
    }
    
    // convert product to csr
    sparse_matrix O_data_csr;
    {
        timer _("product to csr");
        sparse_matrix_product_ref O(&O_data);
        O_data_csr = (sparse_matrix)O;
    }

    // compare csr product to reference output
    {
        timer _("check csr");
        sparse_matrix_ref O(&O_data_csr);
        for (idx_t i = 0; i < O.rows(); i++) {
            Eigen::SparseVector<float> ref = reference.row(i);
            std::map<idx_t, float> ref_row, o_row;
            if (O.nnzof(i) != ref.nonZeros()) {
                pr_error("O[%4d;].nnz = %4d, Ref[%4d;].nnz = %4ld\n", i, O.nnzof(i), i, ref.nonZeros());
            }
            for (Eigen::SparseVector<float>::InnerIterator it(ref); it; ++it) {
                idx_t j = it.index();
                float v = it.value();
                ref_row.insert(std::pair<idx_t, float>(j, v));
            }
            for (idx_t j = 0; j < O.nnzof(i); j++) {
                nonzero nz = O.nonzerosof(i)[j];
                o_row.insert(std::pair<idx_t, float>(nz.idx, nz.val));
            }
            for (auto itr = ref_row.begin(); itr != ref_row.end(); itr++) {
                idx_t j = itr->first;
                float v = itr->second;
                auto ito = o_row.find(j);
                if (ito == o_row.end()) {
                    pr_error("O[%4d,%4d] = %4.4f, Ref[%4d,%4d] = %4.4f\n", i, j, 0.0f, i, j, v);
                } else if (v != ito->second) {
                    pr_error("O[%4d,%4d] = %4.4f, Ref[%4d,%4d] = %4.4f\n", i, j, ito->second, i, j, v);
                }
            }
            for (auto ito = o_row.begin(); ito != o_row.end(); ito++) {
                idx_t j = ito->first;
                float v = ito->second;
                auto itr = ref_row.find(j);
                if (itr == ref_row.end()) {
                    pr_error("O[%4d,%4d] = %4.4f, Ref[%4d,%4d] = %4.4f\n", i, j, v, i, j, 0.0f);
                }
            }
        }
    }
    
    return 0;
}

