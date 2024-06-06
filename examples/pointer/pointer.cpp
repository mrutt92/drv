// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include <DrvAPI.hpp>
#include <stdint.h>
#include <inttypes.h>

#define pr(fmt, ...)                                    \
    do {                                                \
        printf("Core %4d: Thread %4d: " fmt "",         \
               DrvAPIThread::current()->coreId(),       \
               DrvAPIThread::current()->threadId(),     \
               ##__VA_ARGS__);                          \
    } while (0)

struct foo {
    int   baz;
    float bar;
    static void copy(foo &dst, const DrvAPI::value_handle<foo>&src);
    static void copy(DrvAPI::value_handle<foo>&dst, const foo&src);
};

struct bar {
    int   obaz;
    float obar;
    float sum() const { return obaz + obar; }
    static void copy(bar &dst, const DrvAPI::value_handle<bar>& src);
    static void copy(DrvAPI::value_handle<bar> &dst, const bar &src);
};

DRV_API_VALUE_HANDLE_BEGIN(bar)
DRV_API_VALUE_HANDLE_FIELD(bar,obaz,int,obaz)
DRV_API_VALUE_HANDLE_FIELD(bar,obar,float,obar)
float sum() const { return obar() + obaz(); }
DRV_API_VALUE_HANDLE_END(bar)
using bar_ref = DrvAPI::value_handle<bar>;

DRV_API_VALUE_HANDLE_BEGIN(foo)
DRV_API_VALUE_HANDLE_FIELD(foo,baz,int,baz)
DRV_API_VALUE_HANDLE_FIELD(foo,bar,float,bar)
DRV_API_VALUE_HANDLE_END(foo)
using foo_ref = DrvAPI::value_handle<foo>;

void foo::copy(foo &dst, const DrvAPI::value_handle<foo>&src) {
  dst.baz = src.baz();
  dst.bar = src.bar();
}
void foo::copy(DrvAPI::value_handle<foo>&dst, const foo&src) {
  dst.baz() = src.baz;
  dst.bar() = src.bar;
}

void bar::copy(bar &dst, const DrvAPI::value_handle<bar>& src) {
  dst.obaz = src.obaz();
  dst.obar = src.obar();
}

void bar::copy(DrvAPI::value_handle<bar> &dst, const bar &src) {
  dst.obaz() = src.obaz;
  dst.obar() = src.obar;
}

int PointerMain(int argc, char* argv[])
{
    using namespace DrvAPI;
    if (DrvAPIThread::current()->threadId() == 0 &&
        DrvAPIThread::current()->coreId() == 0) {
        pr("%s\n", __PRETTY_FUNCTION__);
        DrvAPIPointer<uint64_t> DRAM_BASE = DrvAPI::DrvAPIVAddress::MyL2Base().encode();
        *DRAM_BASE = 0x55;
        pr(" DRAM_BASE    = 0x%016" PRIx64 "\n", static_cast<uint64_t>(DRAM_BASE));
        pr("&DRAM_BASE[4] = 0x%016" PRIx64 "\n", static_cast<uint64_t>(&DRAM_BASE[4]));
        pr(" DRAM_BASE[0] = 0x%016" PRIx64 "\n", static_cast<uint64_t>(DRAM_BASE[0]));
        // foo_ref fptr = foo_ref(0x80000000ull);
        // fptr.baz() = 7;
        // fptr.bar() = 3.14159f;
        // pr("fptr.baz() = %d\n", static_cast<int>(fptr.baz()));
        // pr("fptr.bar() = %f\n", static_cast<float>(fptr.bar()));
        DrvAPIPointer<bar> bptr(0x80000000ull);
        bar_ref bref = bptr[0];
        bref.obaz() = 7;
        bref.obar() = 3.14159f;
        pr("bref.obaz() = %d\n", static_cast<int>(bref.obaz()));
        pr("bref.obar() = %f\n", static_cast<float>(bref.obar()));
        pr("bref.sum()  = %f\n", bref.sum());

        bptr->obaz() = 42;
        bptr->obar() = 2.71828f;
        pr("bptr->obaz() = %d\n", static_cast<int>(bptr->obaz()));
        pr("bptr->obar() = %f\n", static_cast<float>(bptr->obar()));
        pr("bptr->sum()  = %f\n", bptr->sum());

        // void pointer
        DrvAPIPointer<void> voidptr = DrvAPI::DrvAPIVAddress::MyL2Base().encode();
        pr("voidptr = 0x%016" PRIx64 "\n", static_cast<uint64_t>(voidptr));
    }
    return 0;
}

declare_drv_api_main(PointerMain);
