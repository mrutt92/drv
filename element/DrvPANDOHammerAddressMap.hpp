#pragma once
#include <sst/core/component.h>
#include <sst/core/subcomponent.h>
#include <sst/core/link.h>
#include <sst/core/event.h>
#include <sst/core/interfaces/stdMem.h>
#include "DrvAddressMap.hpp"
namespace SST {
namespace Drv {

class DrvPANDOHammerAddressMap : public DrvAddressMap {
public:
  // register this subcomponent into the element library
    SST_ELI_REGISTER_SUBCOMPONENT(
        SST::Drv::DrvPANDOHammerAddressMap,
        "Drv",
        "DrvPANDOHammerAddressMap",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "PANDO Hammer Address Map",
        SST::Drv::DrvAddressMap
    )

  // document the parameters that this component accepts
  SST_ELI_DOCUMENT_PARAMS(
      // debug flags
      {"verbose", "Verbosity of logging", "0"},
  )
  /**
   * constructor
    * @param[in] id The component id.
    * @param[in] params Parameters for this component.
    */
  DrvPANDOHammerAddressMap(SST::ComponentId_t id, SST::Params& params);

  /**
   * destructor
   */
  virtual ~DrvPANDOHammerAddressMap();
    
  /**
   * @brief Convert a virtual address to a physical address
   * 
   * @param addrVirtual The virtual address
   * @return SST::Interfaces::StandardMem::Addr The physical address
   */
  virtual SST::Interfaces::StandardMem::Addr
  addrVirtualToPhysical(uint64_t virt) const override;

  virtual void init(unsigned int phase) override {}
  virtual void setup() override {}
  virtual void finish() override {}
  
private:
  SST::Output output_; //!< @brief The output stream for this component
};

}
}
