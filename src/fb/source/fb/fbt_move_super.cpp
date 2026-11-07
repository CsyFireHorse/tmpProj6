
#include <stdio.h>
#include "fbt_fb_base.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_impl.h"
// #include "common/misc/TrapzVelToyForwarder.hpp"
#include "vel/include/Vel4Wrap.h"
#include "fbt_move_impl.hpp"
#include "misc_helper.hpp"
#include "math.h"

namespace zmc{


class FbtMoveSuperImposedImpl : public FbExeAbortingTmpl<FbtMoveSuperImposedImpl, FbtMoveSuperImposedStruct> {
    protected:
        Vel4Wrap trapzVel;
        // Indicates whether this motion is superimposed on other motions
        bool isSuperImposed = false;
        double beginPos = 0;
        double distanceTravelled_ = 0;
    public:
        FbtMoveSuperImposedImpl() {
            this->fbType = FB_TYPE_MOVESUPERIMPOSED;
        }
        void onStarted() {
            if (FB_STAT_IDLE != this->fbStat) return;
            if (!this->get_axis_ref()) {
                return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            }
            auto& ax = this->getAxImpl();
            if (!ax.pNode.isEmpty()){
                if (!ax.pNode.currentNode().canSuperimposedNext())
                    return this->onError(ZMC_ERR_MSI_SUPERIMPOSED_NOT_SUPPORTED);
                this->isSuperImposed = true;
                ax.pNode.pushSuperimposedNode((zmc::FbtImplParam *) this);
                this->fbStat = FB_STAT_BUSY_ACTIVE;
            }else{
                this->isSuperImposed = false;
                this->pushToBufferAndActivate(this->getAxisStateWhenActivated());
            }
        }
        inline MC_AXIS_STATES getAxisStateWhenActivated() const {
            return mcDiscreteMotion;
        }
        void onPlan() {
            auto& param = *this->pParam;
            auto& ax = this->getAxImpl();
            if (param.VelocityDiff <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0) {
                this->onError(ZMC_INVALID_VELACCJERK_VALUES);
                return;
            }
            if (!isSuperImposed){
                ax.fMarkPosition = ax.fSavePosition;
                ax.fMarkVelocity = ax.fSaveVelocity;
                ax.fMarkAcceleration = ax.fSaveAcceleration;
                beginPos = ax.fSavePosition;
            }else{
                beginPos = 0;
            }
            double targetPos = beginPos + param.Distance;
            trapzVel.set_inversable(true);
            trapzVel.set_mode(ax.eRampType);
            trapzVel.set_begin_pva(beginPos, ax.fMarkVelocity, 0);
            trapzVel.set_end_pva(targetPos, 0, 0);
            trapzVel.set_config(param.VelocityDiff * ax.fFactorVel, 
                               param.Acceleration * ax.fFactorAcc, 
                               param.Deceleration * ax.fFactorAcc, 
                               param.Jerk * ax.fFactorJerk);
    
            if (trapzVel.build() != 0) {
                this->onError(ZMC_ERR_VEL_BUILD_FAILED);
                return;
            }
    
            this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
            this->hasVelPlan = true;
        }
        double getDistanceTravelled()const{
            return distanceTravelled_;
        }

        void onOnlinePlan(){

        }
        void onForward() {
            onOnlinePlan();
            _onForward();
        }
        void _onForward() {
            if (this->isError()) return;
            auto& ax = this->getAxImpl();
            if (ax.bSetValuesModifiedByMoveSuperimposed) return;
    
            ax.cmd = AXIS_CMD_MOTION;
            PosVelAcc pva;

            if (isSuperImposed && ax.pNode.isEmpty()){
                // if (!ax.pNode.isEmpty() && !ax.pNode.currentNode().isInCycle(ax.m_cycleCounter)){
                //     return this->onError(ZMC_MSI_INVALID_EXECUTION_ORDER);
                // }
                isSuperImposed = false;
                // trapzVel.shift_pos();
                beginPos += ax.fSetPosition;
            }

            int ret = trapzVel.get_slice(ax.get_offset_time(this->startTick), pva);

            if (isSuperImposed){
                // if (!ax.pNode.isEmpty() && !ax.pNode.currentNode().isInCycle(ax.m_cycleCounter)){
                //     return this->onError(ZMC_MSI_INVALID_EXECUTION_ORDER);
                // }
                if (1){
                    ax.fSetPosition += pva.pos;
                    ax.fSetVelocity += pva.vel;
                    ax.fSetAcceleration += pva.acc;
                }else{
                    // if switch curNode activated , do what?
                }
            }else{
                ax.fSetPosition = beginPos + pva.pos;
                ax.fSetVelocity = pva.vel;
                ax.fSetAcceleration = pva.acc;
            }

            ax.bSetValuesModifiedByMoveSuperimposed = true;
    
            this->pParam->SuperImposedVelocity = pva.vel;
            this->pParam->SuperImposedAcceleration = pva.acc;
            this->pParam->DistanceTravelled = pva.pos;
            distanceTravelled_ = pva.pos;

            if (ret == VEL_END_PT_REACHED) {
                this->fbStat = FB_STAT_DONE;

                if (isSuperImposed && !ax.pNode.isEmpty()){
                    shiftFbImplVelDistance(&ax.pNode.currentNode(), distanceTravelled_);// ax.pNode.
                }

                if (!isSuperImposed){
                    ax.axisStat = mcStandstill;
                    if (ax.pNode.isCurrentNode(this))
                        ax.pNode.pop_front();
                }else{
                    ax.pNode.popSuperimposedNode();
                }
            }
        }
    
        void onClear() {
            this->error_code = ZMC_NO_ERROR;
            this->fbStat = FB_STAT_IDLE;
            this->bNodeStat_ = 0;
            this->bCtrlParam_ = 0;
            this->startTick = 0;
            this->hasVelPlan = false;
            this->isSuperImposed = false;
            this->distanceTravelled_ = 0;
        }
    
        inline void onCapture() {
            this->execute = this->pParam->Execute;
            if (this->pParam->Abort && this->isBusy()) {
                // this->sendAborted();
                // TODO
            }
        }
    
        void onUpdate() {
            this->pParam->Busy = this->isBusy();
            this->pParam->Done = this->isDone();
            this->pParam->Error = this->isError();
            this->pParam->CommandAborted = this->isAborted();
            this->pParam->ErrorID = this->getErrorID();
        }
    };
    
    FbtMoveSuperImposed::FbtMoveSuperImposed() { 
        make_impl<FbtMoveSuperImposedImpl, FbtMoveSuperImposedStruct>(*this);
    }
    
    FbtMoveSuperImposed::~FbtMoveSuperImposed() {
        if (pImpl) {
            delete static_cast<FbtMoveSuperImposedImpl*>(pImpl);
            pImpl = nullptr;
        }
    }   
    
    void FbtMoveSuperImposed::runCycle() {
        auto impl = static_cast<FbtMoveSuperImposedImpl*>(pImpl);
        if (impl)
            impl->runCycle();
    }   
    
// Empty shell implementations for FbtHaltSuperImposed
FbtHaltSuperImposed::FbtHaltSuperImposed() {
}

FbtHaltSuperImposed::~FbtHaltSuperImposed() {
}

void FbtHaltSuperImposed::runCycle() {
}
    

double getFbImplDistanceTravelled(const FbtExeBase* pImpl){
    return static_cast<const FbtMoveSuperImposedImpl*>(pImpl)->getDistanceTravelled();
}

}