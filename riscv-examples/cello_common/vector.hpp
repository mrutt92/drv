#ifndef VECTOR_HPP
#define VECTOR_HPP
#include "pointer.hpp"
#include "field.hpp"
#ifdef RISCV
#include <pandohammer/allocator.h>
#else
#include <DrvAPI.hpp>
#endif

namespace common {
template <typename T>
struct vector {
    typedef T value_type;
    typedef uint64_t size_type;
    FIELD(pointer<value_type>, data, _data);
    FIELD(size_type, size, _size);

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        {
            pointer<value_type> _;
            _ = src.data();
            dst.data() = _;
        }
        {
            size_type _;
            _ = src.size();
            dst.size() = _;
        }
    }
    /**
     * Access element at index i
     */
    static
    reference<value_type> AT(pointer<vector> vec, size_type i) {
        return vec->data()[i];
    }

    /**
     * Access element at index i
     */
    static
    const_reference<value_type> AT(const_pointer<vector> vec, size_type i) {
        return vec->data()[i];
    }

    /**
     * initialize from a pointer and a size
     */
    static
    void INIT(pointer<vector> vec, pointer<value_type> data, size_type size) {
        vec->data() = data;
        vec->size() = size;
    }

    /**
     * allocate memory for a vector
     */
    template <typename Allocator>
    static
    void ALLOCATE(pointer<vector> vec, size_type size, Allocator &allocator) {
        pointer<value_type> data = (pointer<value_type>)allocator.allocate(size);
        INIT(vec, data, size);
    }

    /**
     * deallocate memory for a vector
     */
    template <typename Allocator>
    static
    void DEALLOCATE(pointer<vector> vec, Allocator &allocator) {
        allocator.deallocate(vec->data(), vec->size());
    }

#ifdef RISCV
    /**
     * default allocator
     */
    struct allocator {
        pointer<value_type> allocate(size_type size) {
            return allocate_dram(size);
        }
        void deallocate(pointer<value_type> data, size_type size) {
            return deallocate_dram(data, size);
        }
    };

    /**
     * initialize from a pointer and a size
     */
    void init(pointer<value_type> data, size_type size) {
        INIT(this, data, size);
    }

    /**
     * allocate memory for a vector
     */
    void allocate(size_type size) {
        ALLOCATE(this, size, allocator{});
    }

    /**
     * deallocate memory for a vector
     */
    void deallocate() {
        DEALLOCATE(this, allocator{});
    }
    
    /**
     * Access element at index i
     */
    reference<value_type> at(size_type i) {
        return data()[i];
    }

    /**
     * Access element at index i
     */
    const_reference<value_type> at(size_type i) const {
        return data()[i];
    }

    /**
     * Access element at index i
     */
    reference<value_type> operator[](size_type i) {
        return at(i);
    }

    /**
     * Access element at index i
     */
    const_reference<value_type> operator[](size_type i) const {
        return at(i);
    }
#endif
};
}
#ifndef RISCV
namespace DrvAPI
{
template <typename T>
class value_handle<vector<T>> {
    typedef vector<T> vector_type;
    VH_DEFAULTS(vector_type);
    VH_FIELD(vector_type, data, _data);
    VH_FIELD(vector_type, size, _size);

    /**
     * Access element at index i
     */
    reference<T> at(size_t i) {
        return vector_type::AT(address(), i);
    }

    /**
     * Access element at index i
     */
    const_reference<T> at(size_t i) const {
        return vector_type::AT(address(), i);
    }

    /**
     * Access element at index i
     */
    reference<T> operator[](size_t i) {
        return at(i);
    }

    /**
     * Access element at index i
     */
    const_reference<T> operator[](size_t i) const {
        return at(i);
    }

    /**
     * initialize from a pointer and a size
     */
    void init(pointer<T> data, size_t size) {
        vector_type::INIT(address(), data, size);
    }

    struct allocator {
        pointer<T> allocate(size_t size) {
            return DrvAPI::allocate<T>(size);
        }
        void deallocate(pointer<T> data, size_t size) {
            DrvAPI::deallocate<T>(data, size);
        }
    };
};
} // namespace DrvAPI
}
#endif
#endif
