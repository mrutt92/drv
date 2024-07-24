#pragma once
#include <sst/core/component.h>
#include <sst/core/link.h>
#include <sst/core/interfaces/stdMem.h>
#include <sst/core/event.h>

namespace SST {
namespace Drv {

class DrvCoreBase : public Component {
    SST_ELI_REGISTER_COMPONENT(
        DrvCoreBase,
        "Drv",
        "DrvCoreBase",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "A core base class",
        COMPONENT_CATEGORY_UNCATEGORIZED
    );

    // Document the parameters that this component accepts
#define CORE_BASE_PARAMS                        \
    {"clock", "clock frequency", "1GHz"},       \
    {"debug_level", "debug level", "0"}

    // Document the ports that this component accepts

#define CORE_BASE_PORTS                                         \
    {"loopback", "A loopback link", {"Drv.DrvEvent", ""}}

    SST_ELI_DOCUMENT_PORTS(
        CORE_BASE_PORTS,
    );

  // Document the subcomponents that this component has

    #define CORE_BASE_SUBCOMPONENT_SLOTS \
        {"memory", "Interface to a memory hierarchy", "SST::Interfaces::StandardMem"}

    SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(CORE_BASE_SUBCOMPONENT_SLOTS);

};

}
}
