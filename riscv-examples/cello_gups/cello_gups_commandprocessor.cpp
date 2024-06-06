#define COMMAND_PROCESSOR_DEBUG
#include "cello_core_drvr_commandprocessor.hpp"

cello_command_processor_app* MakeApp(int argc, char *argv[])
{
    printf("hello from gups\n");        
    return new cello_command_processor_app(argc, argv);
}
