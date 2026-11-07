#ifndef AXFB_CMD_API_H
#define AXFB_CMD_API_H

#include "zmc_common.h"
// #include "AxFbCmdManager.h"
#include "FbFactoryDef.h"
#include "axfb_attr_api.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct AxFbRunConfig {
    double cycleTime; 
    int exitCount; 
    bool verbose; 
    uint16_t verboseModulo;
}AxFbRunConfig;

typedef struct AxFbCmdManagerWrap AxFbCmdManagerWrap;
typedef AxFbCmdManagerWrap* axfb_cmd_manager_t;
typedef struct AxFbCmdConfig AxFbCmdConfig;

ZMC_DLLPORT AxFbCmdConfig* axfb_cmd_config_create();
ZMC_DLLPORT void axfb_cmd_config_destroy(AxFbCmdConfig* pCfg);
ZMC_DLLPORT int load_axfb_cmd_config_from_json(AxFbCmdConfig* pCfg, const char* fn);


ZMC_DLLPORT axfb_cmd_manager_t axfb_cmd_manager_create();


ZMC_DLLPORT void axfb_cmd_manager_destroy(axfb_cmd_manager_t handle);

ZMC_DLLPORT void axfb_set_config(axfb_cmd_manager_t handle, const AxFbCmdConfig* cfg);

ZMC_DLLPORT void axfb_set_run_config(axfb_cmd_manager_t handle, const AxFbRunConfig* rCfg);


ZMC_DLLPORT void axfb_create_axis_array(axfb_cmd_manager_t handle, int num, int ax_type);
ZMC_DLLPORT void axfb_create_fb(axfb_cmd_manager_t handle, int fb_type);


ZMC_DLLPORT void axfb_push_command(axfb_cmd_manager_t handle, const CommandStruct* cmd);


ZMC_DLLPORT void axfb_run_cycle(axfb_cmd_manager_t handle);
ZMC_DLLPORT void axfb_run(axfb_cmd_manager_t handle);

ZMC_DLLPORT AxFbAttrHandle axfb_get_axis_ptr(axfb_cmd_manager_t handle, int id);


ZMC_DLLPORT AxFbAttrHandle axfb_get_fb_ptr(axfb_cmd_manager_t handle, int id);

#ifdef __cplusplus
}
#endif

#endif // AXFB_CMD_API_H