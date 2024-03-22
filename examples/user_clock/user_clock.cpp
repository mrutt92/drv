#include <DrvAPI.hpp>

using namespace DrvAPI;

dram_static<int> counter;

int Main(int argc, char *argv[])
{
    counter = 0;
    pointer<int> counter_ptr = counter.address();
    registerUserClock("50MHz", [counter_ptr]() mutable {
        std::cout << "counter is " << *(counter_ptr.to_native()) << std::endl;
        return false;
    });
    for (int i = 0; i < 10; i++) {
        counter = counter + 1;
    }
    return 0;
}

declare_drv_api_main(Main);
