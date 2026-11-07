#include "AxisGrpZmot.h"

#include <new>

#include "AxisGrpZmotImpl.h"


namespace zmc {

AxisGrpZmot::AxisGrpZmot() {
    static_assert(sizeof(AxisGrpZmotInner) <= kAxisGrpZmotImplSize,
                  "AxisGrpZmotInner too large for storage");
    new (storage_) AxisGrpZmotInner();
}

AxisGrpZmot::~AxisGrpZmot() { get_grp_inner(*this).~AxisGrpZmotInner(); }

void AxisGrpZmot::runCycle() { get_grp_inner(*this).runCycle(); }

void AxisGrpZmot::reset_runtime() {
    AxisGrpZmotInner& inner = get_grp_inner(*this);
    inner.groupState = Disabled;
    inner.groupStateBak = Disabled;
    inner.ungroupAll();
    inner.tick = 0;
    inner.cmdVel = 100.0;
    inner.cmdAcc = 200.0;
    inner.cmdDec = 200.0;
    inner.cmdJerk = 0.0;
    inner.fastDecel = 200.0;
    inner.cycleTime = 0.001;
    inner.fSetPos.fill(0.0);
    inner.targetVel.fill(0.0);
    inner.targetAcc.fill(0.0);
    inner.fSavePos.fill(0.0);
}

}  // namespace zmc
