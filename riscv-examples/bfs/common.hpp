#pragma once
#ifdef COMMAND_PROCESSOR
#include <DrvAPI.hpp>
#else
#include <pandohammer/cpuinfo.h>
#include <pandohammer/atomic.h>
#endif
#include <cstdint>

/////////////////////////////////////
// Utility variables and functions //
/////////////////////////////////////
/**
 * Get the number of threads running
 */
#ifdef COMMAND_PROCESSOR
#define THREADS                                 \
    (DrvAPI::myCoreThreads() * DrvAPI::numPodCores())
#else
#define THREADS                                 \
    (myCoreThreads() * numPodCores())
#endif

/**
 * wait for x cycles
 */
#ifdef COMMAND_PROCESSOR
inline void wait(int x) {
    DrvAPI::wait(x);
}
#else
inline void wait(volatile int x) {
    for(int i  = 0; i < x; i++) {
        asm volatile("nop");
    }    
}
#endif

//////////////////////////////
// reference class wrappers //
//////////////////////////////
#ifdef COMMAND_PROCESSOR
#define REF_CLASS_BEGIN(type)                   \
    DRV_API_REF_CLASS_BEGIN(type)

#define REF_CLASS_DATA_MEMBER(type, member)     \
    DRV_API_REF_CLASS_DATA_MEMBER(type, member)
                                  
#define REF_CLASS_END(type)                     \
    DRV_API_REF_CLASS_END(type)
#else
#define REF_CLASS_BEGIN(type)                                   \
    class type##_ref {                                          \
    public:                                                     \
    type##_ref(type *ptr) : ptr_(ptr) {}                        \
    type##_ref() = delete;                                      \
    type##_ref(const type##_ref &other) = default;              \
    type##_ref(type##_ref &&other) = default;                   \
    type##_ref &operator=(const type##_ref &other) = default;   \
    type##_ref &operator=(type##_ref &&other) = default;        \
    ~type##_ref() = default;                                    \
    type* operator&() { return ptr_; }                          \
    type *ptr_;

#define REF_CLASS_DATA_MEMBER(type, member)             \
    decltype(std::declval<type>().member) &             \
    member() const { return ptr_->member; }

#define REF_CLASS_END(type)                     \
    };

#endif


///////////
// types //
///////////
using vertex_t = int32_t;

#ifdef COMMAND_PROCESSOR
using vertex_pointer_t = DrvAPI::DrvAPIPointer<vertex_t>;
#else
using vertex_pointer_t = vertex_t *;
#endif

/**
 * @brief The frontier_data struct
 * This struct is used to store frontier data.
 */
struct frontier_data {
    int64_t          size;
    vertex_pointer_t vertices;
    bool             is_dense;
};

REF_CLASS_BEGIN(frontier_data)
REF_CLASS_DATA_MEMBER(frontier_data, size)
REF_CLASS_DATA_MEMBER(frontier_data, vertices)
REF_CLASS_DATA_MEMBER(frontier_data, is_dense)
REF_CLASS_END(frontier_data)

using frontier_ref = frontier_data_ref;


