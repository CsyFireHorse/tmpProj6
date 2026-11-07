#ifndef AXIS_VIRTUAL_API_H
#define AXIS_VIRTUAL_API_H 

#include "zmc_common.h"
#include "AxisRef.h"
#include "AxisStorage.h"
#include "AxisCiaConfig.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AxisCiaConfig AxisCia402Config;

typedef struct AxisStorage AxisVirtualInpWrap;
typedef AxisVirtualInpWrap* axis_virtual_handle_t;

ZMC_DLLPORT axis_virtual_handle_t axis_virtual_new();

ZMC_DLLPORT void axis_virtual_set_config(axis_virtual_handle_t handle, const AxisCiaConfig* cfg);

ZMC_DLLPORT AxisCiaConfig axis_virtual_get_config(axis_virtual_handle_t handle);
ZMC_DLLPORT AxisCiaConfig* axis_virtual_get_config_addr(axis_virtual_handle_t handle);

ZMC_DLLPORT void axis_virtual_runcycle(axis_virtual_handle_t handle);

ZMC_DLLPORT AXIS_REF axis_virtual_get_ref(axis_virtual_handle_t handle);

ZMC_DLLPORT void axis_virtual_delete(axis_virtual_handle_t handle);

ZMC_DLLPORT void axis_virtual_print(axis_virtual_handle_t handle, int);
ZMC_DLLPORT bool axis_virtual_is_valid(axis_virtual_handle_t handle);
ZMC_DLLPORT uint32_t axis_virtual_get_count(axis_virtual_handle_t handle);

#ifdef __cplusplus
}
#endif
#endif // !AXIS_VIRTUAL_API_H
