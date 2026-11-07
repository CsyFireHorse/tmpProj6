

#include "axis_virtual_api.h"
#include "fbt_axis_virtual.h"


axis_virtual_handle_t axis_virtual_new()
{
    return (axis_virtual_handle_t) new zmc::AxisVirtual();
}

 void axis_virtual_set_config(axis_virtual_handle_t handle, const AxisCiaConfig* cfg)
{
     ((zmc::AxisVirtual*)handle)->set_config(cfg);
}

 AxisCiaConfig axis_virtual_get_config(axis_virtual_handle_t handle)
{
    return ((zmc::AxisVirtual*)handle)->get_config();
}

 AxisCiaConfig* axis_virtual_get_config_addr(axis_virtual_handle_t handle)
{
    return ((zmc::AxisVirtual*)handle)->get_config_addr();
}

AXIS_REF axis_virtual_get_ref(axis_virtual_handle_t handle)
{
    return (AXIS_REF)(((zmc::AxisVirtual*)handle));
}

void axis_virtual_runcycle(axis_virtual_handle_t handle)
{
    ((zmc::AxisVirtual*)handle)->runCycle();
}

void axis_virtual_delete(axis_virtual_handle_t handle)
{
    delete (zmc::AxisVirtual*)handle;
}

void axis_virtual_print(axis_virtual_handle_t handle, int verboseFlag){
    ((zmc::AxisVirtual*)handle)->print(verboseFlag);
}

bool axis_virtual_is_valid(axis_virtual_handle_t handle){
    return ((zmc::AxisVirtual*)handle)->is_valid();
}

uint32_t axis_virtual_get_count(axis_virtual_handle_t handle){
    return ((zmc::AxisVirtual*)handle)->get_count();
}