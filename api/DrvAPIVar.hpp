// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#ifndef DRV_API_VAR_H
#define DRV_API_VAR_H
#include <DrvAPIPointer.hpp>
#include <DrvAPIInfo.hpp>
#include <DrvAPINativeToAddress.hpp>
#include <atomic>
#include <cassert>
#include <cstdlib>
namespace DrvAPI
{
/**
 * @brief A wrapper around a type
 */
template <typename T>
class DrvAPIVar {
public:
    DrvAPIVar() : value_() {
        std::size_t _;
        DrvAPINativeToAddress(&value_, &address_, &_);
    }
    DrvAPIVar(T value) : DrvAPIVar() {
        *pointer() = value; 
    }
    DrvAPIVar(const DrvAPIVar<T>& other) : DrvAPIVar() {
        *pointer() = *other.pointer();
    }
    DrvAPIVar(DrvAPIVar<T>&& other) : DrvAPIVar() {
        *pointer() = *other.pointer();
    }
    DrvAPIVar<T>& operator=(const DrvAPIVar<T>& other) {
        *pointer() = *other.pointer();
        return *this;
    }
    DrvAPIVar<T>& operator=(DrvAPIVar<T>&& other) {
        *pointer() = *other.pointer();
        return *this;
    }

    // assignment operator
    DrvAPIVar<T>& operator=(T value) {
        *pointer() = value;
        return *this;
    }

    // read operator
    operator T() const {
        return *pointer();
    }
    
    DrvAPIPointer<T> pointer() const {
        return DrvAPIPointer<T>(address());
    }

    DrvAPIAddress address() const {
        return address_;
    }

    T value_;
    DrvAPIAddress address_;
};

}
#endif
