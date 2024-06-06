#ifndef CELLO_CORE_DRVR_COMMANDPROCESSOR_HPP
#define CELLO_CORE_DRVR_COMMANDPROCESSOR_HPP
#include "cello_core_drvr_config.hpp"
#include "pandocommand/executable.hpp"
#include "pandocommand/loader.hpp"
#include "pandocommand/control.hpp"
#include "pandocommand/debug.hpp"

namespace DrvAPI
{

template <>
class value_handle<cello_config>
{
    DRV_API_VALUE_HANDLE_DEFAULTS(cello_config);
    DRV_API_VALUE_HANDLE_FIELD(cello_config, allocator_base, int64_t, allocator_base_);
    DRV_API_VALUE_HANDLE_FIELD(cello_config, allocator_size, int64_t, allocator_size_);
    DRV_API_VALUE_HANDLE_FIELD(cello_config, main_returned, int64_t, main_returned_);
};

}

class cello_command_processor_app {
public:
    cello_command_processor_app(int argc, char **argv);
    cello_command_processor_app(const cello_command_processor_app &other) = delete;
    cello_command_processor_app &operator=(const cello_command_processor_app &other) = delete;
    cello_command_processor_app(cello_command_processor_app &&other) = delete;
    cello_command_processor_app &operator=(cello_command_processor_app &&other) = delete;    
    virtual ~cello_command_processor_app() {}

    void run();
    void run_cores();
    void load_executable();
    void configure_cello();
    virtual void allocate_application_data() {}
    virtual void input_application_data() {}
    virtual void output_application_data() {}

    pandocommand::PANDOHammerExe exe_;
    DrvAPI::pointer<cello_config> cfg_ = {};
};


cello_command_processor_app *MakeApp(int argc, char **argv);

#endif // CELLO_CORE_DRVR_COMMANDPROCESSOR_HPP
