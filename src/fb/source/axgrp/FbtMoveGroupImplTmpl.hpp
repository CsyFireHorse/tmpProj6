#ifndef ZMC_FBT_MOVE_GROUP_IMPL_TMPL_HPP
#define ZMC_FBT_MOVE_GROUP_IMPL_TMPL_HPP

#include "../axgrp/AxisGrpZmotImpl.h"
#include "impl/fbt_impl_enum_def.h"
#include "impl/fbt_fb_base.hpp"
#include "traj/include/TrajCppWrap.h"
#include "vel/include/Vel4Wrap.h"

namespace zmc {

template <class T, class P>
class FbtMoveGroupImplTmpl : public FbtExeTmpl<T, FbtImplParamTmpl<P>> {
   public:
    TrajWrap trajGeom;
    Vel4Wrap velPlanner;
    uint32_t startTick = 0;

    void onCapture() { this->execute = (this->pParam && this->pParam->Execute); }

    void onErrorWhenBuild(MC_ERROR_CODE err) {
        this->onError(err);
        if (this->pParam && this->pParam->AxisGroup) {
            auto& d = get_grp_inner(*this->pParam->AxisGroup);
            d.errorId = static_cast<uint16_t>(err);
            d.groupState = ErrorStop;
        }
    }

    void onStarted() {
        if (!this->pParam || !this->pParam->AxisGroup) {
            this->onError(ZMC_ERR_NO_AXISGRP_REF);
            return;
        }
        auto& d = get_grp_inner(*this->pParam->AxisGroup);
        if (d.groupState == Disabled || d.groupState == ErrorStop) {
            this->onError(ZMC_AXIS_NOT_READY_FOR_MOTION);
            return;
        }
        if (d.axNum <= 0) {
            this->onError(ZMC_ERR_AXISGRP_EMPTY);
            return;
        }

        this->fbStat = FB_STAT_BUSY_ACTIVE;
        d.groupState = Moving;
    }

    void onPlan() {
        auto& d = get_grp_inner(*this->pParam->AxisGroup);
        const int trajInitRet = trajGeom.init_axis_num(static_cast<uint8_t>(d.axNum));
        if (trajInitRet != 0) {
            onErrorWhenBuild(static_cast<MC_ERROR_CODE>(trajInitRet));
            return;
        }
        trajGeom.set_begin_pos(d.fSavePos.data());
        static_cast<T*>(this)->onTraj();
        if (trajGeom.build() != 0) {
            onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        const double pathLen = trajGeom.get_plan();
        if (this->pParam->Velocity <= 0 || this->pParam->Acceleration <= 0 || this->pParam->Deceleration <= 0) {
            onErrorWhenBuild(ZMC_INVALID_VELACCJERK_VALUES);
            return;
        }
        startTick = d.tick;
        velPlanner.set_config(this->pParam->Velocity, this->pParam->Acceleration, this->pParam->Deceleration, this->pParam->Jerk);
        velPlanner.set_begin_pva(0.0, 0.0, 0.0);
        velPlanner.set_end_pva(pathLen, 0.0, 0.0);
        if (velPlanner.build() != 0) {
            onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
        }
    }

    void onExecuting() {
        if (!this->checkReceiveAbortError()) return;
        if (this->isRisingEdge()) {
            onPlan();
        }
        if (this->isError()) return;
        if (!this->pParam || !this->pParam->AxisGroup) {
            onErrorWhenBuild(ZMC_ERR_NO_AXISGRP_REF);
            return;
        }
        auto& d = get_grp_inner(*this->pParam->AxisGroup);
        if (!this->isRunableByUpdateCycle(d.tick)) return;
        this->onForward();
    }

    void onForward() {
        auto& d = get_grp_inner(*this->pParam->AxisGroup);
        if (d.groupState == ErrorStop) {
            onErrorWhenBuild(ZMC_ERR_ERRORSTOP_WHEN_MOVING);
            return;
        }

        double elapsedTime = (d.tick - startTick) * d.cycleTime;
        PosVelAcc pva{};
        const int ret = velPlanner.get_slice(elapsedTime, pva);
        trajGeom.get_slice3(pva.pos, pva.vel, pva.acc, d.fSetPos.data(), d.targetVel.data(), d.targetAcc.data());

        if (ret == VEL_END_PT_REACHED) {
            this->fbStat = FB_STAT_DONE;
            d.groupState = Standby;
        }
    }

    void onUpdate() {
        if (!this->pParam) return;
        this->pParam->Busy = this->isBusy();
        this->pParam->Active = this->isActive();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
        this->pParam->CommandAborted = this->isAborted();
        this->pParam->CommandAccepted = this->execute;
    }

    void onClear() { FbtBase::onClear(); }
};

}  // namespace zmc

#endif
