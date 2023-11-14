// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#ifndef RISCVHART_HPP
#define RISCVHART_HPP
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
class RISCVHart {
public:
    using InternalFPType = double;

    uint64_t        _x[32];
    InternalFPType  _f[32];
    uint64_t        _pc;
    /**
     * mutable reference
     */
    template <typename ScalarT>
    class reference_handle {
    public:
        reference_handle(ScalarT &ref, bool zero = false)
            : _ref(ref)
            , _zero(zero) {
        }
        operator ScalarT() const { return _zero ? static_cast<ScalarT>(0) : _ref; }

        reference_handle &operator=(ScalarT val) {
            if (!_zero) _ref = val;
            return *this;
        }

        reference_handle &operator+=(ScalarT val) {
            if (!_zero) _ref += val;
            return *this;
        }

        ScalarT &_ref;
        bool _zero;
    };

    /**
     * const reference
     */
    template <typename ScalarT>
    class const_reference_handle {
    public:
        const_reference_handle(const ScalarT &ref, bool zero = false)
            : _ref(ref)
            , _zero(zero) {
        }
        operator ScalarT() const { return _zero ? static_cast<ScalarT>(0) : _ref; }
        const ScalarT &_ref;
        bool _zero;
    };

    /**
     * Mutable reference specialization for floating point types
     */
    class reference_handle<float> {
        refernce_handle(InternalFPType &ref)
            : _ref(ref) {
        }

        operator float() const { return static_cast<float>(_ref); }

        reference_handle &operator=(float val) {
            _ref = static_cast<InternalFPType>(val);
            return *this;
        }

        InternalFPType &_ref;
    };

    /**
     * Const reference specialization for floating point types
     */
    class const_reference_handle<float> {
        const_reference_handle(const InternalFPType &ref)
            : _ref(ref) {
        }

        operator float() const { return static_cast<float>(_ref); }

        const InternalFPType &_ref;
    };
    
    template <typename IdxT>
    reference_handle<uint64_t> x(IdxT i) {
        return reference_handle<uint64_t>(_x[i], i == 0);
    }

    template <typename IdxT>
    const const_reference_handle<uint64_t> x(IdxT i) const {
        return const_reference_handle<uint64_t>(_x[i], i == 0);
    }

    template <typename IdxT>
    reference_handle<int64_t> sx(IdxT i) {
        return reference_handle<int64_t>(reinterpret_cast<int64_t &>(_x[i]), i == 0);
    }

    template <typename IdxT>
    const const_reference_handle<int64_t> sx(IdxT i) const {
        return const_reference_handle<int64_t>(reinterpret_cast<int64_t &>(_x[i]), i == 0);
    }

    template <typename IdxT>
    reference_handle<uint64_t> a(IdxT i) {
        assert(i < 8);
        return x(10 + i);
    }

    template <typename IdxT>
    const const_reference_handle<uint64_t> a(IdxT i) const {
        assert(i < 8);
        return x(10 + i);
    }

    template <typename IdxT>
    reference_handle<int64_t> sa(IdxT i) {
        assert(i < 8);
        return sx(10 + i);
    }

    template <typename IdxT>
    const const_reference_handle<int64_t> sa(IdxT i) const {
        assert(i < 8);
        return sx(10 + i);
    }


    template <typename IdxT>
    reference_handle<float> sf(IdxT i) {
        assert(i < 32);
        return reference_handle<float>(_f[i]);
    }

    template <typename IdxT>
    const const_reference_handle<float> sf(IdxT i) const {
        assert(i < 32);
        return const_reference_handle<float>(_f[i]);
    }
    
    reference_handle<uint64_t> pc() {
        return reference_handle<uint64_t>(_pc, false);
    }

    const const_reference_handle<uint64_t> pc() const {
        return const_reference_handle<uint64_t>(_pc, false);
    }

    reference_handle<uint64_t> sp() {
        return x(2);
    }

    const const_reference_handle<uint64_t> sp() const {
        return x(2);
    }
    
    std::string to_string() const {
        std::stringstream ss;
        ss << "pc: " << std::hex << pc() << std::endl;
        for (int i = 0; i < 32; i++) {
            ss << "x" << std::dec << i << ": " << std::hex << x(i) << std::endl;
        }
        return ss.str();
    }
};

#endif
