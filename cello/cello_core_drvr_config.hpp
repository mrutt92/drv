#ifndef CELLO_CORE_DRVR_CONFIG_HPP
#define CELLO_CORE_DRVR_CONFIG_HPP
#include <cstdint>

// cello configuration
#ifndef FIELD
#define FIELD(type, field, field_data)          \
    type field_data;                            \
    type& field() { return field_data; }        \
    const type& field() const { return field_data; }
#endif

/**
 * @brief cello configuration
 */
struct cello_config {
public:
    FIELD(int64_t, allocator_base, allocator_base_);
    FIELD(int64_t, allocator_size, allocator_size_);
    FIELD(int64_t, main_returned, main_returned_);
    
    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.allocator_base() = src.allocator_base();
        dst.allocator_size() = src.allocator_size();
        dst.main_returned() = src.main_returned();
    }
};

extern "C" cello_config cello_configuration;
#undef FIELD
#endif
