// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington
#include <DrvAPI.hpp>
#include <cstdio>
#include <inttypes.h>
#include <vector>

#define pr_info(fmt, ...)                                               \
    do {                                                                \
        printf("PXN %3d: POD: %3d: CORE %3d: " fmt ""                   \
               ,myPXNId()                                               \
               ,myPodId()                                               \
               ,myCoreId()                                              \
               ,##__VA_ARGS__);                                         \
    } while (0)


struct id_type {
    int64_t pxn_;
    int64_t pod_;
    int64_t core_;
    int64_t thread_;

    int64_t &pxn() { return pxn_; }
    int64_t &pod() { return pod_; }
    int64_t &core() { return core_; }
    int64_t &thread() { return thread_; }

    const int64_t &pxn() const { return pxn_; }
    const int64_t &pod() const { return pod_; }
    const int64_t &core() const { return core_; }
    const int64_t &thread() const { return thread_; }

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
        dst.pxn() = src.pxn();
	dst.pod() = src.pod();
	dst.core() = src.core();
	dst.thread() = src.thread();
    }
};

DRV_API_VALUE_HANDLE_BEGIN(id_type)
DRV_API_VALUE_HANDLE_FIELD(id_type, pxn, int64_t, pxn_)
DRV_API_VALUE_HANDLE_FIELD(id_type, pod, int64_t, pod_)
DRV_API_VALUE_HANDLE_FIELD(id_type, core, int64_t, core_)
DRV_API_VALUE_HANDLE_FIELD(id_type, thread, int64_t, thread_)
DRV_API_VALUE_HANDLE_END(id_type)
using id_type_ref = DrvAPI::value_handle<id_type>;

int ToAddressMain(int argc, char *argv[])
{
    using namespace DrvAPI;
    struct id_type id;
    DrvAPIAddress addr = 0;
    std::size_t size = 0;
    DrvAPINativeToAddress(&id, &addr, &size);

    id_type_ref id_ref = *DrvAPIPointer<id_type>(addr);

    id_ref.pxn() = myPXNId();
    id_ref.pod() = myPodId();
    id_ref.core() = myCoreId();
    id_ref.thread() = myThreadId();

    id_type *native = nullptr;
    size_t _;
    DrvAPIAddressToNative(&id_ref, (void**)&native, &_);
    if (native != &id) {
        pr_info("FAIL: AddressToNative(NativeToAddress(&id)) != &id\n");
    } else if (id.pxn() != myPXNId() ||
	       id.pod() != myPodId() ||
	       id.core() != myCoreId() ||
	       id.thread() != myThreadId()) {
        pr_info("FAIL: id fields don't match mine\n");
    } else {
        pr_info("PASS: all checks succeeded \n");
    }

    return 0;
}

declare_drv_api_main(ToAddressMain);
