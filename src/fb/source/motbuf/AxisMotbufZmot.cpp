#include "AxisMotbufZmot.h"

#include <cstring>

#include "common/zmc/fbt_errorcode_def.h"
#include "fbt_impl_enum_def.h"
#include "motbuf/FbMoveWrapV3.hpp"
#include "motbuf/MotionCmdTypes.hpp"
#include "pou/Ctx.h"
#include "traj/include/TrajEnumDef.h"
namespace zmc {

namespace {

static bool decodeCircularMode(uint16_t cmdType, uint16_t& motionType, uint16_t& circMode,
                               uint16_t& pathChoice, const MotionCmdCircularParam& param) {
    switch (cmdType) {
        case FBM_MULTI_CIRCULAR:
            if (param.pathChoice != 0 && param.pathChoice != 1) {
                return false;
            }
            motionType = 1;
            circMode = static_cast<uint16_t>(CIRC_MODE_CENTER);
            pathChoice = param.pathChoice;
            return true;
        case FBM_MULTI_CIRCULAR_ABS:
            if (param.pathChoice != 0 && param.pathChoice != 1) {
                return false;
            }
            motionType = 0;
            circMode = static_cast<uint16_t>(CIRC_MODE_CENTER);
            pathChoice = param.pathChoice;
            return true;
        case FBM_MULTI_CIRCULAR2:
            motionType = 1;
            circMode = static_cast<uint16_t>(CIRC_MODE_BORDER);
            pathChoice = 0;
            return true;
        case FBM_MULTI_CIRCULAR2_ABS:
            motionType = 0;
            circMode = static_cast<uint16_t>(CIRC_MODE_BORDER);
            pathChoice = 0;
            return true;
        default:
            return false;
    }
}

}  // namespace

FbMotionGroupZmot::FbMotionGroupZmot() {}

void FbMotionGroupZmot::reset() {
    group_ = nullptr;
    fb_.reset();
}

void FbMotionGroupZmot::bindGroup(AxisGrpZmotWrap* group) { group_ = group; }

bool FbMotionGroupZmot::bindCmd(const CommandStructDefault& cmd) {
    if (!group_ || group_->axNum() <= 0 || group_->axNum() > kZmotMaxGroupAxes) return false;

    fb_.AxisGroup = group_;
    fb_.Velocity = group_->cmdVel();
    fb_.Acceleration = group_->cmdAcc();
    fb_.Deceleration = group_->cmdDec();
    fb_.Jerk = group_->cmdJerk();
    fb_.BufferMode = mcBuffered;

    if (cmd.cmdType == FBM_MULTI_MOVE || cmd.cmdType == FBM_MULTI_MOVE_REL) {
        size_t count = 0;
        const double* posArr = cmd.getDoubleArray(count);
        if (!posArr || static_cast<int>(count) != group_->axNum()) return false;

        fb_.MotionType = (cmd.cmdType == FBM_MULTI_MOVE_REL) ? 1 : 0;
        fb_.MMode = 0;
        for (int i = 0; i < group_->axNum(); ++i) {
            fb_.EndPosition.v[i] = posArr[i];
        }
        return true;
    }

    if (group_->axNum() != 2 || cmd.paramLen != sizeof(MotionCmdCircularParam)) {
        return false;
    }

    MotionCmdCircularParam param = {};
    std::memcpy(&param, cmd.getParamData(), sizeof(param));
    uint16_t motionType = 0;
    uint16_t circMode = 0;
    uint16_t pathChoice = 0;
    if (!decodeCircularMode(cmd.cmdType, motionType, circMode, pathChoice, param)) {
        return false;
    }

    fb_.MotionType = motionType;
    fb_.MMode = 1;
    fb_.PathChoice = pathChoice;
    fb_.CircMode = circMode;
    for (int i = 0; i < 2; ++i) {
        fb_.AuxPoint.v[i] = param.aux[i];
        fb_.EndPosition.v[i] = param.end[i];
    }
    return true;
}

void FbMotionGroupZmot::setExecute(bool on) { fb_.Execute = on; }

uint8_t FbMotionGroupZmot::getStat() const { return fb_.getStat(); }

uint16_t FbMotionGroupZmot::getErrorId() const { return fb_.getErrorId(); }

MC_BUFFER_MODE FbMotionGroupZmot::getBufferMode() const { return fb_.BufferMode; }

void FbMotionGroupZmot::setNext(FbMotionGroupZmot* next) { FbMoveErasedBase::setNext(next); }

FbMotionGroupZmot* FbMotionGroupZmot::getNext() const {
    return static_cast<FbMotionGroupZmot*>(FbMoveErasedBase::getNext());
}

void FbMotionGroupZmot::runCycle() { fb_.runCycle(); }

int AxisMotbufZmot::pushCommand(const CommandStructDefault& cmd) {
    if (cmd.cmdType == FBM_GROUP_SELECT) {
        setGroupSelect(cmd.getInt());
        return 0;
    }

    if (groupSelect_ < 0 || groupSelect_ >= kZmotMaxGroups) {
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }
    groups_[groupSelect_].pushCmd(&cmd);
    return 0;
}

void AxisMotbufZmot::runCycleTop() {
    for (size_t i = 0; i < groupCount_; ++i) {
        auto& group = groups_[i];
        while (!group.cmdQueueEmpty()) {
            CommandStructDefault cmd{};
            if (!group.cmdQueueFrontAndPop(&cmd)) break;
            const int error = processGroupCommand(group, cmd);
            if (cmd.id != 0 && confirmTarget_) {
                CmdResult confirm = {};
                confirm.id = cmd.id;
                confirm.command_type = static_cast<uint16_t>(cmd.cmdType);
                confirm.target_index = static_cast<uint8_t>(i);
                confirm.target_kind = static_cast<uint8_t>(CMD_TARGET_GROUP);
                const bool acceptedOnly =
                    (cmd.cmdType >= FBM_MULTI_MOVE && cmd.cmdType <= FBM_MULTI_CIRCULAR2_ABS) ||
                    cmd.cmdType == FBM_GROUP_ENABLE || cmd.cmdType == FBM_GROUP_DISABLE ||
                    cmd.cmdType == FBM_GROUP_RESET;
                confirm.status = error          ? FB_STAT_ERROR
                                 : acceptedOnly ? FB_STAT_BUSY_ACTIVE
                                                : FB_STAT_DONE;
                confirm.error_code = static_cast<uint16_t>(error);
                confirm.tick = confirmTarget_->cnt;
                confirmTarget_->pushCmdConfirm(confirm);
            }
        }
    }
}

void AxisMotbufZmot::runCycleBottom() {
    for (size_t i = 0; i < groupCount_; ++i) {
        auto& group = groups_[i];
        // group.runCycle();
        group.runAdminFbs();
        runGroupFbList(group);
        group.AxisGrpZmot::runCycle();
    }
}

void FbMotionGroupZmot::reset_runtime() { reset(); }

void AxisMotbufZmot::reset_runtime() {
    // Reset all groups in place before returning their fixed-pool nodes to the free list.
    for (size_t i = 0; i < groups_.size(); ++i) {
        groups_[i].reset_runtime();
    }
    groupMovePool_.reset();
    groupCount_ = 1;
    groupSelect_ = 0;
}

void AxisMotbufZmot::setGroupSelect(int idx) {
    if (idx >= 0 && idx < kZmotMaxGroups) {
        groupSelect_ = idx;
        ensureGroupCount(static_cast<size_t>(groupSelect_ + 1));
    }
}

void AxisMotbufZmot::ensureGroupCount(size_t n) {
    if (n > static_cast<size_t>(kZmotMaxGroups)) {
        n = static_cast<size_t>(kZmotMaxGroups);
    }
    if (n > groupCount_) {
        groupCount_ = n;
    }
}

size_t AxisMotbufZmot::groupCount() const { return groupCount_; }

int AxisMotbufZmot::groupSelect() const { return groupSelect_; }

AxisGrpZmotWrap* AxisMotbufZmot::group(int idx) {
    if (idx < 0 || idx >= static_cast<int>(kZmotMaxGroups)) {
        return nullptr;
    }
    if (idx >= static_cast<int>(groupCount_)) {
        return nullptr;
    }
    return &groups_[static_cast<size_t>(idx)];
}

const AxisGrpZmotWrap* AxisMotbufZmot::group(int idx) const {
    if (idx < 0 || idx >= static_cast<int>(kZmotMaxGroups)) {
        return nullptr;
    }
    if (idx >= static_cast<int>(groupCount_)) {
        return nullptr;
    }
    return &groups_[static_cast<size_t>(idx)];
}

int AxisMotbufZmot::setGroupAxes(AxisGrpZmotWrap& group, const CommandStructDefault& cmd) {
    size_t count = 0;
    const int* axisIdx = cmd.getIntArray(count);
    const Ctx* context = ctx();
    if (!context || !axisIdx || count == 0 || count > static_cast<size_t>(kZmotMaxGroupAxes)) {
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }

    std::array<AXIS_REF, kZmotMaxGroupAxes> axes{};
    for (size_t i = 0; i < count; ++i) {
        if (axisIdx[i] < 0) {
            return ZMC_ERR_MOTCMD_INVALID_ARG;
        }
        axes[i] = context->getAxisByIndex(static_cast<uint32_t>(axisIdx[i]));
        if (!axes[i]) {
            return ZMC_AXIS_IS_NO_AXIS_REF;
        }
    }
    return group.setAxisRefArr(axes.data(), count);
}

int AxisMotbufZmot::processGroupCommand(AxisGrpZmotWrap& group, const CommandStructDefault& cmd) {
    switch (cmd.cmdType) {
        case FBM_AXIS_GROUP: {
            const int error = setGroupAxes(group, cmd);
            return error < 0 ? ZMC_ERR_MOTCMD_INVALID_ARG : error;
        }
        case FBM_SET_SPEED:
            group.setCmdVel(cmd.getDouble());
            return 0;
        case FBM_SET_ACCEL:
            group.setCmdAcc(cmd.getDouble());
            return 0;
        case FBM_SET_DECEL:
            group.setCmdDec(cmd.getDouble());
            return 0;
        case FBM_SET_JERK:
            group.setCmdJerk(cmd.getDouble());
            return 0;
        case FBM_GROUP_ENABLE:
            group.setAdminEnable(true);
            return 0;
        case FBM_GROUP_DISABLE:
            group.setAdminDisable(true);
            return 0;
        case FBM_GROUP_RESET:
            group.setAdminReset(true);
            return 0;
        case FBM_GROUP_SET_VEL:
            group.setCmdVel(cmd.getDouble());
            return 0;
        case FBM_GROUP_SET_ACC:
            group.setCmdAcc(cmd.getDouble());
            return 0;
        case FBM_GROUP_SET_DEC:
            group.setCmdDec(cmd.getDouble());
            return 0;
        case FBM_GROUP_SET_JERK:
            group.setCmdJerk(cmd.getDouble());
            return 0;
        case FBM_MULTI_MOVE:
        case FBM_MULTI_MOVE_REL:
        case FBM_MULTI_CIRCULAR:
        case FBM_MULTI_CIRCULAR_ABS:
        case FBM_MULTI_CIRCULAR2:
        case FBM_MULTI_CIRCULAR2_ABS: {
            FbMotionGroupZmot* fb = groupMovePool_.pop();
            if (!fb) {
                return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            }
            fb->reset();
            fb->setPool(&groupMovePool_);
            fb->bindGroup(&group);
            if (!fb->bindCmd(cmd)) {
                groupMovePool_.push(fb);
                return ZMC_ERR_MOTCMD_INVALID_ARG;
            }
            fb->setExecute(true);
            group.fbListPushBack(fb);
            return 0;
        }
        default:
            return ZMC_ERR_MOTCMD_UNKNOWN_COMMAND;
    }
}

void AxisMotbufZmot::runGroupFbList(AxisGrpZmotWrap& group) {
    FbMoveErasedBase* prev = static_cast<FbMoveErasedBase*>(group.fbListSentinel());
    FbMoveErasedBase* head = prev->getNext();
    while (head) {
        auto* fb = static_cast<FbMotionGroupZmot*>(head);
        fb->runCycle();
        const uint8_t stat = fb->getStat();
        if (stat != FB_STAT_DONE && stat != FB_STAT_ERROR) break;

        FbMoveErasedBase* removed = static_cast<FbMoveErasedBase*>(group.fbListRemoveNext(prev));
        auto* fbRemoved = static_cast<FbMotionGroupZmot*>(removed);
        fbRemoved->reset();
        groupMovePool_.push(fbRemoved);
        head = prev->getNext();
    }
}

}  // namespace zmc
