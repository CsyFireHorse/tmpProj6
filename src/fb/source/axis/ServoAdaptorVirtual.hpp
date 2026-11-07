#ifndef _ZMC_SERVO_ADAPTER_VIRTUAL_HPP_
#define _ZMC_SERVO_ADAPTER_VIRTUAL_HPP_

#include "ServoAdaptorBase.hpp"
#include "fbt_axis_virtual.h"
#include "fbt_axis_impl.h"
#include "fbt_axis_impl_cfg.hpp"

namespace zmc{

template<typename T>
inline AxisImpl* to_axis_impl(T* pAxis){
    return (AxisImpl*) pAxis;
}

template<typename T>
inline const AxisImpl* to_axis_impl(const T* pAxis){
    return (AxisImpl*) pAxis;
}

// all axis herit class share common construction function
// eAxisMtype  define class type!
template<typename T, AXIS_CTYPE AX_TYPE, bool B_VIRT>
inline void axisInit(AxisBase& axis){
    AxisCiaConfig& cfg = axis.get_config_ref();
    cfg.is_axis_virtual = B_VIRT;
    cfg.debug.eAxisMtype = AX_TYPE;
    axis.reload_config();
    auto& ax = *to_axis_impl(&axis);
    ax.bind_servo_adapter(new T());
}

class ServoAdapterVirtual:public ServoAdapterBase{
public:
    void onInit(AxisBase& axis){
        axisInit<ServoAdapterVirtual, AXIS_TYPE_VIRTUAL, true>(axis);
        // AxisCiaConfig& cfg = axis.get_config_ref();
        // cfg.is_axis_virtual = true;
        // cfg.debug.eAxisMtype = AXIS_TYPE_VIRTUAL;
        // axis.reload_config();
        // auto& ax = *to_axis_impl(&axis);
        // ax.bind_servo_adapter(new ServoAdapterVirtual());
    }
    void onSetFakConfig(AxisBase& axis, const AxisCiaConfig* pCfg){
        // axis.set_config(pCfg);
        auto& ax = *to_axis_impl(&axis);
        if (pCfg->is_axis_virtual){
            ax.init_position(pCfg->debug.servo_fak_pos);
            // ax.fActPosition = pCfg->debug.servo_fak_pos;
            // ax.fSetPosition = pCfg->debug.servo_fak_pos;
        }
        // if (AXIS_TYPE_VIRTUAL == pCfg->debug.eAxisMtype){
            // 
        // }
    } 

    void onFakRunCycle(AxisBase& axis) {
        auto& ax = *to_axis_impl(&axis);
        if (!ax.bSimulation) return;
        if (ax.m_cycleCounter < 2) return;
        ax.driveError = false;
        ax.regulatorRealState = ax.bRegulatorOn;
        ax.driveStartRealState = ax.bDriveStart;
        // pAxis->wStatWord = srvFak.wStatusword;
        ax.diActPosition = ax.diSetPosition;
        ax.diActVelocity = ax.diSetVelocity;
        ax.diActTorque = ax.diSetTorque;

        ax.fActPosition = ax.fSetPosition;
        ax.fActVelocity = ax.fSetVelocity;
        ax.fActAcceleration = ax.fSetAcceleration;
        ax.byRealControllerMode = ax.byControllerMode;
    }
};

}

#endif