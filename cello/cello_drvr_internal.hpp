#ifndef CELLO_DRVR_INTERNAL_HPP
#define CELLO_DRVR_INTERNAL_HPP
#define FIELD(type, field, field_data)                  \
private:                                                \
 type field_data;                                       \
public:                                                 \
 type & field() { return field_data; }                  \
 const type & field() const { return field_data; }

#endif
