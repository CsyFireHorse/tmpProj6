/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-01
@ArtifactStatusAuthority GPT-5 Codex
*/

#include <stdio.h>
#include "fbt_fb_base.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_impl.h"
#include "common/misc/TrapzVelToyForwarder.hpp"
#include "fbt_move_impl.hpp"
#include "VelGearFollower.h"
#include "VelPosFollower.h"

namespace zmc {


class FbtImplFollowPosition
    : public FbExeLevelAbortingTmpl<FbtImplFollowPosition, FbtFollowPositionStruct> {
   public:
    double posZeroPoint = 0;
    inline MC_AXIS_STATES getAxisStateWhenActivated()const{
        return mcSynchronizedMotion;
    }
    inline AxisStateDetail getAxisStateDetailWhenActivated() const {
        return AXIS_STATE_DETAIL_FOLLOW;
    }
    void onPlan(){
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        if (ax.is_modulo_type()) {
            posZeroPoint = ax.calc_modulo(ax.fSavePosition) - ax.fSavePosition;
        }else{
            posZeroPoint = 0;
        }
    }
    void onForward() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);

        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition = posZeroPoint + param.fSetPosition;
        ax.fSetVelocity = (ax.fSetPosition - ax.fSavePosition) / ax.cycleTime;
        ax.fSetAcceleration = (ax.fSetVelocity - ax.fSaveVelocity) / ax.cycleTime;
    }

    void onClear() {
        FbtBase::onClear();
        posZeroPoint = 0;
    }
    inline void onCapture(){
        this->execute = this->pParam->bExecute;
    }
    inline void onUpdate(){
        this->pParam->bBusy = this->isBusy();
        this->pParam->bCommandAborted = this->isAborted();
        this->pParam->bError = this->isError();
        this->pParam->iErrorID = this->getErrorID();
    }
};

FbtFollowPosition::FbtFollowPosition() {
    make_impl<FbtImplFollowPosition, FbtFollowPositionStruct>(*this);
}

FbtFollowPosition::~FbtFollowPosition() { 
    if (pImpl) {
        delete static_cast<FbtImplFollowPosition*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtFollowPosition::runCycle() {
    auto impl = static_cast<FbtImplFollowPosition*>(pImpl);
    impl->runCycle();
}


class FbtImplFollowVelocity
    : public FbExeLevelAbortingTmpl<FbtImplFollowVelocity, FbtFollowVelocityStruct> {
   public:
    inline MC_AXIS_STATES getAxisStateWhenActivated()const{
        return mcSynchronizedMotion;
    }
    inline AxisStateDetail getAxisStateDetailWhenActivated() const {
        return AXIS_STATE_DETAIL_FOLLOW;
    }
    void onForward() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);

        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition = ax.fSavePosition + param.fSetVelocity * ax.cycleTime;
        ax.fSetVelocity = param.fSetVelocity;
        ax.fSetAcceleration = (ax.fSetVelocity - ax.fSaveVelocity) / ax.cycleTime;
    }

    inline void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
    }
    inline void onCapture(){
        this->execute = this->pParam->bExecute;
    }
    inline void onUpdate(){
        this->pParam->bBusy = this->isBusy();
        this->pParam->bCommandAborted = this->isAborted();
        this->pParam->bError = this->isError();
        this->pParam->iErrorID = this->getErrorID();
    }
};

FbtFollowVelocity::FbtFollowVelocity(){
    make_impl<FbtImplFollowVelocity, FbtFollowVelocityStruct>(*this);
}
FbtFollowVelocity::~FbtFollowVelocity(){
    if (pImpl) {
        delete static_cast<FbtImplFollowVelocity*>(pImpl);
        pImpl = nullptr;
    }
}
void FbtFollowVelocity::runCycle(){
    if (pImpl) {
        auto impl = static_cast<FbtImplFollowVelocity*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplFollowPositionVelocity
    : public FbExeLevelAbortingTmpl<FbtImplFollowPositionVelocity, FbtFollowPositionVelocityStruct> {
   public:
    inline MC_AXIS_STATES getAxisStateWhenActivated()const{
        return mcSynchronizedMotion;
    }
    inline AxisStateDetail getAxisStateDetailWhenActivated() const {
        return AXIS_STATE_DETAIL_FOLLOW;
    }
    double posZeroPoint = 0;
    void onPlan(){
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        if (ax.is_modulo_type()) {
            posZeroPoint = ax.calc_modulo(ax.fSavePosition) - ax.fSavePosition;
        }else{
            posZeroPoint = 0;
        }
    }
    void onForward() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);

        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition = posZeroPoint + param.fSetPosition;
        ax.fSetVelocity = param.fSetVelocity;
        ax.fSetAcceleration = (ax.fSetVelocity - ax.fSaveVelocity) / ax.cycleTime;
    }
    void onClear() {
        FbtBase::onClear();
        posZeroPoint = 0;
    }
    inline void onCapture(){
        this->execute = this->pParam->bExecute;
    }
    inline void onUpdate(){
        this->pParam->bBusy = this->isBusy();
        this->pParam->bCommandAborted = this->isAborted();
        this->pParam->bError = this->isError();
        this->pParam->iErrorID = this->getErrorID();
    }
};

FbtFollowPositionVelocity::FbtFollowPositionVelocity(){
    make_impl<FbtImplFollowPositionVelocity, FbtFollowPositionVelocityStruct>(*this);
}
FbtFollowPositionVelocity::~FbtFollowPositionVelocity(){
    if (pImpl) {
        delete static_cast<FbtImplFollowPositionVelocity*>(pImpl);
        pImpl = nullptr;
    }
}
void FbtFollowPositionVelocity::runCycle(){
    if (pImpl) {
        auto impl = static_cast<FbtImplFollowPositionVelocity*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplFollowSetValues
    : public FbExeLevelAbortingTmpl<FbtImplFollowSetValues, FbtFollowSetValuesStruct> {
   public:
    double posZeroPoint = 0;
    inline MC_AXIS_STATES getAxisStateWhenActivated()const{
        return mcSynchronizedMotion;
    }
    inline AxisStateDetail getAxisStateDetailWhenActivated() const {
        return AXIS_STATE_DETAIL_FOLLOW;
    }
    void onPlan(){
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        if (ax.is_modulo_type()) {
            posZeroPoint = ax.calc_modulo(ax.fSavePosition) - ax.fSavePosition;
        }else{
            posZeroPoint = 0;
        }
    }
    void onForward() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);

        // Check for abort signal
        if (param.bAbort) {
            this->fbStat = FB_STAT_ABORTED;
            // this->onAbort();
            return;
        }

        ax.cmd = AXIS_CMD_MOTION;
        
        // Apply values based on dwValueMask bits
        // Bit 0: fSetPosition
        if (param.dwValueMask & 0x01) {
            ax.fSetPosition = posZeroPoint + param.fSetPosition;
        }
        
        // Bit 1: fSetVelocity  
        if (param.dwValueMask & 0x02) {
            ax.fSetVelocity = param.fSetVelocity;
        }
        
        // Bit 2: fSetAcceleration
        if (param.dwValueMask & 0x04) {
            ax.fSetAcceleration = param.fSetAcceleration;
        }
        
        // Bit 3: fSetJerk - Note: axis doesn't have fSetJerk member, skip for now
        // if (param.dwValueMask & 0x08) {
        //     ax.fSetJerk = param.fSetJerk;
        // }
        
        // Bit 4: fSetTorque
        if (param.dwValueMask & 0x10) {
            ax.fSetTorque = param.fSetTorque;
        }
        
        // Bit 5: fSetCurrent - Note: axis doesn't have fSetCurrent member, skip for now
        // if (param.dwValueMask & 0x20) {
        //     ax.fSetCurrent = param.fSetCurrent;
        // }
        
        // Calculate derived values if not explicitly set
        // If position is not explicitly set but velocity is, calculate position
        if (!(param.dwValueMask & 0x01) && (param.dwValueMask & 0x02)) {
            ax.fSetPosition = ax.fSavePosition + ax.fSetVelocity * ax.cycleTime;
        }
        
        // If acceleration is not explicitly set but velocity is, calculate acceleration
        if (!(param.dwValueMask & 0x04) && (param.dwValueMask & 0x02)) {
            ax.fSetAcceleration = (ax.fSetVelocity - ax.fSaveVelocity) / ax.cycleTime;
        }
    }

    void onClear() {
        FbtBase::onClear();
        posZeroPoint = 0;
    }
    
    inline void onCapture(){
        this->execute = this->pParam->bExecute;
    }
    
    inline void onUpdate(){
        this->pParam->bBusy = this->isBusy();
        this->pParam->bCommandAborted = this->isAborted();
        this->pParam->bError = this->isError();
        this->pParam->iErrorID = this->getErrorID();
    }
};

FbtFollowSetValues::FbtFollowSetValues(){
    make_impl<FbtImplFollowSetValues, FbtFollowSetValuesStruct>(*this);
}

FbtFollowSetValues::~FbtFollowSetValues(){
    if (pImpl) {
        delete static_cast<FbtImplFollowSetValues*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtFollowSetValues::runCycle(){
    if (pImpl) {
        auto impl = static_cast<FbtImplFollowSetValues*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplTrackAxis 
    : public FbExeAbortingTmpl<FbtImplTrackAxis, FbtTrackAxisStruct> {
public:
    VelPvaFollower posFollower;
    double posZeroPoint = 0;
    double slvPosZeroPt = 0;
    FbtImplTrackAxis(){
        fbType = FB_TYPE_TRACKAXIS;
    }
    inline MC_AXIS_STATES getAxisStateWhenActivated() const {
        return mcSynchronizedMotion;
    }

    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!this->get_axis_ref()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate(this->getAxisStateWhenActivated());
    }

    void onPlan(){
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(param.Slave);
        auto& ax = get_axis_impl_ref(param.Master);
        if (ax.is_modulo_type()) {
            posZeroPoint = ax.calc_modulo(ax.fSavePosition) - ax.fSavePosition;
        }else{
            posZeroPoint = 0;
        }
        if (slave.is_modulo_type()) {
            slvPosZeroPt = slave.calc_modulo(slave.fSavePosition) - slave.fSavePosition;
        }else{
            slvPosZeroPt = 0;
        }

        if (param.Velocity <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0) {
            this->onError(ZMC_INVALID_VELACCJERK_VALUES);
            return;
        }
        posFollower.set_config(param.Velocity, param.Acceleration, 
                              param.Deceleration, param.Jerk);
        posFollower.set_begin_pva(slave.fSavePosition - slvPosZeroPt, slave.fSaveVelocity, 
                                 slave.fSaveAcceleration);
        posFollower.set_Ts(slave.cycleTime);

    }
    void onOnlinePlan(){
        auto& param = *this->pParam;
        auto& master = get_axis_impl_ref(param.Master);
        auto& ax = this->getAxImpl();
        if (this->startTick == ax.m_cycleCounter + 1) return;
        this->startTick = ax.m_cycleCounter + 1;
        PosVelAcc masterPva{
            master.fSetPosition - posZeroPoint,
            master.fSetVelocity,
            master.fSetAcceleration
        };
        posFollower.follow(masterPva);
        posFollower.runCycle();
    }

    void onForward(){
        onOnlinePlan();
        _onForward();
    }
    void _onForward(){
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(param.Slave);
        // Get slave axis command
        PosVelAcc slavePva;
        int status = posFollower.get_cur_slice(slavePva);
        this->inVelocity = (status == GEAR_STATUS_GEAR_IN);

        // Update slave axis command
        slave.cmd = AXIS_CMD_MOTION;
        slave.fSetPosition = slavePva.pos + slvPosZeroPt;
        slave.fSetVelocity = slavePva.vel;
        slave.fSetAcceleration = slavePva.acc;
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->inVelocity = false;
        posZeroPoint = 0;
        slvPosZeroPt = 0;
    }

    void onCapture(){
        this->execute = this->pParam->bExecute;
    }

    void onUpdate(){
        this->pParam->bBusy = this->isBusy();
        this->pParam->bError = this->isError();
        this->pParam->bInSync = this->inVelocity;
        this->pParam->bCommandAborted = this->isAborted();
        this->pParam->iErrorID = this->getErrorID();
    }
};

FbtTrackAxis::FbtTrackAxis() {
    make_impl<FbtImplTrackAxis, FbtTrackAxisStruct>(*this);
}

FbtTrackAxis::~FbtTrackAxis() {
    if (pImpl) {
        delete static_cast<FbtImplTrackAxis*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtTrackAxis::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplTrackAxis*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplTrackSetValues
    : public FbExeLevelAbortingTmpl<FbtImplTrackSetValues, FbtTrackSetValuesStruct> {
   public:
    VelPvaFollower posFollower;

    FbtImplTrackSetValues() {
        fbType = FB_TYPE_TRACKSETVALUES;
    }

    inline MC_AXIS_STATES getAxisStateWhenActivated()const{
        return mcSynchronizedMotion;
    }

    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!this->get_axis_ref()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        onPlan();
        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onPlan() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        if (param.Velocity <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0) {
            this->onError(ZMC_INVALID_VELACCJERK_VALUES);
            return;
        }
        posFollower.set_config(param.Velocity, param.Acceleration, 
                              param.Deceleration, param.Jerk);
        posFollower.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, 
                                 ax.fSaveAcceleration);
        posFollower.set_Ts(ax.cycleTime);
    }

    void onForward() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        if (this->startTick == ax.m_cycleCounter + 1) return;
        this->startTick = ax.m_cycleCounter + 1;
        // Create master setpoint from parameters
        PosVelAcc masterPva{
            param.fSetPosition,
            param.fSetVelocity,
            param.fSetAcceleration
        };

        // Update follower
        posFollower.follow(masterPva);
        posFollower.runCycle();

        // Get and apply commands
        PosVelAcc slavePva;
        int status = posFollower.get_cur_slice(slavePva);
        this->inVelocity = (status == GEAR_STATUS_GEAR_IN);

        // Update axis commands
        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition = slavePva.pos;
        ax.fSetVelocity = slavePva.vel;
        ax.fSetAcceleration = slavePva.acc;
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->inVelocity = false;
    }

    void onCapture(){
        this->execute = this->pParam->bExecute;
    }

    void onUpdate(){
        this->pParam->bBusy = this->isBusy();
        this->pParam->bInSync = this->inVelocity;
        this->pParam->bCommandAborted = this->isAborted();
        this->pParam->bError = this->isError();
        this->pParam->iErrorID = this->getErrorID();
    }
};

FbtTrackSetValues::FbtTrackSetValues() {
    make_impl<FbtImplTrackSetValues, FbtTrackSetValuesStruct>(*this);
}

FbtTrackSetValues::~FbtTrackSetValues() { 
    if (pImpl) {
        delete static_cast<FbtImplTrackSetValues*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtTrackSetValues::runCycle() {
    auto impl = static_cast<FbtImplTrackSetValues*>(pImpl);
    impl->runCycle();
}

class FbtImplGearIn 
    : public FbExeBufTmpl<FbtImplGearIn, FbtGearInStruct> {
public:
    bool _inGear = false;
    EnumGearStatus _gear_stat = GEAR_STATUS_IDLE;
    // double _cur_time = 0;
    VelGearFollower gearFollower;
public:
    FbtImplGearIn(){
        fbType = FB_TYPE_GEARIN;
    }
    inline MC_AXIS_STATES getAxisStateWhenActivated() const {
        return mcSynchronizedMotion;
    }

    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!this->get_axis_ref() || !this->pParam->Master) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate(this->getAxisStateWhenActivated());
    }
    void onPlan(){
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(param.Slave);
        if (param.Acceleration <= 0 || param.Deceleration <= 0) {
            this->onError(ZMC_INVALID_VELACCJERK_VALUES);
            return;
        }
        gearFollower.set_config(1e99, param.Acceleration, 
                                param.Deceleration, param.Jerk);
        gearFollower.set_begin_pva(slave.fSavePosition, slave.fSaveVelocity, 
                                    slave.fSaveAcceleration);
        gearFollower.set_num_den(param.RatioNumerator, param.RatioDenominator);
        gearFollower.set_Ts(slave.cycleTime);
    }
    void onOnlinePlan(){
        auto& param = *this->pParam;
        auto& master = get_axis_impl_ref(param.Master);
        auto& ax = this->getAxImpl();
        if (this->startTick == ax.m_cycleCounter + 1) return;
        this->startTick = ax.m_cycleCounter + 1;
        PosVelAcc masterPva{
            master.fSetPosition,
            master.fSetVelocity,
            master.fSetAcceleration
        };
        gearFollower.follow(masterPva);
        gearFollower.runCycle();
    }

    void onForward(){
        onOnlinePlan();
        _onForward();
    }
    void _onForward(){
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(param.Slave);

        PosVelAcc slavePva;
        _gear_stat = (EnumGearStatus) gearFollower.get_cur_slice(slavePva);


        slave.cmd = AXIS_CMD_MOTION;
        slave.fSetPosition = slavePva.pos;
        slave.fSetVelocity = slavePva.vel;
        slave.fSetAcceleration = slavePva.acc;
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        _gear_stat = (EnumGearStatus) GEAR_STATUS_IDLE;
        auto& param = *this->pParam;
        param.InGear = false;
        param.Active = false;
    }
    void onCapture(){
        this->execute = this->pParam->Execute;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->InGear = this->_gear_stat == GEAR_STATUS_GEAR_IN;
        this->pParam->Active = this->isActive();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtGearIn::FbtGearIn() {
    make_impl<FbtImplGearIn, FbtGearInStruct>(*this);
    RatioNumerator = 1;
    RatioDenominator = 1;
}

FbtGearIn::~FbtGearIn() {
    delete static_cast<FbtImplGearIn*>(pImpl);
}

void FbtGearIn::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplGearIn*>(pImpl);
        impl->runCycle();
    }
}


class FbtImplGearInPos 
    : public FbExeBufTmpl<FbtImplGearInPos, FbtGearInPosStruct> {
public:
    bool _inGear = false;
    EnumGearStatus _gear_stat = GEAR_STATUS_IDLE;
    // double _cur_time = 0;
    VelGearFollower gearFollower;
public:
    FbtImplGearInPos(){
        fbType = FB_TYPE_GEARINPOS;
    }
    inline MC_AXIS_STATES getAxisStateWhenActivated() const {
        return mcSynchronizedMotion;
    }

    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!this->get_axis_ref() || !this->pParam->Master) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate(this->getAxisStateWhenActivated());
    }
    void onPlan(){
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(param.Slave);
        // FIXME
        // gearFollower.set_config(1e99, param.Acceleration, 
        //                         param.Deceleration, param.Jerk);
        gearFollower.set_begin_pva(slave.fSavePosition, slave.fSaveVelocity, 
                                    slave.fSaveAcceleration);
        gearFollower.set_num_den(param.RatioNumerator, param.RatioDenominator);
        gearFollower.set_Ts(slave.cycleTime);
    }
    void onForward(){
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(param.Slave);
        auto& master = get_axis_impl_ref(param.Master);
        PosVelAcc masterPva{
            master.fSetPosition,
            master.fSetVelocity,
            master.fSetAcceleration
        };

        gearFollower.follow(masterPva);
        gearFollower.runCycle();

        PosVelAcc slavePva;
        _gear_stat = (EnumGearStatus) gearFollower.get_cur_slice(slavePva);

        slave.cmd = AXIS_CMD_MOTION;
        slave.fSetPosition = slavePva.pos;
        slave.fSetVelocity = slavePva.vel;
        slave.fSetAcceleration = slavePva.acc;
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        _gear_stat = (EnumGearStatus) GEAR_STATUS_IDLE;
        auto& param = *this->pParam;
        param.InSync = false;
        param.Active = false;
    }
    void onCapture(){
        this->execute = this->pParam->Execute;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->InSync = this->_gear_stat == GEAR_STATUS_GEAR_IN;
        this->pParam->Active = this->isActive();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtGearInPos::FbtGearInPos() { 
    make_impl<FbtImplGearInPos, FbtGearInPosStruct>(*this);
    RatioNumerator = 1;
    RatioDenominator = 1;
}

FbtGearInPos::~FbtGearInPos() {
    if (pImpl) {
        delete static_cast<FbtImplGearInPos*>(pImpl);
        pImpl = nullptr;
    }
}
void FbtGearInPos::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplGearInPos*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplGearOut 
    : public FbExeAbortingTmpl<FbtImplGearOut, FbtGearOutStruct> {
public:
    FbtImplGearOut(){
        fbType = FB_TYPE_GEAROUT;
    }
    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        auto& slave = get_axis_impl_ref(this->pParam->Slave);

        if (slave.axisStat != mcSynchronizedMotion) {
            onError(ZMC_ERR_AXIS_NOT_IN_SYNC_MOTION);
            return;
        }
        if (slave.pNode.isEmpty())
            return onError(ZMC_ERR_AXIS_NOT_IN_GEAR);
        const auto& fb = slave.pNode.currentNode();
        if (fb.fbType == FB_TYPE_CAMIN)
            return onError(ZMC_ERR_AXIS_NOT_IN_GEAR);
        
        this->pushToBufferAndActivate(this->getAxisStateWhenActivated());
    }
    MC_AXIS_STATES getAxisStateWhenActivated()const{
        return mcSynchronizedMotion;
    }
    void onForward() {
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(this->pParam->Slave);

        slave.cmd = AXIS_CMD_MOTION;
        slave.fSetVelocity = slave.fSaveVelocity;
        slave.fSetPosition = slave.fSavePosition + slave.fSaveVelocity * slave.cycleTime;
        slave.fSetAcceleration = 0;

        if (slave.fSetVelocity) {
            slave.setAxisState(mcContinuousMotion, AXIS_STATE_DETAIL_GEAR_OUT);
        } else {
            slave.setAxisState(mcStandstill);
        }
        slave.pNode.pop_front();
        fbStat = FB_STAT_DONE;
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
    }
    void onCapture(){
        this->execute = this->pParam->Execute;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};


FbtGearOut::FbtGearOut() {
    make_impl<FbtImplGearOut, FbtGearOutStruct>(*this);
}
FbtGearOut::~FbtGearOut(){
    delete static_cast<FbtImplGearOut*>(pImpl);
}

void FbtGearOut::runCycle() {
    auto impl = static_cast<FbtImplGearOut*>(pImpl);
    // impl->execute = this->Execute;
    impl->runCycle();
}


class FbtImplPhasing 
    : public FbExeAbortingTmpl<FbtImplPhasing, FbtPhasingStruct> {
public:
    VelPvaFollower posFollower;
    double posZeroPoint = 0;
    double slvPosZeroPt = 0;
    double phaseShiftPos = 0;
    
    FbtImplPhasing(){
        fbType = FB_TYPE_PHASING;
    }
    
    inline MC_AXIS_STATES getAxisStateWhenActivated() const {
        return mcSynchronizedMotion;
    }

    void onPlan(){
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(param.Slave);
        auto& master = get_axis_impl_ref(param.Master);
        
        // Check for modulo type axes and calculate zero points
        if (master.is_modulo_type()) {
            posZeroPoint = master.calc_modulo(master.fSavePosition) - master.fSavePosition;
        } else {
            posZeroPoint = 0;
        }
        if (slave.is_modulo_type()) {
            slvPosZeroPt = slave.calc_modulo(slave.fSavePosition) - slave.fSavePosition;
        } else {
            slvPosZeroPt = 0;
        }

        // Validate motion parameters
        if (param.Velocity <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0) {
            this->onError(ZMC_INVALID_VELACCJERK_VALUES);
            return;
        }
        
        // Configure the position follower
        posFollower.set_config(param.Velocity, param.Acceleration, 
                              param.Deceleration, param.Jerk);
        posFollower.set_begin_pva(slave.fSavePosition - slvPosZeroPt, slave.fSaveVelocity, 
                                 slave.fSaveAcceleration);
        posFollower.set_Ts(slave.cycleTime);
        
        phaseShiftPos = param.PhaseShift;

    }    
    void onOnlinePlan(){
        auto& param = *this->pParam;
        auto& master = get_axis_impl_ref(param.Master);
        auto& ax = this->getAxImpl();
        if (this->startTick == ax.m_cycleCounter + 1) return;
        this->startTick = ax.m_cycleCounter + 1;
        
        // Create master PVA with phase shift applied
        PosVelAcc masterPva{
            master.fSetPosition - posZeroPoint + phaseShiftPos,
            master.fSetVelocity,
            master.fSetAcceleration
        };
        
        posFollower.follow(masterPva);
        posFollower.runCycle();
    }
    void onForward(){
        onOnlinePlan();
        _onForward();
    }
    void _onForward(){
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(param.Slave);
        
        // Get slave axis command from follower
        PosVelAcc slavePva;
        int status = posFollower.get_cur_slice(slavePva);
        this->inVelocity = (status == GEAR_STATUS_GEAR_IN);

        // Update slave axis command
        slave.cmd = AXIS_CMD_MOTION;
        slave.fSetPosition = slavePva.pos + slvPosZeroPt;
        slave.fSetVelocity = slavePva.vel;
        slave.fSetAcceleration = slavePva.acc;
        
        // Check if phase synchronization is complete
        if (this->inVelocity) {
            this->fbStat = FB_STAT_DONE;
        }
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->inVelocity = false;
        posZeroPoint = 0;
        slvPosZeroPt = 0;
        phaseShiftPos = 0;
    }

    void onCapture(){
        this->execute = this->pParam->Execute;
    }

    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isInXxx();
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = this->isAborted();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtPhasing::FbtPhasing() { 
    make_impl<FbtImplPhasing, FbtPhasingStruct>(*this);
}

FbtPhasing::~FbtPhasing() {
    if (pImpl) {
        delete static_cast<FbtImplPhasing*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtPhasing::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplPhasing*>(pImpl);
        impl->runCycle();
    }
}

// FIXME
FbtSetTorque::FbtSetTorque(){
    all_zero_init<FbtSetTorqueStruct>(*this);
}
FbtSetTorque::~FbtSetTorque(){}
void FbtSetTorque::runCycle(){
    if (bEnable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(this->Axis);
            if (ax.axisStat == mcDisabled || ax.axisStat == mcErrorStop) {
                nErrorID = ZMC_FB_ACTIVE_AXIS_DISABLED;
                bError = true;
                bBusy = false;
                return;
            } else if (ax.axisStat == mcStopping) {
                nErrorID = ZMC_AXIS_NOT_READY_FOR_MOTION;
                bError = true;
                bBusy = false;
                return;
            }else if (ax.axisStat == mcHoming) {
                nErrorID = ZMC_AXIS_NOT_READY_FOR_MOTION;
                bError = true;
                bBusy = false;
                return;
            }
            // TODO pushToBuffer?
            // How to quit?
            ax.cmd = AXIS_CMD_MOTION;
            ax.setAxisState(mcSynchronizedMotion);
            ax.fSetTorque = this->fTorque;
            bBusy = true;
            bError = false;
            nErrorID = ZMC_NO_ERROR;
        } else {
            nErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            bError = true;
            bBusy = false;
        }
    } else {
        bBusy = false;
        bError = false;
        nErrorID = ZMC_NO_ERROR;
    }
}

// Empty implementation class for BacklashCompensation
class FbtImplBacklashCompensation {
public:
    FbtBacklashCompensationStruct* pParam = nullptr;

    void runCycle() {
        auto& param = *pParam;
        
        if (!param.bExecute) {
            param.bBusy = false;
            param.bCommandAborted = false;
            param.bError = false;
            param.iErrorID = ZMC_NO_ERROR;
            param.bCompensating = false;
            return;
        }

        if (!param.Master || !param.Slave) {
            param.bBusy = false;
            param.bCommandAborted = false;
            param.bError = true;
            param.iErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            param.bCompensating = false;
            return;
        }

        // Empty implementation - interface only
        param.bBusy = false;
        param.bCommandAborted = false;
        param.bError = false;
        param.iErrorID = ZMC_NO_ERROR;
        param.bCompensating = false;
    }
};

FbtBacklashCompensation::FbtBacklashCompensation() {
    make_impl<FbtImplBacklashCompensation, FbtBacklashCompensationStruct>(*this);
    
    // Initialize default values
    this->eBacklashMode = MC_BL_AUTO;
}

FbtBacklashCompensation::~FbtBacklashCompensation() {
    if (pImpl) {
        delete static_cast<FbtImplBacklashCompensation*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtBacklashCompensation::runCycle() {
    if (pImpl) {
        static_cast<FbtImplBacklashCompensation*>(pImpl)->runCycle();
    }
}


}  // namespace zmc
