/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#ifndef _ZMC_AXIS_COE_ROUTER_HPP_
#define _ZMC_AXIS_COE_ROUTER_HPP_

#include "axis/axis_coe_channel.h"
#include "bridge/AxisFbBinding.hpp"

namespace zmc {

class SdoRt;
struct SdoResult;

// Bridge-side pump for the axis CoE one-shot channel. The request id carries
// the destination axis and tag, so no reverse slave route is needed.
// The router owns no queues of its own; it only moves plain data between the
// axis channels and the SdoRt request queue.
class AxisCoeRouter {
   public:
    AxisCoeRouter() = default;

    // Bind the bridge-owned axis references, SDO runtime, and axis-to-slave mapping.
    void bind(const AxisFbBinding& binding, SdoRt* sdo);

    // Drop per-axis channel state.
    void reset();
    // Route completed one-shot results directly by request id.
    void dispatch(const SdoResult& batch);
    // Submit queued CoE requests in bridge order.
    void pump_cycle();

   private:
    // Fail the FIFO head locally: tombstone into the ring, then pop.
    void fail_head(AXIS_REF axis, const AxisCoeRequest& head, int32_t code);

    SdoRt* sdo_ = nullptr;
    AxisFbBinding binding_{};
};

}  // namespace zmc

#endif  // _ZMC_AXIS_COE_ROUTER_HPP_
