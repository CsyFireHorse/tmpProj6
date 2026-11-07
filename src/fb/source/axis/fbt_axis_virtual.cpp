#include "fbt_axis_virtual.h"
#include "fbt_axis_impl.h"
#include "fbt_axis_impl_cfg.hpp"
#include "velocity_utils.hpp"
#include "ServoAdaptorBase.hpp"
#include "ServoAdaptorVirtual.hpp"

namespace zmc{


class ServoAdapterMotEmu:public ServoAdapterVirtual{
public:
    void onInit(AxisBase& axis){
        AxisCiaConfig& cfg = axis.get_config_ref();
        cfg.is_axis_virtual = true;
        cfg.debug.eAxisMtype = AXIS_TYPE_VIRTUAL;
        axis.reload_config();
        auto& ax = *to_axis_impl(&axis);
        ax.bDriveStart = true;
        ax.bRegulatorOn = true;
        ax.driveStartRealState = true;
        ax.regulatorRealState = true;
        ax.axisStat = mcStandstill;

        ax.bind_servo_adapter(new ServoAdapterMotEmu());
    }
    void onSetConfig(AxisBase& axis, const AxisCiaConfig* pCfg){
        // axis.set_config(pCfg);
        velChk_.Ts = pCfg->cycle_time / 1e6;

        auto& ax = *to_axis_impl(&axis);
        if (AXIS_TYPE_VIRTUAL == pCfg->debug.eAxisMtype){
            ax.init_position(pCfg->debug.servo_fak_pos);
        }
    }   
    // void onAfterRunCycle(AxisBase& axis) {
    //     return onFakRunCycle(axis);
    // }
    int check(double pos, double vel, double acc){
        return velChk_.check(pos, vel, acc);
    }
    VelOnlineChecker velChk_;
};


using AxisVirtualImpl= MixinAxisServoAdapter<ServoAdapterVirtual, AxisBase>;

AxisVirtual::AxisVirtual(){
    ((AxisVirtualImpl&)*this).onInit();
}
void AxisVirtual::set_config(const AxisCiaConfig* cfg){
    ((AxisVirtualImpl&)*this).set_config(cfg);
}
void AxisVirtual::reload_config(){
    ((AxisVirtualImpl&)*this).reload_config();
}


void AxisVirtual::runCycle() {
    ((AxisVirtualImpl&)*this).runCycle();
}

using  AxisMotEmuImpl= MixinAxisServoAdapter<ServoAdapterMotEmu, AxisBase>;

AxisMotEmu::AxisMotEmu(){
    ((AxisMotEmuImpl&)*this).onInit();
}

void AxisMotEmu::set_config(const AxisCiaConfig* cfg){
    ((AxisMotEmuImpl&)*this).set_config(cfg);
}

void AxisMotEmu::reload_config(){
    ((AxisMotEmuImpl&)*this).reload_config();
}

void AxisMotEmu::runCycle() {
    ((AxisMotEmuImpl&)*this).runCycle();
}

int AxisMotEmu::check(double pos, double vel, double acc){
    return ((AxisMotEmuImpl&)*this).get_servo_adapter().check(pos, vel, acc);
}
int AxisMotEmu::check_set_pva(){
    auto& ax = *to_axis_impl(this);
    auto& adp = ((AxisMotEmuImpl&)*this).get_servo_adapter();
    adp.velChk_.set_state(ax.axisStat);
    return adp.check(ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
}
}