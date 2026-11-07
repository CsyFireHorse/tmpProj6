/**
 * History:
 * ================================================================
 * 2025-08-27 ChenSy <ChenSy@zlg.cn> created.
 *
 */

#include "fb/fb_cnc_axis_def.h"

#include <algorithm>
#include "fbt_axis_impl.h"
#include "fbt_fb_base.hpp"
#include "fbt_move_impl.hpp"
#include "misc_helper.hpp"
#include "VelPosFollower.h"
#include "vel/include/Vel4Wrap.h"
#include "mc_cnc_enum_def.h"
#include "../axis/AxisMixChecker.h"

namespace zmc {

template<typename T, typename B>
class FbtImplControlAxisTmpl
    : public FbtParamNodeTmpl<B> {
   public:
    Vel4Wrap gapFollower;
    double posZeroPoint = 0;
    bool bStopIpoActive = false;

    inline MC_AXIS_STATES getAxisStateWhenActivated() const { 
        return mcSynchronizedMotion; 
    }
    inline AxisImplBase& getAxImpl() {
        return get_axis_impl_ref(this->pParam->Axis);
    }

	
    bool checkAxisStatBeforeActivate() {
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        if (ax.axisStat == mcDisabled || ax.axisStat == mcErrorStop) {
            this->onError(ZMC_FB_ACTIVE_AXIS_DISABLED);
            return false;
        } else if (ax.axisStat == mcStopping) {
            this->onAborted();
            return false;
        }
        return true;
    }
    // use in onExecuting
    bool checkAxisStatWhenActivated() {
        auto& ax = this->getAxImpl();
        if (ax.axisStat == mcErrorStop) {
            this->onError(ZMC_ERR_ERRORSTOP_WHEN_MOVING);
            return false;
        } else if (ax.axisStat == mcDisabled) {
            this->onError(ZMC_ERR_POWEROFF_WHEN_MOVING);
            return false;
        }
        return true;
    }
    // bool checkWakableWhenBuffering() 

    void pushToBufferAndActivate(MC_AXIS_STATES axStat) {
        if (!checkAxisStatBeforeActivate()) return;
        auto& ax = this->getAxImpl();
        if (!isAxisStateUpdatable(ax.axisStat, axStat)){
            this->onError(ZMC_AXIS_NOT_READY_FOR_MOTION);
            return;
        }
        if (!ax.pNode.isCurrentNode(this)) {
            ax.pNode.clearFbByAxisStat(mcStopping);
            ax.pNode.push_back(this);
        }
        ax.axisStat = axStat;
        this->fbStat = FB_STAT_BUSY_ACTIVE;        
    }	
    void onExecuting() {
        if (!this->checkReceiveAbortError()) return;
        if (!this->checkAxisStatWhenActivated()) return;
        if (!this->hasEmitFirst)
        static_cast<T*>(this)->onPlan();
        if (this->isError()) return;
        if (!this->isRunableByUpdateCycle(this->getAxImpl().m_cycleCounter)) return;
        static_cast<T*>(this)->onForward();
    }
    void onEnable(){
        if (this->pParam->iStatus == IPO_UNKNOWN || this->pParam->iStatus == IPO_INIT) {
            this->isRunableByUpdateCycle(this->getAxImpl().m_cycleCounter);
            return;
        }else if (IPO_FINISHED == this->pParam->iStatus) {
            this->isRunableByUpdateCycle(this->getAxImpl().m_cycleCounter);
        }else if(IPO_WAIT == this->pParam->iStatus) {
            onExecuting();
        }else{
            onStarted();
            onExecuting();
        }
    }
    void _runCycle() {
        if (this->pParam->bEnable) {
            onEnable();
        }else{
            onClear();
        }
    }

	void runCycle() {
        onCapture();
        _runCycle();
        this->_executeBak = this->execute;
        onUpdate();
    }

    void onStarted() {
        if (!this->pParam->Axis || !this->getAxImpl().is_axis_valid()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        if (this->fbStat != FB_STAT_IDLE) return;
        this->pushToBufferAndActivate(getAxisStateWhenActivated());
    }
    void onPlan() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        posZeroPoint = ax.calc_zero_point(ax.fSavePosition);
    }

    void onForward() {
        
    }
    void onClear() {
        auto& ax = get_axis_impl_ref(this->pParam->Axis);
        if (ax.pNode.isCurrentNode(this)){
            if (ax.axisStat == mcSynchronizedMotion && ax.isSetPvaUnmoved()){
                ax.pNode.pop_front();
                ax.axisStat = mcStandstill;
            }
        }
        FbtBase::onClear();
        posZeroPoint = 0;
        bStopIpoActive = false;
    }

    inline void onCapture() { 
        this->execute = this->pParam->bEnable; 
    }

    inline void onUpdate() {
        this->pParam->bBusy = this->isBusy();
        this->pParam->bCommandAborted = this->isAborted();
        this->pParam->bError = this->isError();
        this->pParam->iErrorID = this->getErrorID();
        this->pParam->bStopIpo = this->bStopIpoActive;
    }
};

class FbtImplControlAxisByPos:public FbtImplControlAxisTmpl<FbtImplControlAxisByPos, FbtControlAxisByPosStruct>{
public:
    FbtImplControlAxisByPos() {
        fbType = FB_TYPE_CONTROLAXISBYPOS;
    }
    void onPlan() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        posZeroPoint = ax.calc_zero_point(ax.fSavePosition);
    }

    void onForward() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        
        // if (this->startTick == ax.m_cycleCounter + 1) return;
        // this->startTick = ax.m_cycleCounter + 1;

        // Calculate target position
        double targetPos = param.fSetPosition;

        // Check for position jumps if gap avoidance is enabled
        PosVelAcc followerPva;
        if (bStopIpoActive){
            int ret = gapFollower.get_slice(ax.get_offset_time(this->startTick), followerPva);
            ax.cmd = AXIS_CMD_MOTION;
            ax.bControlAxis = true;
            ax.fSetPosition = followerPva.pos;
            ax.fSetVelocity = followerPva.vel;
            ax.fSetAcceleration = followerPva.acc;
            bStopIpoActive = VEL_END_PT_REACHED != ret;
            return;
        }
        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition =  posZeroPoint + targetPos;
        ax.fSetVelocity = (ax.fSetPosition - ax.fSavePosition) / ax.cycleTime;
        ax.fSetAcceleration = (ax.fSetVelocity - ax.fSaveVelocity) / ax.cycleTime;
        if (!this->hasEmitFirst) this->hasEmitFirst = true;
        if (!param.bAvoidGaps) {
            bStopIpoActive = false;
            return;
        }
        if (!axisCheckDynamicLimit(ax)) {
            return;
        }
        bStopIpoActive = true;        
        gapFollower.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, ax.fSaveAcceleration);
        gapFollower.set_end_pva(posZeroPoint + targetPos, 0);
        gapFollower.set_config(param.fGapVelocity, param.fGapAcceleration, 
            param.fGapDeceleration, param.fGapJerk);
        gapFollower.set_inversable(true);
        gapFollower.build();
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        int ret = gapFollower.get_slice(ax.get_offset_time(this->startTick), followerPva);
        ax.cmd = AXIS_CMD_MOTION;
        ax.bControlAxis = true;
        ax.fSetPosition = followerPva.pos;
        ax.fSetVelocity = followerPva.vel;
        ax.fSetAcceleration = followerPva.acc;
    }    
};

FbtControlAxisByPos::FbtControlAxisByPos() {
    make_impl<FbtImplControlAxisByPos, FbtControlAxisByPosStruct>(*this);
}

FbtControlAxisByPos::~FbtControlAxisByPos() {
    if (pImpl) {
        delete static_cast<FbtImplControlAxisByPos*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtControlAxisByPos::runCycle() { 
    if (!pImpl) return;
    auto impl = static_cast<FbtImplControlAxisByPos*>(pImpl);
    impl->runCycle();
}


class FbtImplControlAxisByPosVel:public FbtImplControlAxisTmpl<FbtImplControlAxisByPosVel, FbtControlAxisByPosVelStruct>{
public:
    FbtImplControlAxisByPosVel() {
        fbType = FB_TYPE_CONTROLAXISBYPOSVEL;
    }
};

// FbtControlAxisByPosVel implementation
FbtControlAxisByPosVel::FbtControlAxisByPosVel() {
    make_impl<FbtImplControlAxisByPosVel, FbtControlAxisByPosVelStruct>(*this);
}

FbtControlAxisByPosVel::~FbtControlAxisByPosVel() {
    if (pImpl) {
        delete static_cast<FbtImplControlAxisByPosVel*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtControlAxisByPosVel::runCycle() {
    auto impl = static_cast<FbtImplControlAxisByPosVel*>(pImpl);
    if (impl)
    impl->runCycle();
}

class FbtImplControlAxisByVel:public FbtImplControlAxisTmpl<FbtImplControlAxisByVel, FbtControlAxisByVelStruct>{
public:
    FbtImplControlAxisByVel() {
        fbType = FB_TYPE_CONTROLAXISBYVEL;
    }
};
// FbtControlAxisByVel implementation
FbtControlAxisByVel::FbtControlAxisByVel() {
    make_impl<FbtImplControlAxisByVel, FbtControlAxisByVelStruct>(*this);
}

FbtControlAxisByVel::~FbtControlAxisByVel() {
    // Cleanup implementation if needed
    if (pImpl) {
        delete static_cast<FbtImplControlAxisByVel*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtControlAxisByVel::runCycle() {
    // TODO: Implement the actual functionality
    // This is a placeholder implementation

    // Check if enabled
    auto impl = static_cast<FbtImplControlAxisByVel*>(pImpl);
    if (impl)
    impl->runCycle();

    // Set busy state
    // bBusy = true;

    // TODO: Add actual implementation logic here
    // - Monitor axis for jumps in velocity
    // - Check velocity limits against fSWMaxVelocity
    // - Handle gap avoidance with velocity control only
    // - Update bStopIpo based on conditions
    // - Move axis to set velocity using gap parameters

    // Placeholder: Set error state for demonstration

}

}  // namespace zmc
