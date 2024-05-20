#include <cello_core_drvr_commandprocessor.hpp>
#include <DrvAPI.hpp>
#include <Eigen/Dense>
#include "cello_gemm.hpp"

DrvAPI::dram_static<matrix_type_A> A;
DrvAPI::dram_static<matrix_type_B> B;
DrvAPI::dram_static<matrix_type_C> C;

template <typename MatrixType>
struct populator {
    populator(MatrixType& matrix) : matrix_(matrix) {}
    float operator()(idx_type i, idx_type j) {
        return matrix_(i, j);
    }
    MatrixType& matrix_;
};

template <typename MatrixType>
populator<MatrixType> make_populator(MatrixType& matrix) {
    return populator<MatrixType>(matrix);
}

class gemm_app : public cello_command_processor_app {
public:
    gemm_app(int argc, char *argv[]);
    virtual ~gemm_app() {}
    virtual void input_application_data() override;
    virtual void output_application_data() override;
    idx_type M, N, K;
    typedef Eigen::MatrixXf ref_matrix_type;
    ref_matrix_type A_ref;
    ref_matrix_type B_ref;
    ref_matrix_type C_ref;
    pointer<gemm_config> cfg;
};

gemm_app::gemm_app(int argc, char *argv[])
    : cello_command_processor_app(argc, argv)
{
    M = GEMM_M;
    N = GEMM_N;
    K = GEMM_K;
    A_ref = ref_matrix_type::Random(M, N);
    B_ref = ref_matrix_type::Random(N, K);
    C_ref = ref_matrix_type::Zero(M, K);
    std::cout << "M = " << M << ", N = " << N << ", K = " << K << std::endl;
}

void gemm_app::input_application_data()
{
    cfg = exe_.symbol<gemm_config>
        ("gemm_cfg", pandocommand::Place{});
    A.populate(M, N, make_populator(A_ref));
    B.populate(N, K, make_populator(B_ref));
    cfg->A() = A.address();
    cfg->B() = B.address();
    cfg->C() = C.address();
}

void gemm_app::output_application_data()
{
    C_ref = A_ref * B_ref;
    C.foreach([this](idx_type i, idx_type j) {
        float ref = C_ref(i, j);
        float res = C(i, j);
        if (std::abs(ref - res) > 1e-5) {
            printf("C_ref(%d, %d) = %+2.6f, C(%d, %d) = %+2.6f\n", i, j, ref, i, j, res);
        }
    });
}

cello_command_processor_app* MakeApp(int argc, char *argv[])
{
    std::cout << __FILE__ << ": MakeApp" << std::endl;
    return new gemm_app(argc, argv);
}
