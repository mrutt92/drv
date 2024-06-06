#ifndef PANDOHAMMER_STORAGE_HPP
#define PANDOHAMMER_STORAGE_HPP
#include "pandohammer/internal_field.hpp"
#define SECTION_DRAM_SIZE \
    ((intptr_t*)DRAM_BASE_ADDR(myPXNId()))

template <typename T>
class dram_storage {
public:
    /**
     * data structure
     */
    struct ctrl {
        long init;
        T    data;
    };

    /**
     * constructor
     */
    dram_storage(const T& init) {
        if (myThreadId() == 0) {
            size_t align = std::alignment_of<ctrl>::value;
            intptr_t o = atomic_fetch_add(SECTION_DRAM_SIZE, align+sizeof(ctrl));
            offset_ = (o + align - 1) & ~(align - 1);
            new (&data()) T(init);
            ctrl.init = 1;
        }
    }

    dram_storage() : dram_storage(T{}) {}     
    dram_storage(const dram_storage& other) = delete;
    dram_storage(dram_storage&& other) = delete;
    dram_storage& operator=(const dram_storage& other) = delete;
    dram_storage& operator=(dram_storage&& other) = delete;
    ~dram_storage() {
        if (myThreadId() == 0) {
            (&data())->~T();
        }
    }

    /**
     * assignment to type T
     */
    dram_storage& operator=(const T& value) {
        if (myThreadId() == 0) {
            data() = value;
        }
        return *this;
    }

    /**
     * assignment to type T
     */
    dram_storage& operator=(T && value) {
        if (myThreadId() == 0) {
            (&ctrl().data)->~T();
            ctrl().data = std::move(value);
        }
        return *this;
    }

    /**
     * cast to type T reference
     */
    operator T&() {
        return data();
    }

private:
    /**
     *@brief pointer to the control structure
     */
    ctrl* ctrl_ptr() {
        return reinterpret_cast<ctrl*>(DRAM_BASE_ADDR(myPXNId()) + offset_);
    }
    /**
     *@brief pointer to the control structure
     */
    const ctrl* ctrl_ptr() const {
        return reinterpret_cast<ctrl*>(DRAM_BASE_ADDR(myPXNId()) + offset_);
    }
    /**
     *@brief reference to the control structure
     */
    ctrl& ctrl() {
        return *storage_ctrl_ptr();
    }
    /**
    *@brief reference to the control structure
    */
    const ctrl& ctrl() const {
        return *storage_ctrl_ptr();
    }

    /**
     *@brief reference to the data
     */
    T& data() {
        return ctrl().data;
    }

    /**
     *@brief const reference to the data
     */
    const T& data() const {
        return ctrl().data;
    }

    intptr_t offset_;
};

#endif
