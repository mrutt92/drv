// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include <DrvAPI.hpp>
#include <stdint.h>
#include <inttypes.h>
#include <sstream>
namespace v2
{

template <typename T>
class pointer;

template <typename T>
class value_handle {
public:
    value_handle(DrvAPI::DrvAPIAddress ptr):
        _ptr(ptr) {
    }
    value_handle():
        _ptr(0) {
    }    

    value_handle(const value_handle&) = default;
    value_handle(value_handle &&) = default;

    value_handle & operator=(const value_handle &other) {
        *this = (T)other;
        return *this;
    }

    value_handle & operator=(value_handle &&other) {
        *this = (T)other;
        return *this;
    }

    ~value_handle() = default;

    operator T() const {
        return DrvAPI::read<T>(_ptr);
    }

    value_handle & operator=(const T&v) {
        DrvAPI::write<T>(_ptr, v);
        return *this;
    }    

    pointer<T> operator&() {
        return pointer<T>(_ptr);
    }
    
    DrvAPI::DrvAPIAddress _ptr;
};

#define SPECIALIZE_VALUE_HANDLE_CONSTRURCTORS(type)             \
    public:                                             \
        value_handle(DrvAPI::DrvAPIAddress ptr):        \
            _ptr(ptr) {                                 \
        }                                               \
        value_handle():                                 \
            _ptr(0) {                                   \
        }                                               \
        value_handle(const value_handle&) = default;    \
        value_handle(value_handle &&) = default;        \
        ~value_handle() = default;                      \

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

#define SPECIALIZE_VALUE_HANDLE_CAST_OPERATORS(type)             \
    public:                                             \
        operator type() const {                 \
            type r;                             \
            type::copy(r, *this);               \
            return r;                           \
        }
#define SPECIALIZE_VALUE_HANDLE_ADDRESSOF_OPERATORS(type)         \
    public:                                             \
        pointer<type> operator&() {             \
            return pointer<type>(_ptr);         \
        }

#define SPECIALIZE_VALUE_HANDLE_FIELDS(type)            \
    public:                                             \
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
    public:                                     \
        value_handle(DrvAPI::DrvAPIAddress ptr):\
            _ptr(ptr) {                         \
        }                                       \
        value_handle():                          \
            _ptr(0) {                           \
        }                                       \
        value_handle(const value_handle&) = default; \
        value_handle(value_handle &&) = default; \
        value_handle & operator=(const value_handle &other) { \
            *this = (type)other;                \
            return *this;                       \
        }                                       \
        value_handle & operator=(value_handle &&other) { \
            *this = (type)other;                \
            return *this;                       \
        }                                       \
        ~value_handle() = default;              \
        operator type() const {                 \
            type r;                             \
            type::copy(r, *this);               \
            return r;                           \
        }                                       \
        value_handle & operator=(const type&v) {\
            type::copy(*this, v);               \
            return *this;                       \
        }                                       \
        pointer<type> operator&() {             \
            return pointer<type>(_ptr);         \
        }                                       \
        DrvAPI::DrvAPIAddress _ptr;


/**
 * generates accessors for data members
 * type - the type of the object for which this is a handle
 * field - the name of the field, will be accessed by value_handle<type>::field()
 * field_type - the type of the field
 * field_data - the data member of the field, this must be a concreete data member inside the type
 */
#define SPECIALIZE_VALUE_HANDLE_FIELD(type, field, field_type, field_data) \
    value_handle<field_type> field() {                                  \
        return value_handle<field_type>(_ptr + offsetof(type, field_data)); \
    }                                                                   \
    const value_handle<field_type> field() const {                      \
        return value_handle<field_type>(_ptr + offsetof(type, field_data)); \
    }


/**
 * ends specialization of value_handle for a type
 */
#define SPECIALIZE_VALUE_HANDLE_END()           \
    };

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

template <typename T>
class value_handle<pointer<T>> {
    SPECIALIZE_VALUE_HANDLE_CONSTRURCTORS(pointer<T>)

    operator pointer<T>() const {
        return DrvAPI::read<pointer<T>>(_ptr);
    }

    value_handle & operator=(const pointer<T>&v) {
        DrvAPI::write<pointer<T>>(_ptr, v);
        return *this;
    }


    value_handle & operator=(DrvAPI::DrvAPIAddress v) {
        *this = pointer<T>(v);
        return *this;
    }

    operator DrvAPI::DrvAPIAddress() const {
        pointer<T> p = *this;
        return (DrvAPI::DrvAPIAddress)p;
    }

    value_handle<T> operator[](size_t index) {
        return value_handle<T>(_ptr + index * sizeof(T));
    }

    const value_handle<T> operator[](size_t index) const {
        return value_handle<T>(_ptr + index * sizeof(T));
    }
    
    SPECIALIZE_VALUE_HANDLE_ADDRESSOF_OPERATORS(pointer<T>)
    SPECIALIZE_VALUE_HANDLE_FIELDS(pointer<T>)
};

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
    typedef T& reference_type;
    typedef const T& const_reference_type;
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
    value_handle(DrvAPI::DrvAPIAddress ptr):
        _ptr(ptr) {
    }
    value_handle():
        _ptr(0) {
    }
    value_handle(const value_handle &other) = default;
    value_handle(value_handle &&other) = default;
    value_handle & operator=(const value_handle &other) {
        _ptr = (vector<T>)other;
        return *this;
    }
    value_handle & operator=(value_handle &&other) {
        _ptr = (vector<T>)other;
        return *this;
    }
    ~value_handle() = default;
    operator vector<T>() const {
        vector<T> v;
        vector<T>::copy(v, *this);
    }
    value_handle<vector<T>> operator=(const vector<T> &other) {
        vector<T>::copy(*this, other);
        return *this;
    }
    pointer<T> operator&() {
        return pointer<T>(_ptr + offsetof(vector<T>, data_));
    }

    SPECIALIZE_VALUE_HANDLE_FIELD(vector<T>, size, size_t, size_)
    SPECIALIZE_VALUE_HANDLE_FIELD(vector<T>, capacity, size_t, capacity_)
    SPECIALIZE_VALUE_HANDLE_FIELD(vector<T>, data, pointer<T>, data_)
    
    DrvAPI::DrvAPIAddress _ptr;
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
void print_bar(const BarT& bar) {
    printf("bar.x() = %d, bar.y() = %f\n", (int)bar.x(), (float)bar.y());
}

int PointerMain(int argc, char* argv[])
{
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
    return 0;
}

declare_drv_api_main(PointerMain);
