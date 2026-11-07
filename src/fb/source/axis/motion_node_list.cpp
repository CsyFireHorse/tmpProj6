#include "motion_node_list.h"
#include "motion_node_super.h"
#include "fbt_fb_base.hpp"
namespace zmc {
    
MotionNodeList::MotionNodeList() {
    head.pParam = nullptr;
    tail.pParam = nullptr;
}
void MotionNodeList::clearFbByAborting() {
    while (pHead->pNext != pTail) {
        current()->sendAborted();
        pop_front();
    }
}
void MotionNodeList::clearFbByAxisStat(MC_AXIS_STATES axisStat){
    if (axisStat == mcDisabled) {
        _clearFbWhenAxisDisabled();
    } else if (axisStat == mcErrorStop) {
        _clearFbWhenErrorStop();
    } else if (axisStat == mcStopping){
        clearFbByAborting();
    }
}

inline void sendFbDisable(FbtImplParam& node ){
    if (!node.isError()) {
        if (node.isActive()) {
            node.sendError(ZMC_ERR_POWEROFF_WHEN_MOVING);
        } else {
            node.sendError(ZMC_FB_ACTIVE_AXIS_DISABLED);
        }
    }
}
inline void sendFbError(FbtImplParam& node ){
    if (!node.isError() && !node.isErrorNext()) {
        if (node.isActive()) {
            node.sendError(ZMC_ERR_ERRORSTOP_WHEN_MOVING);
        } else {
            node.sendError(ZMC_ERR_FB_ACTIVATE_AXIS_ERRORSTOP);
        }
    }
}

void MotionNodeList::_clearFbWhenAxisDisabled() {
    while (pHead->pNext != pTail) {
        auto& node = *current();
        sendFbDisable(node);
        pop_front();
    }
}

void MotionNodeList::_clearFbWhenErrorStop() {
    while (pHead->pNext != pTail) {
        auto& node = *current();
        sendFbError(node);
        pop_front();
    }
}


void MotionNodeSuper::clearFbByAborting() {
    MotionNodeList::clearFbByAborting();
    if (pSuperimposedNode) {
        pSuperimposedNode->sendAborted();
        pSuperimposedNode = nullptr;
    }
}

void MotionNodeSuper::clearFbByAxisStat(MC_AXIS_STATES axisStat) {
    MotionNodeList::clearFbByAxisStat(axisStat);
    if (pSuperimposedNode) {
        if (axisStat == mcDisabled) {
            sendFbDisable(*pSuperimposedNode);
        } else if (axisStat == mcErrorStop) {
            sendFbError(*pSuperimposedNode);
        } else if (axisStat == mcStopping){
            pSuperimposedNode->sendAborted();
        }
        pSuperimposedNode = nullptr;
    }
}

void MotionNodeSuper::pushSuperimposedNode(FbtImplParam* pNode) {
    if (pSuperimposedNode) {
        pSuperimposedNode->sendAborted();
    }
    pSuperimposedNode = pNode;
}
FbtImplParam* MotionNodeSuper::popSuperimposedNode() {
    FbtImplParam* pNode = pSuperimposedNode;
    pSuperimposedNode = nullptr;
    return pNode;
}
void MotionNodeSuper::runCycle() {
    if (pSuperimposedNode && isEmpty()) { // && is_axis_stat_moving(axisStatBak)
        // 	movesuper.vel.shift_pos(fSetPos - distanceTravel_);
    }
    if (!isEmpty() && !pSuperimposedNode){
        // curNode.vel.shift_pos(distanceTravel_);
    }
}

}