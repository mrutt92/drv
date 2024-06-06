#include <cello_core_drvr_commandprocessor.hpp>
#include "cello_jaccard.hpp"
#include "read_graph.hpp"
#include "transpose_graph.hpp"

DrvAPI::dram_static<sparse_type> the_csr;
DrvAPI::dram_static<matrix_type> the_matrix;

class cello_jaccard_app : public cello_command_processor_app {
public:
    cello_jaccard_app(int argc, char *argv[]);
    virtual ~cello_jaccard_app() {}
    virtual void input_application_data() override;
    virtual void output_application_data() override;
    pointer<jaccard_config> cfg;
    std::vector<idx_type> fwd_offsets, fwd_edges;
    std::string graph_file;
};

cello_jaccard_app::cello_jaccard_app(int argc, char *argv[])
    : cello_command_processor_app(argc, argv)
{
    graph_file = argv[2];
}

void cello_jaccard_app::input_application_data()
{
    
    cfg = exe_.symbol<jaccard_config>
        ("jaccard_cfg", pandocommand::Place{});
    idx_type V, E;
    read_graph(graph_file, &V, &E,
               fwd_offsets, fwd_edges);
    the_csr.init(V, E, fwd_offsets, fwd_edges);
    the_matrix.init(V, V);
    cfg->csr() = the_csr.address();
    cfg->matrix() = the_matrix.address();
}

void cello_jaccard_app::output_application_data()
{
}

cello_command_processor_app*
MakeApp(int argc, char *argv[])
{
    return new cello_jaccard_app(argc, argv);
}
