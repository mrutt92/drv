// SPDX-License-Identifier: MIT
// Copyright (c) 2023 University of Washington

#include <DrvAPI.hpp>
#include <inttypes.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
using namespace DrvAPI;

struct foo {
    int a_;
    int b_;    
    const int & a() const { return a_; }
    const int & b() const { return b_; }
    int & a() { return a_; }
    int & b() { return b_; }

    template <typename Dst, typename Src>
    static void copy(Dst &dst, const Src &src) {
      dst.a() = src.a();
      dst.b() = src.b();
    }
};

DRV_API_VALUE_HANDLE_BEGIN(foo)
DRV_API_VALUE_HANDLE_FIELD(foo, a, int, a_)
DRV_API_VALUE_HANDLE_FIELD(foo, b, int, b_)
DRV_API_VALUE_HANDLE_END(foo)

DrvAPIGlobalL2SP<int> i;
DrvAPIGlobalL2SP<foo> f;
DrvAPIGlobalL2SP<DrvAPIPointer<int>> pi;

int AllocatorMain(int argc, char *argv[])
{
    std::stringstream ss;
    using namespace DrvAPI;
    DrvAPIMemoryAllocatorInit();
    std::vector<std::tuple<DrvAPIMemoryType, std::string, DrvAPIAddress>> tests = {
        {DrvAPIMemoryL1SP, "L1SP", 0x1000},
        {DrvAPIMemoryL2SP, "L2SP", 0x1000},
        {DrvAPIMemoryDRAM, "DRAM", 0x1000},
        {DrvAPIMemoryL1SP, "L1SP", sizeof(uint64_t)},
        {DrvAPIMemoryL2SP, "L2SP", sizeof(uint64_t)},
        {DrvAPIMemoryDRAM, "DRAM", sizeof(uint64_t)},
        {DrvAPIMemoryL1SP, "L1SP", 2*sizeof(uint64_t)},
        {DrvAPIMemoryL2SP, "L2SP", 2*sizeof(uint64_t)},
        {DrvAPIMemoryDRAM, "DRAM", 2*sizeof(uint64_t)},
    };

    for (auto &t: tests) {
	DrvAPIMemoryType type;
	std::string type_name;
        DrvAPIAddress size;
	std::tie(type, type_name, size) = t;
        DrvAPIPointer<int> p0 = DrvAPIMemoryAlloc(type, size);
        DrvAPIPointer<int> p1 = DrvAPIMemoryAlloc(type, size);
        ss << "Core " << myCoreId() << " Thread " << myThreadId() <<":";
        ss << "p0 = " << DrvAPIVAddress{p0}.to_string() << " should be " << type_name << std::endl;
        ss << "Core " << myCoreId() << " Thread " << myThreadId() <<":";
        ss << "p1 = " << DrvAPIVAddress{p1}.to_string() << " should be " << type_name << std::endl;
        ss << "Core " << myCoreId() << " Thread " << myThreadId() <<":";
        ss << "p0 = 0x" << std::hex << p0 << std::endl;
        ss << "Core " << myCoreId() << " Thread " << myThreadId() <<":";
        ss << "p1 = 0x" << std::hex << p1 << std::endl;
        DrvAPIMemoryFree(p0, size);
        DrvAPIMemoryFree(p1, size);
        
    }
    f.a() = 1;
    f.b() = 2;
    ss << "Core " << myCoreId() << " Thread " << myThreadId() <<":";
    ss << "&f = 0x" << std::hex << &f << std::endl;
    ss << "Core " << myCoreId() << " Thread " << myThreadId() <<":";
    ss << "f.a = " << f.a() << std::endl;
    pi[0] = 1;
    int x = pi[0];
    ss << "Core " << myCoreId() << " Thread " << myThreadId() <<":";
    ss << "pi[0] = " << x << std::endl;
    std::cout << ss.str();
    return 0;
}
declare_drv_api_main(AllocatorMain);
