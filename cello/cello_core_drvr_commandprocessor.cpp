//#define PANDOCOMMAND_DEBUG
#include <iomanip>
#include "cello_core_drvr_commandprocessor.hpp"

using namespace DrvAPI;
using namespace pandocommand;

#define FMT_ADDR(x)                                                     \
    "0x" << std::setfill('0') << std::setw(16) << std::hex << x << std::dec

#define FMT_SIZE(x)                             \
    "0x" << std::hex << x << std::dec

cello_command_processor_app::cello_command_processor_app(int argc, char **argv)
    : exe_(argv[1])
{
}

void cello_command_processor_app::run()
{
    load_executable();
    allocate_application_data();
    input_application_data();
    configure_cello();
    run_cores();
    output_application_data();
}

void cello_command_processor_app::load_executable()
{
    loadProgram(exe_);
}

void cello_command_processor_app::configure_cello()
{
    cfg_ = exe_.symbol<cello_config>("cello_configuration", Place{});
    DrvAPIAddress base = DrvAPIVAddress::MainMemBase(myPXNId()).encode() + 0x1000;
    DrvAPIAddress size = pxnDRAMSize() - 0x1000;
    CMD_DBG("base: " << FMT_ADDR(base) << ", size: " << FMT_SIZE(size) << std::endl);
    cfg_->allocator_base() = base;
    cfg_->allocator_size() = size;    
}

void cello_command_processor_app::run_cores()
{
    assertResetAll(false);
    while (cfg_->main_returned() == 0) {
        DrvAPI::wait(1000);
    }
    CMD_DBG("main returned: " << cfg_->main_returned() << std::endl);
}

int CelloCommandPorcessorMain(int argc, char *argv[])
{
    cmd_dbg("Hello, Cello Command Processor!\n");
    auto *app = MakeApp(argc, argv);
    app->run();
    return 0;
}

declare_drv_api_main(CelloCommandPorcessorMain);
