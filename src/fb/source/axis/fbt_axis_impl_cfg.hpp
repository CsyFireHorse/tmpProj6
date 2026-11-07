/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#ifndef _ZMC_FBT_AXIS_IMPL_CFG_H_
#define _ZMC_FBT_AXIS_IMPL_CFG_H_

#include "ServoFakerToy.h"
#include "PdoItemFlag.hpp"
#include "AxisCiaConfig.hpp"
// #include "fbt_servo_ada.h"
// #include "ServoCmdDef.hpp"
// #include "Cia402Easy.hpp"

// #include "fbt_axis_impl.h"
#include "axLogBase.hpp"
#include "axis/axis_coe_channel.h"
#include <memory>

namespace zmc{

class ServoAdapterBase;

// Container state for the axis CoE one-shot protocol: pending request FIFO,
// result ring and the monotonic tag generator. Plain data pumped by the bridge;
// no SDO runtime types leak into the axis layer.
struct AxisCoeChannel {
    AxisCoeRequest queue[AXIS_COE_QUEUE_DEPTH]{};  // Pending requests, tag order
    uint32_t head = 0;                             // FIFO head: next bridge submit
    uint32_t tail = 0;                             // FIFO tail: next free enqueue slot
    uint32_t count = 0;                            // Pending requests; head==tail is ambiguous
    uint32_t next_tag = 1;                         // 24-bit generator; 0 marks empty
    AxisCoeResult ring[AXIS_COE_RING_DEPTH]{};     // Terminal states, oldest first
    uint32_t ring_head = 0;                        // Ring slot holding the oldest result
};

struct AxisInnerContent{
    // PdoEnableFlag pdoEnFlag;
    ServoAdapterBase* pServoAdapter = nullptr;
    std::unique_ptr<AxLogger> logger;
    AxisCiaConfig axisCfg;
    // Axis-owned CoE one-shot channel (pending FIFO + result ring).
    AxisCoeChannel coe;
    // AXIS_REF pAxis;
};

}

#endif

