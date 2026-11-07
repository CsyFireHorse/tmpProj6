#ifndef ZMC_AXIS_MOTBUF_ZMOT_H
#define ZMC_AXIS_MOTBUF_ZMOT_H

#include <array>
#include <cstdint>

#include "AxisMotbufBase.h"
#include "AxisGrpZmotWrap.h"
#include "FbMoveList.hpp"
#include "axgrp/FbtMoveCircular.h"
#include "MotCmd.hpp"
#include "common/misc/ObjectPoolAllocator.hpp"
#include "fbmEnum.h"
#include "common/zmc/zmc_predef.h"

namespace zmc {

static constexpr int kZmotMaxGroups = 4;
static constexpr int kZmotGroupMovePoolSize = ZMC_FB_MOVE_POOL_SIZE;

class FbMotionGroupZmot : public FbMoveErasedBase {
   public:
    FbMotionGroupZmot();

    // clear axis error
    void reset();
    // reinit
    void reset_runtime() override;
    void bindGroup(AxisGrpZmotWrap* group);
    bool bindCmd(const CommandStructDefault& cmd);
    void setExecute(bool on);
    uint8_t getStat() const;
    uint16_t getErrorId() const;
    MC_BUFFER_MODE getBufferMode() const;
    void runCycle() override;

    void setNext(FbMotionGroupZmot* next);
    FbMotionGroupZmot* getNext() const;

   private:
    AxisGrpZmotWrap* group_ = nullptr;
    FbtMoveMix fb_;
};

class FbMoveWrapWithAxis;

class AxisMotbufZmot : public AxisMotbufBase {
   public:
    // Accepts group-range commands and group-targeted kinematic set commands.
    int pushCommand(const CommandStructDefault& cmd);
    void setConfirmTarget(FbMoveWrapWithAxis* parent) { confirmTarget_ = parent; }
    void runCycleTop();
    void runCycleBottom();
    void reset_runtime();

    void setGroupSelect(int idx);
    void ensureGroupCount(size_t n);
    size_t groupCount() const;
    int groupSelect() const;
    AxisGrpZmotWrap* group(int idx);
    const AxisGrpZmotWrap* group(int idx) const;

   private:
    // Resolve group-command indices before crossing into the group layer.
    int setGroupAxes(AxisGrpZmotWrap& group, const CommandStructDefault& cmd);
    int processGroupCommand(AxisGrpZmotWrap& group, const CommandStructDefault& cmd);
    void runGroupFbList(AxisGrpZmotWrap& group);

   private:
    std::array<AxisGrpZmotWrap, kZmotMaxGroups> groups_{};
    size_t groupCount_ = 1;
    int groupSelect_ = 0;  // axis group selection
    ObjectPoolAllocator<FbMotionGroupZmot, kZmotGroupMovePoolSize> groupMovePool_;
    FbMoveWrapWithAxis* confirmTarget_ = nullptr;
};

}  // namespace zmc

#endif
