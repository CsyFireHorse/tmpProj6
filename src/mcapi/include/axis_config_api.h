#ifndef AXIS_CONFIG_API_H
#define AXIS_CONFIG_API_H 

#include "zmc_common.h"
#include "AxisCiaConfig.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define AXIS_CONFIG_SIZE (312)
typedef struct AxisConfigStorage{
    char storage_[AXIS_CONFIG_SIZE];
}AxisConfigStorage;

ZMC_DLLPORT AxisCiaConfig* axis_config_new();
ZMC_DLLPORT void axis_config_default_init(AxisCiaConfig* cfg);
ZMC_DLLPORT int axis_config_check(const AxisCiaConfig* pCfg);
ZMC_DLLPORT void axis_configl_delete(AxisCiaConfig* handle);

#ifdef __cplusplus
}
#endif
#endif // !AXIS_VIRTUAL_API_H
