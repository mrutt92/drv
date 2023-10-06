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
        Params sys_params = params.get_scoped_params("sys");
        data_.num_pxn_ = params.find<int64_t>("num_pxn", 1);
        data_.pxn_pods_ = params.find<int64_t>("pxn_pods", 1);
        data_.pod_cores_ = params.find<int64_t>("pod_cores", 1);
        data_.core_threads_ = params.find<int64_t>("core_threads", 1);
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
