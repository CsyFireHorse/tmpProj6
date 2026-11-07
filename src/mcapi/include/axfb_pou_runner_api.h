#ifndef AXFB_POU_RUNNER_API_H
#define AXFB_POU_RUNNER_API_H

#include "zmc_common.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct AxPouTaskBinderAutoWrap AxPouTaskBinderAutoWrap;
typedef AxPouTaskBinderAutoWrap* axfb_pou_runner_handle_t;

// Create and destroy
ZMC_DLLPORT axfb_pou_runner_handle_t axfb_pou_runner_create(const char* iniPath, const char* iniContent);
ZMC_DLLPORT void axfb_pou_runner_destroy(axfb_pou_runner_handle_t handle);

// Configuration setters
ZMC_DLLPORT void axfb_pou_runner_set_cycles(axfb_pou_runner_handle_t handle, int cycles);
ZMC_DLLPORT int axfb_pou_runner_get_verbose_flag(axfb_pou_runner_handle_t handle);
ZMC_DLLPORT void axfb_pou_runner_set_verbose_flag(axfb_pou_runner_handle_t handle, int flag);
ZMC_DLLPORT int axfb_pou_runner_get_cycle_time(axfb_pou_runner_handle_t handle);
ZMC_DLLPORT void axfb_pou_runner_set_cycle_time(axfb_pou_runner_handle_t handle, int us);

// Task binding
ZMC_DLLPORT void axfb_pou_runner_bind_task(axfb_pou_runner_handle_t handle, int index, void* task);

// Run control
ZMC_DLLPORT void axfb_pou_runner_run(axfb_pou_runner_handle_t handle);
ZMC_DLLPORT void axfb_pou_runner_run_cycle(axfb_pou_runner_handle_t handle);
ZMC_DLLPORT void axfb_pou_runner_pre_run(axfb_pou_runner_handle_t handle);
ZMC_DLLPORT void axfb_pou_runner_post_run_cycle(axfb_pou_runner_handle_t handle);

// Static methods for axis access
ZMC_DLLPORT void* axfb_pou_runner_get_axis_by_index(uint32_t index);
ZMC_DLLPORT int axfb_pou_runner_get_axis_count(void);

#ifdef __cplusplus
}
#endif

#endif // AXFB_POU_RUNNER_API_H

