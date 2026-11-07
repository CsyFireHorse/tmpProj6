/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#include "axis/axis_coe_channel.h"

#include "fbt_axis_impl.h"
#include "fbt_axis_impl_cfg.hpp"

namespace zmc {

// ---- CoE one-shot protocol on the axis-owned channel ----

// Reach the per-axis CoE channel storage; null for an invalid axis ref.
// Same storage idiom as fbt_axis_opaque.cpp: the AxisImpl lives inside the
// AxisOpaque storage block and owns the AxisInnerContent through native.
static AxisCoeChannel* coe_channel(AXIS_REF axis) {
    if (!axis) return nullptr;
    return &static_cast<AxisInnerContent*>(((AxisImpl*)axis)->native)->coe;
}

static int32_t axis_coe_req_enqueue_impl(AXIS_REF axis, uint16_t index, uint8_t subindex,
                                         uint8_t len, uint8_t direction, int32_t value,
                                         uint32_t* out_tag) {
    AxisCoeChannel* ch = coe_channel(axis);
    if (!ch || !out_tag) return -2;  // Invalid axis ref or output pointer
    if (ch->count >= AXIS_COE_QUEUE_DEPTH) return -1;  // FIFO full
    AxisCoeRequest& slot = ch->queue[ch->tail];
    slot.tag = ch->next_tag;
    if (++ch->next_tag > AXIS_COE_TAG_MASK) ch->next_tag = 1u;
    slot.index = index;
    slot.subindex = subindex;
    slot.len = len;
    slot.direction = direction;
    slot.value = value;
    ch->tail = (ch->tail + 1u) % AXIS_COE_QUEUE_DEPTH;
    ++ch->count;
    *out_tag = slot.tag;
    return 0;
}

int32_t axis_coe_req_enqueue(AXIS_REF axis, uint16_t index, uint8_t subindex, uint8_t len,
                             int32_t value, uint32_t* out_tag) {
    return axis_coe_req_enqueue_impl(axis, index, subindex, len, AXIS_COE_REQUEST_DOWNLOAD,
                                     value, out_tag);
}

int32_t axis_coe_req_enqueue_read(AXIS_REF axis, uint16_t index, uint8_t subindex, uint8_t len,
                                  uint32_t* out_tag) {
    return axis_coe_req_enqueue_impl(axis, index, subindex, len, AXIS_COE_REQUEST_UPLOAD, 0,
                                     out_tag);
}

int32_t axis_coe_req_head(AXIS_REF axis, AxisCoeRequest* out_entry) {
    AxisCoeChannel* ch = coe_channel(axis);
    if (!ch || !out_entry) return 1;  // Treated as an empty FIFO by the caller
    if (ch->count == 0) return 1;     // Empty FIFO (head==tail is ambiguous)
    *out_entry = ch->queue[ch->head];
    return 0;
}

void axis_coe_req_pop(AXIS_REF axis) {
    AxisCoeChannel* ch = coe_channel(axis);
    if (!ch || ch->count == 0) return;
    ch->queue[ch->head] = AxisCoeRequest();  // Reset the released slot
    ch->head = (ch->head + 1u) % AXIS_COE_QUEUE_DEPTH;
    --ch->count;
}

void axis_coe_req_clear(AXIS_REF axis) {
    AxisCoeChannel* ch = coe_channel(axis);
    if (!ch) return;
    for (uint32_t i = 0; i < AXIS_COE_QUEUE_DEPTH; ++i) ch->queue[i] = AxisCoeRequest();
    ch->head = 0;
    ch->tail = 0;
    ch->count = 0;
    // next_tag keeps growing so tags issued before the clear never match again.
}

void axis_coe_result_push(AXIS_REF axis, uint32_t tag, int32_t code, uint16_t index,
                          uint8_t subindex, uint8_t len, uint32_t data) {
    AxisCoeChannel* ch = coe_channel(axis);
    if (!ch || tag == 0) return;
    // Overwrite the oldest ring slot; the depth covers all in-flight requests.
    AxisCoeResult& slot = ch->ring[ch->ring_head];
    slot.tag = tag;
    slot.code = code;
    slot.index = index;
    slot.subindex = subindex;
    slot.len = len;
    slot.data = data;
    ch->ring_head = (ch->ring_head + 1u) % AXIS_COE_RING_DEPTH;
}

int32_t axis_coe_result_query(AXIS_REF axis, uint32_t tag, int32_t* out_code,
                              uint32_t* out_data) {
    AxisCoeChannel* ch = coe_channel(axis);
    if (!ch || tag == 0 || !out_code) return AXIS_COE_RESULT_PENDING;
    bool newer = false;
    for (uint32_t i = 0; i < AXIS_COE_RING_DEPTH; ++i) {
        const AxisCoeResult& slot = ch->ring[i];
        if (slot.tag == 0) continue;  // Cleared slot
        if (slot.tag == tag) {
            *out_code = slot.code;
            if (out_data) *out_data = slot.data;
            return AXIS_COE_RESULT_FOUND;
        }
        const uint32_t distance = (slot.tag - tag) & AXIS_COE_TAG_MASK;
        newer = newer || (distance != 0u && distance <= (AXIS_COE_TAG_MASK >> 1));
    }
    return newer ? AXIS_COE_RESULT_SUPERSEDED : AXIS_COE_RESULT_PENDING;
}

void axis_coe_result_clear(AXIS_REF axis) {
    AxisCoeChannel* ch = coe_channel(axis);
    if (!ch) return;
    for (uint32_t i = 0; i < AXIS_COE_RING_DEPTH; ++i) ch->ring[i] = AxisCoeResult();
    ch->ring_head = 0;
}

}  // namespace zmc
