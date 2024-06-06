// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include <DrvAPI.hpp>
#include <stdint.h>
#include <inttypes.h>
#include <sstream>

#define FIELD(type, name, field)                \
    type field;                                 \
    const type& name() const { return field; }  \
    type& name() { return field; }              \

struct bar {
    FIELD(int, x, x_)
    FIELD(float, y, y_)

    template <typename DstBarT, typename SrcBarT>
    static void copy(DstBarT &dst, const SrcBarT &src) {
        dst.x() = src.x();
        dst.y() = src.y();
    }
};

template <>
class DrvAPI::value_handle<bar> {
    DRV_API_VALUE_HANDLE_DEFAULTS(bar)
    DRV_API_VALUE_HANDLE_FIELD(bar, x, int, x_)
    DRV_API_VALUE_HANDLE_FIELD(bar, y, float, y_)
};

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
    FIELD(bar, b, b_)
    FIELD(int, x, x_)
    FIELD(float, y, y_)

    template <typename DstFooT, typename SrcFooT>
    static void copy(DstFooT &dst, const SrcFooT &src) {
        dst.b() = src.b();
        dst.x() = src.x();
        dst.y() = src.y();
    }
};

template <>
class DrvAPI::value_handle<foo> {
    DRV_API_VALUE_HANDLE_DEFAULTS(foo)
    DRV_API_VALUE_HANDLE_FIELD(foo, b, bar, b_)
    DRV_API_VALUE_HANDLE_FIELD(foo, x, int, x_)
    DRV_API_VALUE_HANDLE_FIELD(foo, y, float, y_)
};

template <typename T>
struct vector {
    typedef DrvAPI::value_handle<T> reference_type;
    typedef const DrvAPI::value_handle<T> const_reference_type;
    typedef T value_type;

    FIELD(DrvAPI::pointer<T>, data, data_)
    FIELD(size_t, size, size_)
    FIELD(size_t, capacity, capacity_)    
};

template <typename T>
class DrvAPI::value_handle<vector<T>> {
public:
    typedef value_handle<T> reference_type;
    typedef const value_handle<T> const_reference_type;
    typedef T value_type;
    DRV_API_VALUE_HANDLE_DEFAULTS(vector<T>)
    DRV_API_VALUE_HANDLE_FIELD(vector<T>, size, size_t, size_)
    DRV_API_VALUE_HANDLE_FIELD(vector<T>, capacity, size_t, capacity_)
    DRV_API_VALUE_HANDLE_FIELD(vector<T>, data, pointer<T>, data_)
};

template <typename VectorDataT>
class vector_impl {
public:
    vector_impl(const VectorDataT &data):
        vector(data) {
        vector.data() = DrvAPI::pointer<typename VectorDataT::value_type>(-1ul);
        vector.capacity() = 0;
        vector.size() = 0;
    }

    void resize(size_t new_size) {
        if (vector.capacity() < new_size) {
            if (vector.data() != -1ul)
                DrvAPI::DrvAPIMemoryFree((DrvAPI::DrvAPIAddress)vector.data(), vector.capacity() * sizeof(typename VectorDataT::value_type));
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
void print_bar(const DrvAPI::value_handle<bar> &bar) {
    print_bar_(bar);
}

DrvAPI::l1sp_static<int> l1sp_int;
DrvAPI::l2sp_static<int> l2sp_int;
DrvAPI::dram_static<int> dram_int;
DrvAPI::dram_static<bar> l1sp_bar;
DrvAPI::dram_static<vector<int>> dram_vector;


int PointerMain(int argc, char* argv[])
{
    DrvAPI::DrvAPIMemoryAllocatorInit();
    using namespace DrvAPI;
    {
        DrvAPIAddress a = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(int64_t));
        DrvAPIAddress b = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(int64_t));
        DrvAPIAddress c = DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(int64_t));
        DrvAPI::value_handle<int64_t> i(a), j(b);
        i = 71;
        j = i;        
        printf("i = %ld, j = %ld\n", (int64_t)i, (int64_t)j);
        DrvAPI::pointer<int64_t> kp(c);
        *kp = i;
        printf("*kp = %ld\n", (int64_t)*kp);
    }
    {
        DrvAPI::pointer<bar> bp (DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(bar)));
        DrvAPI::pointer<foo> fp (DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(foo)));
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
        DrvAPI::value_handle<int64_t>
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
        DrvAPI::value_handle<vector<int>> v(DrvAPIMemoryAlloc(DrvAPIMemoryDRAM, sizeof(vector<int>)));
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
        //DrvAPI::value_handle<vector<int>> v(dram_vector);
        vector_impl<DrvAPI::value_handle<vector<int>>> v_impl(dram_vector);
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
        DrvAPI::value_handle<bar> bar_alias (&l1sp_bar);
        l1sp_bar.x() = 32;
        l1sp_bar.y() = M_PI;
        printf("l1sp_bar.x() = %d, l1sp_bar.y() = %f\n", (int)l1sp_bar.x(), (double)l1sp_bar.y());
        print_bar(l1sp_bar);
        print_bar(bar_alias);
    }
    {
        DrvAPI::l1sp_dynamic<bar> b;
        b.x() = 71;
        b.y() = 2*M_PI;
        print_bar(b);
        printf("&b = %lx\n", (DrvAPIAddress)&b);
        
    }
    {
        DrvAPI::l1sp_dynamic<int> x = 1;
        DrvAPI::l1sp_dynamic<int> y = 2;
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
