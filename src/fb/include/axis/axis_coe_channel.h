/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#ifndef _ZMC_AXIS_COE_CHANNEL_H_
#define _ZMC_AXIS_COE_CHANNEL_H_

#include <stdint.h>

#include "AxisRef.h"

namespace zmc {

// ---- Axis-owned CoE one-shot protocol (plain data, bridge pumped) ----

// Fixed depths of the per-axis CoE pending FIFO and terminal result ring.
enum { AXIS_COE_QUEUE_DEPTH = 8, AXIS_COE_RING_DEPTH = 8 };
static const uint32_t AXIS_COE_TAG_MASK = 0x00FFFFFFu;

enum AxisCoeRequestDirection {
    AXIS_COE_REQUEST_UPLOAD = 0,
    AXIS_COE_REQUEST_DOWNLOAD = 1,
};

// One pending CoE request; the bridge submits FIFO entries in order.
struct AxisCoeRequest {
    uint32_t tag;      // Axis-local 24-bit id assigned at enqueue time
    uint16_t index;    // CoE object index
    uint8_t subindex;  // CoE object subindex
    uint8_t len;       // Payload length in bytes: 1, 2 or 4
    uint8_t direction; // AxisCoeRequestDirection
    int32_t value;     // Download value already narrowed to len bytes
};

// Terminal state of one CoE request, pushed by the bridge into the result ring.
struct AxisCoeResult {
    uint32_t tag;
    int32_t code;      // 0 on success, otherwise the raw transport error code
    uint16_t index;
    uint8_t subindex;
    uint8_t len;
    uint32_t data;     // Echoed payload kept for diagnostics
};

// Outcomes of axis_coe_result_query.
enum AxisCoeResultState {
    AXIS_COE_RESULT_FOUND = 0,      // Terminal state available through out_code
    AXIS_COE_RESULT_PENDING = 1,    // Request still queued or in flight
    AXIS_COE_RESULT_SUPERSEDED = 2, // Ring evicted the entry before it was read
};

// Enqueue one CoE download on the axis FIFO. len must be a length already
// validated by zmc_parameter_coe_decode (1, 2 or 4); this boundary does not
// re-validate the encoding rule. Returns 0 and fills out_tag on success,
// -1 when the FIFO is full and -2 on an invalid argument.
int32_t axis_coe_req_enqueue(AXIS_REF axis, uint16_t index, uint8_t subindex, uint8_t len,
                             int32_t value, uint32_t* out_tag);
// Enqueue one CoE upload on the axis FIFO with the same tag and capacity rules.
int32_t axis_coe_req_enqueue_read(AXIS_REF axis, uint16_t index, uint8_t subindex, uint8_t len,
                                  uint32_t* out_tag);
// Peek the FIFO head for the bridge submit step; 0 when an entry is present.
int32_t axis_coe_req_head(AXIS_REF axis, AxisCoeRequest* out_entry);
// Pop the FIFO head after ownership transfers to SdoRt or a local failure is published.
void axis_coe_req_pop(AXIS_REF axis);
// Drop all queued CoE requests; the tag generator stays monotonic.
void axis_coe_req_clear(AXIS_REF axis);
// Push one terminal result into the axis result ring (bridge side only).
void axis_coe_result_push(AXIS_REF axis, uint32_t tag, int32_t code, uint16_t index,
                          uint8_t subindex, uint8_t len, uint32_t data);
// Query the terminal state of one tag; fills out_code and, when requested,
// the result payload when FOUND.
int32_t axis_coe_result_query(AXIS_REF axis, uint32_t tag, int32_t* out_code,
                              uint32_t* out_data = nullptr);
// Clear the result ring; the tag generator stays monotonic.
void axis_coe_result_clear(AXIS_REF axis);

}  // namespace zmc

#endif  // _ZMC_AXIS_COE_CHANNEL_H_
