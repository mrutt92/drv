#include "DrvAPISysConfig.hpp"

namespace DrvAPI
{

DrvAPISysConfig sysconfig;

}

extern "C" DrvAPI::DrvAPISysConfig* DrvAPIGetSysConfig()
{
    return &DrvAPI::sysconfig;
}

extern "C" void DrvAPISetSysConfig(DrvAPI::DrvAPISysConfig* sys_config)
{
    DrvAPI::sysconfig = *sys_config;
}
