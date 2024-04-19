#include "cello_core_drvr_commandprocessor.hpp"
__attribute__((weak))
cello_command_processor_app *MakeApp(int argc, char **argv)
{
    return new cello_command_processor_app(argc, argv);
}

