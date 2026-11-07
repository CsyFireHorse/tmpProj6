
#include <iostream>
#include <cmath>
#include "math.h"
#include <stdio.h>
#include "fbt_fb_base.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_impl.h"
#include "vel/include/Vel4Wrap.h"
#include "fbt_move_impl.hpp"
#include "misc_helper.hpp"
#include "motion_node_lookahead_list.h"
// #include "fbt_motion_traits.hpp"


namespace zmc {


inline int get_sign(double val, double tolerance = 1e-6) {
    if (std::abs(val) < tolerance) return 0;
    return (val > 0) ? 1 : -1;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b)? (a) : (b))
inline double get_abs_min(double absVal, double val) {
    if (val > 0) {
        return MIN(absVal, val);
    } else {
        return MAX(-absVal, val);
    }
}



template <typename T, typename P>
class FbtImplMoveBlendBase : public FbExeBlendTmpl<T, P> {
public:
   protected:
    Vel4Wrap trapzVel;

   public: 
    double vEndLimit_ = 0;
    double vEndRelax_ = 0;
    double fResTime = 0;
    bool checkWakableAndActivate(MC_AXIS_STATES axStat){
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
            ax.axisStat = axStat;
            this->fbStat = FB_STAT_BUSY_ACTIVE;
            return true;
        }
        return false;
    }
    void onExecuting() {
        if (!this->checkReceiveAbortError()) return;
        if (this->fbStat != FB_STAT_BUSY_ACTIVE) {
            if (checkWakableAndActivate(static_cast<T*>(this)->getAxisStateWhenActivated())){
                static_cast<T*>(this)->onWakeup();
                onForward();
                return;
            }
            return;
        }else if (!this->checkAxisStatWhenActivated()) return;
        
        if (this->isRisingEdge()) { 
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
    inline void set_vel_config(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        trapzVel.set_config(param.Velocity * ax.fFactorVel, param.Acceleration * ax.fFactorAcc, param.Deceleration * ax.fFactorAcc, param.Jerk * ax.fFactorJerk);
    }
    void onWakeup(){ 
        auto& ax = this->getAxImpl();
        if (ax.byResTimeNumer > 0){
            double resTime = ax.get_res_time();
            onWakeupBlend(resTime);
        }else{
            onWakeupEasy();
        }
    }
    void onWakeupEasy(){
        if (!this->bTrajBuilt)   
            onBuildTraj();
        else{
            trapzVel.set_inversable(true);
        }
        if (!this->hasVelPlan)     
            onPlanVel();
    }
    
    void onPlan() {
        onBuildTraj();
        onPlanVel();
    }
    void onBuildTraj(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        double posOrDist = get_pos_or_distance();
        if (1) { // !this->hasVelPlan
            ax.fMarkPosition = ax.fSavePosition;
            ax.fMarkVelocity = ax.fSaveVelocity;
            ax.fMarkAcceleration = ax.fSaveAcceleration;
            ax.fAimPosition = calc_end_position(ax.fMarkPosition);
            trapzVel.set_mode(ax.eRampType);
            trapzVel.set_begin_pva(ax.fMarkPosition, ax.fMarkVelocity, ax.fMarkAcceleration);
            trapzVel.set_end_pva(ax.fAimPosition, 0, 0);
            this->bTrajBuilt = true;
        }
        trapzVel.set_inversable(true);
    }

    void onContinuousUpdate(){
        auto& param = *this->pParam;
        // FIXME: chop trapzVel
        auto& ax = this->getAxImpl();
        if (param.Velocity * ax.fFactorVel == trapzVel.get_max_vel()) return;
        onReplanVel();
    }
    void onOverride(){
        auto& ax = this->getAxImpl();
        if (ax.isOverrideMatch(this->bOverrideParity)) return;
        onReplanVel();
    }
    void onBlend(){
        onReplanVel();
        onForward();
    }
    void onReplanVel(){   
        onChop();
        onPlanVel();
    }
    void onChop(){   
        auto& ax = this->getAxImpl();
        if (this->startTick + 1 >= ax.m_cycleCounter) return;
        trapzVel.chop_left_time(ax.get_offset_time(this->startTick + 1));
    }

    void onPlanVel(){
        auto& param = *this->pParam;
        auto& ax = this->getAxImpl();
        
        trapzVel.set_end_vel(vEndRelax_);
        if (param.Velocity <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0)
            return this->onError(ZMC_INVALID_VELACCJERK_VALUES);
        if (!ax.isOverrideMatch(this->bOverrideParity)) this->incOverrideParity();
        set_vel_config();
        if (trapzVel.build() != 0) 
            return this->onError(ZMC_ERR_VEL_BUILD_FAILED);
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
        this->hasVelPlan = true;
    }

    void onForward(){
        if (this->isError()) return;
        auto& ax = this->getAxImpl();
        this->updateCycleTick(this->getAxImpl().m_cycleCounter);

        ax.cmd = AXIS_CMD_MOTION;
        PosVelAcc pva;
        int ret = trapzVel.get_slice(ax.get_offset_time(this->startTick), pva);

        ax.fSetPosition = pva.pos;
        ax.fSetVelocity = pva.vel;
        ax.fSetAcceleration = pva.acc;

        if (ret == VEL_END_PT_REACHED) {
            ax.axisStat = mcStandstill;
            this->fbStat = FB_STAT_DONE;
            if (ax.pNode.hasNext()){
                auto pNextFb = static_cast<FbtImplParamBufTmpl<FbtMoveAbsoluteStruct>*>((void*) this->pNext);
                if (1 || pNextFb->canBlendNext()) { // is_blendable()
                    double fResTime = ax.cycleTime - ax.get_offset_time(this->startTick) + trapzVel.get_end_time();
                    ax.set_res_time(fResTime);
                    // pNextFb->onWakeupBlend(fResTime);
                }
            }
            if (ax.pNode.isCurrentNode(this))
                ax.pNode.pop_front();
            
        }
    }
    void onWakeupBlend(double resTime){
        auto& ax = this->getAxImpl();
        ax.fAimPosition = update_next_begin_position(ax.fSetPosition);
        trapzVel.set_begin_vel(ax.fSetVelocity);
        this->bTrajBuilt = true;
        onPlanVel();
        trapzVel.shift_time(resTime);
        // onForward();
    }
    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->bNodeStat_ = 0;
        this->bCtrlParam_ = 0;
        this->startTick = 0;
        this->vEndLimit_ = 0;
        this->vEndRelax_ = 0;
        this->fResTime = 0;
    }
    inline void onCapture(){
        this->execute = this->pParam->Execute;
    }

    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!this->get_axis_ref()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        this->pushToBufferAndActivate(static_cast<T*>(this)->getAxisStateWhenActivated());
        if (this->fbStat == FB_STAT_UNACTIVATED) {
            this->byBufferMode_ = get_buffer_mode();
            if (get_buffer_mode()>mcBuffered && this->pPrev->canBlendNext()) {

                // this->bCanBlending = true;
                this->getAxImpl().byMotbufEvent |= 1;
            }
        }
    }

    // Lookahead algorithm adaptation functions
    // can blend next fb
    inline bool is_fb_blendable() const {
        return (this->fbType == FB_TYPE_MOVEABSOLUTE || 
                this->fbType == FB_TYPE_MOVERELATIVE || (
                    this->fbType == FB_TYPE_MOVEDISCRETE
                ));
    }
    // can blend prev fb
    bool is_blendable() const {
        return (this->is_fb_blendable() && isBufferModeBlendable(get_buffer_mode()));
    }

    double get_pos_or_distance() const {
        if (this->fbType == FB_TYPE_MOVEABSOLUTE) {
            return ((FbtMoveAbsoluteStruct*)this->pParam)->Position;
        } else if(this->fbType == FB_TYPE_MOVERELATIVE) {
            return ((FbtMoveRelativeStruct*)this->pParam)->Distance;
        } else if (this->fbType == FB_TYPE_MOVEDISCRETE) {
            return ((FbtMoveDiscreteStruct*)this->pParam)->PosOrDist;
        } else {
            return ((FbtMoveRelativeStruct*)this->pParam)->Distance;
        }
    }

    inline double get_velocity() const {
        if (this->fbType == FB_TYPE_MOVEABSOLUTE) {
            return this->pParam->Velocity;
        }else if (this->fbType == FB_TYPE_MOVERELATIVE) {
            return this->pParam->Velocity;
        }else if (this->fbType == FB_TYPE_MOVEDISCRETE) {
            return this->pParam->Velocity;
        }else{
            return this->pParam->Velocity;
        }
    }

    // double get_acceleration() const {
    //     return this->pParam->Acceleration;
    // }

    // double get_deceleration() const {
    //     return this->pParam->Deceleration;
    // }

    MC_BUFFER_MODE get_buffer_mode() const {
        if (this->fbType == FB_TYPE_MOVEABSOLUTE) {
            return ((FbtMoveAbsoluteStruct*)this->pParam)->BufferMode;
        } else if (this->fbType == FB_TYPE_MOVERELATIVE) {
            return ((FbtMoveRelativeStruct*)this->pParam)->BufferMode;
        } else if (this->fbType == FB_TYPE_MOVEDISCRETE) {
            return ((FbtMoveDiscreteStruct*)this->pParam)->BufferMode;
        } else {
            return mcAborting;
        }
        // return get_buffer_mode_or_default(*this->pParam);
    }
    
    double update_next_begin_position(double begin_pos) {
        double endPos = calc_end_position(begin_pos);
        trapzVel.set_begin_pos(begin_pos);
        trapzVel.set_end_pos(endPos);
        return endPos;
    }
    double calc_end_position(double begin_pos) const{
        auto& ax = this->getAxImpl();
        switch (this->fbType)
        {
        case FB_TYPE_MOVEABSOLUTE:{
            double endPos = get_pos_or_distance();
            if (this->pParam->Axis && ax.is_modulo_type())
                endPos = calcTargetPosByDirection(get_direction_or_default<P>(*this->pParam), begin_pos, endPos,
                     ax.fPositionPeriod, ax.nDirection==mcNegativeDirection);
            return endPos;
        }
        case FB_TYPE_MOVERELATIVE:
            return get_pos_or_distance() + begin_pos;
        case FB_TYPE_MOVEDISCRETE:
            if (static_cast<FbtMoveDiscreteStruct*>((void*) this->pParam)->MotionType == 0)
                return get_pos_or_distance();
            else 
                return get_pos_or_distance() + begin_pos;
        default:
            return begin_pos;
        }
    }
    double get_traj_end_position() const {
        return trapzVel.get_end_pos();

    }
    double get_traj_delta_pos() const {
        return trapzVel.get_delta_pos();
    }
    
    int get_movement_direction() const {
        return get_sign(trapzVel.get_delta_pos());
    }
    
    double cal_reachable_forward_vel(double current_vel) const {
        return trapzVel.cal_reachable_forward_vel(current_vel);
    }
    
    double cal_reachable_backward_vel(double target_vel) const {
        return trapzVel.cal_reachable_backward_vel(target_vel);
    }
    
    double calculate_v_end_limit(const FbtImplMoveBlendBase* next_node) const {
        if (!next_node) return 0.0;
        
        double current_vel = get_velocity();
        double next_vel = next_node->get_velocity();
        
        switch (next_node->get_buffer_mode()) {
            case MC_BUFFER_MODE::mcBlendingLow:
                return MIN(current_vel, next_vel);
            case MC_BUFFER_MODE::mcBlendingHigh:
                return MAX(current_vel, next_vel);
            case MC_BUFFER_MODE::mcBlendingPrevious:
                return current_vel;
            case MC_BUFFER_MODE::mcBlendingNext:
                return next_vel;
            default:
                return 0.0;
        }
    }

    // Getters and setters for v_end_limit and v_end_relax
    double get_v_end_limit() const { return vEndLimit_; }
    void set_v_end_limit(double val) { vEndLimit_ = val; }
    double get_v_end_relax() const { return vEndRelax_; }
    void set_v_end_relax(double val) { vEndRelax_ = val; }
};


// only use for type conversion in motion_node_lookahead
class FbtMoveAbsOrRel : public FbtImplMoveBlendBase<FbtMoveAbsOrRel, FbtMoveAbsoluteStruct>{
};


void setFbImplVelEndLimit(void* pImpl, double val){
    auto p = static_cast<FbtExeBase*>(pImpl);
    if (p->fbType == FB_TYPE_MOVEABSOLUTE || p->fbType == FB_TYPE_MOVERELATIVE || p->fbType == FB_TYPE_MOVEDISCRETE){
        auto pMove = static_cast<FbtMoveAbsOrRel*>(pImpl);
        pMove->vEndLimit_ = val;
    }
}

class FbtMoveAbsoluteImpl : public FbtImplMoveBlendBase<FbtMoveAbsoluteImpl, FbtMoveAbsoluteStruct> {
public:
    FbtMoveAbsoluteImpl(){
        this->fbType = FB_TYPE_MOVEABSOLUTE;
    }

    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = (this->isDone());
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->Active = this->isActive();
        this->pParam->ErrorID = this->getErrorID();
    }
};


FbtMoveAbsolute::FbtMoveAbsolute() {
    make_impl<FbtMoveAbsoluteImpl, FbtMoveAbsoluteStruct>(*this);
}

FbtMoveAbsolute::~FbtMoveAbsolute() {
    delete static_cast<FbtMoveAbsoluteImpl*>(pImpl);
    pImpl = nullptr;
}

void FbtMoveAbsolute::runCycle() {
    static_cast<FbtMoveAbsoluteImpl*>(pImpl)->runCycle();
}

class FbtMoveRelativeImpl : public FbtImplMoveBlendBase<FbtMoveRelativeImpl, FbtMoveRelativeStruct> {
public:
    FbtMoveRelativeImpl(){
        this->fbType = FB_TYPE_MOVERELATIVE;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = (this->isDone());
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->Active = this->isActive();
        this->pParam->ErrorID = this->getErrorID();;
    }
};

FbtMoveRelative::FbtMoveRelative() {
    make_impl<FbtMoveRelativeImpl, FbtMoveRelativeStruct>(*this);
}

FbtMoveRelative::~FbtMoveRelative() {
    delete static_cast<FbtMoveRelativeImpl*>(pImpl);
    pImpl = nullptr;
}
void FbtMoveRelative::runCycle() {
    auto impl = static_cast<FbtMoveRelativeImpl*>(pImpl);
    impl->runCycle();
}

class FbtMoveDiscreteImpl : public FbtImplMoveBlendBase<FbtMoveDiscreteImpl, FbtMoveDiscreteStruct> {
public:
    FbtMoveDiscreteImpl(){
        this->fbType = FB_TYPE_MOVEDISCRETE;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = (this->isDone());
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->Active = this->isActive();
        this->pParam->ErrorID = this->getErrorID();;
    }
};

FbtMoveDiscrete::FbtMoveDiscrete() {
    make_impl<FbtMoveDiscreteImpl, FbtMoveDiscreteStruct>(*this);
}
FbtMoveDiscrete::~FbtMoveDiscrete() {
    delete static_cast<FbtMoveDiscreteImpl*>(pImpl);
    pImpl = nullptr;
}
void FbtMoveDiscrete::runCycle() {
    static_cast<FbtMoveDiscreteImpl*>(pImpl)->runCycle();
}

class FbtMoveDisOrVelImpl : public FbtImplMoveBlendBase<FbtMoveDisOrVelImpl, FbtMoveDisOrVelStruct> {
public:
    FbtMoveDisOrVelImpl(){
        this->fbType = FB_TYPE_MOVEDISORVEL;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = (this->isDone());
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = (this->isAborted());
        this->pParam->Active = this->isActive();
        this->pParam->ErrorID = this->getErrorID();;
    }
};

FbtMoveDisOrVel::FbtMoveDisOrVel() {
    make_impl<FbtMoveDisOrVelImpl, FbtMoveDisOrVelStruct>(*this);
}
FbtMoveDisOrVel::~FbtMoveDisOrVel() {
    delete static_cast<FbtMoveDisOrVelImpl*>(pImpl);
    pImpl = nullptr;
}
void FbtMoveDisOrVel::runCycle() {
    static_cast<FbtMoveDisOrVelImpl*>(pImpl)->runCycle();
}

class FbtMoveDisOrContImpl : public FbtImplMoveBlendBase<FbtMoveDisOrContImpl, FbtMoveDisOrContStruct> {
public:
    FbtMoveDisOrContImpl(){
        this->fbType = FB_TYPE_MOVEDISORCONT;
    }
};

FbtMoveDisOrCont::FbtMoveDisOrCont() {
    make_impl<FbtMoveDisOrContImpl, FbtMoveDisOrContStruct>(*this);
}
FbtMoveDisOrCont::~FbtMoveDisOrCont() {
    delete static_cast<FbtMoveDisOrContImpl*>(pImpl);
    pImpl = nullptr;
}
void FbtMoveDisOrCont::runCycle() {
    static_cast<FbtMoveDisOrContImpl*>(pImpl)->runCycle();
}



// Helper function to check if a node is blendable
bool MotionNodeLookaheadList::isNodeBlendable(const Node<FbtImplParam>* node) const {
    if (!node) return false;
    
    // Direct cast to FbtImplMoveBlendBase and check if it's blendable
    auto moveBlendNode = static_cast<const FbtMoveAbsOrRel*>((void*) node);
    return moveBlendNode->is_blendable();
}

// Detect number of consecutive blendable nodes
int MotionNodeLookaheadList::detectBlendableNum() const {
    auto p = pHead->pNext;
    if (p == pTail) return 0;
    
    // Check if first node can blend next
    if (!p->canBlendNext()) {
        return 0; // !fbArr[0].bCanBlend 
    }
    
    int cnt = 1; // At least one node (the first one)
    auto current = p;
    auto next = p->pNext;
    
    // Check consecutive pairs: current->bCanBlend && next->bCanBlending
    while (next != pTail && current->canBlendNext() && next->canBlendPrev()) {
        cnt++;
        current = next;
        next = next->pNext;
    }
    
    return cnt;
}

void MotionNodeLookaheadList::push_fb(void* pFb) {
    // Cast the FB to get its pImpl
    auto fbStruct = static_cast<FbtMoveAbsoluteStruct*>(pFb);
    auto node = static_cast<Node<FbtImplParam>*>(fbStruct->pImpl);
    auto node2 = static_cast<FbtMoveAbsOrRel*>((void*) fbStruct->pImpl);
    node2->byBufferMode_ = node2->get_buffer_mode();
    push_back(node);
}

// Process begin position and check direction consistency
int MotionNodeLookaheadList::process_begin_position(double next_begin_pos) {
    int prev_direction = 0;
    int actual_count = 0;
    
    auto p = pHead->pNext;
    if (p == pTail) return 0;
    
    // Check if first node can blend next
    if (!p->canBlendNext()) {
        return 0; // !fbArr[0].bCanBlend
    }
    
    int i = 0;
    auto current = p;
    auto next = p->pNext;
    
    while (current != pTail) {
        // Check if current node can blend with next node
        if (next != pTail && !(current->canBlendNext() && next->canBlendPrev())) {
            break;
        }
        
        auto moveBlendNode = static_cast<FbtMoveAbsOrRel*>((void*) current);
        int current_direction;
        if (!moveBlendNode->bTrajBuilt)
             moveBlendNode->update_next_begin_position(next_begin_pos);
        next_begin_pos = moveBlendNode->get_traj_end_position();
        current_direction = get_sign(moveBlendNode->get_traj_delta_pos());
        
        // Break lookahead if direction changes (ignore zero direction)
        if (i > 0 && current_direction != prev_direction && 
            prev_direction != 0 && current_direction != 0) {
            break;
        }
        if (i > 0) {
            current->bBlendPrev = true;
            if (next != pTail) {
                next->bBlendNext = true;
            }
        }
        actual_count++;
        prev_direction = current_direction;
        
        // Move to next node
        current = next;
        if (next != pTail) {
            next = next->pNext;
        }
        i++;
    }
    return actual_count;
}

// Calculate vEndLimit for each FB based on blend mode
void MotionNodeLookaheadList::calculate_v_end_limit(int count) {
    auto p = pHead->pNext;
    for (int i = 0; i < count - 1 && p != pTail && p->pNext != pTail; i++) {
        auto next_node = p->pNext;
        if (!p || !next_node) {
            p = p->pNext;
            continue;
        }
        
        auto moveBlendNode = static_cast<FbtMoveAbsOrRel*>((void*) p);
        auto nextMoveBlendNode = static_cast<FbtMoveAbsOrRel*>((void*) next_node);
        
        if (moveBlendNode && nextMoveBlendNode) {
            double v_end_limit = moveBlendNode->calculate_v_end_limit(nextMoveBlendNode);
            if (v_end_limit > 0 ){
                if (moveBlendNode->get_v_end_limit() == 0 || v_end_limit <= moveBlendNode->get_v_end_limit()){
                    moveBlendNode->set_v_end_limit(v_end_limit);
                    moveBlendNode->set_v_end_relax(v_end_limit); // Initialize
                }
            }
        }
        p = p->pNext;
    }
}

// Step 2&3: Combined forward and backward pass
int MotionNodeLookaheadList::forward_backward_pass(int count, double current_vel) {
    // Forward pass - calculate ascending transition velocities
    double vc = current_vel;
    auto p = pHead->pNext;
    static_cast<FbtMoveAbsOrRel*>((void*) p)->onChop();
    for (int i = 0; i < count && p != pTail; i++) {
        if (!p) {
            p = p->pNext;
            continue;
        }
        
        auto moveBlendNode = static_cast<FbtMoveAbsOrRel*>((void*) p);
        if (!moveBlendNode) {
            p = p->pNext;
            continue;
        }
        
        moveBlendNode->set_vel_config();
        // Calculate maximum reachable velocity for forward transition
        vc = moveBlendNode->cal_reachable_forward_vel(vc);
        
        // Take minimum with vEndLimit and save to vEndRelax
        vc = get_abs_min(moveBlendNode->get_v_end_limit(), vc);
        moveBlendNode->set_v_end_relax(vc);
        
        p = p->pNext;
    }
    double vd = vc;
    // Backward pass - calculate descending transition velocities
    vc = 0.0;
    
    // Start from the last processed node (p is now at the end)
    p = p->pPrev;
    
    // Backward iteration using double-linked pointers
    while (p != pHead->pNext && p->pPrev != pHead) {
        auto prev_node = p->pPrev;
        
        auto moveBlendNode = static_cast<FbtMoveAbsOrRel*>((void*) p);
        auto prevMoveBlendNode = static_cast<FbtMoveAbsOrRel*>((void*) prev_node);
        
        if (moveBlendNode && prevMoveBlendNode) {
            // Calculate maximum reachable velocity for backward transition
            vc = moveBlendNode->cal_reachable_backward_vel(vc);
            
            // Take minimum with current vEndRelax and update previous node
            vc = get_abs_min(std::abs(prevMoveBlendNode->get_v_end_relax()), vc);
            prevMoveBlendNode->set_v_end_relax(vc);
        }
        
        p = p->pPrev;
    }
    {
        auto moveBlendNode = static_cast<FbtMoveAbsOrRel*>((void*) p);
        vc = moveBlendNode->cal_reachable_backward_vel(vc);
    }
    if (fabs(vc) < fabs(current_vel)) {
        printf("unreachable \n");
        return -1;
    }
    return 0;
}

// Step 4: Replan the first FB with new end velocity
void MotionNodeLookaheadList::replan_first_fb(Node<FbtImplParam>* first_node) {
    if (!first_node) return;
    
    auto moveBlendNode = static_cast<FbtMoveAbsOrRel*>((void*) first_node);
    if (moveBlendNode) {
        moveBlendNode->onBlend();
        // In real implementation, this would trigger trajectory replanning
        // For now, we just store the calculated end velocity
    }
}

// Main lookahead algorithm implementation (4 steps)
bool MotionNodeLookaheadList::lookahead(const MotionNodeLookaheadParam& param) {
    // Step 1: Traverse FBs, process only those with bBlendNext==true
    // Calculate movement direction, break lookahead if direction changes
    int actual_count = process_begin_position(param.current_position);
    if (actual_count < 2) {
        return false;
    }
    
    // Step 2: Calculate vEndLimit for each FB based on blend mode
    calculate_v_end_limit(actual_count);
    
    // Step 3&4: Combined forward and backward pass
    int ret = forward_backward_pass(actual_count, param.current_velocity);
    if (ret == -1) {
        return false;
    }
    
    // Step 5: Select first FB and recalculate velocity planning using vEndRelax
    replan_first_fb(pHead->pNext);
    
    return true;
}

void MotionNodeLookaheadList::print_results(int count) const {
    std::cout << "\n--- Final Results ---" << std::endl;
    auto p = pHead->pNext;
    for (int i = 0; i < count && p != pTail; i++) {
        if (!p) {
            p = p->pNext;
            continue;
        }
        
        auto moveBlendNode = static_cast<const FbtMoveAbsOrRel*>((void*) p);
        if (moveBlendNode) {
            const char* typeNameArr[] = {"NOP", "MOVEABS", "MOVEREL", "MOVEADD"};
            const char* type_name = typeNameArr[static_cast<int>(moveBlendNode->fbType - FB_TYPE_MOVEABSOLUTE + 1)];

            std::cout << "Node " << i << ": Type=" << type_name 
                     << ", Pos/Dist=" << moveBlendNode->get_pos_or_distance()
                     << ", Vel=" << moveBlendNode->get_velocity()
                     << ", vEndLimit=" << moveBlendNode->get_v_end_limit()
                     << ", vEndRelax=" << moveBlendNode->get_v_end_relax() << std::endl;
        }
        
        p = p->pNext;
    }
}

// Get lookahead information as vector for testing
std::vector<LookaheadNodeInfo> MotionNodeLookaheadList::getLookaheadInfo(int count) const {
    std::vector<LookaheadNodeInfo> result;
    auto p = pHead->pNext;
    
    for (int i = 0; i < count && p != pTail; i++) {
        if (!p) {
            p = p->pNext;
            continue;
        }
        
        LookaheadNodeInfo info;
        info.nodeIndex = i;
        
        auto moveBlendNode = static_cast<const FbtMoveAbsOrRel*>((void*) p);
        if (moveBlendNode) {
            const char* typeNameArr[] = {"NOP", "MOVEABS", "MOVEREL", "MOVEADD"};
            info.typeName = typeNameArr[static_cast<int>(moveBlendNode->fbType - FB_TYPE_MOVEABSOLUTE + 1)];
            info.posOrDist = moveBlendNode->get_traj_delta_pos();
            info.velocity = moveBlendNode->get_velocity();
            info.vEndLimit = moveBlendNode->get_v_end_limit();
            info.vEndRelax = moveBlendNode->get_v_end_relax();
            info.canBlendNext = moveBlendNode->canBlendNext();
            info.canBlendPrev = moveBlendNode->canBlendPrev();
        }
        
        result.push_back(info);
        p = p->pNext;
    }
    
    return result;
}

// Main cycle function - detects new nodes and triggers lookahead
void MotionNodeLookaheadList::runCycle(const MotionNodeLookaheadParam& param) {
    if (param.event.value == 0) return;
    if (!hasNext()) return;
    lookahead(param);
    // print_results(detectBlendableNum());
}


} // namespace zmc 

using namespace zmc;

// Demo function for testing the integrated lookahead algorithm
void lookahead_demo() {
    std::cout << "Motion Lookahead Algorithm Demo (Integrated)" << std::endl;

    // Create lookahead list
    MotionNodeLookaheadList lookahead_list;
    MotionNodeLookaheadParam param;
    param.event.isBlend = true;
    param.current_position = 0.0;
    param.current_velocity = 10.0;
    
    // Create motion commands
    FbtMoveAbsolute move1;
    move1.Position = -30.0;
    move1.Velocity = 50.0;
    move1.Acceleration = 80.0;
    move1.Deceleration = 60.0;
    move1.BufferMode = MC_BUFFER_MODE::mcBlendingLow;
    
    FbtMoveRelative move2;
    move2.Distance = -25.0;
    move2.Velocity = 40.0;
    move2.Acceleration = 70.0;
    move2.Deceleration = 50.0;
    move2.BufferMode = MC_BUFFER_MODE::mcBlendingHigh;
    
    FbtMoveRelative move3;
    move3.Distance = -15.0;
    move3.Velocity = 60.0;
    move3.Acceleration = 90.0;
    move3.Deceleration = 70.0;
    move3.BufferMode = MC_BUFFER_MODE::mcBlendingNext;
    
    FbtMoveAbsolute move4;
    move4.Position = 200.0;
    move4.Velocity = 45.0;
    move4.Acceleration = 100.0;
    move4.Deceleration = 80.0;
    move4.BufferMode = MC_BUFFER_MODE::mcBlendingNext;
    
    // Add nodes to list
    lookahead_list.push_fb(&move1);
    lookahead_list.push_fb(&move2);
    lookahead_list.push_fb(&move3);
    lookahead_list.push_fb(&move4);
    
    int actual_count = lookahead_list.detectBlendableNum();
    printf("detectBlendableNum = %d\n", actual_count);
    
    // Run the lookahead algorithm
    lookahead_list.runCycle(param);
    
    // Print final results
    lookahead_list.print_results(actual_count);
    
    // Get lookahead information as vector for testing
    std::cout << "\n--- Lookahead Info Vector ---" << std::endl;
    auto lookaheadInfo = lookahead_list.getLookaheadInfo(actual_count);
    for (const auto& info : lookaheadInfo) {
        std::cout << "Node " << info.nodeIndex 
                 << ": Type=" << info.typeName
                 << ", Pos/Dist=" << info.posOrDist
                 << ", Vel=" << info.velocity
                 << ", vEndLimit=" << info.vEndLimit
                 << ", vEndRelax=" << info.vEndRelax
                 << ", canBlendNext=" << (info.canBlendNext ? "true" : "false")
                 << ", canBlendPrev=" << (info.canBlendPrev ? "true" : "false")
                 << std::endl;
    }
}