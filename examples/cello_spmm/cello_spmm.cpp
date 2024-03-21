// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#include <DrvAPI.hpp>
#include <cello.hpp>
#include <read_graph.hpp>
#include <util/timer.hpp>
#include <Eigen/Sparse>
#include <set>
#include <iomanip>
#include <iostream>

#define DEBUG

using namespace DrvAPI;
using namespace util;    

//////////////////////////
// forward declarations //
//////////////////////////
struct sparse_matrix;
struct sparse_matrix_product;
struct sparse_vector;
struct pool_link;
struct nonzero_pool_link;

/*
 * convenience macro for defining ostream operator
 */
#define ostream_format_function(type)           \
    std::ostream& operator<<(std::ostream& os, const type& t) { \
        os << t.str();                                          \
        return os;                                              \
    }
/*
 * convenience macro for defining fields in a struct
 * defines getter and setter for a field
 */
#define FIELD(type, pub, priv)                                          \
    public:                                                             \
    type priv;                                                          \
    const type& pub() const { return priv; }                            \
    type& pub() { return priv; }

#define FMT_IDX(idx) std::dec << std::setw(5) << idx
#define FMT_VAL(val) std::setprecision(2) << std::setw(5) << val
#define FMT_TID(tid) std::dec << std::setw(4) << tid
#define FMT_ADDR(addr) DrvAPIVAddress{addr}.to_string()
/*
 * wrap a iostream statement in a debug macro
 * disabled if DEBUG not set
 */
#ifdef  DEBUG
#define DEBUG_STMT(stmt)                                \
    do {                                                \
        std::stringstream ss;                           \
        ss << "DEBUG: ";                                \
        ss << "tid=" << FMT_TID(cello::tid()) << ": ";  \
        ss << stmt;                                     \
        std::cout << ss.str() << std::endl;             \
    } while (0)
#else
#define DEBUG_STMT(stmt)                        \
    do {                                        \
    } while (0)
#endif

/*
 * wrap a iostream statement in a info macro
 */
#define INFO_STMT(stmt)                                                 \
    do {                                                                \
        std::stringstream ss;                                           \
        ss << "INFO: ";                                                 \
        ss << "tid=" << FMT_TID(cello::tid()) << ": ";                  \
        ss << stmt;                                                     \
        ss << std::endl;                                                \
        std::cout << ss.str();                                          \
    } while (0)

/*
 * wrap a iostream statement in a error macro
 */
#define ERROR_STMT(stmt)                                                \
    do {                                                                \
        std::stringstream ss;                                           \
        ss << "ERROR: ";                                                \
        ss << "tid=" << FMT_TID(cello::tid()) << ": ";                  \
        ss << stmt;                                                     \
        ss << std::endl;                                                \
        std::cerr << ss.str();                                          \
    } while (0)

/*
 * sparse matrix type from Eigen
 */
template <typename T>
using EigenSparseMatrix = Eigen::SparseMatrix<T, Eigen::RowMajor>;

template <typename T>
using EigenSparseVector = Eigen::SparseVector<T>;

///////////////
// link type //
///////////////
struct pool_link {
    FIELD(pointer<pool_link>, next, next_);

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.next() = src.next();
    }
};

template<>
class DrvAPI::value_handle<pool_link> {
    DRV_API_VALUE_HANDLE_DEFAULTS(pool_link);
    DRV_API_VALUE_HANDLE_FIELD(pool_link, next, pointer<pool_link>, next_);    
};

///////////////////////
// idx and val types //
///////////////////////
// idx type
using idx_type = int32_t;

// value type
using val_type = float_type;

//////////////////
// nonzero type //
//////////////////
struct nonzero {
    FIELD(idx_type, idx, idx_);
    FIELD(val_type, val, val_);

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.idx() = src.idx();
        dst.val() = src.val();
    }

    operator std::tuple<idx_type, val_type>() const {
        return std::make_tuple(idx(), val());
    }

    /*
     * format as string
     */       
    std::string str() const {
        std::stringstream ss;
        idx_type idx = this->idx();
        float val = (float)this->val();
        ss << "(" << FMT_IDX(idx) << ", " << FMT_VAL(val) << ")";
        return ss.str();
    }    
};
ostream_format_function(nonzero);

template <>
class DrvAPI::value_handle<nonzero> {
    DRV_API_VALUE_HANDLE_DEFAULTS(nonzero);
    DRV_API_VALUE_HANDLE_FIELD(nonzero, idx, idx_type, idx_);
    DRV_API_VALUE_HANDLE_FIELD(nonzero, val, val_type, val_);
};

/////////////////////////////////////////
// memory pool for allocating nonzeros //
/////////////////////////////////////////
struct nonzero_pool_link {
    FIELD(idx_type, capacity, capacity_);
    FIELD(pointer<nonzero_pool_link>, next, next_);

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.capacity() = src.capacity();
        dst.next() = src.next();
    }
};

template <>
class DrvAPI::value_handle<nonzero_pool_link> {
    DRV_API_VALUE_HANDLE_DEFAULTS(nonzero_pool_link);
    DRV_API_VALUE_HANDLE_FIELD(nonzero_pool_link, capacity, idx_type, capacity_);
    DRV_API_VALUE_HANDLE_FIELD(nonzero_pool_link, next, pointer<nonzero_pool_link>, next_);
};

static constexpr idx_type NONZERO_POOL_S = 0; //!< small pool
static constexpr idx_type NONZERO_POOL_M = 1; //!< medium pool
static constexpr idx_type NONZERO_POOL_L = 2; //!< large pool
static constexpr idx_type NONZERO_POOL_N = 3; //!< number of pools

/**
 * @brief nonzero_size_to_pool
 *
 * @param size
 *
 * @return
 */
static idx_type nonzero_size_to_pool(idx_type size) {
    if (size <= 16) {
        return NONZERO_POOL_S;
    } else if (size <= 128) {
        return NONZERO_POOL_M;
    } else {
        return NONZERO_POOL_L;
    }
}
/**
 * @brief nonzero_pool
 */
static l1sp_static<nonzero_pool_link> nonzero_pool[NONZERO_POOL_N][SPMM_THREADS];
static l1sp_static<idx_type> allocated_nonzeros[NONZERO_POOL_N][SPMM_THREADS];

/**
 * @brief allocate_nonzeros
 *
 * @param size
 */
pointer<nonzero> allocate_nonzeros(idx_type size) {
    idx_type pool = nonzero_size_to_pool(size);

    pointer<nonzero_pool_link> head = nonzero_pool[pool][DrvAPI::myThreadId()].address();
    pointer<nonzero_pool_link> prev = head;
    pointer<nonzero_pool_link> curr = head->next();
    while (curr != 0) {
        // will this fit?
        if (curr->capacity() >= size) {
            //DEBUG_STMT(__PRETTY_FUNCTION__ << "allocated from pool " << pool << " size " << size);
            // remove from the list
            pointer<nonzero_pool_link> next = curr->next();
            prev->next() = next;
            curr->next() = 0;
            return curr->next().address();
        }
        prev = curr;
        curr = curr->next();
    }
    // allocate a new buffer
    // todo: can save a word here?
    //DEBUG_STMT(__PRETTY_FUNCTION__ << "allocated from heap " << pool << " size " << size);    
    pointer<nonzero_pool_link> new_link = DrvAPIMemoryAlloc
        (DrvAPIMemoryDRAM, sizeof(nonzero_pool_link) + size * sizeof(nonzero));

    atomic_add(allocated_nonzeros[pool][DrvAPI::myThreadId()].address(),
               sizeof(nonzero_pool_link) + size * sizeof(nonzero));
    
    new_link->capacity() = size;
    new_link->next() = 0;
    return new_link->next().address();
}

/**
 * @brief deallocate_nonzeros
 */
void deallocate_nonzeros(pointer<nonzero> ptr) {
    pointer<nonzero_pool_link> pool_link
        = ((pointer<void>)ptr)
        - offsetof(nonzero_pool_link, next_);

    //DEBUG_STMT(__PRETTY_FUNCTION__ << "deallocated from pool " << pool_link->capacity());

    idx_type pool = nonzero_size_to_pool(pool_link->capacity());
    pointer<nonzero_pool_link> head = nonzero_pool[pool][DrvAPI::myThreadId()].address();
    pointer<nonzero_pool_link> next = head->next();
    pool_link->next() = next;
    head->next() = pool_link;
}

//////////////////////////
// native sparse matrix //
//////////////////////////
/**
 * @brief native_sparse_matrix
 *
 */
struct native_sparse_matrix {
    FIELD(int, rows, rows_); //!< number of rows
    FIELD(int, cols, cols_); //!< number of columns
    FIELD(int, nnz, nnz_); //!< number of nonzeros

    std::vector<int> rowptr_; //!< row pointers
    std::vector<int> & rowptr() {
        return rowptr_;
    }
    const std::vector<int> & rowptr() const {
        return rowptr_;
    }
    
    std::vector<std::pair<int,float>> nonzeros_; //!< nonzeros
    std::vector<std::pair<int,float>> & nonzeros() {
        return nonzeros_;
    }
    const std::vector<std::pair<int,float>> & nonzeros() const {
        return nonzeros_;
    }

    /*
     * constructors
     */
    native_sparse_matrix() = default;
    native_sparse_matrix(const native_sparse_matrix &) = delete;
    native_sparse_matrix(native_sparse_matrix &&) = default;
    /*
     * assignment
     */
    native_sparse_matrix & operator=(const native_sparse_matrix &) = delete;
    native_sparse_matrix & operator=(native_sparse_matrix &&) = default;
    /*
     * destructor
     */
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
            &m.rows(),
            &m.nnz(),
            m.rowptr(),
            m.nonzeros()
        );
        m.cols() = m.rows();
        return m;
    }

    /**
     * convert to and Eigen sparse matrix
     */
    EigenSparseMatrix<float> to_eigen() const {
        EigenSparseMatrix<float> m(rows(), cols());
        std::vector<Eigen::Triplet<float>> triplets;
        for (int i = 0; i < rows(); i++) {
            for (int j = rowptr_[i]; j < rowptr_[i + 1]; j++) {
                triplets.push_back({i, nonzeros_[j].first, nonzeros_[j].second});
            }
        }
        m.setFromTriplets(triplets.begin(), triplets.end());
        return m;
    }

    std::string str() const {
        std::stringstream ss;
        ss <<  rows() << " X " << cols() << ", nnz=" << nnz();
        return ss.str();
    }
};
ostream_format_function(native_sparse_matrix);

////////////////////////
// sparse matrix type //
////////////////////////
struct sparse_matrix {
public:
    FIELD(idx_type, rows, rows_); //!< number of rows
    FIELD(idx_type, cols, cols_); //!< number of columns
    FIELD(idx_type, nnz, nnz_); //!< number of nonzeros
    FIELD(pointer<idx_type>, rowptr, rowptr_); //!< row pointers
    FIELD(pointer<nonzero>, nonzeros, nonzeros_); //!< nonzeros

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.rows() = src.rows();
        dst.cols() = src.cols();
        dst.nnz() = src.nnz();
        dst.rowptr() = src.rowptr();
        dst.nonzeros() = src.nonzeros();
    }

};

template <>
class DrvAPI::value_handle<sparse_matrix> {
    DRV_API_VALUE_HANDLE_DEFAULTS(sparse_matrix);
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, rows, idx_type, rows_);
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, cols, idx_type, cols_);
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, nnz, idx_type, nnz_);
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, rowptr, pointer<idx_type>, rowptr_);
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix, nonzeros, pointer<nonzero>, nonzeros_);
    void initFromNative(const native_sparse_matrix &src) {
        rows() = src.rows();
        cols() = src.cols();
        nnz() = src.nnz();        
        rowptr() = (pointer<idx_type>)DrvAPIMemoryAlloc
            (DrvAPIMemoryDRAM,(src.rows()+1)*sizeof(idx_type));
        nonzeros() = (pointer<nonzero>)DrvAPIMemoryAlloc
            (DrvAPIMemoryDRAM,src.nnz()*sizeof(nonzero));
        
        cello::parallel_for(0, src.rows()+1, 1, [&](int i) {
            rowptr()[i] = src.rowptr()[i];
        });
        cello::parallel_for(0, src.nnz(), 1, [&](int i) {
            nonzeros()[i].idx() = src.nonzeros()[i].first;
            nonzeros()[i].val() = src.nonzeros()[i].second;
        });
    }

    /**
     * multiply two sparse matrices together
     */
    static void multiply
    (DrvAPI::value_handle<sparse_matrix_product> &o,
     DrvAPI::value_handle<sparse_matrix> &i0,
     DrvAPI::value_handle<sparse_matrix> &i1);

    /**
     * get nonzeros of row
     */
    std::pair<pointer<nonzero>,idx_type> nonzerosof(idx_type i) {
        pointer<nonzero> nonzeros = this->nonzeros();
        pointer<idx_type> rowptr = this->rowptr();
        return {nonzeros[rowptr[i]].address(), rowptr[i+1]-rowptr[i]};
    }
};

////////////////////////
// sparse vector type //
////////////////////////
struct sparse_vector {
    FIELD(idx_type, capacity, capacity_); //!< capacity
    FIELD(idx_type, size, size_); //!< number of elements
    FIELD(pointer<nonzero>, nonzeros, nonzeros_); //!< nonzeros

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.size() = src.size();
        dst.capacity() = src.capacity();
        dst.nonzeros() = src.nonzeros();
    }

    std::string str() const {
        std::stringstream ss;
        ss << "size=" << size() << " ";
        ss << "{";
        for (idx_type i = 0; i < size(); i++) {
            ss << nonzeros()[i];
            ss << " ; ";
        }
        ss << "}";
        return ss.str();
    }
};
ostream_format_function(sparse_vector);

template <>
class DrvAPI::value_handle<sparse_vector> {
    DRV_API_VALUE_HANDLE_DEFAULTS(sparse_vector);
    DRV_API_VALUE_HANDLE_FIELD(sparse_vector, capacity, idx_type, capacity_);
    DRV_API_VALUE_HANDLE_FIELD(sparse_vector, size, idx_type, size_);
    DRV_API_VALUE_HANDLE_FIELD(sparse_vector, nonzeros, pointer<nonzero>, nonzeros_);
    void initFromNative(const std::vector<std::pair<int,float>> &src) {
        capacity() = src.size();
        size() = src.size();
        nonzeros() = (pointer<nonzero>)DrvAPIMemoryAlloc
            (DrvAPIMemoryDRAM,src.size()*sizeof(nonzero));
        cello::parallel_for(0, (idx_type)src.size(), 1, [&](int i) {
            nonzeros()[i].idx() = src[i].first;
            nonzeros()[i].val() = src[i].second;
        });
    }
    void initFromSize(idx_type size) {
        this->capacity() = size;
        this->size() = size;
        this->nonzeros() = allocate_nonzeros(size);
    }
    void initFromCapacity(idx_type capacity) {
        this->capacity() = capacity;
        this->size() = 0;
        this->nonzeros() = allocate_nonzeros(capacity);
    }
    void dest() {
        deallocate_nonzeros(nonzeros());
        this->capacity() = 0;
        this->size() = 0;
        this->nonzeros() = 0;
    }

    void update(value_handle<sparse_vector> other, idx_type max_capacity) {
        // determine new capacity
        idx_type sum = this->size() + other.size(); // worst case no compression
        idx_type new_capacity = std::min(sum, max_capacity);
        // allocate new nonzeros
        l1sp_dynamic<sparse_vector> into;
        into.initFromCapacity(new_capacity);
        // merge nonzeros
        idx_type i = 0, j = 0, k = 0;
        while (i < this->size() && j < other.size()) {
            if (this->nonzeros()[i].idx() < other.nonzeros()[j].idx()) {
                into.nonzeros()[k++] = this->nonzeros()[i++];
            } else if (this->nonzeros()[i].idx() > other.nonzeros()[j].idx()) {
                into.nonzeros()[k++] = other.nonzeros()[j++];
            } else {
                into.nonzeros()[k].idx() = this->nonzeros()[i].idx();
                into.nonzeros()[k++].val() = this->nonzeros()[i++].val() + other.nonzeros()[j++].val();
            }
        }
        while (i < this->size()) {
            into.nonzeros()[k++] = this->nonzeros()[i++];
        }
        while (j < other.size()) {
            into.nonzeros()[k++] = other.nonzeros()[j++];
        }
        into.size() = k;
        // move into to this
        dest();
        (*this) = into;
    }
};

/*
 * memory pool for sparse_vector
 */
static l1sp_static<pointer<pool_link>> sparse_vector_pool [SPMM_THREADS];

/*
 * specialize allocation for sparse_vector to keep a memory pool 
 */
template <>
pointer<sparse_vector>
DrvAPI::DrvAPIMemoryAllocateType<sparse_vector>(DrvAPIMemoryType type) {
    pointer<pool_link> head = sparse_vector_pool[DrvAPI::myThreadId()];
    if (head != 0) {
        //DEBUG_STMT("allocating from pool");
        sparse_vector_pool[DrvAPI::myThreadId()] = head->next();
        return (pointer<sparse_vector>)head;
    }
    //DEBUG_STMT("allocating from heap");
    pointer<pool_link> newpool_link = (pointer<pool_link>)DrvAPIMemoryAlloc(type,sizeof(sparse_vector));    
    newpool_link->next() = 0;
    return (pointer<sparse_vector>)newpool_link;
}

/*
 * specialize deallocation for sparse_vector to keep a memory pool 
 */
template <>
void DrvAPI::DrvAPIMemoryDeallocateType<sparse_vector>(const pointer<sparse_vector> &ptr)
{
    //DEBUG_STMT("deallocating to pool");
    pointer<pool_link> new_head = (pointer<pool_link>)ptr;
    pointer<pool_link> old_head = sparse_vector_pool[DrvAPI::myThreadId()];
    new_head->next() = old_head;
    sparse_vector_pool[DrvAPI::myThreadId()] = new_head;
}

///////////////////////////
// sparse matrix product //
///////////////////////////
struct sparse_matrix_product {
    FIELD(idx_type, rows, rows_); //!< number of rows
    FIELD(idx_type, cols, cols_); //!< number of columns
    FIELD(pointer<sparse_vector>, row_vecs, row_vecs_); //!< row pointers
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.rows() = src.rows();
        dst.cols() = src.cols();
        dst.row_vecs() = src.row_vecs();
    }
};

template <>
class DrvAPI::value_handle<sparse_matrix_product> {
    DRV_API_VALUE_HANDLE_DEFAULTS(sparse_matrix_product);
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix_product, rows, idx_type, rows_);
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix_product, cols, idx_type, cols_);
    DRV_API_VALUE_HANDLE_FIELD(sparse_matrix_product, row_vecs, pointer<sparse_vector>, row_vecs_);

    void initFromOperands(DrvAPI::value_handle<sparse_matrix> &i0,
                          DrvAPI::value_handle<sparse_matrix> &i1) {
        rows() = i0.rows();
        cols() = i1.cols();
        row_vecs() = (pointer<sparse_vector>)DrvAPIMemoryAlloc
            (DrvAPIMemoryDRAM,i0.rows()*sizeof(sparse_vector));
    }
};


/////////////////////////////////////////////////////////////////////////////
// multiply: produce a sparse_matrix_product from two sparse_matrix inputs //
/////////////////////////////////////////////////////////////////////////////
void DrvAPI::value_handle<sparse_matrix>::multiply
(DrvAPI::value_handle<sparse_matrix_product> &o,
 DrvAPI::value_handle<sparse_matrix> &i0,
 DrvAPI::value_handle<sparse_matrix> &i1){
    // initialize the output
    o.initFromOperands(i0, i1);
    // for each row of i0...
    cello::parallel_for(0, (idx_type)i0.rows(), 1, [&](idx_type i0_row){
        // ror each nonzero in the row...
        pointer<nonzero> i0_row_nz;
        idx_type i0_row_nnz;
        std::tie(i0_row_nz, i0_row_nnz) = i0.nonzerosof(i0_row);
        l1sp_dynamic<sparse_vector> sum;
        sum.initFromCapacity(4); // todo: make an init empty
        for (idx_type i0_col_idx = 0; i0_col_idx < i0_row_nnz; i0_col_idx++) {
            // compute i0_nz.val() * i1[i0_nz.idx();]
            nonzero i0_nz = i0_row_nz[i0_col_idx];
            pointer<nonzero> i1_col_nz;
            idx_type i1_col_nnz;
            std::tie(i1_col_nz, i1_col_nnz) = i1.nonzerosof(i0_nz.idx());
            l1sp_dynamic<sparse_vector> psum;            
            psum.initFromSize(i1_col_nnz);            
            sparse_vector psumv = psum;
            for (idx_type i1_col_idx = 0; i1_col_idx < i1_col_nnz; i1_col_idx++) {
                nonzero i1_nz = i1_col_nz[i1_col_idx];
                psum.nonzeros()[i1_col_idx].idx() = i1_nz.idx();
                psum.nonzeros()[i1_col_idx].val() = i0_nz.val() * i1_nz.val();
            }
            psum = psumv;
            sum.update(psum, i1.cols());
            //DEBUG_STMT("row " << FMT_IDX(i0_row) << ": sum = " << sum);
            psum.dest();
        }
        // move sum into the output
        DEBUG_STMT("row: " << FMT_IDX(i0_row) << ": " << FMT_IDX(sum.size()) << " nonzeros");
        o.row_vecs()[i0_row] = sum;
    });
}

/*
 * compare two rows of a sparse matrix product
 * in the form of an std::map
 */
void compare_rows
(idx_type i,
 std::map<idx_type,float> &ref_map,
 std::map<idx_type,float>&sol_map)
{
    for (auto itr = ref_map.begin(); itr != ref_map.end(); ++itr) {
        idx_type j = itr->first;
        float v = itr->second;
        auto ito = sol_map.find(j);
        if (ito == sol_map.end()) {
            ERROR_STMT("sol["
                       << FMT_IDX(i) << "," << FMT_IDX(j) << "] = " << 0.0
                       << ", ref["
                       << FMT_IDX(i) << "," << FMT_IDX(j) << "] = " << v);
        } else if(ito->second != v) {
            ERROR_STMT("sol["
                       << FMT_IDX(i) << "," << FMT_IDX(j) << "] = " << ito->second
                       << ", ref["
                       << FMT_IDX(i) << "," << FMT_IDX(j) << "] = " << v);
        }
    }

    for (auto ito = sol_map.begin(); ito != sol_map.end(); ++ito) {
        idx_type j = ito->first;
        float v = ito->second;
        auto itr = ref_map.find(j);
        if (itr == ref_map.end()) {
            ERROR_STMT("sol["
                       << FMT_IDX(i) << "," << FMT_IDX(j) << "] = " << v
                       << ", ref["
                       << FMT_IDX(i) << "," << FMT_IDX(j) << "] = " << 0.0);
        }
    }
}
int CelloMain(int argc, char** argv) {
    std::string i0_name = argv[1];
    std::string i1_name = argv[2];

    INFO_STMT("computing " << i0_name << " X " << i1_name);
    
    native_sparse_matrix i0_native = native_sparse_matrix::FromFile(i0_name);
    native_sparse_matrix i1_native = native_sparse_matrix::FromFile(i1_name);

    INFO_STMT(std::setw(10) << "i0: " << i0_native);
    INFO_STMT(std::setw(10) << "i1: " << i1_native);
    
    // compute a reference product
    EigenSparseMatrix<float> o_ref
        = i0_native.to_eigen()
        * i1_native.to_eigen();

    INFO_STMT(std::setw(10) << "o_ref: "
              << o_ref.rows() << " X " << o_ref.cols()
              << ", nnz=" << o_ref.nonZeros());

    dram_dynamic<sparse_matrix> i0, i1;
    i0.initFromNative(i0_native);
    i1.initFromNative(i1_native);
    dram_dynamic<sparse_matrix_product> o;
    {
        timer _("row-wise product");
        DrvAPI::value_handle<sparse_matrix>::multiply(o, i0, i1);
    }
    {
        // check the reference product
        for (idx_type i = 0; i < o_ref.rows(); i++) {
            EigenSparseVector<float> ref = o_ref.row(i);
            std::map<idx_type, float> ref_map;
            for (EigenSparseVector<float>::InnerIterator it(ref); it; ++it) {
                ref_map[it.index()] = it.value();
            }
            sparse_vector sol = o.row_vecs()[i];
            std::map<idx_type, float> sol_map;
            for (idx_type i = 0; i < sol.size(); i++) {
                nonzero nz = sol.nonzeros()[i];
                sol_map[nz.idx()] = (float)nz.val();
            }
            // compare maps
            compare_rows(i, ref_map, sol_map);
        }        
    }

    idx_type l1_used = 0;
    for (long tid = 0; tid < SPMM_THREADS; tid++) {
        l1_used += allocated_nonzeros[NONZERO_POOL_S][tid];
        l1_used += allocated_nonzeros[NONZERO_POOL_M][tid];
        l1_used += allocated_nonzeros[NONZERO_POOL_L][tid];
    }
    INFO_STMT("l1 used: " << FMT_IDX(l1_used) << " bytes");
    return 0;
}

