// SPDX-License-Identifier: MIT
// Copyright (c) 2024 University of Washington
#define COMMANDPROCESSOR_DEBUG
#include "cello_bfs.hpp"
#include "cello_core_drvr_commandprocessor.hpp"
#include "read_graph.hpp"
#include "transpose_graph.hpp"
#include "breadth_first_search_graph.hpp"
DrvAPI::dram_static<bidirectional_graph> graph;

class bfs_app : public cello_command_processor_app {
public:
    typedef graph_type::vertex_type vertex;
    bfs_app(int argc, char *argv[]);
    virtual ~bfs_app() {}
    virtual void input_application_data() override;
    virtual void output_application_data() override;
    std::string graph_file;
    vertex start;
    pointer<bfs_configuration> cfg;
    std::vector<vertex> ref_distance;
};

bfs_app::bfs_app(int argc, char *argv[])
    : cello_command_processor_app(argc, argv)
    , graph_file(argv[2])
    , start(std::atoi(argv[3]))
{    
}

void bfs_app::input_application_data()
{    
    vertex V, E;
    std::vector<vertex> fwd_offsets, fwd_edges;
    std::vector<vertex> rev_offsets, rev_edges;
    read_graph(graph_file, &V, &E, fwd_offsets, fwd_edges);
    transpose_graph(V, E, fwd_offsets, fwd_edges,
                    rev_offsets, rev_edges);

    breadth_first_search_graph
        (start, V, E, fwd_offsets, fwd_edges,
         ref_distance);
    graph.init(V, E, fwd_offsets, fwd_edges, rev_offsets, rev_edges);

    pointer<vertex> distance =
        DrvAPI::DrvAPIMemoryAlloc
        (DrvAPI::DrvAPIMemoryDRAM, V * sizeof(vertex));

    graph.foreach_vertex([distance](vertex v) mutable {
        distance[v] = -1;
    });
    
    cfg = exe_.symbol<bfs_configuration>
        ("bfs_config", pandocommand::Place{});
    
    cfg->g() = graph.address();
    cfg->start() = start;
    cfg->distance() = distance;
}

void bfs_app::output_application_data()
{
    pointer<vertex> distance = cfg->distance();
    graph.foreach_vertex([distance, this](vertex v) mutable {
        if (distance[v] != ref_distance[v]) {
            std::cout << "distance[" << v << "] = "
                      << distance[v] << " != "
                      << ref_distance[v] << std::endl;
        }
    });
}

cello_command_processor_app *MakeApp(int argc, char *argv[])
{
    return new bfs_app(argc, argv);
}
