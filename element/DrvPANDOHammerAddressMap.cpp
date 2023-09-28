#include "DrvPANDOHammerAddressMap.hpp"
using namespace SST;
using namespace Drv;
using Addr = SST::Interfaces::StandardMem::Addr;

DrvPANDOHammerAddressMap::DrvPANDOHammerAddressMap
(SST::ComponentId_t id, SST::Params& params) :
    DrvAddressMap(id, params) {
    // get parameters
    int64_t verbose = params.find<int64_t>("verbose", false);
   // set up output
    output_.init("[DrvPANDOHammerAddressMap @t:@f:@l: @p]", verbose, 0, SST::Output::STDOUT);    
    output_.verbose(CALL_INFO, 1, 0, "done\n");
}

DrvPANDOHammerAddressMap::~DrvPANDOHammerAddressMap() {
}

Addr DrvPANDOHammerAddressMap::addrVirtualToPhysical(uint64_t virt) const {
    return virt;
}

