/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-19
@ArtifactStatusAuthority GPT-5 Codex
*/
#include "ecat/EcatState.hpp"

#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/pci_errno.h"
#include "common/zmc/zmc_predef.h"

namespace zmc {

namespace {

void clear_master_result(EcatStateResult& result) {
    result.master_query_error = 0;
    result.master_state = 0u;
    result.master_state_valid = 0u;
    result.slave_count_query_error = 0;
    result.slave_count = 0u;
    result.slave_count_valid = 0u;
}

void clear_slave_result(EcatStateResult& result) {
    result.slave_scan_complete = 0u;
    for (uint16_t slave_id = 0u; slave_id < MAX_NUM_SLAVES; ++slave_id) {
        result.slave_states[slave_id] = 0u;
        result.slave_query_errors[slave_id] = 0;
    }
}

}  // namespace

EcatState::EcatState(const EcatStateAdapter* adapter) { bind_adapter(adapter); }

void EcatState::bind_adapter(const EcatStateAdapter* adapter) {
    adapter_ = adapter ? *adapter : EcatStateAdapter{};
}

void EcatState::clear_adapter() { adapter_ = EcatStateAdapter{}; }

void EcatState::set_eni_slave_num(uint16_t eni_slave_num) { eni_slave_num_ = eni_slave_num; }

int32_t EcatState::query_master(EcatStateResult* out) {
    if (!out) {
        return ZMC_ERR_ECAT_STATE_ADAPTER_INVALID;
    }

    clear_master_result(*out);
    int32_t first_error = 0;
    if (!adapter_.get_master_state) {
        out->master_query_error = ZMC_ERR_ECAT_STATE_ADAPTER_INVALID;
        first_error = out->master_query_error;
    } else {
        const int32_t query_result = adapter_.get_master_state(adapter_.ctx, &out->master_state);
        if (query_result != 0) {
            out->master_state = 0u;
            out->master_query_error = ZMC_ERR_ECAT_GET_MASTER_STATE_FAILED;
            first_error = out->master_query_error;
        } else {
            out->master_state_valid = 1u;
        }
    }

    if (!adapter_.get_slave_count) {
        out->slave_count_query_error = ZMC_ERR_ECAT_STATE_ADAPTER_INVALID;
        return first_error != 0 ? first_error : out->slave_count_query_error;
    }

    const int32_t count_result = adapter_.get_slave_count(adapter_.ctx, &out->slave_count);
    if (count_result != 0) {
        out->slave_count = 0u;
        out->slave_count_query_error = ZMC_ERR_ECAT_GET_SLAVE_COUNT_FAILED;
        return first_error != 0 ? first_error : out->slave_count_query_error;
    }

    out->slave_count_valid = 1u;
    if (out->slave_count > MAX_NUM_SLAVES) {
        out->slave_count_query_error = ZMC_ERR_ECAT_SLAVE_COUNT_OVERFLOW;
        return first_error != 0 ? first_error : out->slave_count_query_error;
    }
    return first_error;
}

int32_t EcatState::query_slaves(EcatStateResult* out) {
    if (!out) {
        return ZMC_ERR_ECAT_STATE_ADAPTER_INVALID;
    }

    clear_slave_result(*out);
    if (!adapter_.get_slave_state) {
        return ZMC_ERR_ECAT_STATE_ADAPTER_INVALID;
    }
    if (eni_slave_num_ > MAX_NUM_SLAVES) {
        return ZMC_ERR_ECAT_SLAVE_COUNT_OVERFLOW;
    }

    int32_t first_error = 0;
    for (uint16_t slave_id = 0u; slave_id < eni_slave_num_; ++slave_id) {
        const int32_t query_result =
            adapter_.get_slave_state(adapter_.ctx, slave_id, &out->slave_states[slave_id]);
        if (query_result == ECAT_E_INVALID_SLAVE_INDEX) {
            out->slave_query_errors[slave_id] = ZMC_ERR_ECAT_INVALID_SLAVE_INDEX;
        } else if (query_result != 0) {
            out->slave_query_errors[slave_id] = ZMC_ERR_ECAT_GET_SLAVE_STATE_FAILED;
        }
        if (first_error == 0 && out->slave_query_errors[slave_id] != 0) {
            first_error = out->slave_query_errors[slave_id];
        }
    }
    out->slave_scan_complete = 1u;
    return first_error;
}

}  // namespace zmc
