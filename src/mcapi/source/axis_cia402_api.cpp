#include "axis_cia402_api.h"
#include "fbt_axis_cia402.h"
#include "new"

using zmc::AxisCia402;

struct AxisCWrapInner {
    char storage_[AXIS_IMPL_SIZE];
};
void axis_cia402_init(axis_cia402_handle_t ax){
    static_assert(sizeof(AxisCia402) <= sizeof(AxisCWrapInner::storage_), "AxisCia402 is too large");
    new(((AxisCWrapInner*)ax)) AxisCia402();
}   

void axis_cia402_deinit(axis_cia402_handle_t ax){
    ((AxisCia402*)ax)->~AxisCia402();
}

axis_cia402_handle_t axis_cia402_new()
{
    return (axis_cia402_handle_t) new AxisCia402();
}


void axis_cia402_set_input(axis_cia402_handle_t handle, servo_stat* stat)
{
    ((AxisCia402*)handle)->set_input(stat);
}

void axis_cia402_get_output(axis_cia402_handle_t handle, servo_ctrl*ctrl)
{
    ((AxisCia402*)handle)->get_output(ctrl);
}

 void axis_cia402_set_config(axis_cia402_handle_t handle, const AxisCiaConfig* cfg)
{
     ((AxisCia402*)handle)->set_config(cfg);
}

 AxisCiaConfig axis_cia402_get_config(axis_cia402_handle_t handle)
{
    return ((AxisCia402*)handle)->get_config();
}

AxisCiaConfig* axis_cia402_get_config_addr(axis_cia402_handle_t handle)
{
    return ((AxisCia402*)handle)->get_config_addr();
}

AXIS_REF axis_cia402_get_ref(axis_cia402_handle_t handle)
{
    return (AXIS_REF)(((AxisCia402*)handle));
}
AXIS_REF axis_get_ref(axis_handle_t handle)
{
    return (AXIS_REF)(((AxisCia402*)handle));
}

void axis_cia402_runcycle(axis_cia402_handle_t handle)
{
    ((AxisCia402*)handle)->runCycle();
}

void axis_cia402_delete(axis_cia402_handle_t handle)
{
    delete (AxisCia402*)handle;
}

