/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#include "bridge/AxisCoeRouter.hpp"

#include "common/zmc/fbt_errorcode_def.h"
#include "sdo/sdo_rt.hpp"

namespace zmc {

void AxisCoeRouter::bind(const AxisFbBinding& binding, SdoRt* sdo) {
    binding_ = binding;
    sdo_ = sdo;
}

void AxisCoeRouter::reset() {
    // Drop the per-axis request FIFOs and result rings with the routing.
    if (binding_.axis_ref_arr) {
        const uint32_t axis_count = static_cast<uint32_t>(binding_.axis_ref_arr->size());
        for (uint32_t j = 0; j < axis_count; ++j) {
            AXIS_REF axis = (*binding_.axis_ref_arr)[j];
            if (!axis) continue;
            axis_coe_req_clear(axis);
            axis_coe_result_clear(axis);
        }
    }
}

void AxisCoeRouter::fail_head(AXIS_REF axis, const AxisCoeRequest& head, int32_t code) {
    // Local failure: synthesize the terminal state and release the FIFO head.
    axis_coe_result_push(axis, head.tag, code, head.index, head.subindex, head.len,
                         static_cast<uint32_t>(head.value));
    axis_coe_req_pop(axis);
}

void AxisCoeRouter::dispatch(const SdoResult& batch) {
    if (!binding_.axis_ref_arr || !binding_.axis_slave_ids) return;
    const uint32_t axis_count = static_cast<uint32_t>(binding_.axis_ref_arr->size());
    for (uint8_t i = 0; i < batch.item_count; ++i) {
        const SdoResultItem& item = batch.items[i];
        const uint32_t axis_idx = sdo_request_id_axis_index(item.request_id);
        const uint32_t tag = sdo_request_id_axis_tag(item.request_id);
        if (axis_idx >= axis_count || axis_idx >= binding_.axis_slave_ids->size() ||
            (*binding_.axis_slave_ids)[axis_idx] != item.key.slave_id + 1u) {
            continue;
        }
        AXIS_REF axis = (*binding_.axis_ref_arr)[axis_idx];
        axis_coe_result_push(axis, tag, item.result_code, item.key.index, item.key.subindex,
                             item.data_len, item.data);
    }
}

void AxisCoeRouter::pump_cycle() {
    if (!binding_.axis_ref_arr || !sdo_ || !binding_.axis_slave_ids) return;
    const uint32_t axis_count = static_cast<uint32_t>(binding_.axis_ref_arr->size());

    // Submit one pending FIFO head per axis; request_id carries the result identity.
    for (uint32_t j = 0; j < axis_count; ++j) {
        AXIS_REF axis = (*binding_.axis_ref_arr)[j];
        if (!axis) continue;
        AxisCoeRequest head{};
        if (axis_coe_req_head(axis, &head) != 0) continue;
        // Resolve the bound slave; unbound axes fail fast with UNBOUND.
        const int slave_idx =
            (*binding_.axis_slave_ids)[j] != 0u ? (*binding_.axis_slave_ids)[j] - 1u : -1;
        if (slave_idx < 0) {
            fail_head(axis, head, ZMC_ERR_WP_SDO_UNBOUND);
            continue;
        }
        SdoKey key{};
        key.slave_id = static_cast<uint16_t>(slave_idx);
        key.index = head.index;
        key.subindex = head.subindex;
        // Keys owned by periodic download watches are rejected up front.
        // if (sdo_->is_download_watch_key(key)) {
        //     fail_head(axis, head, ZMC_ERR_WP_SDO_WATCH_KEY);
        //     continue;
        // }
        const bool is_upload = head.direction == AXIS_COE_REQUEST_UPLOAD;
        const uint32_t request_id = is_upload ? sdo_axis_coe_read_request_id(j, head.tag)
                                              : sdo_axis_coe_write_request_id(j, head.tag);
        const int submit_result =
            is_upload ? sdo_->sdo_read_once(key, head.len, request_id)
                      : sdo_->sdo_write(key, head.len, &head.value, request_id);
        if (submit_result != 0) {
            fail_head(axis, head, submit_result);
            continue;
        }
        // SdoRt now owns the request copy; Axis FIFO keeps pending requests only.
        axis_coe_req_pop(axis);
    }
}

}  // namespace zmc
