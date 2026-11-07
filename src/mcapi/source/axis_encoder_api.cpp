

#include "axis_encoder_api.h"
#include "fbt_axis_encoder.h"


axis_encoder_handle_t axis_encoder_new()
{
    return (axis_encoder_handle_t) new zmc::AxisEncoder();
}

 void axis_encoder_set_config(axis_encoder_handle_t handle, const AxisCiaConfig* cfg)
{
     ((zmc::AxisEncoder*)handle)->set_config(cfg);
}

 AxisCiaConfig axis_encoder_get_config(axis_encoder_handle_t handle)
{
    return ((zmc::AxisEncoder*)handle)->get_config();
}

AxisCiaConfig* axis_encoder_get_config_addr(axis_encoder_handle_t handle)
{
    return ((zmc::AxisEncoder*)handle)->get_config_addr();
}

AXIS_REF axis_encoder_get_ref(axis_encoder_handle_t handle)
{
    return (AXIS_REF)(((zmc::AxisEncoder*)handle));
}
void axis_encoder_set_encoder_pos(axis_encoder_handle_t handle, int diEncoderPosition){
    ((zmc::AxisEncoder*)handle)->set_encoder_pos(diEncoderPosition);
}
void axis_encoder_runcycle(axis_encoder_handle_t handle)
{
    ((zmc::AxisEncoder*)handle)->runCycle();
}

void axis_encoder_delete(axis_encoder_handle_t handle)
{
    delete (zmc::AxisEncoder*)handle;
}

