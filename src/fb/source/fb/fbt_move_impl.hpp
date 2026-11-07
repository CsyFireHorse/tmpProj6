#ifndef _FBT_MOVE_IMPL_HPP_
#define _FBT_MOVE_IMPL_HPP_

#include "fb_cpp_def.h"
#include "fbt_fb_base.hpp"
#include "fbt_axis_impl.h"
#include "fbt_traits.hpp"
#include "motMisc.hpp"

namespace zmc{

template <typename T>
class FbtParamNodeTmpl: public Node<FbtImplParamTmpl<T>> {};

// template <typename T>
// class FbtParamNodeTmpl: public FbtImplParamTmpl<T> {};


template <typename T>
class FbtImplParamMotTmpl : public FbtParamNodeTmpl<T> {
public:
    static_assert(has_axis_or_slave<T>(), "T must have axis or slave");

    inline const AXIS_REF get_axis_ref()const{
        return get_axis_or_slave(*this->pParam);
    }
    inline AXIS_REF get_axis_ref(){
        return get_axis_or_slave(*this->pParam);
    }
    inline AxisImplBase& getAxImpl() {
        return get_axis_impl_ref(this->get_axis_ref());
    }
    inline const AxisImplBase& getAxImpl()const {
        return get_axis_impl_ref(this->get_axis_ref());
    }  
    inline MC_AXIS_STATES getAxisStateWhenActivated() const {
        return get_axis_stat_when_fb_activated<T>();
    }
    // Ordinary motion commands clear any producer-owned extended-state detail.
    inline AxisStateDetail getAxisStateDetailWhenActivated() const {
        return AXIS_STATE_DETAIL_DEFAULT;
    }
};


// use for camOut, gearOut, homing, 
template <typename T>
class FbtImplParamMotSpecialTmpl : public FbtImplParamMotTmpl<T> {
};


template<typename T>
bool fbCheckAxisStatBeforeActivate(T& self, MC_AXIS_STATES axisStat) {
    switch (axisStat)
    {
    case mcErrorStop:
        self.onError(ZMC_FB_ACTIVE_AXIS_DISABLED);
        return false;
    case mcDisabled:
        self.onError(ZMC_FB_ACTIVE_AXIS_DISABLED);
        return false;
    case mcStopping:
        self.onError(ZMC_ERR_FB_ACTIVATE_AXIS_STOPPING);
        return false;
    default:
        return true;
    } 
}


template<typename T>
bool fbCheckAxisStatWhenActivated(T& self, MC_AXIS_STATES axisStat) {
    switch (axisStat)
    {
    case mcErrorStop:
        self.onError(ZMC_ERR_ERRORSTOP_WHEN_MOVING);
        return false;
    case mcDisabled:
        self.onError(ZMC_ERR_POWEROFF_WHEN_MOVING);
        return false;
    // case mcStopping:
    //     self.onError(ZMC_ERR_FB_ACTIVATE_AXIS_STOPPING);
    //     return false;
    default:
        return true;
    } 
}

// use for moveVelocity, moveAbs, moveRel, gearIn
template <typename T>
class FbtImplParamBufTmpl : public FbtImplParamMotTmpl<T> {
   public:    
    bool checkAxisStatBeforeActivate() {
        auto& ax = this->getAxImpl();
        return fbCheckAxisStatBeforeActivate(*this, ax.axisStat);
    }

    bool checkAxisStatWhenActivated() {
        auto& ax = this->getAxImpl();
        return fbCheckAxisStatWhenActivated(*this, ax.axisStat);
    }
    bool checkWakableWhenBuffering() {
        auto& ax = this->getAxImpl();
        if (this->fbStat == FB_STAT_UNACTIVATED && ax.pNode.isCurrentNode(this)) {
            if (!checkAxisStatBeforeActivate()) {
                ax.pNode.pop_front();
                return false;
            }
            return true;
        } else
            return false;
    }
    void pushToBuffer() {
        auto& ax = this->getAxImpl();
        switch (get_buffer_mode_or_default(*this->pParam)) {
            case mcAborting:
                ax.pNode.clearFbByAxisStat(mcStopping);
                ax.pNode.push_back(this);
                break;
            case mcBuffered:
                if(!ax.pNode.isEmpty() && !ax.pNode.currentNode().canBufferNext()) {
                    return this->onError(ZMC_BUFFERED_NOT_SUPPORTED_BY_PREVIOUS_MOVEMENT);
                }   
                ax.pNode.push_back(this);
                break;
            default:
                if(!ax.pNode.isEmpty() && !ax.pNode.currentNode().canBlendNext()) 
                    return this->onError(ZMC_BLENDING_NOT_SUPPORTED_BY_PREVIOUS_MOVEMENT);
                ax.pNode.push_back(this);
                break;
        }
    }
    // use in onStarted
    void pushToBufferAndActivate(
        MC_AXIS_STATES axStat,
        AxisStateDetail detail = AXIS_STATE_DETAIL_DEFAULT) {
        if (!checkAxisStatBeforeActivate()) return;
        auto& ax = this->getAxImpl();
        if (!isAxisStateUpdatable(ax.axisStat, axStat)){
            this->onError(ZMC_AXIS_NOT_READY_FOR_MOTION);
            return;
        }
        pushToBuffer();
        if (!ax.pNode.isCurrentNode(this)) {
            this->fbStat = FB_STAT_UNACTIVATED;
        } else {
            ax.setAxisState(axStat, detail);
            this->fbStat = FB_STAT_BUSY_ACTIVE;
        } 
    }
    bool wakeUpAndCheckExecute(
        MC_AXIS_STATES axStat,
        AxisStateDetail detail = AXIS_STATE_DETAIL_DEFAULT) {
        if (this->fbStat == FB_STAT_UNACTIVATED){
            auto& ax = this->getAxImpl();
            if (ax.pNode.isCurrentNode(this)) {
                if (!this->checkAxisStatBeforeActivate()) {
                    ax.pNode.pop_front();
                    return false;
                }
                if (!isAxisStateUpdatable(ax.axisStat, axStat)){
                    this->onError(ZMC_AXIS_NOT_READY_FOR_MOTION);
                    return false;
                }
                ax.setAxisState(axStat, detail);
                this->fbStat = FB_STAT_BUSY_ACTIVE;
                return true;
            }
            return false;
        }else{
            if (!this->checkAxisStatWhenActivated()) return false;
            return true;
        }
    }
    void onErrorWhenBuild(MC_ERROR_CODE err){
        auto& ax = this->getAxImpl();
        this->onError(err);
        if (ax.axisStatBak == mcStandstill || ax.axisStatBak == mcDisabled){
            ax.axisStat = mcStandstill;
            ax.cmd = ax.cmdBak;
            if (ax.pNode.isCurrentNode(this))
                ax.pNode.pop_front();
        }
    }
};

// use for moveVelocity, gearIn
template <typename T, typename P>
class FbExeBufTmpl : public FbtExeTmpl<T, FbtImplParamBufTmpl<P>> {
public:
    void onBlend(){} // call by axis motion_node_list
    void onOverride(){} // call by axis motion_node_list
    void onContinuousUpdate(){} // call by fb self
    void onReload(){ return static_cast<T*>(this)->onPlan();}
    void onWakeup(){}
    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!this->get_axis_ref()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate(
            static_cast<T*>(this)->getAxisStateWhenActivated(),
            static_cast<T*>(this)->getAxisStateDetailWhenActivated());
    }
    void onExecuting() {
        // if (!this->checkReceiveAbortError()) return;
        // if (this->fbStat != FB_STAT_BUSY_ACTIVE) {
        //     if (checkWakableAndActivate(static_cast<T*>(this)->getAxisStateWhenActivated())){
        //         static_cast<T*>(this)->onWakeup();
        //         onForward();
        //         return;
        //     }
        //     return;
        // }else if (!this->checkAxisStatWhenActivated()) return;

        if (!this->checkReceiveAbortError()) return;
        if (!this->wakeUpAndCheckExecute(
                static_cast<T*>(this)->getAxisStateWhenActivated(),
                static_cast<T*>(this)->getAxisStateDetailWhenActivated()))
            return;
        if (this->fbStat != FB_STAT_BUSY_ACTIVE) return;

        if (!this->hasVelPlan || this->isRisingEdge()) {
            static_cast<T*>(this)->onPlan();
        }else if (this->isContinuousUpdate()) {
            static_cast<T*>(this)->onContinuousUpdate();
        }
        if (this->isError()) return;
        if (!this->isRunableByUpdateCycle(this->getAxImpl().m_cycleCounter)) return;
        static_cast<T*>(this)->onForward();
    }
};
// use for  moveAbs, moveRel
template <typename T, typename P>
class FbExeBlendTmpl : public FbtExeTmpl<T, FbtImplParamBufTmpl<P>> {
public:
    void onBlend(){} // call by axis motion_node_list
    void onOverride(){} // call by axis motion_node_list
    void onContinuousUpdate(){} // call by fb self
    void onReload(){ return static_cast<T*>(this)->onPlan();}
    void onWakeup(){}
    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!this->get_axis_ref()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate(static_cast<T*>(this)->getAxisStateWhenActivated());
    }
    void onExecuting() {
        if (!this->checkReceiveAbortError()) return;
        if (!this->wakeUpAndCheckExecute(static_cast<T*>(this)->getAxisStateWhenActivated())) return;
        if (this->fbStat != FB_STAT_BUSY_ACTIVE) return;

        if (!this->hasVelPlan || this->isRisingEdge()) { // !hasVelPlan || 
            static_cast<T*>(this)->onPlan();
        }else if (this->isContinuousUpdate()) {
            static_cast<T*>(this)->onContinuousUpdate();
        }else{
            static_cast<T*>(this)->onOverride();
        }
        if (this->isError()) return;
        if (!this->isRunableByUpdateCycle(this->getAxImpl().m_cycleCounter)) return;
        static_cast<T*>(this)->onForward();
    }
};


template <typename T>
class FbtImplParamAbortingTmpl : public FbtImplParamMotTmpl<T> {
   public:
    // use in onStarted
    bool checkAxisStatBeforeActivate() {
        auto& ax = this->getAxImpl();
        return fbCheckAxisStatBeforeActivate(*this, ax.axisStat);
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
                ax.pNode.clearFbByAxisStat(mcStopping);
                ax.pNode.push_back(this);
                break;
            default:
                ax.pNode.push_back(this);
                break;
        }
    }

    void pushToBufferAndActivate(
        MC_AXIS_STATES axStat,
        AxisStateDetail detail = AXIS_STATE_DETAIL_DEFAULT) {
        if (!checkAxisStatBeforeActivate()) return;
        auto& ax = this->getAxImpl();
        if (!isAxisStateUpdatable(ax.axisStat, axStat)){
            this->onError(ZMC_AXIS_NOT_READY_FOR_MOTION);
            return;
        }
        pushToBuffer();
        if (!ax.pNode.isCurrentNode(this)) {
            this->fbStat = FB_STAT_UNACTIVATED;
        } else {
            ax.setAxisState(axStat, detail);
            this->fbStat = FB_STAT_BUSY_ACTIVE;
        } 
    }
    void onErrorWhenBuild(MC_ERROR_CODE err){
        auto& ax = this->getAxImpl();
        this->onError(err);
        if (ax.axisStatBak == mcStandstill || ax.axisStatBak == mcDisabled){
            ax.axisStat = mcStandstill;
            ax.cmd = ax.cmdBak;
            if (ax.pNode.isCurrentNode(this))
                ax.pNode.pop_front();
        }
    }
};



template <template<typename B, typename S> class BaseTmpl, typename T, typename P>
class FbExeAbortingTmplBase: public BaseTmpl<T, FbtImplParamAbortingTmpl<P>> {
public:
    void onPlan(){}
    void onForward(){}
    void onStarted() {
        if (FB_STAT_IDLE != this->fbStat) return;
        if (!this->get_axis_ref()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate(
            static_cast<T*>(this)->getAxisStateWhenActivated(),
            static_cast<T*>(this)->getAxisStateDetailWhenActivated());
    }
    void onExecuting() {
        if (!this->checkReceiveAbortError()) return;
        if (!this->checkAxisStatWhenActivated()) return;
        if (this->isRisingEdge()) {
            static_cast<T*>(this)->onPlan();
        }
        if (this->isError()) return;
        if (!this->isRunableByUpdateCycle(this->getAxImpl().m_cycleCounter)) return;
        static_cast<T*>(this)->onForward();
    }
};

// use for moveAdditive, halt , moveContinuous
template <typename T, typename P>
using FbExeAbortingTmpl = FbExeAbortingTmplBase<FbtExeTmpl, T, P>;

// use for trackPosition
template <typename T, typename P>
using FbExeLevelAbortingTmpl = FbExeAbortingTmplBase<FbtExeLevelTmpl, T, P>;

}
#endif
