/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-22
@ArtifactStatusAuthority GPT-5 Codex
*/
#ifndef _ZMC_AXIS_FB_BINDING_HPP_
#define _ZMC_AXIS_FB_BINDING_HPP_

#include <array>
#include <cstdint>
#include <vector>

#include "AxisRef.h"
#include "common/zmc/zmc_predef.h"

namespace zmc {

// Bundle the bridge-owned axis references, slave mapping, and ENI slave count.
class AxisFbBinding {
   public:
    std::vector<AXIS_REF>* axis_ref_arr;
    const std::array<uint16_t, MC_MOTION_MAX_AXES>* axis_slave_ids;
    uint16_t eni_slave_num;
};

}  // namespace zmc

#endif  // _ZMC_AXIS_FB_BINDING_HPP_
