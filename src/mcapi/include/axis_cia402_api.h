#ifndef AXIS_CIA402_API_H
#define AXIS_CIA402_API_H 

#include "zmc_common.h"
#include "pdo_struct.h"
#include "AxisCiaConfig.hpp"
#include "AxisRef.h"
#include "AxisStorage.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AxisCiaConfig AxisCia402Config;

// struct AxisCia402InpWrap: AxisStorage {};
typedef struct AxisStorage AxisCia402InpWrap;

typedef AxisStorage* axis_handle_t;
typedef AxisCia402InpWrap* AxisCia402InpRef;
typedef AxisCia402InpWrap* axis_cia402_handle_t;

// use for AxisCia402InpWrap
ZMC_DLLPORT void axis_cia402_init(axis_cia402_handle_t);
ZMC_DLLPORT void axis_cia402_deinit(axis_cia402_handle_t);

ZMC_DLLPORT axis_cia402_handle_t axis_cia402_new();
ZMC_DLLPORT void axis_cia402_set_input(axis_cia402_handle_t handle, servo_stat* stat);

ZMC_DLLPORT void axis_cia402_get_output(axis_cia402_handle_t handle, servo_ctrl*ctrl);

ZMC_DLLPORT void axis_cia402_set_config(axis_cia402_handle_t handle, const AxisCiaConfig* cfg);

ZMC_DLLPORT AxisCiaConfig axis_cia402_get_config(axis_cia402_handle_t handle);
ZMC_DLLPORT AxisCiaConfig* axis_cia402_get_config_addr(axis_cia402_handle_t handle);

ZMC_DLLPORT void axis_cia402_runcycle(axis_cia402_handle_t handle);

ZMC_DLLPORT AXIS_REF axis_cia402_get_ref(axis_cia402_handle_t handle);

ZMC_DLLPORT void axis_cia402_delete(axis_cia402_handle_t handle);

ZMC_DLLPORT AXIS_REF axis_get_ref(axis_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif // !AXIS_CIA402_API_H
