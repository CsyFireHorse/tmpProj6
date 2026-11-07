#include "fbt_axis_cia402.h"
#include "fbt_axis.h"
#include "ServoCmdDef.hpp"
#include "common/ecat/Cia402Easy.hpp"
#include "ServoFakerToy.h"
#include "fbt_axis_impl.h"
#include "fbt_axis_impl_cfg.hpp"
#include "Cia402Ex.h"
#include "ServoAdaptorBase.hpp"
#include "ServoAdaptorVirtual.hpp"

namespace zmc{

inline Cia402Easy::axis_cmd_t DriveCtrlcmd2Cia402Cmd(DriveCtrlCmd cmd) {
    switch (cmd) {
        case DRIVE_CTRL_CMD_RESET:
            return Cia402Easy::axis_cmd_alarm_clear;
        case DRIVE_CTRL_CMD_POWEROFF:
            return Cia402Easy::axis_cmd_power_off;
        case DRIVE_CTRL_CMD_POWERON:
            return Cia402Easy::axis_cmd_ready_poweron_enable;
        case DRIVE_CTRL_CMD_QUICKSTOP:
            return Cia402Easy::axis_cmd_quick_stop;
        case DRIVE_CTRL_CMD_HOME:
            return Cia402Easy::axis_cmd_poweroff_home;
        case DRIVE_CTRL_CMD_MOVE:
            return Cia402Easy::axis_cmd_enable;
        case DRIVE_CTRL_CMD_NOP:
        default:
            return Cia402Easy::axis_cmd_nop;
    }
}

template<typename T>
inline const PdoEnableFlag& get_pdo_flag(const T* pAxis) {
    return static_cast<AxisInnerContent*>(to_axis_impl(pAxis)->native)->axisCfg.pdo_flag;
}


void AxisCia402::set_input(const servo_stat* inp) {
    auto pAxis = to_axis_impl(this);
    const auto& pdoEnFlag = get_pdo_flag(this);
    if (pAxis->bSimulation) return;
    if (pdoEnFlag.enable_status_word) {
        pAxis->wStatusWord = inp->status_word;
    }
    if (pdoEnFlag.enable_actual_postion) pAxis->diActPosition = inp->actual_position;
    if (pdoEnFlag.enable_actual_velocity) pAxis->diActVelocity = inp->actual_velocity;
    if (pdoEnFlag.enable_actual_torque) pAxis->diActTorque = (inp->actual_torque);
    if (pdoEnFlag.enable_modes_of_operation_display)
        pAxis->byRealControllerMode =  (E_OP_MODE_TO_CTRL_MODE - (int)inp->mode_of_operation_display);
}

void AxisCia402::get_output(servo_ctrl* oup) const {
    auto pAxis = to_axis_impl(this);
    const auto& pdoEnFlag = get_pdo_flag(this);

    if (pdoEnFlag.enable_control_word) {
        oup->control_word  = pAxis->wControlWord;
    }
    if (pdoEnFlag.enable_set_position) oup->set_position = pAxis->diSetPosition;
    if (pdoEnFlag.enable_set_velocity) oup->set_velocity = pAxis->diSetVelocity;
    if (pdoEnFlag.enable_set_torque) oup->set_torque = pAxis->diSetTorque;
    if (pdoEnFlag.enable_modes_of_operation) oup->mode_of_operation = E_OP_MODE_TO_CTRL_MODE - pAxis->byControllerMode;
};

class ServoAdapterCia402:public ServoAdapterVirtual{
public:
    void onInit(AxisBase& axis){
        axisInit<ServoAdapterCia402, AXIS_TYPE_CIA402, false>(axis);
    }

    void onBeforeRunCycle(AxisBase& axis) {
        auto pAxis = (AxisImpl*)&axis;
        const auto& pdoEnFlag = get_pdo_flag(&axis);
        auto sm = Cia402Easy::get_sm(pAxis->wStatusWord);
        pAxis->driveError = sm == Cia402Easy::axis_sm_fault_reaction_active || sm == Cia402Easy::axis_sm_fault;
        pAxis->regulatorRealState = sm == Cia402Easy::axis_sm_operation_enabled || sm == Cia402Easy::axis_sm_quick_stop_active;
        pAxis->driveStartRealState = sm == Cia402Easy::axis_sm_operation_enabled;
        Cia402SmEx smEx = {sm, getHomeHaltStatus(pAxis->wStatusWord)};
        pAxis->swSmEx = smEx.val;
    }
    void onAfterRunCycle(AxisBase& axis) {
        auto pAxis = (AxisImpl*)&axis;
        auto cmd = (DriveCtrlCmd)pAxis->cmd;
        pAxis->wControlWord = Cia402Easy::get_ctrl(DriveCtrlcmd2Cia402Cmd((DriveCtrlCmd) cmd), pAxis->wStatusWord);
        // return onFakRunCycle(axis);
    }
    void onFakRunCycle(AxisBase& axis) {
        auto& ax = *to_axis_impl(&axis);
        if (!ax.bSimulation) return;
        if (ax.m_cycleCounter < 2) return;
        ServoAdapterVirtual::onFakRunCycle(axis);
        ax.wStatusWord = ax.wControlWord? Cia402Easy::STATWORD_OPERATION_ENABLED:0;
        // ax.driveError = false;
    }
};


class ServoAdapterCia402Fak:public ServoAdapterCia402{
public:
    ServoFakerToy srvFak;
    void onInit(AxisBase& axis){
        axisInit<ServoAdapterCia402Fak, AXIS_TYPE_CIA402_SIMULATED, true>(axis);
    }
    void onSetFakConfig(AxisBase& axis, const AxisCiaConfig* pCfg){
        auto& cfg = *pCfg;
        if (!cfg.is_axis_virtual) return;
        
        ServoFakerToyCfg scfg;
        scfg.cycleTime = cfg.cycle_time*1e-6;
        scfg.posInit = cfg.debug.servo_fak_pos;
        scfg.smInit = cfg.debug.servo_fak_statw_sm;
        scfg.rcfilter_scale = cfg.debug.servo_fak_rcfilter_scale;
        scfg.decel = 1000;

        // auto& ax = *to_axis_impl(&axis);
        srvFak.set_config(scfg);
    }

    void onFakRunCycle(AxisBase& axis) {
        auto pAxis = (AxisImpl*)&axis;
        if (!pAxis->bSimulation) return;

        const auto& pdoEnFlag = get_pdo_flag(&axis);
        if (pdoEnFlag.enable_control_word) {
            srvFak.wCtrlword = pAxis->wControlWord;
        }
        if (pdoEnFlag.enable_set_position)
            srvFak.diSetPosition = pAxis->diSetPosition;
        else if (pdoEnFlag.enable_set_velocity)
            srvFak.diSetVelocity = pAxis->diSetVelocity;
        else if (pdoEnFlag.enable_set_torque)
            srvFak.diSetTorque = pAxis->diSetTorque;
        if (pdoEnFlag.enable_modes_of_operation) 
            srvFak.byOpMode = E_OP_MODE_TO_CTRL_MODE - pAxis->byControllerMode;

        srvFak.runCycle();

        if (pdoEnFlag.enable_status_word) pAxis->wStatusWord = srvFak.wStatusword;
        if (pdoEnFlag.enable_actual_postion) pAxis->diActPosition = srvFak.diActPosition;
        if (pdoEnFlag.enable_actual_velocity) pAxis->diActVelocity = srvFak.diActVelocity;
        if (pdoEnFlag.enable_actual_torque)
            pAxis->diActTorque = srvFak.diActTorque;
        if (pdoEnFlag.enable_modes_of_operation_display)
            pAxis->byRealControllerMode = E_OP_MODE_TO_CTRL_MODE - srvFak.byOpModeDisplay;
    }
};


using  AxisCia402Impl = MixinAxisServoAdapter<ServoAdapterCia402, AxisBase>;


AxisCia402::AxisCia402(){
    ((AxisCia402Impl&)*this).onInit();
}
void AxisCia402::set_config(const AxisCiaConfig* config){
    ((AxisCia402Impl&)*this).set_config(config);
}
void AxisCia402::reload_config(){
    ((AxisCia402Impl&)*this).reload_config();
}

void AxisCia402::runCycle() {
    ((AxisCia402Impl&)*this).runCycle();
}

using  AxisCia402FakImpl = MixinAxisServoAdapter<ServoAdapterCia402Fak, AxisBase>;
AxisCia402Fak::AxisCia402Fak(){
    ((AxisCia402FakImpl&)*this).onInit();
}
void AxisCia402Fak::set_config(const AxisCiaConfig* config){
    ((AxisCia402FakImpl&)*this).set_config(config);
}
void AxisCia402Fak::reload_config(){
    ((AxisCia402FakImpl&)*this).reload_config();
}

void AxisCia402Fak::runCycle() {
    ((AxisCia402FakImpl&)*this).runCycle();
}
ServoFakerToy* get_axis_cia402_servo_fak(AxisCia402Fak* pAxis){
    return &(static_cast<AxisCia402FakImpl*>((void*) pAxis)->get_servo_adapter().srvFak);
}

class ServoAdapterCia402Virt:public ServoAdapterCia402{
public:
    void onInit(AxisBase& axis){
        axisInit<ServoAdapterCia402Virt, AXIS_TYPE_CIA402, true>(axis);
    }
};

using  AxisCia402VirtImpl = MixinAxisServoAdapter<ServoAdapterCia402Virt, AxisBase>;
AxisCia402Virt::AxisCia402Virt(){
    ((AxisCia402VirtImpl&)*this).onInit();
}

}