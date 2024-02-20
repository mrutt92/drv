// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include <DrvAPI.hpp>
#include <stdint.h>
#include <inttypes.h>
#include <sstream>
namespace v2
{

/**
 * forward declaration of pointer
 */
template <typename T>
class pointer;

/**
 * @brief default value_handle suitable for most builtin types
 */
template <typename T>
class value_handle {
public:
    value_handle(DrvAPI::DrvAPIAddress ptr):
        _ptr(ptr) {
    }
    value_handle():
        _ptr(0) {
    }    

    value_handle(const value_handle&o) {
        _ptr = o.address();
    }
    
    value_handle(value_handle && o) {
        _ptr = o.address();
    }

    value_handle & operator=(const value_handle &other) {
        *this = (T)other;
        return *this;
    }

    value_handle & operator=(value_handle &&other) {
        *this = (T)other;
        return *this;
    }

    virtual ~value_handle() = default;

    operator T() const {
        return DrvAPI::read<T>(address());        
    }

    value_handle & operator=(const T&v) {
        DrvAPI::write<T>(address(), v);
        return *this;
    }    

    pointer<T> operator&() {
        return pointer<T>(address());
    }

    virtual DrvAPI::DrvAPIAddress address() const {
        return _ptr;
    }
    
    DrvAPI::DrvAPIAddress _ptr;
};

/**
 * generate default constructors for value_handle specializations
 */
#define SPECIALIZE_VALUE_HANDLE_CONSTRUCTORS(type)             \
    public:                                             \
        value_handle(DrvAPI::DrvAPIAddress ptr):        \
            _ptr(ptr) {                                 \
        }                                               \
        value_handle():                                 \
            _ptr(0) {                                   \
        }                                               \
        value_handle(const value_handle& o) {           \
            _ptr = o.address();                         \
        }                                               \
        value_handle(value_handle && o) {               \
            _ptr = o.address();                         \
        }                                               \
        virtual ~value_handle() = default;               \

/**
 * generate default assignment operators for value_handle specializations
 */
#define SPECIALIZE_VALUE_HANDLE_ASSIGNMENT_OPERATORS(type)      \
    public:                                             \
        value_handle & operator=(const value_handle &other) { \
            *this = (type)other;                \
            return *this;                       \
        }                                       \
        value_handle & operator=(value_handle &&other) { \
            *this = (type)other;                \
            return *this;                       \
        }                                       \
        value_handle & operator=(const type&v) { \
            type::copy(*this, v);               \
            return *this;                       \
        }                                       \

/**
 * generate default assignment operators for value_handle specializations
 * for scalar types
 */
#define SPECIALIZE_VALUE_HANDLE_ASSIGNMENT_OPERATORS_TRIVIAL(type)      \
    public:                                             \
        value_handle & operator=(const value_handle &other) { \
            *this = (type)other;                                  \
            return *this;                       \
        }                                       \
        value_handle & operator=(value_handle &&other) { \
             *this = (type)other;                \
             return *this;                       \
        }                                       \
        value_handle & operator=(const type&v) { \
            DrvAPI::write<type>(address(), v);      \
            return *this;                       \
        }

/**
 * generate default cast operators for value_handle specializations
 */
#define SPECIALIZE_VALUE_HANDLE_CAST_OPERATORS(type)             \
    public:                                             \
        operator type() const {                 \
            type r;                             \
            type::copy(r, *this);               \
            return r;                           \
        }

/**
 * generate default cast operators for value_handle specializations
 * for scalar types
 */
#define SPECIALIZE_VALUE_HANDLE_CAST_OPERATORS_TRIVIAL(type)            \
    public:                                                             \
    operator type() const {                                             \
        return DrvAPI::read<T>(address());                              \
    }

/**
 * generate default addressof operators for value_handle specializations
 */
#define SPECIALIZE_VALUE_HANDLE_ADDRESSOF_OPERATORS(type)         \
    public:                                             \
        pointer<type> operator&() {             \
            return pointer<type>(address());        \
        }

/**
 * generate the internal members for value_handle specializations
 */
#define SPECIALIZE_VALUE_HANDLE_INTERNAL(type)          \
    public:                                             \
    virtual DrvAPI::DrvAPIAddress address() const {     \
        return _ptr;                                    \
    }                                                   \
    DrvAPI::DrvAPIAddress _ptr;

/**
 * begins specialization of value_handle for a type
 * type - the type to specialize for
 *
 * it is expected that type defines static member functions
 * of the following forms:
 *
 * static void copy(DstType& &dst, const SrcType &src);
 * for DstType = [type, value_handle<type>] and SrcType = [type, value_handle<type>]
 */
#define SPECIALIZE_VALUE_HANDLE_BEGIN(type)     \
    template <>                                 \
    class v2::value_handle<type> {              \
    SPECIALIZE_VALUE_HANDLE_CONSTRUCTORS(type)             \
    SPECIALIZE_VALUE_HANDLE_ASSIGNMENT_OPERATORS(type) \
    SPECIALIZE_VALUE_HANDLE_CAST_OPERATORS(type) \
    SPECIALIZE_VALUE_HANDLE_ADDRESSOF_OPERATORS(type) \
    SPECIALIZE_VALUE_HANDLE_INTERNAL(type)

/**
 * generates accessors for data members
 * type - the type of the object for which this is a handle
 * field - the name of the field, will be accessed by value_handle<type>::field()
 * field_type - the type of the field
 * field_data - the data member of the field, this must be a concreete data member inside the type
 */
#define SPECIALIZE_VALUE_HANDLE_FIELD(type, field, field_type, field_data) \
    value_handle<field_type> field() {                                  \
        return value_handle<field_type>(address() + offsetof(type, field_data)); \
    }                                                                   \
    const value_handle<field_type> field() const {                      \
        return value_handle<field_type>(address() + offsetof(type, field_data)); \
    }


/**
 * ends specialization of value_handle for a type
 */
#define SPECIALIZE_VALUE_HANDLE_END()           \
    };

/**
 * The pointer class
 *
 * This class is used to represent a pointer to a value in the target process
 * supports dereferencing and array indexing
 *
 * Does not support the -> operator (sorry)
 *
 * To get support something like the -> operator, use the value_handle class
 * and the helper macros to specialize it for your type
 */
template <typename T>
class pointer {
public:
    pointer(DrvAPI::DrvAPIAddress ptr):
        _ptr(ptr) {
    }

    pointer():
        _ptr(0) {
    }

    pointer(const pointer &other) = default;
    pointer(pointer &&other) = default;
    pointer & operator=(const pointer &other) = default;
    pointer & operator=(pointer &&other) = default;
    ~pointer() = default;

    operator DrvAPI::DrvAPIAddress() const {
        return _ptr;
    }

    value_handle<T> operator*() {
        value_handle<T> handle(_ptr);
        return handle;
    }

    const value_handle<T> operator*() const {
        value_handle<T> handle(_ptr);
        return handle;
    }

    value_handle<T> operator[](size_t index) {
        return value_handle<T>(_ptr + index * sizeof(T));
    }

    const value_handle<T> operator[](size_t index) const {
        return value_handle<T>(_ptr + index * sizeof(T));
    }
    
    DrvAPI::DrvAPIAddress _ptr;
};

/**
 * Specialization of value_handle for pointer type
 */
template <typename T>
class value_handle<pointer<T>> {
    SPECIALIZE_VALUE_HANDLE_CONSTRUCTORS(pointer<T>)
    SPECIALIZE_VALUE_HANDLE_CAST_OPERATORS_TRIVIAL(pointer<T>)
    SPECIALIZE_VALUE_HANDLE_ASSIGNMENT_OPERATORS_TRIVIAL(pointer<T>)
    
    value_handle & operator=(DrvAPI::DrvAPIAddress v) {
        *this = pointer<T>(v);
        return *this;
    }

    operator DrvAPI::DrvAPIAddress() const {
        pointer<T> p = *this;
        return (DrvAPI::DrvAPIAddress)p;
    }

    value_handle<T> operator[](size_t index) {
        pointer<T> p = *this;
        return p[index];
    }

    const value_handle<T> operator[](size_t index) const {
        pointer<T> p = *this;
        return p[index];
    }

    SPECIALIZE_VALUE_HANDLE_ADDRESSOF_OPERATORS(pointer<T>)
    SPECIALIZE_VALUE_HANDLE_INTERNAL(pointer<T>)
};

/**
 * statically allocatable data; can be allocated in special memory regions
 */
template <typename T, DrvAPI::DrvAPIMemoryType MEMTYPE>
class static_data : public value_handle<T> {
public:
    /**
     * @brief constructor
     */
    static_data() {
        _offset = DrvAPI::DrvAPISection::GetSection(MEMTYPE).increaseSizeBy(sizeof(T));
    }
    static_data(const static_data &other) = delete;
    static_data(static_data &&other) = delete;
    ~static_data() = default;

    /**
     * handle assignment is a deep copy
     */
    static_data & operator=(const static_data &other) {
        value_handle<T> me(address());
        value_handle<T> you(other.address());
        me = you;
        return *this;
    }

    /**
     * handle assignment is a deep copy
     */    
    static_data & operator=(static_data &&other) {
        value_handle<T> me(address());
        value_handle<T> you(other.address());
        me = you;
        return *this;
    }

    /**
     * assignment operators
     */
    static_data & operator=(const T &v) {
        value_handle<T> handle(address());
        handle = v;
        return *this;
    }

    /**
     * materialize the address of the static data
     */
    DrvAPI::DrvAPIAddress address() const override {
        DrvAPI::DrvAPIAddress r = DrvAPI::DrvAPISection::GetSection(MEMTYPE)
            .getBase(DrvAPI::myPXNId(), DrvAPI::myPodId(), DrvAPI::myCoreId())
            + _offset;
        return r;   
    }
    

    DrvAPI::DrvAPIAddress _offset;
};

/**
 * static data in L1SP
 */
template <typename T>
using l1sp_static = static_data<T, DrvAPI::DrvAPIMemoryType::DrvAPIMemoryL1SP>;

/**
 * static data in L2SP
 */
template <typename T>
using l2sp_static = static_data<T, DrvAPI::DrvAPIMemoryType::DrvAPIMemoryL2SP>;

/**
 * static data in L3SP
 */
template <typename T>
using dram_static = static_data<T, DrvAPI::DrvAPIMemoryType::DrvAPIMemoryDRAM>;

/**
 * dynamic data; can be allocated in special memory regions
 */
template <typename T, DrvAPI::DrvAPIMemoryType MEMTYPE>
class dynamic_data : public value_handle<T> {
public:
    /**
     * @brief constructor
     */
    dynamic_data():
        value_handle<T>(DrvAPI::DrvAPIMemoryAlloc(MEMTYPE, sizeof(T))) {        
    }

    dynamic_data(const T&v) :
        dynamic_data() {
        value_handle<T> handle(this->_ptr);
        handle = v;
    }

    dynamic_data(const dynamic_data &other) = delete;
    dynamic_data(dynamic_data &&other) {
        this->_ptr = other._ptr;
        other._ptr = 0;
    }

    dynamic_data & operator=(const dynamic_data &other) {
        value_handle<T> me(this->_ptr);
        value_handle<T> you(other._ptr);
        me = you;
        return *this;
    }

    dynamic_data & operator=(dynamic_data &&other) = delete;

    dynamic_data & operator=(const T &v) {
        value_handle<T> handle(this->_ptr);
        handle = v;
        return *this;
    }
    
    ~dynamic_data() {
        DrvAPI::DrvAPIMemoryFree(this->_ptr);
    }
};

/**
 * dynamic data in L1SP
 */
template <typename T>
using l1sp_dynamic = dynamic_data<T, DrvAPI::DrvAPIMemoryType::DrvAPIMemoryL1SP>;

/**
 * dynamic data in L2SP
 */
template <typename T>
using l2sp_dynamic = dynamic_data<T, DrvAPI::DrvAPIMemoryType::DrvAPIMemoryL2SP>;

/**
 * dynamic data in L3SP
 */
template <typename T>
using dram_dynamic = dynamic_data<T, DrvAPI::DrvAPIMemoryType::DrvAPIMemoryDRAM>;

}


struct bar {
    int         & x() { return x_; }
    const int   & x() const { return x_; }
    float       & y() { return y_; }
    const float & y() const { return y_; }

    template <typename DstBarT, typename SrcBarT>
    static void copy(DstBarT &dst, const SrcBarT &src) {
        dst.x() = src.x();
        dst.y() = src.y();
    }
    
    int   x_;
    float y_;    
};

SPECIALIZE_VALUE_HANDLE_BEGIN(bar)
    SPECIALIZE_VALUE_HANDLE_FIELD(bar, x, int, x_)
    SPECIALIZE_VALUE_HANDLE_FIELD(bar, y, float, y_)
SPECIALIZE_VALUE_HANDLE_END()

template <typename BarDataT>
class bar_impl {
public:
    bar_impl(const BarDataT &data):
        bar(data) {
    }
    std::string to_string() const {
        std::stringstream ss;
        ss << "x: " << bar.x() << ", y: " << bar.y();
        return ss.str();
    }
    BarDataT bar;
};

struct foo {
    bar&         b() { return b_; }
    const bar&   b() const { return b_; }
    int&         x() { return x_; }
    const int&   x() const { return x_; }
    float&       y() { return y_; }
    const float& y() const { return y_; }

    template <typename DstFooT, typename SrcFooT>
    static void copy(DstFooT &dst, const SrcFooT &src) {
        dst.b() = src.b();
        dst.x() = src.x();
        dst.y() = src.y();
    }

    bar   b_;
    int   x_;
    float y_;    
};

SPECIALIZE_VALUE_HANDLE_BEGIN(foo)
    SPECIALIZE_VALUE_HANDLE_FIELD(foo, b, bar, b_)
    SPECIALIZE_VALUE_HANDLE_FIELD(foo, x, int, x_)
    SPECIALIZE_VALUE_HANDLE_FIELD(foo, y, float, y_)
SPECIALIZE_VALUE_HANDLE_END()


template <typename T>
struct vector {
    typedef v2::value_handle<T> reference_type;
    typedef const v2::value_handle<T> const_reference_type;
    typedef T value_type;

    v2::pointer<T> data_;
    size_t         size_;
    size_t     capacity_;
    size_t &           size() { return size_; }
    const size_t &     size() const { return size_; }
    size_t &       capacity() { return capacity_; }
    const size_t & capacity() const { return capacity_; }
    const v2::pointer<T> & data() const { return data_; }
    v2::pointer<T> &       data() { return data_; }
    
};

template <typename T>
class v2::value_handle<vector<T>> {
public:
    typedef value_handle<T> reference_type;
    typedef const value_handle<T> const_reference_type;
    typedef T value_type;
    SPECIALIZE_VALUE_HANDLE_CONSTRUCTORS(vector<T>)
    SPECIALIZE_VALUE_HANDLE_ASSIGNMENT_OPERATORS(vector<T>)
    SPECIALIZE_VALUE_HANDLE_CAST_OPERATORS(vector<T>)

    SPECIALIZE_VALUE_HANDLE_FIELD(vector<T>, size, size_t, size_)
    SPECIALIZE_VALUE_HANDLE_FIELD(vector<T>, capacity, size_t, capacity_)
    SPECIALIZE_VALUE_HANDLE_FIELD(vector<T>, data, pointer<T>, data_)
    SPECIALIZE_VALUE_HANDLE_INTERNAL()
};

template <typename VectorDataT>
class vector_impl {
public:
    vector_impl(const VectorDataT &data):
        vector(data) {
        vector.data() = v2::pointer<typename VectorDataT::value_type>(0ul);
        vector.capacity() = 0;
        vector.size() = 0;
    }

    void resize(size_t new_size) {
        if (vector.capacity() < new_size) {
            DrvAPI::DrvAPIMemoryFree((DrvAPI::DrvAPIAddress)vector.data());
            vector.data() = (DrvAPI::DrvAPIAddress)DrvAPI::DrvAPIMemoryAlloc(DrvAPI::DrvAPIMemoryDRAM, new_size * sizeof(typename VectorDataT::value_type));
            vector.capacity() = new_size;
            vector.size() = new_size;
        } else {
            vector.size() = new_size;
        }
    }

    typename VectorDataT::reference_type operator[](size_t index) {
        return vector.data()[index];
    }
    typename VectorDataT::const_reference_type operator[](size_t index) const {
        return vector.data()[index];
    }

    size_t size() const {
        return vector.size();
    }
    
    VectorDataT vector;

};

template <typename BarT, typename FooT>
void test(BarT bar, FooT foo) {
    bar = foo.b();
    foo.x() = 1;
    foo.y() = 2.0f;
    bar = foo.b();
    foo.b() = bar;
    printf("foo.b().x() = %d, foo.b().y() = %f\n", (int)foo.b().x(), (float)foo.b().y());
    printf("foo.x() = %d, foo.y() = %f\n", (int)foo.x(), (float)foo.y());
}

template <typename BarT>
void print_bar_(BarT bar) {
    printf("%s: bar.x() = %d, bar.y() = %f\n"
           , __PRETTY_FUNCTION__
           , (int)bar.x()
           , (float)bar.y());
}

void print_bar(const bar& bar) {
    print_bar_(bar);
}
void print_bar(const v2::value_handle<bar> &bar) {
    print_bar_(bar);
}

v2::l1sp_static<int> l1sp_int;
v2::l2sp_static<int> l2sp_int;
v2::dram_static<int> dram_int;
v2::dram_static<bar> l1sp_bar;
v2::dram_static<vector<int>> dram_vector;


int PointerMain(int argc, char* argv[])
{
    DrvAPI::DrvAPIMemoryAllocatorInit();
    using namespace DrvAPI;
    {
        DrvAPIAddress a = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(int64_t));
        DrvAPIAddress b = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(int64_t));
        DrvAPIAddress c = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(int64_t));
        v2::value_handle<int64_t> i(a), j(b);
        i = 71;
        j = i;        
        printf("i = %ld, j = %ld\n", (int64_t)i, (int64_t)j);
        v2::pointer<int64_t> kp(c);
        *kp = i;
        printf("*kp = %ld\n", (int64_t)*kp);
    }
    {
        v2::pointer<bar> bp (DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(bar)));
        v2::pointer<foo> fp (DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(foo)));
        (*fp).b().x() = 32;
        (*fp).b().y() = M_PI;
        test(*bp, *fp);
        bar_impl<decltype(*bp)> bar(*bp);
        printf("bar.to_string() = %s\n", bar.to_string().c_str());
    }
    {
        bar b;
        b.x() = 32;
        b.y() = M_PI;
        foo f;
        f.b().x() = 32;
        f.b().y() = M_PI;
        test(b, f);
        bar_impl<decltype(b)> bar(b);
        printf("bar.to_string() = %s\n", bar.to_string().c_str());
    }
    {
        v2::value_handle<int64_t>
            a (DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(int64_t))),
            b (DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(int64_t))),
            c (DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(int64_t)));
        a = 1;
        b = 2;
        c = a * (2*b) + 1;
        printf("a = %ld, b = %ld, c = %ld\n", (int64_t)a, (int64_t)b, (int64_t)c);
        
    }
    {
        printf("native test\n");
        vector<int> v;
        vector_impl<decltype(v)> v_impl(v);
        v_impl.resize(10);
        for (size_t i = 0; i < v_impl.size(); ++i) {
            v_impl[i] = i;
        }
        for (size_t i = 0; i < v_impl.size(); ++i) {
            printf("v[%ld] = %d\n", i, (int)v_impl[i]);
        }
    }    
    {
        printf("handle test\n");
        v2::value_handle<vector<int>> v(DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(vector<int>)));
        vector_impl<decltype(v)> v_impl(v);
        v_impl.resize(10);
        for (size_t i = 0; i < v_impl.size(); ++i) {
            v_impl[i] = i;
        }
        for (size_t i = 0; i < v_impl.size(); ++i) {
            printf("v[%ld] = %d\n", i, (int)v_impl[i]);
        }
    }
    {
        printf("static test\n");
        //v2::value_handle<vector<int>> v(dram_vector);
        vector_impl<v2::value_handle<vector<int>>> v_impl(dram_vector);
        v_impl.resize(10);
        for (size_t i = 0; i < v_impl.size(); ++i) {
            v_impl[i] = i;
        }
        for (size_t i = 0; i < v_impl.size(); ++i) {
            printf("v[%ld] = %d\n", i, (int)v_impl[i]);
        }
    }    
    {
        l1sp_int = 1;
        l2sp_int = 2;
        dram_int = 3;
        printf("l1sp_int = %lx, l2sp_int = %lx, dram_int = %lx\n"
               , (int64_t)l1sp_int, (int64_t)l2sp_int, (int64_t)dram_int);
        printf("l1sp_int = %d\n", (int)l1sp_int);
        printf("l2sp_int = %d\n", (int)l2sp_int);
        printf("dram_int = %d\n", (int)dram_int);
        printf("l1sp_int + l2sp_int + dram_int = %d\n", l1sp_int + 4*l2sp_int + dram_int);
    }
    {
        v2::value_handle<bar> bar_alias (&l1sp_bar);
        l1sp_bar.x() = 32;
        l1sp_bar.y() = M_PI;
        printf("l1sp_bar.x() = %d, l1sp_bar.y() = %f\n", (int)l1sp_bar.x(), (double)l1sp_bar.y());
        print_bar(l1sp_bar);
        print_bar(bar_alias);
    }
    {
        v2::l1sp_dynamic<bar> b;
        b.x() = 71;
        b.y() = 2*M_PI;
        print_bar(b);
        printf("&b = %lx\n", (DrvAPIAddress)&b);
        
    }
    {
        v2::l1sp_dynamic<int> x = 1;
        v2::l1sp_dynamic<int> y = 2;
        printf("x = %d, y = %d\n", (int)x, (int)y);
        printf("&x = %lx, &y = %lx\n", (DrvAPIAddress)&x, (DrvAPIAddress)&y);
        y = x;
        printf("x = %d, y = %d\n", (int)x, (int)y);
        x = l2sp_int;
        printf("x = %d, y = %d\n", (int)x, (int)y);
        dram_int = x;
        printf("x = %d, dram_int = %d\n", (int)x, (int)dram_int);
    }
    return 0;
}

declare_drv_api_main(PointerMain);
