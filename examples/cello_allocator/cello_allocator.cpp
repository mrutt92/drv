#include <iostream>
#include <iomanip>
#include <sstream>
#include <cello.hpp>
#include <cello_core_drvx_allocator.hpp>

using namespace cello;
using namespace DrvAPI;

std::string to_string(pointer<void> ptr)
{
    return DrvAPI::DrvAPIVAddress{ptr}.to_string();
}

int CelloMain(int argc, char *argv[])
{
    allocator_init();
    {
        pointer<void> p0 = allocate(1024);
        pointer<void> p1 = allocate(1024);
        std::cout << "p0: " << to_string(p0) << std::endl;
        std::cout << "p1: " << to_string(p1) << std::endl;
        deallocate(p0, 1024);
        deallocate(p1, 1024);
        pointer<void> p2 = allocate(1024);
        std::cout << "p2: " << to_string(p2) << std::endl;
        deallocate(p2, 1024);
    }
    {
        pointer<void> p0 = allocate(1024);
        pointer<void> p1 = allocate(1024);
        pointer<void> p2 = allocate(1024);
        std::cout << "p0: " << to_string(p0) << std::endl;
        std::cout << "p1: " << to_string(p1) << std::endl;
        std::cout << "p2: " << to_string(p2) << std::endl;
        deallocate(p0, 1024); // case 1
        deallocate(p1, 1024); // case 2
        deallocate(p2, 1024); // case 4 (pred + succ free)
    }
    {
        pointer<void> p0 = allocate(3*4096);
        pointer<void> p1 = allocate(32);
        std::cout << "p0: " << to_string(p0) << std::endl;
        std::cout << "p1: " << to_string(p1) << std::endl;
        deallocate(p0, 3*4096);
        deallocate(p1, 32);
    }
    {
        pointer<void> p0 = allocate(3*4096);
        pointer<void> p1 = allocate(32);
        std::cout << "p0: " << to_string(p0) << std::endl;
        std::cout << "p1: " << to_string(p1) << std::endl;
        deallocate(p0, 3*4096);
        deallocate(p1, 32);
    }
    return 0;
}
