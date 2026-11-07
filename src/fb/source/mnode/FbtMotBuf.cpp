#include "FbtMotBuf.h"
// #include "plcProj2_compat.h"
#include "fbt_axis_base.h"
#include "fbt_axis_impl.h"
#include "AxisStatInfo.h"
#include "FbImplNode.h"

namespace zmc {



FbtMotBuf::FbtMotBuf() {
    axisImpl_ = new AxisImplBase();
    axisInfo_ = new AxisStatInfo();
    pAxisInfo = axisInfo_;
    headNode_ = nullptr;
    currentNode_ = nullptr;
    initializeAxis();
}

FbtMotBuf::~FbtMotBuf() {
    clearAllFB(ClearReason::CLEAR_DISABLE);
    delete axisImpl_;
    delete axisInfo_;
}

void FbtMotBuf::runCycle() {
    syncToPlcProj2();

    if (isEmpty()) {
        syncFromPlcProj2();
        return;
    }
    
    for (int i = 0; i < 2; i++) {
        auto node = currentNode();
        if (node) {
            node->onExecuting();
            
            if (isAxisContainImpl(node)) {
                break;
            } else {
                pop(node);
            }
        } else {
            break;
        }
    }
    axisImpl_->m_cycleCounter++;
    syncFromPlcProj2();
}

void FbtMotBuf::initializeAxis(double cycleTime) {
    axisImpl_->cycleTime = cycleTime;
    // axisImpl_->fScalefactor = 1.0;
    // axisImpl_->fFactorVel = 1.0;
    // axisImpl_->fFactorAcc = 1.0;
    // axisImpl_->fFactorJerk = 1.0;
    // axisImpl_->axisStat = mcStandstill;
    // axisImpl_->m_cycleCounter = 0;
    

    // axisImpl_->fSWMaxVelocity = 1000.0;
    // axisImpl_->fSWMaxAcceleration = 1000.0;
    // axisImpl_->fSWMaxDeceleration = 1000.0;
    // axisImpl_->fSWMaxJerk = 10000.0;
    

    axisInfo_->axisTs = cycleTime;
    axisInfo_->ax_stat = mcStandstill;
    axisInfo_->tick = 0;
    axisInfo_->pos_cmd = 0.0;
    axisInfo_->vel_cmd = 0.0;
    axisInfo_->acc_cmd = 0.0;
}

void FbtMotBuf::syncToPlcProj2() {
    axisImpl_->fSavePosition = axisInfo_->pos_cmd;
    axisImpl_->fSaveVelocity = axisInfo_->vel_cmd;
    axisImpl_->fSaveAcceleration = axisInfo_->acc_cmd;
    axisImpl_->axisStat = axisInfo_->ax_stat;
    
    if (axisInfo_->tick > 0) {
        axisImpl_->m_cycleCounter = axisInfo_->tick;
    }
}

void FbtMotBuf::syncFromPlcProj2() {
    axisInfo_->pos_cmd = axisImpl_->fSetPosition;
    axisInfo_->vel_cmd = axisImpl_->fSetVelocity;
    axisInfo_->acc_cmd = axisImpl_->fSetAcceleration;
    axisInfo_->ax_stat = axisImpl_->axisStat;
    axisInfo_->tick = axisImpl_->m_cycleCounter;
    axisInfo_->update();
}

void FbtMotBuf::updateAxisInfo() {
    syncFromPlcProj2();
}

int FbtMotBuf::addPfbToQueue(FbImplBase* fb) {
    if (!fb) return -1;
    
    auto fbtNode = static_cast<FbtNode*>(static_cast<FbImplNode*>(fb)->pNode);
    if (!fbtNode) {
        return -3; 
    }

    fbtNode->onStarted();
    

    addNodeToQueue(fbtNode);
    
    return 0;
}

int FbtMotBuf::addFBToQueue(FbBase* fb) {
    FbImplBase* basePtr = static_cast<FbImplBase*>(fb->get_impl_ptr());
    return addPfbToQueue(basePtr);
}

bool FbtMotBuf::isEmpty() const {
    return headNode_ == nullptr;
}

void FbtMotBuf::clearAllFB(ClearReason reason) {
    axisImpl_->pNode.clearFbByAxisStat(static_cast<MC_AXIS_STATES>(reason));
    
    headNode_ = nullptr;
    currentNode_ = nullptr;

    syncFromPlcProj2();
}

void FbtMotBuf::get_commands(double& pos, double& vel, double& acc) const {
    pos = axisInfo_->pos_cmd;
    vel = axisInfo_->vel_cmd;
    acc = axisInfo_->acc_cmd;
}

FbtNode* FbtMotBuf::currentNode() {
    if (currentNode_) {
        return currentNode_;
    }
    
    return headNode_;
}

bool FbtMotBuf::isAxisContainImpl(FbtNode* node) {
    if (!node) return false;
    
    return axisImpl_->pNode.isCurrentNode((FbtImplParam*)node->fbtImpl);
}

void FbtMotBuf::pop(FbtNode* node) {
    if (!node) return;
    
    if (node->pPrev) {
        node->pPrev->pNext = node->pNext;
    } else {
        headNode_ = node->pNext;
    }
    
    if (node->pNext) {
        node->pNext->pPrev = node->pPrev;
    }

    if (currentNode_ == node) {
        currentNode_ = node->pNext;
    }
    
    node->pNext = nullptr;
    node->pPrev = nullptr;
}

void FbtMotBuf::addNodeToQueue(FbtNode* node) {
    if (!node) return;
    
    if (!headNode_) {
        headNode_ = node;
        currentNode_ = node;
        node->pNext = nullptr;
        node->pPrev = nullptr;
    } else {
        FbtNode* tail = headNode_;
        while (tail->pNext) {
            tail = tail->pNext;
        }
        
        tail->pNext = node;
        node->pPrev = tail;
        node->pNext = nullptr;

        if (!currentNode_) {
            currentNode_ = node;
        }
    }
}

} // namespace zmc 