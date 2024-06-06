#include <Eigen/Dense>
#include <cello_core_drvr_commandprocessor.hpp>
#include "cello_sort.hpp"

DrvAPI::dram_static<vector_type> the_vector;

class sort_app : public cello_command_processor_app {
public:
    sort_app(int argc, char *argv[]);
    virtual ~sort_app() {}
    virtual void input_application_data() override;
    virtual void output_application_data() override;

    Eigen::VectorXi  ref_vec;
    pointer<sort_config> cfg;
    idx_type n;    
};

/**
 * This populator is used to fill the vector with values
 */
struct populator {
    /**
     * The value type of the vector
     */
    populator(Eigen::VectorXi& vec)
        : vec(vec) {
    }

    /**
     * The operator that fills the vector
     */
    value_type operator()(idx_type i) {
        return vec(i);
    }

    Eigen::VectorXi& vec;
};

sort_app::sort_app(int argc, char *argv[])
    : cello_command_processor_app(argc, argv)
{
    n = std::atoi(argv[2]);
}

void sort_app::input_application_data()
{
    cfg = exe_.symbol<sort_config>
        ("sort_cfg", pandocommand::Place{});

    ref_vec = Eigen::VectorXi::Random(n);
    std::cout << "Populating vector with " << n << " elements" << std::endl;
    the_vector.populate(n, populator{ref_vec});
    cfg->vec() = the_vector.address();
}

void sort_app::output_application_data()
{
    std::sort(ref_vec.data(), ref_vec.data() + ref_vec.size());
#if 1
    for (idx_type i = 0; i < ref_vec.size(); ++i) {
        if (the_vector[i] != ref_vec(i)) {
            std::cerr << "Error: "
                      << "res[" << i << "] (=" << the_vector[i] << ")"
                      << " != "
                      << "ref(" << i << ") (=" << ref_vec(i) << ")" << std::endl;
        }
    }
#endif
}

cello_command_processor_app*
MakeApp(int argc, char *argv[])
{
    return new sort_app(argc, argv);
}
