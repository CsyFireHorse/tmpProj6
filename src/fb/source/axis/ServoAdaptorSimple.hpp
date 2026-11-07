#ifndef _ZMC_SERVO_ADAPTER_SIMPLE_HPP_
#define _ZMC_SERVO_ADAPTER_SIMPLE_HPP_

#include "ServoAdaptorBase.hpp"
#include "ServoAdaptorVirtual.hpp"
#include "AxisCiaConfig.hpp"
#include "SimpleServoSim.hpp"

namespace zmc{

class ServoAdapterSimpleVirtual:public ServoAdapterBase{
public:
    void onInit(AxisBase& axis){
        axisInit<ServoAdapterSimpleVirtual, AXIS_TYPE_VIRTUAL, true>(axis);
    }

    void onSetFakConfig(AxisBase& axis, const AxisCiaConfig* cfg){
        auto& ax = *to_axis_impl(&axis);
        if (cfg->is_axis_virtual){
            ax.init_position(cfg->debug.servo_fak_pos);
        }
    }

    void onFakRunCycle(AxisBase& axis){
        auto& ax = *to_axis_impl(&axis);
        if (!ax.bSimulation){
            return;
        }

        SimpleServoIn in;
        in.ctrlWord = ax.wControlWord;
        in.statusWord = ax.wStatusWord;
        in.opMode = ax.byControllerMode;
        in.diSetPosition = ax.diSetPosition;
        in.diSetVelocity = ax.diSetVelocity;
        in.diSetTorque = ax.diSetTorque;
        in.cycleTime = ax.cycleTime;
        in.cycleCounter = ax.m_cycleCounter;
        in.isSimulation = ax.bSimulation;

        SimpleServoOut out;
        out.statusWord = ax.wStatusWord;
        out.opModeDisplay = ax.byRealControllerMode;
        out.diActPosition = ax.diActPosition;
        out.diActVelocity = ax.diActVelocity;
        out.diActTorque = ax.diActTorque;
        out.driveError = ax.driveError;
        out.driveStartRealState = ax.driveStartRealState;
        out.regulatorRealState = ax.regulatorRealState;

        SimpleServoRunCycle(in, out);

        ax.wStatusWord = out.statusWord;
        ax.byRealControllerMode = out.opModeDisplay;
        ax.diActPosition = out.diActPosition;
        ax.diActVelocity = out.diActVelocity;
        ax.diActTorque = out.diActTorque;
        ax.driveError = out.driveError;
        ax.driveStartRealState = out.driveStartRealState;
        ax.regulatorRealState = out.regulatorRealState;

        ax.fActPosition = ax.fSetPosition;
        ax.fActVelocity = ax.fSetVelocity;
        ax.fActAcceleration = ax.fSetAcceleration;
    }
};

}

#endif

