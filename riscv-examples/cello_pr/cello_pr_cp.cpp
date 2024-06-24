#define PANDOCOMMAND_DEBUG
#include <vector>
#include <DrvAPI.hpp>
#include <read_graph.hpp>
#include <transpose_graph.hpp>
#include "cello_core_drvr_commandprocessor.hpp"
#include "cello_pr.hpp"

DrvAPI::dram_static<graph> the_graph;

class pagerank_app : public cello_command_processor_app {
public:
    pagerank_app(int argc, char **argv);
    pagerank_app(const pagerank_app &) = delete;
    pagerank_app &operator=(const pagerank_app &) = delete;
    pagerank_app(pagerank_app &&) = delete;
    pagerank_app &operator=(pagerank_app &&) = delete;
    virtual ~pagerank_app() = default;
    virtual void input_application_data() override;
    virtual void output_application_data() override;

    std::string graph_path;
    std::vector<int> fwd_offsets, rev_offsets;
    std::vector<int> fwd_edges, rev_edges;
    int V, E;
    pointer<pagerank_config> cfg;
};

pagerank_app::pagerank_app(int argc, char **argv)
    : cello_command_processor_app(argc, argv)
    , graph_path(argv[2]) {
    read_graph
        (graph_path, &this->V, &this->E, this->fwd_offsets, this->fwd_edges);
    transpose_graph
        (this->V, this->E, this->fwd_offsets, this->fwd_edges,
         this->rev_offsets, this->rev_edges);
}

void pagerank_app::input_application_data() {
    cfg = exe_
        .symbol("pagerank_configure")
        .encode();
    CMD_DBG("found pagerank configure @" << CMD_FMT_ADDR(cfg) << std::endl);
    the_graph.init(V, E, rev_offsets, rev_edges);
    cfg->g() = the_graph.address();
    auto memtype = DrvAPI::DrvAPIMemoryDRAM;
    cfg->old_rank() = (pointer<float>)
        DrvAPI::DrvAPIMemoryAlloc(memtype, V * sizeof(float));
    cfg->new_rank() = (pointer<float>)
        DrvAPI::DrvAPIMemoryAlloc(memtype, V * sizeof(float));
    cfg->out_degree() = (pointer<vertex>)
        DrvAPI::DrvAPIMemoryAlloc(memtype, V * sizeof(int));
    cfg->contrib() = (pointer<float>)
        DrvAPI::DrvAPIMemoryAlloc(memtype, V * sizeof(float));
    the_graph.foreach_vertex([&](vertex v) mutable {
        cfg->old_rank()[v] = 1.0 / V;
        cfg->new_rank()[v] = 0.0;
        cfg->out_degree()[v]
            = fwd_offsets[v + 1]
            - fwd_offsets[v];
        cfg->contrib()[v] = cfg->old_rank()[v] / cfg->out_degree()[v];
    });
}

void pagerank_app::output_application_data() {
    float sum = 0.0;
    the_graph.foreach_vertex([this,&sum](vertex v) mutable {
        sum += cfg->new_rank()[v];
    });
    std::cout << "sum of ranks = " << sum << std::endl;
}

cello_command_processor_app *MakeApp(int argc, char *argv[])
{
    return new pagerank_app(argc, argv);
}
