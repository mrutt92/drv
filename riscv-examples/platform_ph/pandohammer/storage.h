#ifndef PANDOHAMMER_STORAGE_H
#define PANDOHAMMER_STORAGE_H

#define l1sp_storage(type)                      \
    __attribute__((section(".dmem"))) type

#define l2sp_storage(type)                      \
    __attribute__((section(".dram"))) type

#endif
