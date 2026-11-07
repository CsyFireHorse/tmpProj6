#ifndef _ZMC_AXIS_GRP_ZMOT_IMPL_H_
#define _ZMC_AXIS_GRP_ZMOT_IMPL_H_

// Trusted implementation header.
// Only .cpp "impl" files should include this to access AxisGrpZmotInner directly.

#include "AxisGrpZmot.h"
#include "zmc_fb_predef.h"
#include "fb_cpp_def.h"
#include "mc_robot_def.h"
#include "fbt_impl_enum_def.h"
#include <array>

namespace zmc {

struct AxisEnhanced {
    AxisEnhanced() = default;
    AxisEnhanced(const AxisEnhanced&) = delete;
    AxisEnhanced& operator=(const AxisEnhanced&) = delete;
    AxisEnhanced(AxisEnhanced&&) = delete;
    AxisEnhanced& operator=(AxisEnhanced&&) = delete;

    AXIS_REF axisRef = nullptr;
    // FbtFollowPosition fbFollowPosVel;
    FbtFollowSetValues fbFollowPosVel;
    FbtHalt fbHalt;
    FbtReadStatus fbRdStatus;
    FbtReadValues fbRdActValues;
    FbtReadValues fbRdSetValues;

    void onGroupError();
    void bindAxis(AXIS_REF ax);
    void unbindAxis();
    // Slot j takes the same axis binding as slot j+1 (RemoveAxisFromGroup compact left).
    void takeBindingFromNextSlot(const AxisEnhanced& nextSlot);
    void writePosVel(double pos, double vel, double acc);
    void followRunCycle();
    void triggerHalt(double dec);
    void haltRunCycle();

    void readCycle() {
        if (!axisRef) return;
        fbRdStatus.runCycle();
        fbRdSetValues.runCycle();
        fbRdActValues.runCycle();
    }

    double readSetPosition() const {
        return fbRdSetValues.Position;
    }

    bool isAxisErrorStop() const {
        return axisRef && fbRdStatus.ErrorStop;
    }
};

struct AxisGrpZmotInner {
    uint16_t wMagicNumber = AXISGRP_MAGIC_NUMBER; 

    ZMC_AXIS_GROUP_STATE groupState = Disabled;
    ZMC_AXIS_GROUP_STATE groupStateBak = Disabled;
    uint16_t errorId = 0;
    // uint8_t decCntAfterMoving = 0;
    int axNum = 0;
    uint32_t tick = 0;

    double cmdVel = 100.0;
    double cmdAcc = 200.0;
    double cmdDec = 200.0;
    double cmdJerk = 0.0;
    double fastDecel = 200;
    double cycleTime = 0.001;

    std::array<AxisEnhanced, kZmotMaxGroupAxes> axes{};
    std::array<double, kZmotMaxGroupAxes> fSetPos{};
    std::array<double, kZmotMaxGroupAxes> targetVel{};
    std::array<double, kZmotMaxGroupAxes> targetAcc{};
    std::array<double, kZmotMaxGroupAxes> fSavePos{};

    inline bool is_valid() const {
        return wMagicNumber == AXISGRP_MAGIC_NUMBER;
    }
    void runCycle();

    int ungroupAll();
    int addAxis(AXIS_REF ax);
    int removeAxis(AXIS_REF ax);
};

inline AxisGrpZmotInner& get_grp_inner(AxisGrpZmot& g) {
    return *reinterpret_cast<AxisGrpZmotInner*>(g.storage_);
}

inline const AxisGrpZmotInner& get_grp_inner(const AxisGrpZmot& g) {
    return *reinterpret_cast<const AxisGrpZmotInner*>(g.storage_);
}

}  // namespace zmc


#endif