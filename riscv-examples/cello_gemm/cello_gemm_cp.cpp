#include <cello_core_drvr_commandprocessor.hpp>
#include <DrvAPI.hpp>
#include "cello_gemm.hpp"

DrvAPI::dram_static<matrix_type_A> A;
DrvAPI::dram_static<matrix_type_B> B;
DrvAPI::dram_static<matrix_type_C> C;

class gemm_app : public cello_command_processor_app {
public:
    gemm_app(int argc, char *argv[]);
    virtual ~gemm_app() {}
    virtual void input_application_data() override;
    virtual void output_application_data() override;
    idx_type M, N, K;
    pointer<gemm_config> cfg;
};

gemm_app::gemm_app(int argc, char *argv[])
    : cello_command_processor_app(argc, argv)
{
    M = std::atoi(argv[2]);
    N = std::atoi(argv[3]);
    K = std::atoi(argv[4]);

    std::cout << "M = " << M << ", N = " << N << ", K = " << K << std::endl;
}

void gemm_app::input_application_data()
{
    cfg = exe_.symbol<gemm_config>
        ("gemm_cfg", pandocommand::Place{});
    A.populate(M, N);
    B.populate(N, K);
    cfg->A() = A.address();
    cfg->B() = B.address();
    cfg->C() = C.address();
}

void gemm_app::output_application_data()
{
}

cello_command_processor_app* MakeApp(int argc, char *argv[])
{
    std::cout << __FILE__ << ": MakeApp" << std::endl;
    return new gemm_app(argc, argv);
}
