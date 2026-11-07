#ifndef AXIS_ENCODER_API_H
#define AXIS_ENCODER_API_H 

#include "zmc_common.h"
#include "AxisStorage.h"
#include "AxisCiaConfig.hpp"
#include "AxisRef.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AxisCiaConfig AxisCia402Config;
typedef struct AxisStorage AxisEncoderInpWrap;
typedef AxisEncoderInpWrap* axis_encoder_handle_t;

ZMC_DLLPORT axis_encoder_handle_t axis_encoder_new();
ZMC_DLLPORT void axis_encoder_set_config(axis_encoder_handle_t handle, const AxisCiaConfig* cfg);
ZMC_DLLPORT AxisCiaConfig axis_encoder_get_config(axis_encoder_handle_t handle);
ZMC_DLLPORT AxisCiaConfig* axis_encoder_get_config_addr(axis_encoder_handle_t handle);

ZMC_DLLPORT void axis_encoder_set_encoder_pos(axis_encoder_handle_t handle, int diEncoderPosition);
ZMC_DLLPORT void axis_encoder_runcycle(axis_encoder_handle_t handle);

ZMC_DLLPORT AXIS_REF axis_encoder_get_ref(axis_encoder_handle_t handle);

ZMC_DLLPORT void axis_encoder_delete(axis_encoder_handle_t handle);


#ifdef __cplusplus
}
#endif

#endif // !AXIS_ENCODER_API_H
