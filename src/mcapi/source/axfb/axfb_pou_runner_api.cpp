#include "axfb_pou_runner_api.h"
#include "include/pou/AxPou.hpp"
#include "include/pou/AxisManager.h"
#include <string>

struct AxPouTaskBinderAutoWrap : public zmc::AxPouTaskBinderAuto {
    AxPouTaskBinderAutoWrap(const std::string& iniPath, const std::string& iniContent, int axNum=1, const std::string& namePrefix = "axis_")
        : zmc::AxPouTaskBinderAuto(iniPath, iniContent, axNum, namePrefix) {}
};

axfb_pou_runner_handle_t axfb_pou_runner_create(const char* iniPath, const char* iniContent) {
    std::string path = iniPath ? iniPath : "";
    std::string content = iniContent ? iniContent : "";
    return reinterpret_cast<axfb_pou_runner_handle_t>(new AxPouTaskBinderAutoWrap(path, content));
}

void axfb_pou_runner_destroy(axfb_pou_runner_handle_t handle) {
    delete reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
}

void axfb_pou_runner_set_cycles(axfb_pou_runner_handle_t handle, int cycles) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    runner->set_cycles(cycles);
}

void axfb_pou_runner_set_verbose_flag(axfb_pou_runner_handle_t handle, int flag) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    runner->set_verbose_flag(flag);
}
int axfb_pou_runner_get_verbose_flag(axfb_pou_runner_handle_t handle) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    return runner->get_verbose_flag();
}
void axfb_pou_runner_set_cycle_time(axfb_pou_runner_handle_t handle, int us) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    runner->set_cycle_time(us);
}

int axfb_pou_runner_get_cycle_time(axfb_pou_runner_handle_t handle) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    return runner->get_cycle_time();
}

void axfb_pou_runner_bind_task(axfb_pou_runner_handle_t handle, int index, void* task) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    auto taskBase = static_cast<zmc::PouTaskBase*>(task);
    runner->bind(index, taskBase);
}

void axfb_pou_runner_run(axfb_pou_runner_handle_t handle) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    runner->run();
}

void axfb_pou_runner_run_cycle(axfb_pou_runner_handle_t handle) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    runner->runCycle();
}

void axfb_pou_runner_pre_run(axfb_pou_runner_handle_t handle) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    runner->preRun();
}

void axfb_pou_runner_post_run_cycle(axfb_pou_runner_handle_t handle) {
    auto runner = reinterpret_cast<AxPouTaskBinderAutoWrap*>(handle);
    runner->postRunCycle();
}

void* axfb_pou_runner_get_axis_by_index(uint32_t index) {
    return reinterpret_cast<void*>(zmc::SgAxisArr::getAxisByIndex(index));
}

int axfb_pou_runner_get_axis_count(void) {
    return static_cast<int>(zmc::SgAxisArr::getSize());
}

