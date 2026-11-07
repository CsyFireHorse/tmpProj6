#ifndef ZMC_AXIS_GRP_ZMOT_H
#define ZMC_AXIS_GRP_ZMOT_H

#include <cstddef>
#include <cstdint>
#include "mc_robot_def.h"

namespace zmc {

static constexpr int kAxisGrpZmotImplSize = 8192;
struct AxisGrpZmotInner;

class AxisGrpZmot {
public:
    AxisGrpZmot();
    ~AxisGrpZmot();

    void runCycle();
    void reset_runtime();

private:
    alignas(std::max_align_t) char storage_[kAxisGrpZmotImplSize];

    friend struct AxisGrpZmotInner;
    friend AxisGrpZmotInner& get_grp_inner(AxisGrpZmot& g);
    friend const AxisGrpZmotInner& get_grp_inner(const AxisGrpZmot& g);
};


}  // namespace zmc

#endif
