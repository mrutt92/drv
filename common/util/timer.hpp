#ifndef TIMER_HPP
#define TIMER_HPP
#include <DrvAPI.hpp>
namespace util
{
struct timer {
    timer(const std::string &name) : name(name) {
        start = DrvAPI::seconds();
    }

    ~timer() {
        stop = DrvAPI::seconds();
        printf("%20s: Elapsed time: %2.9lf seconds\n", name.c_str(), stop - start);
    }

    std::string name;
    double start;
    double stop;
};
}
#endif
