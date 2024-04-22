#define PANDOCOMMAND_DEBUG
#include "cello_core_drvr_commandprocessor.hpp"
#include "cello_pr.hpp"

class pagerank_app : public cello_command_processor_app {
public:
    pagerank_app(int argc, char **argv);
    pagerank_app(const pagerank_app &) = delete;
    pagerank_app &operator=(const pagerank_app &) = delete;
    pagerank_app(pagerank_app &&) = delete;
    pagerank_app &operator=(pagerank_app &&) = delete;
    virtual ~pagerank_app() = default;
    virtual void allocate_application_data() override;
};

pagerank_app::pagerank_app(int argc, char **argv)
    : cello_command_processor_app(argc, argv) {
}

void pagerank_app::allocate_application_data() {
    pointer<pagerank_config> cfg
        = exe_
        .symbol("pagerank_configure")
        .encode();
    CMD_DBG("found pagerank configure @" << CMD_FMT_ADDR(cfg) << std::endl);
}

cello_command_processor_app *MakeApp(int argc, char *argv[])
{
    return new pagerank_app(argc, argv);
}
