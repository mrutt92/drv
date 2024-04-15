#ifndef PANDOHAMMER_ADDRESSMAP_H
#define PANDOHAMMER_ADDRESSMAP_H
#include <stdint.h>
#include "pandohammer/bitrange_handle.hpp"

struct virtual_address {
    /**
     * generator for a bitfield handle
     */
#define BITFIELD(name, hi, lo)                                          \
    typedef bitrange_handle<uintptr_t, hi, lo> name##_handle_t;         \
    typedef bitrange_handle<const uintptr_t, hi, lo> name##_const_handle_t; \
    name##_handle_t name() { return name##_handle_t(address); }        \
    const name##_const_handle_t name() const { return name##_const_handle_t(address); } \
    virtual_address &set_##name(uintptr_t val) { name() = val; return *this; }
    
    BITFIELD(is_ctrl_register, 63, 63);
    BITFIELD(is_not_scratchpad, 47, 47);
    BITFIELD(pxn, 46, 33);
    BITFIELD(is_global, 32, 32);
    BITFIELD(pod, 31, 26);
    BITFIELD(is_l2_not_l1, 25, 25);
    BITFIELD(core_y, 22, 20);
    BITFIELD(core_x, 19, 17);
    BITFIELD(l1_offset, 16,  0);
    BITFIELD(l2_offset, 24,  0);
    BITFIELD(dram_offset_hi10, 57, 48);
    BITFIELD(dram_offset_lo33, 32,  0);
#undef BITFIELD

    /**
     * constructor
     */
    virtual_address(uintptr_t address) : address(address) {}

    /**
     * convenience constructor from a pointer of any type
     */
    template <typename T>
    virtual_address(T *ptr) : virtual_address(reinterpret_cast<uintptr_t>(ptr)) {}

    /**
     * return true if scratchpad
     */
    bool is_scratchpad() const {
        return !is_not_scratchpad();
    }
    
    /**
     * return true if address is dram
     */
    bool is_dram() const {
        return !is_ctrl_register() && is_not_scratchpad();
    }

    /**
     * return true if is l2
     */
    bool is_l2() const {
        return !is_ctrl_register() && is_scratchpad() && is_l2_not_l1();
    }

    /**
     * return true if is l1
     */
    bool is_l1() const {
        return !is_ctrl_register() && is_scratchpad() && !is_l2_not_l1();
    }

    /**
     * cast interchangeably with uintptr_t
     */
    operator uintptr_t() const {
        return address;
    }

    /**
     * cast interchangeably with a pointer of any type
     */
    template <typename T>
    operator T*() const {
        return reinterpret_cast<T*>(address);
    }

private:
    uintptr_t address;
};

#endif
