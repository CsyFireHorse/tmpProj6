#include "axis_config_api.h"

ZMC_DLLPORT AxisCiaConfig* axis_config_new() {
    static_assert(sizeof(AxisCiaConfig)==AXIS_CONFIG_SIZE, "sizeof AxisCiaConfig not equal AXIS_CONFIG_SIZE");
    return new AxisCiaConfig();
}
ZMC_DLLPORT void axis_config_default_init(AxisCiaConfig* cfg) {
    AxisCiaConfig_default_init(cfg);
}
ZMC_DLLPORT int axis_config_check(const AxisCiaConfig* pCfg) {
    return AxisCiaConfig_check(pCfg);
}
ZMC_DLLPORT void axis_configl_delete(AxisCiaConfig* handle){
    delete handle;
}