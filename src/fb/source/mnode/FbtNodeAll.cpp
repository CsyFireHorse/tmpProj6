#include "FbtNodeAll.h"


#include "../fb/fbt_motion_impl.hpp"
#include "FbtMotBuf.h"
#include "FbtNode.h"

namespace zmc {
class FbtImplDisContBase;
using FbtNodeDisContBase = FbtNodeTmpl<FbtImplDisContBase>;

// mov.setMoveMode(mcMoveAbsolute);

PfbMoveDiscrete::PfbMoveDiscrete() { 
    pNode = new FbtNodeDisContBase(); 
}
PfbMoveDiscrete::~PfbMoveDiscrete() { delete (FbtNodeDisContBase*)pNode; }

void PfbMoveDiscrete::setBuf(FbtMotBuf* pMotbuf) { this->pMotbuf = pMotbuf; }
void PfbMoveDiscrete::setExecute(bool val) {
    this->ctrlParam.execute_ = val;
    static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam->Execute = val;
}

void PfbMoveDiscrete::runCycle() {
    // UnFbCtrl ctrlParam = {_ctrlParam};
    // static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->onCapture();

    auto fbtNode = static_cast<FbtNode*>(pNode);
    if (!fbtNode) {
        return;
    }
    _fbOStat.executed_ = ctrlParam.execute_;
    auto param = static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam;
    if (is_high_edge())
        param->Axis = (AXIS_REF) pMotbuf->axisImpl_;
    fbtNode->onStarted();
    if (is_high_edge()) {
        pMotbuf->addNodeToQueue(fbtNode);
    }
    _fbOStat.executed_bak = this->_fbOStat.executed_;
}

void PfbMoveDiscrete::setVelConfig(double vel, double acc, double dec, double jerk) {
    auto param = static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam;
    param->Velocity = vel;
    param->Acceleration = acc;
    param->Deceleration = dec;
    param->Jerk = jerk;
}
void PfbMoveDiscrete::setMotionType(int val) {
    static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam->MotionType = val;
}
void PfbMoveDiscrete::setPosition(double pos) {
    auto param = static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam;
    param->PosOrDist = pos;
}
void PfbMoveDiscrete::setBufferMode(MC_BUFFER_MODE val) {
    auto param = static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam;
    param->BufferMode = val;
}
bool PfbMoveDiscrete::isDone() const {
    auto param = static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam;
    return param->Done;
}
bool PfbMoveDiscrete::isBusy() const {
    auto param = static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam;
    return param->Busy;
}
bool PfbMoveDiscrete::isActive() const {
    return static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam->Active;
}
bool PfbMoveDiscrete::isCommandAborted() const {
    return static_cast<FbtNodeDisContBase*>(pNode)->getImplPtr()->pParam->CommandAborted;
}
}  // namespace zmc