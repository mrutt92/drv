#include <cello_core_drvr_commandprocessor.hpp>
#include "cello_tc.hpp"
#include "triangle_counting.hpp"
#include "read_graph.hpp"
DrvAPI::dram_static<sparse_type> the_csr;

class cello_tc_app : public cello_command_processor_app {
public:
    cello_tc_app(int argc, char *argv[]);
    virtual ~cello_tc_app() {}
    virtual void input_application_data() override;
    virtual void output_application_data() override;
    pointer<tc_configure> cfg;
    std::string graph_file;
    idx_type V, E;
    std::vector<idx_type> fwd_offsets, fwd_edges;
    std::set<tc::triangle> triangles_reference;
};

cello_tc_app::cello_tc_app(int argc, char *argv[])
    : cello_command_processor_app(argc, argv)
{
    graph_file = argv[2];
}

void cello_tc_app::input_application_data()
{

    cfg = exe_.symbol<tc_configure>
        ("tc_cfg", pandocommand::Place{});
    read_graph(graph_file, &V, &E,
               fwd_offsets, fwd_edges);
    the_csr.init(V, E, fwd_offsets, fwd_edges);
    cfg->csr() = the_csr.address();
}

void cello_tc_app::output_application_data()
{
    tc::triangle_counting(V, E, fwd_offsets, fwd_edges, triangles_reference);    
    if ((idx_type)triangles_reference.size() != cfg->triangles()) {
        std::cerr << "Error: triangle count mismatch: ";
        std::cerr << "reference found " << triangles_reference.size() << " triangles, ";
        std::cerr << "but got " << cfg->triangles() << " triangles." << std::endl;
        return;
    } else {
        std::cout << "Triangle count matches: " << cfg->triangles() << " triangles." << std::endl;
    }
}

cello_command_processor_app*
MakeApp(int argc, char *argv[])
{
    return new cello_tc_app(argc, argv);
}
