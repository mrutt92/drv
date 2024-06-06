// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#ifndef DRV_API_NUMERIC_H
#define DRV_API_NUMERIC_H
#include <atomic>
#include <string>
namespace DrvAPI
{

/**
 * @brief stats structure for a numeric type
 */
struct numeric_stats {
    std::atomic<int> num_mul;
    std::atomic<int> num_div;
    std::atomic<int> num_add;
    std::atomic<int> num_sub;
    std::atomic<int> num_muladd;

    numeric_stats() : num_mul(0), num_div(0), num_add(0), num_sub(0), num_muladd(0) {}
    numeric_stats(const numeric_stats &o) = delete;
    numeric_stats(numeric_stats &&o) = delete;
    numeric_stats &operator=(const numeric_stats &o) {
        num_mul = o.num_mul.load();
        num_div = o.num_div.load();
        num_add = o.num_add.load();
        num_sub = o.num_sub.load();
        num_muladd = o.num_muladd.load();
        return *this;
    }
    numeric_stats &operator=(numeric_stats &&o) = delete;
    ~numeric_stats() = default;
    
    std::string to_string() const {
        return "num_mul: " + std::to_string(num_mul)
            + " num_div: " + std::to_string(num_div)
            + " num_add: " + std::to_string(num_add)
            + " num_sub: " + std::to_string(num_sub)
            + " num_muladd: " + std::to_string(num_muladd);
    }
};


/**
 * @brief the numeric type wrapper class
 */
template <typename data_type>
class numeric_type {
public:
    typedef data_type underlying_type;
    numeric_type(underlying_type value) : value(value) {}
    numeric_type() : value(0) {}
    numeric_type(const numeric_type &o) = default;
    numeric_type(numeric_type &&o) = default;
    numeric_type &operator=(const numeric_type &o) = default;
    numeric_type &operator=(numeric_type &&o) = default;
    ~numeric_type() = default;

    explicit operator underlying_type() const {
        return value;
    }

    numeric_type& operator+=(numeric_type &&o) {
        value += o.value;
        Stats().num_add++;
        return *this;
    }

    static inline numeric_stats & Stats() {
        static numeric_stats stats;
        return stats;
    }

    underlying_type value;
};

/**
 * @brief specialization of value_handles for numeric types
 *
 * this lets us have pointer<numeric_type>, static_data<numeric_type>, and dynamic_data<numeric_type>
 * and also be able to cast directly from a handle to the underlying type of the numeric
 */
template <typename T>
class value_handle<numeric_type<T>> {
public:
    DRV_API_VALUE_HANDLE_DEFAULTS_TRIVIAL(numeric_type<T>)

    explicit operator typename numeric_type<T>::underlying_type () const {
        return static_cast<typename numeric_type<T>::underlying_type>
            (  static_cast<numeric_type<T>>
               ( *this )
            );

    }

    value_handle &operator=(typename numeric_type<T>::underlying_type value) {
        *this = numeric_type<T>(value);
        return *this;
    }
};

#define DRV_API_NUMERIC_TYPE_ADD(type)                                  \
    inline numeric_type<type> operator+(const numeric_type<type> &a, type b) { \
        numeric_type<type>::Stats().num_add++;                          \
        return ((type)a) + ((type)b);                                   \
    }                                                                   \
    inline numeric_type<type> operator+(type a, const numeric_type<type> &b) { \
        numeric_type<type>::Stats().num_add++;                          \
        return ((type)a) + ((type)b);                                   \
    }                                                                   \
    inline numeric_type<type> operator+(const numeric_type<type> &a, const numeric_type<type> &b) { \
        numeric_type<type>::Stats().num_add++;                          \
        return ((type)a) + ((type)b);                                   \
    }                                                                   \

#define DRV_API_NUMERIC_TYPE_SUB(type)                                  \
    template <typename T>                                               \
    inline numeric_type<type> operator-(const numeric_type<type> &a, T b) { \
        numeric_type<type>::Stats().num_sub++;                          \
        return ((type)a) - ((type)b);                                   \
    }                                                                   \
    inline numeric_type<type> operator-(type a, const numeric_type<type> &b) { \
        numeric_type<type>::Stats().num_sub++;                          \
        return ((type)a) - ((type)b);                                   \
    }                                                                   \
    inline numeric_type<type> operator-(const numeric_type<type> &a, const numeric_type<type> &b) { \
        numeric_type<type>::Stats().num_sub++;                          \
        return ((type)a) - ((type)b);                                   \
    }                                                                   \

#define DRV_API_NUMERIC_TYPE_MUL(type)                                  \
    inline numeric_type<type> operator*(const numeric_type<type> &a,  type b) { \
        numeric_type<type>::Stats().num_mul++;                          \
        return ((type)a) * ((type)b);                                   \
    }                                                                   \
    inline numeric_type<type> operator*(type a, const numeric_type<type> &b) { \
        numeric_type<type>::Stats().num_mul++;                          \
        return ((type)a) * ((type)b);                                   \
    }                                                                   \
    inline numeric_type<type> operator*(const numeric_type<type> &a, const numeric_type<type> &b) { \
        numeric_type<type>::Stats().num_mul++;                          \
        return ((type)a) * ((type)b);                                   \
    }                                                                   \

#define DRV_API_NUMERIC_TYPE_DIV(type)                  \
    inline numeric_type<type> operator/(const numeric_type<type> &a, type b) { \
        numeric_type<type>::Stats().num_div++;                          \
        return ((type)a) / ((type)b);                                   \
    }                                                                   \
    inline numeric_type<type> operator/(type a, const numeric_type<type> &b) { \
        numeric_type<type>::Stats().num_div++;                          \
        return ((type)a) / ((type)b);                                   \
    }                                                                   \
    inline numeric_type<type> operator/(const numeric_type<type> &a, const numeric_type<type> &b) { \
        numeric_type<type>::Stats().num_div++;                          \
        return ((type)a) / ((type)b);                                   \
    }                                                                   \

#define DRV_API_NUMERIC_TYPE_MULADD(type)                               \
    inline numeric_type<type> muladd(const numeric_type<type> &a, const numeric_type<type> &b, const numeric_type<type> &c) { \
        using underlying_type = numeric_type<type>::underlying_type;    \
        underlying_type a_u = (type)a, b_u = (type)b, c_u = (type)c;    \
        numeric_type<type>::Stats().num_muladd++;                       \
        return a_u*b_u + c_u;                                           \
    }

#define DRV_API_NUMERIC_TYPE_OPS(type)                  \
    DRV_API_NUMERIC_TYPE_ADD(type)                      \
    DRV_API_NUMERIC_TYPE_SUB(type)                      \
    DRV_API_NUMERIC_TYPE_MUL(type)                      \
    DRV_API_NUMERIC_TYPE_DIV(type)

typedef numeric_type<float>  float_type;
typedef numeric_type<double> double_type;

DRV_API_NUMERIC_TYPE_OPS(float)
DRV_API_NUMERIC_TYPE_MULADD(float)

DRV_API_NUMERIC_TYPE_OPS(double)
DRV_API_NUMERIC_TYPE_MULADD(double)

}

#endif
