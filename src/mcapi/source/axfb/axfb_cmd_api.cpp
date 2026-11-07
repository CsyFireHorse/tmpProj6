#include "axfb_cmd_api.h"
#include "include/axfb/AxFbCmdManager.h"
#include "include/axfb/AxFbCmdParser.hpp"

struct AxFbCmdManagerWrap : public zmc::AxFbCmdManager {};

AxFbCmdConfig* axfb_cmd_config_create(){
    return new AxFbCmdConfig();
}
void axfb_cmd_config_destroy(AxFbCmdConfig* pCfg){
    delete pCfg;
}

int load_axfb_cmd_config_from_json(AxFbCmdConfig* pCfg, const char* fn){
    return load_AxFbCmdConfig_from_json(*pCfg, fn);
}

axfb_cmd_manager_t axfb_cmd_manager_create() {
    return reinterpret_cast<axfb_cmd_manager_t>(new AxFbCmdManagerWrap());
}

void axfb_cmd_manager_destroy(axfb_cmd_manager_t handle) {
    delete reinterpret_cast<AxFbCmdManagerWrap*>(handle);
}

void axfb_set_config(axfb_cmd_manager_t handle, const AxFbCmdConfig* cfg){
    auto mgr = reinterpret_cast<AxFbCmdManagerWrap*>(handle);
    mgr->set_config(*cfg);
}

void axfb_set_run_config(axfb_cmd_manager_t handle, const AxFbRunConfig* rCfg){
    auto mgr = reinterpret_cast<AxFbCmdManagerWrap*>(handle);
    mgr->set_run_config(* static_cast<const RunConfig*>((void*) rCfg));
}

void axfb_create_axis_array(axfb_cmd_manager_t handle, int num, int ax_type) {
    auto mgr = reinterpret_cast<AxFbCmdManagerWrap*>(handle);
    mgr->createAxisArray(num, static_cast<AX_FACTORY_TYPE>(ax_type));
}
void axfb_create_fb(axfb_cmd_manager_t handle, int fb_type) {
    auto mgr = reinterpret_cast<AxFbCmdManagerWrap*>(handle);
    mgr->createFb(static_cast<FB_TYPE>(fb_type));
}

void axfb_push_command(axfb_cmd_manager_t handle, const CommandStruct* cmd) {
    auto mgr = reinterpret_cast<AxFbCmdManagerWrap*>(handle);
    mgr->pushCommand(*cmd);
}

void axfb_run_cycle(axfb_cmd_manager_t handle) {
    auto mgr = reinterpret_cast<AxFbCmdManagerWrap*>(handle);
    mgr->runCycle();
}
void axfb_run(axfb_cmd_manager_t handle) {
    auto mgr = reinterpret_cast<AxFbCmdManagerWrap*>(handle);
    mgr->run();
}


AxFbAttrHandle axfb_get_axis_ptr(axfb_cmd_manager_t handle, int id) {
    auto mgr = reinterpret_cast<AxFbCmdManagerWrap*>(handle);
    return (AxFbAttrHandle) mgr->getAxis(id);
}

AxFbAttrHandle axfb_get_fb_ptr(axfb_cmd_manager_t handle, int id) {
    auto mgr = reinterpret_cast<AxFbCmdManagerWrap*>(handle);
    return (AxFbAttrHandle) mgr->getFb(id);
}