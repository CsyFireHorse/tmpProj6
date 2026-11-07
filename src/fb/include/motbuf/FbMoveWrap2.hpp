#ifndef _ZMC_FB_MOVE_WRAP2_HPP
#define _ZMC_FB_MOVE_WRAP2_HPP
#include <cstring>

#include "AxPou.hpp"
#include "common/misc/pou_base.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_base.h"
#include "stdio.h"
#include "string.h"
#include <vector>

namespace zmc {

struct AxisMultFb {
    enum EnumFbmCmd {
        FBM_RESET,
        FBM_POWER,

        FBM_MOVE_ABSOLUTE,
        FBM_MOVE_RELATIVE,
        FBM_MOVE_ADDITIVE,
        FBM_MOVE_VELOCITY,
        FBM_MOVE_CONTINUOUS_ABSOLUTE,
        FBM_MOVE_CONTINUOUS_RELATIVE,
        FBM_GEAR_IN,
        FBM_GEAR_OUT,
        FBM_CAM_IN,
        FBM_CAM_OUT,
        FBM_HALT,
        FBM_STOP,

        FBM_MOVE_ABSOLUTE_2,
        FBM_MOVE_RELATIVE_2,
        FBM_MOVE_ADDITIVE_2,
        FBM_MOVE_VELOCITY_2,
        FBM_MOVE_CONTINUOUS_ABSOLUTE_2,
        FBM_MOVE_CONTINUOUS_RELATIVE_2,
        FBM_GEAR_IN_2,
        FBM_GEAR_OUT_2,
        FBM_CAM_IN_2,
        FBM_CAM_OUT_2,
        FBM_HALT_2,
        FBM_STOP_2,

        FBM_NUMBER
    };

public:
    void init() {
        _fbBindAxis();
        for (int i = 0; i < FBM_NUMBER; i++) {
            _apply_vel_config(i);
        }
    }
    AXIS_REF getAxisRef(){
        return axis;
    }
    void bindAxis(AXIS_REF ax) { axis = ax; }
    void setPowerState(bool state) { 
        isPowerOn = state; 
        execute[FBM_POWER] = true;
    }
    void setExecute(EnumFbmCmd cmd, bool val) { execute[cmd] = val; }
    void setBufferMode(EnumFbmCmd cmd, int16_t bufferMode) { bufferModeAxis[cmd] = bufferMode; }
    void setSelectIndex(EnumFbmCmd cmd) { fbIdx = cmd; }
    void setSelectIndex2(EnumFbmCmd cmd) { fbIdx2 = cmd; }
    void setPosOrDist(double posOrDist) {
        PosOrDist = posOrDist;
    }
    void setPosOrDist2(double posOrDist) {
        PosOrDist2 = posOrDist;
    }
    void setVelAccDecJerk(double velocity, double acceleration, double deceleration, double jerk) {
        Velocity = velocity;
        Acceleration = acceleration;
        Deceleration = deceleration;
        Jerk = jerk;
    }
    void setVelAccDecJerk2(double velocity, double acceleration, double deceleration, double jerk) {
        Velocity2 = velocity;
        Acceleration2 = acceleration;
        Deceleration2 = deceleration;
        Jerk2 = jerk;
    }
    void setDecelerationHalt(double deceleration) {
        DecelerationHalt = deceleration;
    }
    void setDecelerationHalt2(double deceleration) {
        DecelerationHalt2 = deceleration;
    }

    void runCycle() {
        preRunCycle();
        postRunCycle();
    }
    void preRunCycle() {
        power.bRegulatorOn = isPowerOn;
        power.bDriveStart = isPowerOn;
        _apply_vel_config(fbIdx);
        _applyAllBufferMode();
        applyAllExecute();

        mReset.runCycle();
        power.runCycle();
        moveAbs.runCycle();
        moveRel.runCycle();
        moveAdd.runCycle();
        moveVel.runCycle();
        moveContAbs.runCycle();
        moveContRel.runCycle();
        mGearIn.runCycle();
        mGearOut.runCycle();
        mCamIn.runCycle();
        mCamOut.runCycle();
        mHalt.runCycle();
        mStop.runCycle();
    }
    void postRunCycle() {
        _apply_vel_config(fbIdx2);
        _applyAllBufferMode();
        applyAllExecute();
        
        moveAbs2.runCycle();
        moveRel2.runCycle();
        moveAdd2.runCycle();
        moveVel2.runCycle();
        moveContAbs2.runCycle();
        moveContRel2.runCycle();
        mGearIn2.runCycle();
        mGearOut2.runCycle();
        mCamIn2.runCycle();
        mCamOut2.runCycle();
        mHalt2.runCycle();
        mStop2.runCycle();
    }
private:
    void _fbBindAxis() {
        power.Axis = axis;
        mReset.Axis = axis;
        moveAbs.Axis = axis;
        moveRel.Axis = axis;
        moveAdd.Axis = axis;
        moveVel.Axis = axis;
        moveContAbs.Axis = axis;
        moveContRel.Axis = axis;
        mGearIn.Slave = axis;
        mGearIn.Master = masterAxis;
        mGearOut.Slave = axis;
        mCamIn.Slave = axis;
        mCamIn.Master = masterAxis;
        mCamOut.Slave = axis;
        mHalt.Axis = axis;
        mStop.Axis = axis;
        moveAbs2.Axis = axis;
        moveRel2.Axis = axis;
        moveAdd2.Axis = axis;
        moveVel2.Axis = axis;
        moveContAbs2.Axis = axis;
        moveContRel2.Axis = axis;
        mGearIn2.Slave = axis;
        mGearIn2.Master = masterAxis;
        mGearOut2.Slave = axis;
        mCamIn2.Master = masterAxis;
        mCamIn2.Slave = axis;
        mCamOut2.Slave = axis;
        mHalt2.Axis = axis;
        mStop2.Axis = axis;
    }
    void _apply_vel_config(int cmd) {
        switch (cmd) {
            case FBM_MOVE_ABSOLUTE:
                setFbDistance(&moveAbs, PosOrDist);
                set_fb_vel_config(moveAbs, Velocity, Acceleration, Deceleration, Jerk);
                break;
            case FBM_MOVE_RELATIVE:
                setFbDistance(&moveRel, PosOrDist);
                set_fb_vel_config(moveRel, Velocity, Acceleration, Deceleration, Jerk);
                break;
            case FBM_MOVE_ADDITIVE:
                setFbDistance(&moveAdd, PosOrDist);
                set_fb_vel_config(moveAdd, Velocity, Acceleration, Deceleration, Jerk);
                break;
            case FBM_MOVE_VELOCITY:
                set_fb_vel_config(moveVel, Velocity, Acceleration, Deceleration, Jerk);
                break;
            case FBM_MOVE_CONTINUOUS_ABSOLUTE:
                setFbDistance(&moveContAbs, PosOrDist);
                set_fb_vel_config(moveContAbs, Velocity, Acceleration, Deceleration, Jerk);
                break;
            case FBM_MOVE_CONTINUOUS_RELATIVE:
                setFbDistance(&moveContRel, PosOrDist);
                set_fb_vel_config(moveContRel, Velocity, Acceleration, Deceleration, Jerk);
                break;
            case FBM_GEAR_IN:
                set_fb_acc_config(mGearIn, Acceleration, Deceleration, Jerk);
                break;
            case FBM_GEAR_OUT:
                break;
            case FBM_CAM_IN:
                set_fb_acc_config(mCamIn, Acceleration, Deceleration, Jerk);
                break;
            case FBM_CAM_OUT:
                break;
            case FBM_HALT:
                mHalt.Deceleration = DecelerationHalt;
                mHalt.Jerk = Jerk;
                break;
            case FBM_STOP:
                mStop.Deceleration = DecelerationHalt;
                mStop.Jerk = Jerk;
                break;
            case FBM_MOVE_ABSOLUTE_2:
                setFbDistance(&moveAbs2, PosOrDist2);
                set_fb_vel_config(moveAbs2, Velocity2, Acceleration2, Deceleration2, Jerk2);
                break;
            case FBM_MOVE_RELATIVE_2:
                setFbDistance(&moveRel2, PosOrDist2);
                set_fb_vel_config(moveRel2, Velocity2, Acceleration2, Deceleration2, Jerk2);
                break;
            case FBM_MOVE_ADDITIVE_2:
                setFbDistance(&moveAdd2, PosOrDist2);
                set_fb_vel_config(moveAdd2, Velocity2, Acceleration2, Deceleration2, Jerk2);
                break;
            case FBM_MOVE_VELOCITY_2:
                set_fb_vel_config(moveVel2, Velocity2, Acceleration2, Deceleration2, Jerk2);
                break;
            case FBM_MOVE_CONTINUOUS_ABSOLUTE_2:
                setFbDistance(&moveContAbs2, PosOrDist2);
                set_fb_vel_config(moveContAbs2, Velocity2, Acceleration2, Deceleration2, Jerk2);
                break;
            case FBM_MOVE_CONTINUOUS_RELATIVE_2:
                setFbDistance(&moveContRel2, PosOrDist2);
                set_fb_vel_config(moveContRel2, Velocity2, Acceleration2, Deceleration2, Jerk2);
                break;
            case FBM_GEAR_IN_2:
                set_fb_acc_config(mGearIn2, Acceleration2, Deceleration2, Jerk2);
                break;
            case FBM_GEAR_OUT_2:
                break;
            case FBM_CAM_IN_2:
                set_fb_acc_config(mCamIn2, Acceleration2, Deceleration2, Jerk2);
                break;
            case FBM_CAM_OUT_2:
                break;
            case FBM_HALT_2:
                mHalt2.Deceleration = DecelerationHalt2;
                mHalt2.Jerk = Jerk2;
                break;
            case FBM_STOP_2:
                mStop2.Deceleration = DecelerationHalt2;
                mStop2.Jerk = Jerk2;
                break;
            default:
                break;
        }
    }

    void applyAllExecute() {
        for (int i = 0; i < FBM_NUMBER; i++) {
            _applyExecute((EnumFbmCmd)i);
        }
    }

    void _applyExecute(EnumFbmCmd cmd) {
        switch (cmd) {
            case FBM_POWER:
                power.Enable = execute[FBM_POWER];
                break;
            case FBM_RESET:
                mReset.Execute = execute[FBM_RESET];
                break;
            case FBM_MOVE_ABSOLUTE:
                moveAbs.Execute = execute[FBM_MOVE_ABSOLUTE];
                break;
            case FBM_MOVE_RELATIVE:
                moveRel.Execute = execute[FBM_MOVE_RELATIVE];
                break;
            case FBM_MOVE_ADDITIVE:
                moveAdd.Execute = execute[FBM_MOVE_ADDITIVE];
                break;
            case FBM_MOVE_VELOCITY:
                moveVel.Execute = execute[FBM_MOVE_VELOCITY];
                break;
            case FBM_MOVE_CONTINUOUS_ABSOLUTE:
                moveContAbs.Execute = execute[FBM_MOVE_CONTINUOUS_ABSOLUTE];
                break;
            case FBM_MOVE_CONTINUOUS_RELATIVE:
                moveContRel.Execute = execute[FBM_MOVE_CONTINUOUS_RELATIVE];
                break;
            case FBM_GEAR_IN:
                mGearIn.Execute = execute[FBM_GEAR_IN];
                break;
            case FBM_GEAR_OUT:
                mGearOut.Execute = execute[FBM_GEAR_OUT];
                break;
            case FBM_CAM_IN:
                mCamIn.Execute = execute[FBM_CAM_IN];
                break;
            case FBM_CAM_OUT:
                mCamOut.Execute = execute[FBM_CAM_OUT];
                break;
            case FBM_HALT:
                mHalt.Execute = execute[FBM_HALT];
                break;
            case FBM_STOP:
                mStop.Execute = execute[FBM_STOP];
                break;
            case FBM_MOVE_ABSOLUTE_2:
                moveAbs2.Execute = execute[FBM_MOVE_ABSOLUTE_2];
                break;
            case FBM_MOVE_RELATIVE_2:
                moveRel2.Execute = execute[FBM_MOVE_RELATIVE_2];
                break;
            case FBM_MOVE_VELOCITY_2:
                moveVel2.Execute = execute[FBM_MOVE_VELOCITY_2];
                break;
            case FBM_MOVE_ADDITIVE_2:
                moveAdd2.Execute = execute[FBM_MOVE_ADDITIVE_2];
                break;
            case FBM_MOVE_CONTINUOUS_ABSOLUTE_2:
                moveContAbs2.Execute = execute[FBM_MOVE_CONTINUOUS_ABSOLUTE_2];
                break;
            case FBM_MOVE_CONTINUOUS_RELATIVE_2:
                moveContRel2.Execute = execute[FBM_MOVE_CONTINUOUS_RELATIVE_2];
                break;
            case FBM_GEAR_IN_2:
                mGearIn2.Execute = execute[FBM_GEAR_IN_2];
                break;
            case FBM_GEAR_OUT_2:
                mGearOut2.Execute = execute[FBM_GEAR_OUT_2];
                break;
            case FBM_CAM_IN_2:
                mCamIn2.Execute = execute[FBM_CAM_IN_2];
                break;
            case FBM_CAM_OUT_2:
                mCamOut2.Execute = execute[FBM_CAM_OUT_2];
                break;
            case FBM_HALT_2:
                mHalt2.Execute = execute[FBM_HALT_2];
                break;
            case FBM_STOP_2:
                mStop2.Execute = execute[FBM_STOP_2];
                break;
            default:
                break;
        }
    }
    void _applyAllBufferMode() {
        for (int i = 0; i < FBM_NUMBER; i++) {
            _applyBufferMode((EnumFbmCmd)i);
        }
    }
    void _applyBufferMode(EnumFbmCmd cmd) {
        switch (cmd) {
            case FBM_MOVE_ABSOLUTE:
                moveAbs.BufferMode = (MC_BUFFER_MODE)bufferModeAxis[FBM_MOVE_ABSOLUTE];
                break;
            case FBM_MOVE_RELATIVE:
                moveRel.BufferMode = (MC_BUFFER_MODE)bufferModeAxis[FBM_MOVE_RELATIVE];
                break;
            case FBM_MOVE_VELOCITY:
                moveVel.BufferMode = (MC_BUFFER_MODE)bufferModeAxis[FBM_MOVE_VELOCITY];
                break;
            case FBM_MOVE_ABSOLUTE_2:
                moveAbs2.BufferMode = (MC_BUFFER_MODE)bufferModeAxis[FBM_MOVE_ABSOLUTE_2];
                break;
            case FBM_MOVE_RELATIVE_2:
                moveRel2.BufferMode = (MC_BUFFER_MODE)bufferModeAxis[FBM_MOVE_RELATIVE_2];
                break;
            case FBM_MOVE_VELOCITY_2:
                moveVel2.BufferMode = (MC_BUFFER_MODE)bufferModeAxis[FBM_MOVE_VELOCITY_2];
                break;
            default:
                break;
        }
    }

    // private:
public:
    AXIS_REF axis = nullptr;  // also slave axis
    AXIS_REF masterAxis = nullptr;
    FbtPower power;
    FbtReset mReset;

    FbtMoveAbsolute moveAbs;
    FbtMoveRelative moveRel;
    FbtMoveAdditive moveAdd;
    FbtMoveVelocity moveVel;
    FbtMoveContinuousAbsolute moveContAbs;
    FbtMoveContinuousRelative moveContRel;
    FbtGearIn mGearIn;
    FbtGearOut mGearOut;
    FbtCamIn mCamIn;
    FbtCamOut mCamOut;
    FbtHalt mHalt;
    FbtStop mStop;

    FbtMoveAbsolute moveAbs2;
    FbtMoveRelative moveRel2;
    FbtMoveAdditive moveAdd2;
    FbtMoveVelocity moveVel2;
    FbtMoveContinuousAbsolute moveContAbs2;
    FbtMoveContinuousRelative moveContRel2;
    FbtGearIn mGearIn2;
    FbtGearOut mGearOut2;
    FbtCamIn mCamIn2;
    FbtCamOut mCamOut2;
    FbtHalt mHalt2;
    FbtStop mStop2;
private:
    bool isPowerOn = false;
    EnumFbmCmd fbIdx = FBM_MOVE_ABSOLUTE;
    double PosOrDist = 5;
    double Velocity = 5;
    double Acceleration = 10;
    double Deceleration = 10;
    double Jerk = 0;
    double DecelerationHalt = 20;

    EnumFbmCmd fbIdx2 = FBM_MOVE_ABSOLUTE_2;
    double PosOrDist2 = 5;
    double Velocity2 = 5;
    double Acceleration2 = 10;
    double Deceleration2 = 10;
    double Jerk2 = 0;
    double DecelerationHalt2 = 20;

    bool execute[FBM_NUMBER] = {false};
    int16_t bufferModeAxis[FBM_NUMBER] = {0};
};


class MoveGrp : public PouTaskBase {
   public:
    template <typename T>
    void install(const T& args) {
        // Helper lambda to get values from args
        auto get_i = [&](const char* key, int default_val) -> int {
            return args[key].template get<int>(default_val);
        };
        auto get_d = [&](const char* key, double default_val) -> double {
            return args[key].template get<double>(default_val);
        };
        
        int AxNum = get_i("axis_num", 1);
        axisMultFb.resize(AxNum);
        // Get axis reference from the POU system
        for (int i = 0; i < AxNum; i++) {
            auto axisRef = AxPouTaskBinderAuto::getAxisByIndex(i);
            auto& axMuFb = axisMultFb[i];
            axMuFb.bindAxis(axisRef);
            
            // Set select index
            axMuFb.setSelectIndex((AxisMultFb::EnumFbmCmd)get_i("select", AxisMultFb::FBM_MOVE_ABSOLUTE));
            axMuFb.setSelectIndex2((AxisMultFb::EnumFbmCmd)get_i("select2", AxisMultFb::FBM_MOVE_ABSOLUTE_2));
            
            axMuFb.setPosOrDist(get_d("move1.Position", 10.0));
            axMuFb.setVelAccDecJerk(get_d("move1.Velocity", 10.0), get_d("move1.Acceleration", 10.0), get_d("move1.Deceleration", 10.0), get_d("move1.Jerk", 0.0));

            axMuFb.setPosOrDist2(get_d("move2.Position", 10.0));
            axMuFb.setVelAccDecJerk2(get_d("move2.Velocity", 10.0), get_d("move2.Acceleration", 10.0), get_d("move2.Deceleration", 10.0), get_d("move2.Jerk", 0.0));
            
            // Set deceleration halt
            axMuFb.setDecelerationHalt(get_d("decelHalt", 20.0));
            axMuFb.setDecelerationHalt2(get_d("decelHalt2", 20.0));
            
            axMuFb.init();
        }
    }

    int getIndex() const { return _index; }

    virtual void onPreCmd(){}
    virtual void onAfterPreCmd(){ }
    virtual void onPostCmd(){}
    
    unsigned int size()const { return axisMultFb.size(); }
    void setExecute(AxisMultFb::EnumFbmCmd cmd, bool val) {
        auto sz = size();
        for (int i = 0; i < sz; i++) {
            axisMultFb[i].setExecute(cmd, val);
        }
    }
    void setPowerState(int axIdx, bool state) {
        axisMultFb[axIdx].setPowerState(state);
    }
    AxisMultFb& getAxisMultFb(int idx) {
        return axisMultFb[idx];
    }
    AXIS_REF getAxisRef(int idx){
        return axisMultFb[idx].getAxisRef();
    }
    AxisImplBase* getAxRef(int idx){
        return (AxisImplBase*) axisMultFb[idx].getAxisRef();
    }
    const AxisMultFb& getAxisMultFb(int idx) const {
        return axisMultFb[idx];
    }
    
    void runCycle() override {
        onPreCmd();
        auto sz = size();
        for (int i = 0; i < sz; i++) {
            axisMultFb[i].preRunCycle();
        }
        onAfterPreCmd();
        for (int i = 0; i < sz; i++) {
            axisMultFb[i].postRunCycle();
        }
        onPostCmd();
        _index++;
    }

   private:
    std::vector<AxisMultFb> axisMultFb;
    // Configuration parameters
    int verbose_flag = 0;
    int _index = 0;
    // int extraData = 0;
};

}  // namespace zmc

#endif
