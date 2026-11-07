#include "AxisGrpZmotWrap.h"
#include "../axgrp/AxisGrpZmotImpl.h"
#include "common/zmc/fbt_errorcode_def.h"

namespace zmc {

int AxisGrpZmotWrap::axNum() const {
    return get_grp_inner(*this).axNum;
}

double AxisGrpZmotWrap::cmdVel() const {
    return get_grp_inner(*this).cmdVel;
}

double AxisGrpZmotWrap::cmdAcc() const {
    return get_grp_inner(*this).cmdAcc;
}

double AxisGrpZmotWrap::cmdDec() const {
    return get_grp_inner(*this).cmdDec;
}

double AxisGrpZmotWrap::cmdJerk() const {
    return get_grp_inner(*this).cmdJerk;
}

void AxisGrpZmotWrap::setCmdVel(double v) {
    get_grp_inner(*this).cmdVel = v;
}

void AxisGrpZmotWrap::setCmdAcc(double v) {
    get_grp_inner(*this).cmdAcc = v;
}

void AxisGrpZmotWrap::setCmdDec(double v) {
    get_grp_inner(*this).cmdDec = v;
}

void AxisGrpZmotWrap::setCmdJerk(double v) {
    get_grp_inner(*this).cmdJerk = v;
}
int AxisGrpZmotWrap::setAxisRefArr(const AXIS_REF* axes, size_t count) {
    if (!axes) {
        return ZMC_AXIS_IS_NO_AXIS_REF;
    }
    if (count > static_cast<size_t>(kZmotMaxGroupAxes)) {
        return ZMC_ERR_AXISGRP_FULL;
    }

    // Validate all axes before mutating the group.
    for (size_t i = 0; i < count; ++i) {
        if (!axes[i]) {
            return ZMC_AXIS_IS_NO_AXIS_REF;
        }
    }

    auto& d = get_grp_inner(*this);
    // Repeated commands may select the same axes while the group is enabled.
    if (d.groupState != Disabled && count == static_cast<size_t>(d.axNum)) {
        size_t i = 0;
        while (i < count && d.axes[i].axisRef == axes[i]) ++i;
        if (i == count) return 0;
    }
    int ret = d.ungroupAll();
    if (ret) {
        return ret;
    }

    for (size_t i = 0; i < count; ++i) {
        ret = d.addAxis(axes[i]);
        if (ret) {
            return ret;
        }
    }
    return 0;
}

void AxisGrpZmotWrap::pushCmd(const void* cmdV) {
    const auto& cmd = *static_cast<const CommandStructDefault*>(cmdV);
    cmdQueue_.push(cmd);
}

bool AxisGrpZmotWrap::cmdQueueEmpty() const {
    return cmdQueue_.empty();
}

bool AxisGrpZmotWrap::cmdQueueFrontAndPop(void* cmdOutV) {
    if (cmdQueue_.empty()) return false;
    *static_cast<CommandStructDefault*>(cmdOutV) = cmdQueue_.front();
    cmdQueue_.pop_front();
    return true;
}

void* AxisGrpZmotWrap::fbListSentinel() {
    return fbList_.getSentinel();
}

void AxisGrpZmotWrap::fbListPushBack(void* node) {
    fbList_.pushBack(static_cast<FbMoveErasedBase*>(node));
}

void* AxisGrpZmotWrap::fbListRemoveNext(void* prev) {
    return fbList_.removeNext(static_cast<FbMoveErasedBase*>(prev));
}

bool AxisGrpZmotWrap::fbListEmpty() const {
    return fbList_.empty();
}

void AxisGrpZmotWrap::setAdminEnable(bool on) {
    fbEnable_.AxisGroup = this;
    fbEnable_.Execute = on;
}

void AxisGrpZmotWrap::setAdminDisable(bool on) {
    fbDisable_.AxisGroup = this;
    fbDisable_.Execute = on;
}

void AxisGrpZmotWrap::setAdminReset(bool on) {
    fbReset_.AxisGroup = this;
    fbReset_.Execute = on;
}

void AxisGrpZmotWrap::runAdminFbs() {
    fbEnable_.runCycle();
    fbDisable_.runCycle();
    fbReset_.runCycle();

    if (fbEnable_.Done || fbEnable_.Error) fbEnable_.Execute = false;
    if (fbDisable_.Done || fbDisable_.Error) fbDisable_.Execute = false;
    if (fbReset_.Done || fbReset_.Error) fbReset_.Execute = false;
}

void AxisGrpZmotWrap::runCycle(){
    runAdminFbs();
    AxisGrpZmot::runCycle();
}

void AxisGrpZmotWrap::reset_runtime() {
    FbMoveErasedBase* current = fbList_.begin();
    while (current) {
        FbMoveErasedBase* next = current->getNext();
        current->reset_runtime();
        current = next;
    }
    fbList_.clear();
    cmdQueue_.clear();
    fbEnable_.Execute = false;
    fbDisable_.Execute = false;
    fbReset_.Execute = false;
    AxisGrpZmot::reset_runtime();
}

}  // namespace zmc
