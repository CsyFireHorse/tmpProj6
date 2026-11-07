/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-20
@ArtifactStatusAuthority GPT-5 Codex
*/

#include <stdio.h>
#include "common/ecat/ecat_pdo_key.h"
#include "fbt_fb_base.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_impl.h"
// #include "common/misc/TrapzVelToyForwarder.hpp"
#include "vel/include/Vel4Wrap.h"
#include "fbt_move_impl.hpp"
#include "misc_helper.hpp"
#include "math.h"
namespace zmc {


void setFbImplContinousUpdate(void* pImpl, bool b){
    auto p = static_cast<FbtExeBase*>(pImpl);
    p->bContinuousUpdate_ = b;
}
void setFbImplFlagSpec(void* pImpl, bool b){
    auto p = static_cast<FbtExeBase*>(pImpl);
    p->bFlagSpec = b;
}
struct FooImpl{
    FbtBase* pFb;
};
int getFbImplType(void* pFb){
    auto pImpl = (FooImpl*)pFb;
    if (!pImpl->pFb){
        return FB_TYPE_NOP;
    }
    return pImpl->pFb->fbType;
}
int getFbImplStat(void* pFb){
    auto pImpl = (FooImpl*)pFb;
    if (!pImpl->pFb){
        return FB_STAT_IDLE;
    }
    return pImpl->pFb->fbStat;
}
int getFbImplErrorCode(void* pFb){
    auto pImpl = (FooImpl*)pFb;
    if (!pImpl->pFb){
        return 0;
    }
    return pImpl->pFb->error_code;
}

template <typename T, typename P>
class FbtImplMoveBase : public FbExeAbortingTmpl<T, P> {
   public:
    // double _cur_time = 0;
    Vel4Wrap trapzVel;

   public:   
    void onPlan() {
        onBuildTraj();
        onPlanVel();
    }
    void onBuildTraj(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (1) { // !this->hasVelPlan
            double posOrDist = get_pos_or_dist(param);
            ax.fMarkPosition = ax.fSavePosition;
            ax.fMarkVelocity = ax.fSaveVelocity;
            ax.fMarkAcceleration = ax.fSaveAcceleration;
            bool isRelative = !is_fb_has_absolute<P>();
            bool isAdditive = std::is_same<P, FbtMoveAdditiveStruct>::value;
            if (!isRelative) {
                ax.fAimPosition = posOrDist;
            } else if (!isAdditive)
                ax.fAimPosition = ax.fMarkPosition + posOrDist;
            else{
                ax.fAimPosition = posOrDist + (ax.is_prev_moving() ? ax.fAimPosition : ax.fSavePosition);
            }
            trapzVel.set_mode(ax.eRampType);
            trapzVel.set_inversable(true);
            trapzVel.set_begin_pva(ax.fMarkPosition, ax.fMarkVelocity, ax.fMarkAcceleration);
            trapzVel.set_end_pos(ax.fAimPosition);
        }
    }
    void onOverride(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (ax.isOverrideMatch(this->bOverrideParity)) return;
        onReplanVel();
    }
    void onContinuousUpdate(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (param.Velocity *ax.fFactorVel == trapzVel.get_max_vel()) return;
        onReplanVel();
    }
    void onReplanVel(){
        trapzVel.chop_left_time(this->getAxImpl().get_offset_time(this->startTick + 1));
        onPlanVel();
    }
    void onPlanVel(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (param.Velocity <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0)
            return this->onErrorWhenBuild(ZMC_INVALID_VELACCJERK_VALUES);
        if (!ax.isOverrideMatch(this->bOverrideParity)) this->incOverrideParity();
        trapzVel.set_config(param.Velocity * ax.fFactorVel, param.Acceleration * ax.fFactorAcc, param.Deceleration * ax.fFactorAcc, param.Jerk * ax.fFactorJerk);
        trapzVel.set_end_vel(0);
        if (trapzVel.build() != 0)
            return this->onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
        // _cur_time = this->hasVelPlan? ax.cycleTime: 0;
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        this->hasVelPlan = true;
    }
    void onForward(){
        if (this->isError()) return;
        auto& ax = this->getAxImpl();

        ax.cmd = AXIS_CMD_MOTION;
        PosVelAcc pva;
        int ret = trapzVel.get_slice(ax.get_offset_time(this->startTick), pva);
        ax.fSetPosition = pva.pos;
        ax.fSetVelocity = pva.vel;
        ax.fSetAcceleration = pva.acc;


        if (ret == VEL_END_PT_REACHED) {
            ax.axisStat = mcStandstill;
            this->fbStat = FB_STAT_DONE;
            if (ax.pNode.isCurrentNode(this))
                ax.pNode.pop_front();
        }
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->bNodeStat_ = 0;
        this->bCtrlParam_ = 0;
        this->startTick = 0;
    }
    inline void onCapture(){
        this->execute = this->pParam->Execute;
    }
};


template <typename T, typename P>
class FbtImplMoveAbsRelBase : public FbtImplMoveBase<T, P> {};

template <typename T, typename P>
class FbtImplMoveContinuousBase : public FbtImplMoveBase<T, P> {
public:
    inline double getEndVelocity()const{
        auto& param = *this->pParam;
        return this->bNegDirection ? -param.EndVelocity : param.EndVelocity;
    }
    void onPlan() {
        onBuildTraj();
        onPlanVel();
    }
    void onBuildTraj(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        double posOrDist = get_pos_or_dist(param);
        ax.fMarkPosition = ax.fSavePosition;
        ax.fMarkVelocity = ax.fSaveVelocity;
        ax.fMarkAcceleration = ax.fSaveAcceleration;
        if (is_fb_has_absolute<P>()) {
            if (ax.is_modulo_type())
                ax.fAimPosition = calcTargetPosByDirection(get_direction_or_default<P>(param), ax.fMarkPosition, posOrDist, ax.fPositionPeriod, ax.nDirection==mcNegativeDirection);
            else
                ax.fAimPosition = posOrDist;
        } else
            ax.fAimPosition = ax.fMarkPosition + posOrDist;

        auto& trapzVel = this->trapzVel;
        // FIXME: continuous end velocity
        trapzVel.set_inversable(true);
        trapzVel.set_mode(ax.eRampType);
        trapzVel.set_begin_pva(ax.fMarkPosition, ax.fMarkVelocity);
        trapzVel.set_end_pos(ax.fAimPosition);
    }
    void onPlanVel(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        auto& trapzVel = this->trapzVel;
        if (param.Velocity <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0) {
            this->onErrorWhenBuild(ZMC_INVALID_VELACCJERK_VALUES);
            return;
        }
        this->bNegDirection = (param.EndVelocityDirection == mcNegativeDirection) ||
            (param.EndVelocityDirection == mcCurrentDirection && ax.nDirection == mcNegativeDirection);
        double vEnd = getEndVelocity();
        trapzVel.set_end_vel(vEnd);
        trapzVel.set_config(param.Velocity * ax.fFactorVel, param.Acceleration * ax.fFactorAcc, param.Deceleration * ax.fFactorAcc, param.Jerk * ax.fFactorJerk);

        if (trapzVel.build() != 0) {
            this->onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        this->hasVelPlan = true;
    }
    void onForward(){
        if (this->isError()) return;
        auto& ax = this->getAxImpl();

        ax.cmd = AXIS_CMD_MOTION;
        double vEnd = getEndVelocity();
        auto& trapzVel = this->trapzVel;
        
            PosVelAcc pva;
            int ret =trapzVel.get_slice_continuous(ax.get_offset_time(this->startTick), pva);
            ax.fSetPosition = pva.pos;
            ax.fSetVelocity = pva.vel;
            ax.fSetAcceleration = pva.acc;

        if (this->inVelocity && ax.pNode.hasNext()) {
            this->fbStat = FB_STAT_ABORTED;
            if (ax.pNode.isCurrentNode(this))
                ax.pNode.pop_front();
        }
        if (ret == VEL_END_PT_REACHED){
            this->inReached_ = true;
            this->inVelocity = true;
        }
    }
};


class FbtMoveAdditiveImpl : public FbtImplMoveAbsRelBase<FbtMoveAdditiveImpl, FbtMoveAdditiveStruct> {
public:
    FbtMoveAdditiveImpl(){
        this->fbType = FB_TYPE_MOVEADDITIVE;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = (this->isDone());
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtMoveAdditive::FbtMoveAdditive() {
    make_impl<FbtMoveAdditiveImpl, FbtMoveAdditiveStruct>(*this);
}

FbtMoveAdditive::~FbtMoveAdditive() {
    delete static_cast<FbtMoveAdditiveImpl*>(pImpl);
    pImpl = nullptr;
}

void FbtMoveAdditive::runCycle() {
    auto impl = static_cast<FbtMoveAdditiveImpl*>(pImpl);
    impl->runCycle();
}

class FbtImplStop : public FbtExeTmpl<FbtImplStop, FbtImplParamAbortingTmpl<FbtStopStruct>> {
   protected:
    Vel4Wrap trapzVel;

   private:
    enum HomeStopState : uint8_t {
        HOME_STOP_NONE,
        HOME_STOP_POWER_OFF,
        HOME_STOP_POWER_ON_CSP,
        HOME_STOP_SYNC_PVA
    };

    inline uint8_t toCtrlMode(OP_MODE mode) const { return E_OP_MODE_TO_CTRL_MODE - mode; }

    bool isDriveHomeActive(const AxisImplBase& ax) const {
        return ax.axisStat == mcHoming && !ax.pNode.isEmpty() &&
               ax.pNode.currentNode().fbType == FB_TYPE_HOME;
    }

    void failDriveHomeStop(MC_ERROR_CODE error) {
        auto& ax = getAxImpl();
        this->onError(error);
        this->hasVelPlan = false;
        _homeStopState = HOME_STOP_NONE;
        ax.requestSwError(error);
        ax.cmd = AXIS_CMD_POWEROFF;
    }

    // Handles only the drive-Home interruption before the existing Stop trajectory takes over.
    bool handleDriveHomeStop() {
        auto& ax = getAxImpl();
        const uint8_t cspMode = toCtrlMode(OP_MODE_CSP);

        switch (_homeStopState) {
            case HOME_STOP_NONE:
                return false;
            case HOME_STOP_POWER_OFF:
                ax.byControllerMode = cspMode;
                ax.cmd = AXIS_CMD_POWEROFF;
                if (!ax.regulatorRealState && !ax.driveStartRealState) {
                    _homeStopState = HOME_STOP_POWER_ON_CSP;
                    _homeStopWaitCycles = 0;
                    return true;
                }
                if (++_homeStopWaitCycles >= ZMC_FB_POWER_TIMEOUT_COUNT) {
                    failDriveHomeStop(ZMC_ERR_P_TIMEOUT);
                }
                return true;
            case HOME_STOP_POWER_ON_CSP:
                ax.byControllerMode = cspMode;
                ax.cmd = AXIS_CMD_POWERON;
                if (ax.regulatorRealState && ax.driveStartRealState &&
                    ax.byRealControllerMode == cspMode) {
                    _homeStopState = HOME_STOP_SYNC_PVA;
                    return true;
                }
                if (++_homeStopWaitCycles >= ZMC_FB_CTRL_MODE_TIMEOUT_COUNT) {
                    const MC_ERROR_CODE error =
                        ax.regulatorRealState && ax.driveStartRealState
                            ? ZMC_ERR_SCM_MODE_SWITCH_TIMEOUT
                            : ZMC_ERR_P_TIMEOUT;
                    failDriveHomeStop(error);
                }
                return true;
            case HOME_STOP_SYNC_PVA:
                ax.byControllerMode = cspMode;
                ax.cmd = AXIS_CMD_POWERON;
                if (!ax.regulatorRealState || !ax.driveStartRealState ||
                    ax.byRealControllerMode != cspMode) {
                    _homeStopState = HOME_STOP_POWER_ON_CSP;
                    return true;
                }
                _homeStopState = HOME_STOP_NONE;
                this->hasVelPlan = false;
                return false;
            default:
                failDriveHomeStop(ZMC_ERR_UNKNOWN_SW_ERROR);
                return true;
        }
    }

    HomeStopState _homeStopState = HOME_STOP_NONE;
    uint16_t _homeStopWaitCycles = 0;
    bool _driveHomeStopProfile = false;

   public:
   FbtImplStop(){
        this->fbType = FB_TYPE_STOP;
   }
    void onActivate() {
        this->fbStat = FB_STAT_BUSY_ACTIVE;
        auto& ax = getAxImpl();
        ax.setAxisState(mcStopping, AXIS_STATE_DETAIL_DEFAULT);
    }

    void onStarted() {
        auto& param = *this->pParam; 
        if (!param.Axis) 
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!checkAxisStatBeforeActivate()) return;
        auto& ax = getAxImpl();
        const bool driveHomeActive = isDriveHomeActive(ax);
        pushToBuffer();
        onActivate();
        _driveHomeStopProfile = driveHomeActive;
        _homeStopState = driveHomeActive ? HOME_STOP_POWER_OFF : HOME_STOP_NONE;
        _homeStopWaitCycles = 0;
        this->hasVelPlan = false;
    }
    void onPlan() {
        auto& param = *this->pParam;
        auto& ax = getAxImpl();

        if (param.Deceleration <= 0) {
            this->onErrorWhenBuild(ZMC_INVALID_VELACCJERK_VALUES);
            return;
        }
        ax.fMarkPosition = ax.fSavePosition;
        ax.fMarkVelocity = ax.fSaveVelocity;
        trapzVel.set_mode(ax.eRampType);
        trapzVel.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, ax.fSaveAcceleration);
        trapzVel.set_config(ax.fSWMaxVelocity, param.Deceleration, param.Deceleration, param.Jerk);
        double targetPos = ax.fSavePosition + trapzVel.cal_stop_pos();
        trapzVel.set_end_pva(targetPos, 0, 0);
        if (trapzVel.build() != 0) {
            this->onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        this->hasVelPlan = true;
    }

    void onExecuting() {
        auto& ax = getAxImpl();
        if (!checkReceiveAbortError()) return;
        if (!checkAxisStatWhenActivated()) return;
        if (handleDriveHomeStop()) return;
        if (_driveHomeStopProfile && ax.byRealControllerMode != toCtrlMode(OP_MODE_CSP)) {
            failDriveHomeStop(ZMC_WRONG_CONTROLLER_MODE);
            return;
        }
        if (!this->hasVelPlan) {
            onPlan();
        } else if (isRisingEdge()) {
            onPlan();
        }
        if (this->isError()) return;
        if (!this->isRunableByUpdateCycle(ax.m_cycleCounter)) return;
        onForward();
    }
    void onForward(){
        auto& ax = getAxImpl();
        ax.cmd = AXIS_CMD_MOTION;
        PosVelAcc pva;
        int ret = trapzVel.get_slice(ax.get_offset_time(this->startTick), pva);

        ax.fSetPosition = pva.pos;
        ax.fSetVelocity = pva.vel;
        ax.fSetAcceleration = pva.acc;

        if (ret == VEL_END_PT_REACHED) {
            // HOTFIX
            // avoid motion done in one tick , must be done in two tick at least
            double tEnd = trapzVel.get_end_time();
            if (tEnd > 1e-6 && tEnd < ax.cycleTime && this->fbStat != FB_STAT_DONE &&
            ax.get_offset_time(this->startTick) < tEnd + ax.cycleTime){
                ax.fSetAcceleration = - ax.fSaveVelocity / tEnd;
            }else
                this->fbStat = FB_STAT_DONE;
        }
    }
    void onClear(){
        auto& ax = getAxImpl();
        if (ax.axisStat == mcStopping){
            ax.setAxisState(ax.regulatorRealState ? mcStandstill : mcDisabled);
        }
        if (ax.pNode.isCurrentNode(this))
            ax.pNode.pop_front();
        this->hasVelPlan = false;
        _homeStopState = HOME_STOP_NONE;
        _homeStopWaitCycles = 0;
        _driveHomeStopProfile = false;
        FbtExeTmpl::onClear();
    }
    inline void onCapture(){
        this->execute = this->pParam->Execute; 
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtStop ::FbtStop() {
    make_impl<FbtImplStop, FbtStopStruct>(*this);
    Deceleration = 1000.0;
}

FbtStop ::~FbtStop() { 
    delete static_cast<FbtImplStop*>(pImpl); 
    pImpl = nullptr;
}

// void FbtStop ::setDeceleration(double dec) { this->Deceleration = dec; }

void FbtStop ::runCycle() {
    auto impl = static_cast<FbtImplStop*>(pImpl);
    impl->runCycle();
}

class FbtImplHalt : public FbExeAbortingTmpl<FbtImplHalt, FbtHaltStruct> {
   public:
   protected:
    Vel4Wrap trapzVel;
   public:
   FbtImplHalt(){
        this->fbType = FB_TYPE_HALT;
   }

    void onPlan() {
        auto& param = *this->pParam;
        auto& ax = getAxImpl();

        if (param.Deceleration <= 0) {
            this->onErrorWhenBuild(ZMC_INVALID_VELACCJERK_VALUES);
            return;
        }
        ax.fMarkPosition = ax.fSavePosition;
        ax.fMarkVelocity = ax.fSaveVelocity;
        trapzVel.set_mode(ax.eRampType);
        trapzVel.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, ax.fSaveAcceleration);
        trapzVel.set_config(ax.fSWMaxVelocity, param.Deceleration, param.Deceleration, param.Jerk);
        double targetPos = ax.fSavePosition + trapzVel.cal_stop_pos();
        trapzVel.set_end_pva(targetPos, 0, 0);

        if (trapzVel.build() != 0) {
            this->onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        this->hasVelPlan = true;
    }


    void onForward(){
        auto& ax = getAxImpl();
        PosVelAcc pva;
        int ret = trapzVel.get_slice(ax.get_offset_time(this->startTick), pva);
        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition = pva.pos;
        ax.fSetVelocity = pva.vel;
        ax.fSetAcceleration = pva.acc;

        if (ret == VEL_END_PT_REACHED) {
            // HOTFIX
            double tEnd = trapzVel.get_end_time();
            if (tEnd > 1e-6 && tEnd < ax.cycleTime && this->fbStat != FB_STAT_DONE &&
            ax.get_offset_time(this->startTick) < tEnd + ax.cycleTime){
                ax.fSetAcceleration = - ax.fSaveVelocity / tEnd;
            }else{
                this->fbStat = FB_STAT_DONE;
                ax.axisStat = mcStandstill;
                if (ax.pNode.isCurrentNode(this))
                ax.pNode.pop_front();
            }
        }     
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->hasVelPlan = false;
        this->startTick = 0;
    }
    inline void onCapture(){
        this->execute = this->pParam->Execute; 
    }
    inline void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = this->isAborted();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtHalt::FbtHalt() {
    make_impl<FbtImplHalt, FbtHaltStruct>(*this);
}

FbtHalt::~FbtHalt() { 
    delete static_cast<FbtImplHalt*>(pImpl);
    pImpl = nullptr;
}

// void FbtHalt::setDeceleration(double dec) { this->Deceleration = dec; }

void FbtHalt::runCycle() {
    auto impl = static_cast<FbtImplHalt*>(pImpl);
    impl->runCycle();
}

class FbtImplMoveVelocity
    : public FbExeBufTmpl<FbtImplMoveVelocity, FbtMoveVelocityStruct> {
   public:
    Vel4Wrap trapzVel;

   public:
    FbtImplMoveVelocity(){
        this->fbType = FB_TYPE_MOVEVELOCITY;
    }

    double getEndVelocity()const{
        auto& param = *this->pParam;
        return this->bNegDirection ? -param.Velocity : param.Velocity;
    }
    void onPlan() {
        auto& param = *this->pParam;
        auto& ax = getAxImpl();
        if (param.BufferMode > mcBuffered) {
            // not supported blending now
            return this->onErrorWhenBuild(ZMC_BUFFER_MODE_NOT_SUPPORTED);
        }
        if (param.Velocity <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0) {
            return this->onErrorWhenBuild(ZMC_INVALID_VELACCJERK_VALUES);
        }
        this->bNegDirection = (param.Direction == mcNegativeDirection) ||
            (param.Direction == mcCurrentDirection && ax.nDirection == mcNegativeDirection);
        if (param.Direction == mcFastestWay || param.Direction== mcShortest){
            return this->onErrorWhenBuild(ZMC_MV_DIRECTION_NOT_APPLICABLE);
        }
        double vEnd = getEndVelocity();
        trapzVel.set_mode(ax.eRampType);
        trapzVel.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, ax.fSaveAcceleration);
        trapzVel.set_end_pva(this->bNegDirection ? -1e19 : 1e19, vEnd);
        set_vel_config();
        if (trapzVel.build() != 0) {
            this->onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        this->hasVelPlan = true;
    }

    inline void set_vel_config(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        trapzVel.set_config(param.Velocity * ax.fFactorVel, param.Acceleration * ax.fFactorAcc, param.Deceleration * ax.fFactorAcc, param.Jerk * ax.fFactorJerk);
    }
    void onOverride(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (ax.isOverrideMatch(this->bOverrideParity)) return;
        onReplanVel();
    }
    void onContinuousUpdate(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (param.Velocity *ax.fFactorVel == trapzVel.get_max_vel()) return;
        onReplanVel();
    }
    void onReplanVel(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (!ax.isOverrideMatch(this->bOverrideParity)) this->incOverrideParity();
        trapzVel.chop_left_time(ax.get_offset_time(this->startTick + 1));

        this->bNegDirection = (param.Direction == mcNegativeDirection) ||
            (param.Direction == mcCurrentDirection && ax.nDirection == mcNegativeDirection);
        double vEnd = getEndVelocity();
        trapzVel.set_end_pva(this->bNegDirection ? -1e19 : 1e19, vEnd);
        if (param.Velocity <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0)
            return this->onError(ZMC_INVALID_VELACCJERK_VALUES);
        set_vel_config();
        if (trapzVel.build() != 0) 
            return this->onError(ZMC_ERR_VEL_BUILD_FAILED);
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
    }
    void onForward(){
        auto& param = *this->pParam;
        auto& ax = getAxImpl();
        PosVelAcc pva;
        trapzVel.get_slice(ax.get_offset_time(this->startTick), pva);
        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition = pva.pos;
        ax.fSetVelocity = pva.vel;
        ax.fSetAcceleration = pva.acc;


        if (this->inVelocity && ax.pNode.hasNext()) {
            this->fbStat = FB_STAT_ABORTED;
            if (ax.pNode.isCurrentNode(this))
                ax.pNode.pop_front();
        }

        this->inVelocity = fabs(pva.vel - getEndVelocity()) < 1e-6;
    }

    void onClear() {
		this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->inVelocity = false;
        this->fbStat = FB_STAT_IDLE;
        this->hasVelPlan = false;
        this->startTick = 0;
    }
    inline void onCapture(){
        this->execute = this->pParam->Execute; 
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->InVelocity = this->inVelocity;
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->Active = this->isActive();
        this->pParam->ErrorID = this->getErrorID();
    }
};


FbtMoveVelocity ::FbtMoveVelocity() {
    make_impl<FbtImplMoveVelocity, FbtMoveVelocityStruct>(*this);
    this->Direction = mcCurrentDirection;
}

FbtMoveVelocity ::~FbtMoveVelocity() { 
    delete static_cast<FbtImplMoveVelocity*>(pImpl); 
    pImpl = nullptr;
}

void FbtMoveVelocity ::runCycle() {
    auto impl = static_cast<FbtImplMoveVelocity*>(pImpl);
    impl->runCycle();
}


class FbtMoveContinuousAbsoluteImpl : public FbtImplMoveContinuousBase<FbtMoveContinuousAbsoluteImpl, FbtMoveContinuousAbsoluteStruct> {
public:
    FbtMoveContinuousAbsoluteImpl(){
        this->fbType = FB_TYPE_MOVECONTINUOUSABSOLUTE;
    }
    void onUpdate(){
        this->pParam->InEndVelocity = (this->inVelocity);
        this->pParam->PositionReached = (this->inReached_);
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtMoveContinuousAbsolute::FbtMoveContinuousAbsolute() {
    make_impl<FbtMoveContinuousAbsoluteImpl, FbtMoveContinuousAbsoluteStruct>(*this);
    this->EndVelocityDirection = mcCurrentDirection;
}

FbtMoveContinuousAbsolute::~FbtMoveContinuousAbsolute() {
    delete static_cast<FbtMoveContinuousAbsoluteImpl*>(pImpl);
    pImpl = nullptr;
}

void FbtMoveContinuousAbsolute::runCycle() {
    auto impl = static_cast<FbtMoveContinuousAbsoluteImpl*>(pImpl);
    impl->runCycle();
}


class FbtMoveContinuousRelativeImpl : public FbtImplMoveContinuousBase<FbtMoveContinuousRelativeImpl, FbtMoveContinuousRelativeStruct> {
public:
    FbtMoveContinuousRelativeImpl(){
        this->fbType = FB_TYPE_MOVECONTINUOUSRELATIVE;
    }
    void onUpdate(){
        this->pParam->InEndVelocity = (this->inVelocity);
        this->pParam->DistanceTravelled = (this->inReached_);
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->ErrorID = this->getErrorID();
    }
};
FbtMoveContinuousRelative::FbtMoveContinuousRelative() {
    make_impl<FbtMoveContinuousRelativeImpl, FbtMoveContinuousRelativeStruct>(*this);
    this->EndVelocityDirection = mcCurrentDirection;
}

FbtMoveContinuousRelative::~FbtMoveContinuousRelative() {
    delete static_cast<FbtMoveContinuousRelativeImpl*>(pImpl);
    pImpl = nullptr;
}

void FbtMoveContinuousRelative::runCycle() {
    auto impl = static_cast<FbtMoveContinuousRelativeImpl*>(pImpl);
    impl->runCycle();
}

class FbtImplJog : public FbExeAbortingTmpl<FbtImplJog, FbtJogStruct> {
   protected:
   public:
    Vel4Wrap trapzVel;
    int8_t directionBak = 0;

   public:
    FbtImplJog(){
        this->fbType = FB_TYPE_JOG;
    }
    inline MC_AXIS_STATES getAxisStateWhenActivated() const {
        return get_direction() != 0 ? mcContinuousMotion : mcDiscreteMotion;
    }
    void runCycle() {
        (this)->onCapture();
        bool exeBak = this->_executeBak;

        if (this->execute && !exeBak) {
            (this)->onStarted();
        }
        if (this->isBusy()) {
            (this)->onExecuting();
        } else if (this->isDoneErrorAborted() && !this->execute) {
            (this)->onClear();
        }
        this->_executeBak = this->execute;
        this->directionBak = this->get_direction();
        (this)->onUpdate();
    }
    void onStarted() {
        if (FB_STAT_IDLE != this->fbStat) return;
        if (!this->get_axis_ref()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate((this)->getAxisStateWhenActivated());
    }
    void onExecuting() {
        if (!this->checkReceiveAbortError()) return;
        if (!this->checkAxisStatWhenActivated()) return;
        if (directionBak != get_direction()) {
            (this)->onPlan();
        }
        if (this->isError()) return;
        if (!this->isRunableByUpdateCycle(this->getAxImpl().m_cycleCounter)) return;
        (this)->onForward();
    }
    // 0: stop, 1: forward, -1: backward
    int8_t get_direction()const{
        return this->bPosDirection - this->bNegDirection;
    }
    void onPlan() {
        auto& param = *this->pParam;
        auto& ax = getAxImpl();
        int vel_direct = get_direction();
        double _targetVel = param.Velocity * vel_direct;
        double endPos;
        trapzVel.set_mode(ax.eRampType);
        trapzVel.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, ax.fSaveAcceleration);
        trapzVel.set_config(param.Velocity, param.Acceleration, param.Deceleration, param.Jerk);
        if (vel_direct == 0){
            endPos = trapzVel.cal_stop_pos();
        }else{
            endPos = (vel_direct >= 1) ? 1e19 : -1e19;
        }
        trapzVel.set_end_pva(endPos, _targetVel, 0);

        if (trapzVel.build() != 0) {
            this->onError(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        this->hasVelPlan = true;
        ax.axisStat = getAxisStateWhenActivated();
    }

    void onContinuousUpdate(){
        auto& param = *this->pParam;
        if (param.Velocity == trapzVel.get_max_vel()) return;
        // FIXME: chop trapzVel
        auto& ax = this->getAxImpl();
        trapzVel.chop_left_time(ax.get_offset_time(this->startTick + 1));
        trapzVel.set_config(param.Velocity, param.Acceleration, param.Deceleration, param.Jerk);
        if (trapzVel.build() != 0) {
            this->onError(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
    }
    void onForward(){
        auto& ax = getAxImpl();
        PosVelAcc pva;
        int ret = trapzVel.get_slice(ax.get_offset_time(this->startTick), pva);
        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition = pva.pos;
        ax.fSetVelocity = pva.vel;
        ax.fSetAcceleration = pva.acc;

        if (ret == VEL_END_PT_REACHED) {
            // HOTFIX
            double tEnd = trapzVel.get_end_time();
            if (tEnd > 1e-6 && tEnd < ax.cycleTime && this->fbStat != FB_STAT_DONE &&
            ax.get_offset_time(this->startTick) < tEnd + ax.cycleTime){
                ax.fSetAcceleration = - ax.fSaveVelocity / tEnd;
            }else{
                this->fbStat = FB_STAT_DONE;
                ax.axisStat = mcStandstill;
                if (ax.pNode.isCurrentNode(this))
                ax.pNode.pop_front();
            }
        } 
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
    }
    inline void onCapture(){
        this->execute = this->pParam->JogBackward ^ this->pParam->JogForward;
        if (this->pParam->JogForward & !this->pParam->JogBackward){
            this->bPosDirection = 1;  this->bNegDirection = 0;
        }else if (!this->pParam->JogForward & this->pParam->JogBackward){
            this->bPosDirection = 0;  this->bNegDirection = 1;
        }else{
            this->bPosDirection = 0;  this->bNegDirection = 0;
        }
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = this->isAborted();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtJog::FbtJog() {
    make_impl<FbtImplJog, FbtJogStruct>(*this);
}

FbtJog::~FbtJog() { 
    delete static_cast<FbtImplJog*>(pImpl);
    pImpl = nullptr;
}

void FbtJog::runCycle() {
    auto impl = static_cast<FbtImplJog*>(pImpl);
    impl->runCycle();
}

class FbtImplInch : public FbExeAbortingTmpl<FbtImplInch, FbtInchStruct> {
   protected:
   public:
    Vel4Wrap trapzVel;
    int8_t directionBak = 0;

   public:
    FbtImplInch(){
        this->fbType = FB_TYPE_INCH;
    }
    inline MC_AXIS_STATES getAxisStateWhenActivated() const {
        return mcDiscreteMotion;
    }
    void runCycle() {
        (this)->onCapture();
        bool exeBak = this->_executeBak;

        if (this->execute && !exeBak) {
            (this)->onStarted();
        }
        if (this->isBusy()) {
            (this)->onExecuting();
        } else if (this->isDoneErrorAborted() && !this->execute) {
            (this)->onClear();
        }
        this->_executeBak = this->execute;
        this->directionBak = this->get_direction();
        (this)->onUpdate();
    }
    void onStarted() {
        if (FB_STAT_IDLE != this->fbStat) return;
        if (!this->get_axis_ref()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate((this)->getAxisStateWhenActivated());
    }
    void onExecuting() {
        if (!this->checkReceiveAbortError()) return;
        if (!this->checkAxisStatWhenActivated()) return;
        if (directionBak != get_direction()) {
            (this)->onPlan();
        }
        if (this->isError()) return;
        if (!this->isRunableByUpdateCycle(this->getAxImpl().m_cycleCounter)) return;
        (this)->onForward();
    }
    // 0: stop, 1: forward, -1: backward
    int8_t get_direction()const{
        return this->bPosDirection - this->bNegDirection;
    }
    void onPlan() {
        auto& param = *this->pParam;
        auto& ax = getAxImpl();
        int8_t dir = get_direction();

        double endPos;
        trapzVel.set_mode(ax.eRampType);
        trapzVel.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, ax.fSaveAcceleration);
        trapzVel.set_config(param.Velocity, param.Acceleration, param.Deceleration, param.Jerk);
        if (dir == 0){
            endPos = trapzVel.cal_stop_pos();
        }else{
            endPos = ax.fSavePosition + (dir >= 1 ? param.Distance : -param.Distance);
        }
        trapzVel.set_end_pva(endPos, 0.0, 0);
        if (trapzVel.build() != 0) {
            this->onError(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        this->hasVelPlan = true;
    }

    void onForward(){
        auto& ax = getAxImpl();
        PosVelAcc pva;
        int ret = trapzVel.get_slice(ax.get_offset_time(this->startTick), pva);
        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition = pva.pos;
        ax.fSetVelocity = pva.vel;
        ax.fSetAcceleration = pva.acc;

        if (ret == VEL_END_PT_REACHED) {
            // HOTFIX
            double tEnd = trapzVel.get_end_time();
            if (tEnd > 1e-6 && tEnd < ax.cycleTime && this->fbStat != FB_STAT_DONE &&
            ax.get_offset_time(this->startTick) < tEnd + ax.cycleTime){
                ax.fSetAcceleration = - ax.fSaveVelocity / tEnd;
            }else{
                this->fbStat = FB_STAT_DONE;
                ax.axisStat = mcStandstill;
                if (ax.pNode.isCurrentNode(this))
                ax.pNode.pop_front();
            }
        } 
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
    }
    inline void onCapture(){
        this->execute = this->pParam->InchBackward ^ this->pParam->InchForward;
        if (this->pParam->InchForward & !this->pParam->InchBackward){
            this->bPosDirection = 1;  this->bNegDirection = 0;
        }else if (!this->pParam->InchForward & this->pParam->InchBackward){
            this->bPosDirection = 0;  this->bNegDirection = 1;
        }else{
            this->bPosDirection = 0;  this->bNegDirection = 0;
        }
    }
    void onUpdate(){
        this->pParam->Busy = this->get_direction()!=0;
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = this->isAborted();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtInch::FbtInch() {
    make_impl<FbtImplInch, FbtInchStruct>(*this);
}

FbtInch::~FbtInch() { 
    delete static_cast<FbtImplInch*>(pImpl);
    pImpl = nullptr;
}

void FbtInch::runCycle() {
    auto impl = static_cast<FbtImplInch*>(pImpl);
    impl->runCycle();
}


FbtPositionProfile::FbtPositionProfile(){}
FbtPositionProfile::~FbtPositionProfile(){}
void FbtPositionProfile::runCycle(){}

FbtAccelerationProfile::FbtAccelerationProfile(){}
FbtAccelerationProfile::~FbtAccelerationProfile(){}
void FbtAccelerationProfile::runCycle(){}

FbtVelocityProfile::FbtVelocityProfile(){}
FbtVelocityProfile::~FbtVelocityProfile(){}
void FbtVelocityProfile::runCycle(){}


void shiftFbImplVelDistance(void* pImpl, double distance){
    auto p = static_cast<FbtMoveAdditiveImpl*>(pImpl);
    p->trapzVel.shift_pos(distance);
}

}  // namespace zmc
