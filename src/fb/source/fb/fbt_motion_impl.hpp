#ifndef _ZMC_FBT_MOVE_IMPL_HPP_
#define _ZMC_FBT_MOVE_IMPL_HPP_

#include <stdio.h>

#include "fb_cpp_def.h"
#include "fbt_axis_impl.h"
#include "fbt_fb_base.hpp"
// #include "common/misc/TrapzVelToyForwarder.hpp"
#include "fbt_move_impl.hpp"
#include "math.h"
#include "misc_helper.hpp"
#include "vel/include/Vel4Wrap.h"

namespace zmc {

// template <template<typename B, typename S> class BaseTmpl, typename T, typename P>
class FbtImplDisContBase
    : public FbtExeTmpl<FbtImplDisContBase, FbtImplParamAbortingTmpl<FbtMoveDisOrContStruct>> {
   public:
    FbtImplDisContBase() { 
        pParam = new FbtMoveDisOrContStruct;
        all_zero_init(*pParam);
        
        // p.pImpl = new T();
        // make_impl<FbtMoveAdditiveImpl, FbtMoveDisOrContStruct>(*this);
    }
    inline const AXIS_REF get_axis_ref() const { return get_axis_or_slave(*this->pParam); }
    inline AXIS_REF get_axis_ref() { return get_axis_or_slave(*this->pParam); }
    inline AxisImplBase& getAxImpl() { return get_axis_impl_ref(this->get_axis_ref()); }
    inline const AxisImplBase& getAxImpl() const {
        return get_axis_impl_ref(this->get_axis_ref());
    }
    inline MC_AXIS_STATES getAxisStateWhenActivated() const {
        return this->pParam->MotionType <= 2 ? mcDiscreteMotion : mcContinuousMotion;
    }

    bool checkAxisStatBeforeActivate() {
        auto& param = *this->pParam; 
        auto& ax = this->getAxImpl();
        if (ax.axisStat == mcDisabled || ax.axisStat == mcErrorStop) {
            this->onError(ZMC_FB_ACTIVE_AXIS_DISABLED);
            return false;
        } else if (ax.axisStat == mcStopping) {
            this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_STOPPING);
            return false;
        }
        return true;
    }
    // use in onExecuting
    bool checkAxisStatWhenActivated() {
        auto& ax = this->getAxImpl();
        return fbCheckAxisStatWhenActivated(*this, ax.axisStat);
    }
    // bool checkWakableWhenBuffering()
    void pushToBuffer() {
        auto& ax = this->getAxImpl();
        switch (get_buffer_mode_or_default(*this->pParam)) {
            case mcAborting:
                ax.pNode.clearFbByAborting();
                ax.pNode.push_back(this);
                break;
            default:
                ax.pNode.push_back(this);
                break;
        }
    }

    void pushToBufferAndActivate(MC_AXIS_STATES axStat) {
        if (!checkAxisStatBeforeActivate()) return;
        auto& ax = this->getAxImpl();
        if (!isAxisStateUpdatable(ax.axisStat, axStat)) {
            this->onError(ZMC_AXIS_NOT_READY_FOR_MOTION);
            return;
        }
        pushToBuffer();
        if (!ax.pNode.isCurrentNode(this)) {
            this->fbStat = FB_STAT_UNACTIVATED;
        } else {
            ax.axisStat = axStat;
            this->fbStat = FB_STAT_BUSY_ACTIVE;
        }
    }
    void onRisingEdge(AXIS_REF ax) {
        // if (FB_STAT_IDLE != this->fbStat) return;
        // if (!ax) {
        //     return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        // }

        // auto& param = *this->pParam;
        // auto& ax = get_axis_impl_ref(ax);
        // if (ax.axisStat == mcDisabled || ax.axisStat == mcErrorStop) {
        //     return this->onError(ZMC_FB_ACTIVE_AXIS_DISABLED);
        // } else if (ax.axisStat == mcStopping) {
        //     return this->onAborted();
        // } else if (!isAxisStateUpdatable(ax.axisStat, axStat)){
        //     return this->onError(ZMC_AXIS_NOT_READY_FOR_MOTION);
        // }
        // if (get_buffer_mode_or_default(*this->pParam) == mcAborting) {
        //     ax.pNode.clearFbByAborting();
        // }
        // ax.pNode.push_back(this);
        // if (!ax.pNode.isCurrentNode(this)) {
        //     this->fbStat = FB_STAT_UNACTIVATED;
        // } else {
        //     ax.axisStat = axStat;
        //     this->fbStat = FB_STAT_BUSY_ACTIVE;
        // }
    }
    void onStarted() {
        if (FB_STAT_IDLE != this->fbStat) return;
        if (!this->get_axis_ref()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate(getAxisStateWhenActivated());
    }
    void onExecuting() {
        if (!this->checkReceiveAbortError()) return;
        if (!this->checkAxisStatWhenActivated()) return;
        if (this->isRisingEdge()) {
            onPlan();
        }
        if (!this->isRunableByUpdateCycle(this->getAxImpl().m_cycleCounter)) return;
        onForward();
    }
    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->bNodeStat_ = 0;
        this->bCtrlParam_ = 0;
        this->startTick = 0;
    }
    inline void onCapture() { this->execute = this->pParam->Execute; }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Active = this->isActive();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
        this->pParam->CommandAborted = this->isAborted();
    }
   protected:
    // double _cur_time = 0;
    Vel4Wrap trapzVel;

   public:
    void onPlan() {
        onBuildTraj();
        onPlanVel();
    }
    void onBuildTraj() {
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (1) {  // !this->hasVelPlan
            double posOrDist = param.PosOrDist;
            ax.fMarkPosition = ax.fSavePosition;
            ax.fMarkVelocity = ax.fSaveVelocity;
            ax.fMarkAcceleration = ax.fSaveAcceleration;
            bool isRelative = param.MotionType == 1;  // !is_fb_has_absolute<P>();
            bool isAdditive = param.MotionType == 2;  // std::is_same<P, FbtMoveAdditiveStruct>::value;
            if (!isRelative) {
                ax.fAimPosition = posOrDist;
            } else if (!isAdditive)
                ax.fAimPosition = ax.fMarkPosition + posOrDist;
            else {
                ax.fAimPosition =
                    posOrDist + (ax.is_prev_moving() ? ax.fAimPosition : ax.fSavePosition);
            }
            trapzVel.set_inversable(true);
            trapzVel.set_begin_pva(ax.fMarkPosition, ax.fMarkVelocity, ax.fMarkAcceleration);
            trapzVel.set_end_pos(ax.fAimPosition);
        }
    }
    void onOverride() {
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (ax.isOverrideMatch(this->bOverrideParity)) return;
        onReplanVel();
    }
    void onContinuousUpdate() {
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (param.Velocity * ax.fFactorVel == trapzVel.get_max_vel()) return;
        onReplanVel();
    }
    void onReplanVel() {
        trapzVel.chop_left_time(this->getAxImpl().get_offset_time(this->startTick + 1));
        onPlanVel();
    }
    void onPlanVel() {
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (param.Velocity <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0)
            return this->onError(ZMC_INVALID_VELACCJERK_VALUES);
        if (!ax.isOverrideMatch(this->bOverrideParity)) this->incOverrideParity();
        trapzVel.set_config(param.Velocity * ax.fFactorVel, param.Acceleration * ax.fFactorAcc,
                            param.Deceleration * ax.fFactorAcc, param.Jerk * ax.fFactorJerk);
        trapzVel.set_end_vel(0);
        if (trapzVel.build() != 0) return this->onError(ZMC_ERR_VEL_BUILD_FAILED);
        // _cur_time = this->hasVelPlan? ax.cycleTime: 0;
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        this->hasVelPlan = true;
    }
    void onForward() {
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
            if (ax.pNode.isCurrentNode(this)) ax.pNode.pop_front();
        }
    }
    
    void prepareCycle() {
        onCapture();
        bool exeBak = this->_executeBak;

        if (this->execute && !exeBak) {
            onStarted();
        } else if (this->isDoneErrorAborted() && !this->execute) {
            onClear();
            this->_executeBak = this->execute;
            onUpdate();
        }
    }

    void executeCycle() {
        if (this->isBusy()) {
            onExecuting();
        } else if (this->isDoneErrorAborted() && !this->execute) {
            onClear();
        }
        this->_executeBak = this->execute;
        onUpdate();
    }
};
}  // namespace zmc
#endif