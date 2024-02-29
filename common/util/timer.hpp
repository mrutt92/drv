#ifndef TIMER_HPP
#define TIMER_HPP
#include <DrvAPI.hpp>
namespace util
{
struct timer {
    timer(const std::string &name) : name(name) {
        start = DrvAPI::seconds();
        flops_start = DrvAPI::float_type::Stats();
        double_start = DrvAPI::double_type::Stats();
        DrvAPI::outputStatistics(name+"_start");
    }

    ~timer() {
        stop = DrvAPI::seconds();
        printf("%20s: Elapsed time: %2.9lf seconds\n", name.c_str(), stop - start);
        flops_stop = DrvAPI::float_type::Stats();
        double_stop = DrvAPI::double_type::Stats();
        printf("%20s: fadd: %d\n", name.c_str(),
               (flops_stop.num_add+double_stop.num_add)-(flops_start.num_add+double_start.num_add));
        printf("%20s: fsub: %d\n", name.c_str(),
                (flops_stop.num_sub+double_stop.num_sub)-(flops_start.num_sub+double_start.num_sub));
        printf("%20s: fmul: %d\n", name.c_str(),
                (flops_stop.num_mul+double_stop.num_mul)-(flops_start.num_mul+double_start.num_mul));
        printf("%20s: fdiv: %d\n", name.c_str(),
                (flops_stop.num_div+double_stop.num_div)-(flops_start.num_div+double_start.num_div));
        printf("%20s: fmadd: %d\n", name.c_str(),
                (flops_stop.num_muladd+double_stop.num_muladd)-(flops_start.num_muladd+double_start.num_muladd));
        fflush(stdout);
        DrvAPI::outputStatistics(name+"_stop");
    }
    DrvAPI::numeric_stats flops_start;
    DrvAPI::numeric_stats flops_stop;
    DrvAPI::numeric_stats double_start;
    DrvAPI::numeric_stats double_stop;
    std::string name;
    double start;
    double stop;
};
}
#endif
