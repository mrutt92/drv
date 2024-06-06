#ifndef FIELD_HPP
#define FIELD_HPP
#include <utility>

#ifndef RISCV
#include <DrvAPI.hpp>
#endif

#ifdef FIELD
#undef FIELD
#endif
#define FIELD(type, field, field_data)          \
    type field_data;                            \
    type & field() { return field_data; }       \
    const type & field() const { return field_data; }

#ifndef RISCV
#define VH_DEFAULTS(val_type)                   \
    DRV_API_VALUE_HANDLE_DEFAULTS(val_type)

#define VH_FIELD(val_type, field, field_data)   \
    DRV_API_VALUE_HANDLE_FIELD                  \
    (val_type,                                                  \
        field,                                                  \
        decltype(std::declval<val_type>().field_data),          \
        field_data)
#endif
#endif // FIELD_HPP
