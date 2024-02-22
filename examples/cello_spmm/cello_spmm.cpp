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

#define pr_info(fmt, ...)                                               \
    do {                                                                \
        printf("INFO: tid=%4ld: " fmt ""                                \
               ,cello::tid()                                            \
               ,##__VA_ARGS__);                                         \
        fflush(stdout);                                                 \
    } while (0)

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

    operator std::tuple<idx_t, value_t>() const {
        return std::make_tuple(idx, val);
    }

    std::string to_string() const {
        return "(" + std::to_string(idx) + ":" + std::to_string(val) + ")";
    }
    template <typename Dst>
    static void copy(Dst &dst, const nonzero &src) {
        dst.idx() = src.idx;
        dst.val() = src.val;
    }
    template <typename Src>
    static void copy(nonzero &dst, const Src &src) {
        dst.idx = src.idx();
        dst.val = src.val();
    }
};

using DrvAPI::value_handle;
using DrvAPI::pointer;
template <typename T>
using pointer_t = pointer<T>;
template <typename T>
using handle_t = value_handle<T>;

template<>
class DrvAPI::value_handle<nonzero> {
    DRV_API_VALUE_HANDLE_DEFAULTS(nonzero)
    DRV_API_VALUE_HANDLE_FIELD(nonzero, idx, idx_t, idx)
    DRV_API_VALUE_HANDLE_FIELD(nonzero, val, value_t, val)

    value_handle& operator=(const std::pair<idx_t, value_t> &p) {
        idx() = p.first;
        val() = p.second;
        return *this;
    }

    std::string to_string() const {
        nonzero n;
        n.idx = idx();
        n.val = val();
        return n.to_string();
    }
};

struct sparse_matrix {
    int rows;
    int cols;
    int nnz;
    pointer_t<idx_t> rowptr;
    pointer_t<nonzero> nonzeros;

    template <typename Dst>
    static void copy(Dst &dst, const sparse_matrix &src) {
        dst.rows() = src.rows;
        dst.cols() = src.cols;
        dst.nnz() = src.nnz;
        dst.rowptr() = src.rowptr;
        dst.nonzeros() = src.nonzeros;
    }

    template <typename Src>
    static void copy(sparse_matrix &dst, const Src &src) {
        dst.rows = src.rows();
        dst.cols = src.cols();
        dst.nnz = src.nnz();
        dst.rowptr = src.rowptr();
        dst.nonzeros = src.nonzeros();
    }
};

/**
 * @brief iterator over nonzero pointer
 */
class nonzero_iterator {
    pointer_t<nonzero> p;
    idx_t i;
public:
    nonzero_iterator(pointer_t<nonzero> p, idx_t i) : p(p), i(i) {}
    bool operator!=(const nonzero_iterator &other) const {
        return i != other.i;
    }
    nonzero_iterator &operator++() {
        i++;
        return *this;
    }
    nonzero operator*() const {
        return p[i];
    }
};

/**
 * @brief range over nonzero pointer
 */
class nonzero_range {
    pointer_t<nonzero> p;
    idx_t n;
public:
    nonzero_range(pointer_t<nonzero> p, idx_t n) : p(p), n(n) {}
    nonzero_iterator begin() const {
        return nonzero_iterator(p, 0);
    }
    nonzero_iterator end() const {
        return nonzero_iterator(p, n);
    }
};

template<>
class DrvAPI::value_handle<sparse_matrix> {
    DRV_API_VALUE_HANDLE_DEFAULTS(sparse_matrix)
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, rows, idx_t, rows)
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, cols, idx_t, cols)
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, nnz, idx_t, nnz)
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, rowptr, pointer_t<idx_t>, rowptr)
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, nonzeros, pointer_t<nonzero>, nonzeros)

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

    /**
     * return nonzeros of outer index i
     */
    nonzero_range nonzeros(idx_t i) {
        return nonzero_range(nonzerosof(i), nnzof(i));
    }

    handle_t<nonzero> nonzero_at(idx_t i) {
        pointer_t<nonzero> p = nonzeros();
        return p[i];
    }

    void init(native_sparse_matrix &native) {
        rows() = native.rows;
        cols() = native.cols;
        nnz() = native.nnz;
        rowptr() = (pointer_t<idx_t>)DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, (rows()+1)*sizeof(idx_t));
        nonzeros() = (pointer_t<nonzero>)DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, nnz()*sizeof(nonzero));
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
};

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
            s += std::to_string(data[i].idx()) + ":" + std::to_string(data[i].val()) + " ";
        }
        return s;
    }

    handle_t<nonzero> operator[](idx_t i) {
        return data[i];
    }

    nonzero_iterator begin() {
        return nonzero_iterator(data, 0);
    }

    nonzero_iterator end() {
        return nonzero_iterator(data, size);
    }
    
    template <typename Dst>
    static void copy(Dst &dst, const vector &src) {
        dst.size() = src.size;
        dst.data() = src.data;
        dst.capacity() = src.capacity;
    }

    template <typename Src>
    static void copy(vector &dst, const Src &src) {
        dst.size = src.size();
        dst.data = src.data();
        dst.capacity = src.capacity();
    }
};

template<>
class DrvAPI::value_handle<vector> {
    DRV_API_VALUE_HANDLE_DEFAULTS(vector)
    DRV_API_VALUE_HANDLE_FIELD(vector, size, idx_t, size)
    DRV_API_VALUE_HANDLE_FIELD(vector, data, pointer_t<nonzero>, data)
    DRV_API_VALUE_HANDLE_FIELD(vector, capacity, idx_t, capacity)

    void init(idx_t capacity) {
        this->capacity() = capacity;
        this->size() = 0;
        this->data() = (pointer_t<nonzero>)DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, capacity*sizeof(nonzero));
    }

    void clear() {
        size() = 0;
    }

    handle_t<nonzero> at(idx_t i) {
        pointer_t<nonzero> p = data();
        return p[i];
    }

    handle_t<nonzero> operator[](idx_t i) {
        return at(i);
    }

    void push_back(const nonzero& val) {
        idx_t i = DrvAPI::atomic_add(size().address(), 1);
        at(i) = val;
    }
    
    operator vector() {
        return vector{size(), data(), capacity()};
    }


    std::string to_string() {
        vector v = *this;
        return v.to_string();
    }
};

/**
 * swap two vectors of nonzeros
 */
void swap(handle_t<vector> a, handle_t<vector> b) {
    vector tmp = a;
    a = b;
    b = tmp;
}

/**
 * merge two sorted vectors of nonzeros
 */
template <typename merge_value>
void merge(handle_t<vector> o_, handle_t<vector> i0_, handle_t<vector> i1_, merge_value &&mergef) {
    idx_t i = 0, j = 0, k = 0;
    o_.clear();

    vector o = o_;
    vector i0 = i0_;
    vector i1 = i1_;    
    if (i0.size == 0 && i1.size == 0) {
        pr_dbg("merge [o: %s]\n", o.to_string().c_str());
        return;
    } else if (i0.size == 0) {
        swap(o_, i1_);
        pr_dbg("merge [o: %s]\n", o.to_string().c_str());
        return;
    } else if (i1.size == 0) {
        swap(o_, i0_);
        pr_dbg("merge [o: %s]\n", o.to_string().c_str());
        return;
    }
    while (i < i0.size && j < i1.size) {
        if (i0[i].idx() < i1[j].idx()) {
            o[k++] = i0[i++];
        } else if (i0[i].idx() > i1[j].idx()) {
            o[k++] = i1[j++];
        } else {
            o[k].idx() = i0[i].idx();
            o[k].val() = mergef(i0[i].val(), i1[j].val());
            k++;
            i++;
            j++;
        }
    }
    while (i < i0.size) {
        o[k++] = i0[i++];
    }
    while (j < i1.size) {
        o[k++] = i1[j++];
    }
    i0_.clear();
    i1_.clear();
    o_.size() = k;
    pr_dbg("merge [o: %s]\n", o.to_string().c_str());
}

/**
 * a product class with place holder data
 */
struct sparse_matrix_product {
    idx_t rows;
    idx_t cols;
    pointer_t<vector> row_data;

    template <typename Dst>
    static void copy(Dst &dst, const sparse_matrix_product &src) {
        dst.rows() = src.rows;
        dst.cols() = src.cols;
        dst.row_data() = src.row_data;
    }

    template <typename Src>
    static void copy(sparse_matrix_product &dst, const Src &src) {
        dst.rows = src.rows();
        dst.cols = src.cols();
        dst.row_data = src.row_data();
    }
};

template <>
class DrvAPI::value_handle<sparse_matrix_product> {
    DRV_API_VALUE_HANDLE_DEFAULTS(sparse_matrix_product)
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix_product, rows, idx_t, rows)
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix_product, cols, idx_t, cols)
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix_product, row_data, pointer_t<vector>, row_data)

    handle_t<vector> row_data(idx_t i) {
        pointer_t<vector> p = row_data();
        handle_t<vector> ref(&p[i]);
        return ref;
    }

    idx_t get_rows() const {
        return rows();
    }

    void init(handle_t<sparse_matrix> I0, handle_t<sparse_matrix> I1) {
        rows() = I0.get_rows();
        cols() = I1.get_cols();
        row_data() = (pointer_t<vector>)DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, (1+rows())*sizeof(vector));
        row_data(rows()).size() = 0;    
    }

    ///////////////////////////////////////////////
    // Create a CSR from a Sparse Matrix Proudct //
    ///////////////////////////////////////////////
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

        DrvAPI::DrvAPIVar<sparse_matrix> O;
        // 0. allocate row vector
        O.rows() = (idx_t)rows();
        O.cols() = (idx_t)cols();
        idx_t N = O.rows()+1;
        O.rowptr() = (pointer_t<idx_t>)DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, N*sizeof(idx_t));

        idx_t regions = std::min(1l<<floor_log2(8*cello::num_threads()),
                                 1l<<floor_log2(N));
        idx_t tree_size = 1<<ceil_log2(regions);
        pointer_t<idx_t> tree = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, tree_size*sizeof(idx_t));
        cello::parallel_for(0, tree_size, 1, [tree](idx_t i) mutable {
            tree[i] = 0;
        });

        // 1. computer row vector with prefix sum
        // 1. i. compute local nnz in each region
        pr_dbg("regions = %d, tree_size = %d\n", regions, tree_size);
        cello::parallel_for(0, regions, 1, [=, &O](idx_t i) mutable {
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
                DrvAPI::atomic_add(tree[r].address(), nnz);
                m >>= 1;
                if (m & i) {
                    r = tree_rchild(r);
                } else {
                    r = tree_lchild(r);
                }
            }
        });
        // 1. ii. update region with nnz from its subtree
        cello::parallel_for(0, regions, 1, [=, &O](idx_t i) mutable {
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
                idx_t r = DrvAPI::atomic_add(O.rowptr(j).address(), nnz);
            }
        });
        // 2. allocate flat nonzero vector
        pr_dbg("allocating float nonzero vector (size = %d)\n", (idx_t)O.rowptr(O.rows()));
        O.nonzeros() = (pointer_t<nonzero>)DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, O.rowptr(O.rows()) * sizeof(nonzero));
        // 3. copy nonzeros into flat nonzero vector
        cello::parallel_for(0, (idx_t)O.rows(), 1, [&O, this](idx_t i) mutable {
            vector src_v = row_data(i);
            pointer_t<nonzero> src = src_v.data;
            pointer_t<nonzero> dst = O.nonzerosof(i);
            for (idx_t j = 0; j < src_v.size; j++) {
                dst[j] = src[j];
            }
        });
        DrvAPIMemoryFree(tree);
        return O;
    }
};

/////////////////////////////////////
// Compute a Sparse Matrix Product //
/////////////////////////////////////
sparse_matrix_product operator*(handle_t<sparse_matrix> I0, handle_t<sparse_matrix> I1)
{
    DrvAPI::DrvAPIVar<sparse_matrix_product> O;
    O.init(I0, I1);
    std::atomic<idx_t> rows_done(0); // for the heartbeat
    idx_t rows = O.get_rows();
    float report_step = std::max(1.0f, (float)rows / 100);
    cello::parallel_for(0, O.get_rows(), 1, [I0, I1, &O, &rows_done, report_step](idx_t i) mutable {
        // initialize buffers
        DrvAPI::DrvAPIVar<vector> nonzero_buffers[3];
        handle_t<vector>
            merge_buffer (nonzero_buffers[0]),
            fadd_buffer (nonzero_buffers[1]),
            result_buffer (nonzero_buffers[2]);

        merge_buffer.init(I1.cols());
        fadd_buffer.init(I1.cols());
        result_buffer.init(I1.cols());

        // for each nonzero in I0[i]
        for (nonzero nz : I0.nonzeros(i)) {
            idx_t j = nz.idx;
            float v = nz.val;
            // for each nonzero in I1[j]
            for (nonzero nz : I1.nonzeros(j)) {
                idx_t k = nz.idx;
                float w = nz.val;
                fadd_buffer.push_back(nonzero{k, v*w});
            }
            merge(merge_buffer, fadd_buffer, result_buffer, [](value_t a, value_t b) -> value_t { return a+b; });
            swap(merge_buffer, result_buffer);
            fadd_buffer.clear();
        }

        pr_dbg("O[%4d;].size() = %4d\n", i, (idx_t)result_buffer.size());
        O.row_data(i) = result_buffer;
        pr_dbg("O[%4d;] = [%s]\n", i, O.row_data(i).to_string().c_str());
        idx_t done = rows_done++;
        if (std::fmod(done,report_step) < 1) {
            pr_info("%4d/%4d rows_done\n", done, (idx_t)O.get_rows());
        }
    });
    return O;
}

int CelloMain(int argc, char** argv) {
    std::string i0 = argv[1];
    std::string i1 = argv[2];

    native_sparse_matrix I0_ = native_sparse_matrix::FromFile(i0);
    native_sparse_matrix I1_ = native_sparse_matrix::FromFile(i1);

    pr_info("CelloMain: I0.rows = %d, nnz = %d\n", I0_.rows, I0_.nnz);
    pr_info("CelloMain: I1.rows = %d, nnz = %d\n", I1_.rows, I1_.nnz);

    // compute a reference product
    EigenSparseMatrix<float> reference
        = I0_.to_eigen()
        * I1_.to_eigen();

    pr_info("CelloMain: reference{rows,nnz} = %4d, %4d\n", (idx_t)reference.rows(), (idx_t)reference.nonZeros());

    using namespace util;
    
    DrvAPI::DrvAPIVar<sparse_matrix> I0, I1;
    // init the inputs
    {
        timer _("inputs init");
        I0.init(I0_);
        I1.init(I1_);
    }

    // find the product
    DrvAPI::DrvAPIVar<sparse_matrix_product> O;
    {
        timer _("row-wise product");
        O = I0 * I1;
    }

    // compare result to reference output
    {
        timer _("check product");

        pr_dbg("O.rows = %d\n", (idx_t)O.get_rows());
        for (idx_t i = 0; i < O.rows(); i++) {
            vector o = O.row_data(i);
            Eigen::SparseVector<float> ref = reference.row(i);
            std::map<idx_t, float> ref_row, o_row;
            for (Eigen::SparseVector<float>::InnerIterator it(ref); it; ++it) {
                idx_t j = it.index();
                float v = it.value();
                ref_row.insert(std::pair<idx_t, float>(j, v));
            }
            for (idx_t j = 0; j < o.size; j++) {
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
    DrvAPI::DrvAPIVar<sparse_matrix> O_csr;
    {
        timer _("product to csr");
        O_csr = (sparse_matrix)O;
    }

    // compare csr product to reference output
    {
        timer _("check csr");
        handle_t<sparse_matrix> O(O_csr);
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
            for (nonzero nz : O.nonzeros(i)) {
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

