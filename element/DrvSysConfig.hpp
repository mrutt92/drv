#pragma once
#include <DrvAPISysConfig.hpp>
#include <sst/core/component.h>

namespace SST
{
namespace Drv
{
class DrvSysConfig
{
public:
    /**
     * Constructor
     */
    DrvSysConfig() {}

    /**
     * initialize the system configuration
     */
    void init(Params &params) {
        data_.num_pxn_ = params.find<int64_t>("sys_num_pxn", 1);
        data_.pxn_pods_ = params.find<int64_t>("sys_pxn_pods", 1);
        data_.pod_cores_ = params.find<int64_t>("sys_pod_cores", 1);
    }
    
    /**
     * return the configuration data
     */
    const DrvAPI::DrvAPISysConfigData & configData() const {
        return data_;
    }    

    /**
     * return the sys config data
     */
    DrvAPI::DrvAPISysConfig config() const {
        return DrvAPI::DrvAPISysConfig(configData());
    }

private:
    DrvAPI::DrvAPISysConfigData data_; //!< system configuration data

};
}
}
