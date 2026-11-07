/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-19
@ArtifactStatusAuthority GPT-5 Codex
*/
#include "ecat/EcatErrorManager.hpp"

#include "AxisCiaConfig.hpp"
#include "axis/fbt_axis_opaque.h"
#include "common/zmc/fbt_errorcode_def.h"

namespace zmc {

namespace {

bool state_is_faulted(uint8_t state) {
    return state == 0u || (state & ECAT_STATE_MASK) != ECAT_STATE_OPERATIONAL ||
           (state & ECAT_STATE_ERROR_FLAG) != 0u;
}

}  // namespace

void EcatErrorManager::bind(const AxisFbBinding& binding) {
    binding_ = binding;
    ecat_state_.set_eni_slave_num(binding.eni_slave_num);
}

int EcatErrorManager::bind_adapter(const EcatStateAdapter* adapter) {
    if (!adapter || !adapter->get_slave_count || !adapter->get_master_state ||
        !adapter->get_slave_state) {
        return ZMC_ERR_BRIDGE_ECAT_STATE_ADAPTER_INVALID;
    }

    ecat_state_.bind_adapter(adapter);
    adapter_bound_ = true;
    master_check_pending_ = true;
    clear_state();
    return 0;
}

void EcatErrorManager::reset() {
    ecat_state_.clear_adapter();
    adapter_bound_ = false;
    master_check_pending_ = false;
    ecat_state_.set_eni_slave_num(0u);
    clear_state();
}

void EcatErrorManager::update() {
    check_master_state();
    check_slave_states();
}

void EcatErrorManager::apply() const { apply_slave_errors(); }

void EcatErrorManager::check_master_state() {
    if (!adapter_bound_ || !master_check_pending_) {
        return;
    }

    (void)ecat_state_.query_master(&state_result_);
    master_check_pending_ = false;
}

void EcatErrorManager::check_slave_states() {
    if (!adapter_bound_) {
        return;
    }
    (void)ecat_state_.query_slaves(&state_result_);
}

void EcatErrorManager::apply_slave_errors() const {
    if (!binding_.axis_ref_arr) {
        return;
    }
    const bool snapshot_complete = state_result_.slave_scan_complete != 0u;
    const uint32_t axis_count = static_cast<uint32_t>(binding_.axis_ref_arr->size());
    for (uint32_t axis_idx = 0u; axis_idx < axis_count; ++axis_idx) {
        AXIS_REF axis = (*binding_.axis_ref_arr)[axis_idx];
        const AxisCiaConfig* config = axis ? axis->get_config_addr() : nullptr;
        if (!axis || !config || config->is_axis_virtual || AxisCiaConfig_is_axis_dummy(config) ||
            !adapter_bound_ || !binding_.axis_slave_ids ||
            axis_idx >= binding_.axis_slave_ids->size()) {
            continue;
        }

        const uint16_t encoded_slave_id = (*binding_.axis_slave_ids)[axis_idx];
        if (encoded_slave_id == 0u) {
            continue;
        }

        const uint16_t slave_id = static_cast<uint16_t>(encoded_slave_id - 1u);
        // Treat state zero as communication loss even when the query succeeds.
        const bool communication_error = !snapshot_complete || slave_id >= binding_.eni_slave_num ||
                                         state_result_.slave_query_errors[slave_id] != 0 ||
                                         state_result_.slave_states[slave_id] == 0u;
        axisSetCommunError(axis, communication_error);
        axisSetNotOperationalError(
            axis, !communication_error && state_is_faulted(state_result_.slave_states[slave_id]));
    }
}

void EcatErrorManager::clear_state() {
    state_result_ = EcatStateResult{};
}

}  // namespace zmc
