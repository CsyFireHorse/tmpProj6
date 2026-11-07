/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-19
@ArtifactStatusAuthority GPT-5 Codex
*/
#include <gtest/gtest.h>

#include "EcatState.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/pci_errno.h"
#include "common/zmc/zmc_predef.h"

namespace {

struct FakeEcatContext {
    uint16_t slave_count{0u};
    uint8_t master_state{zmc::ECAT_STATE_OPERATIONAL};
    uint8_t slave_states[MAX_NUM_SLAVES]{};
    int32_t slave_count_result{0};
    int32_t master_state_result{0};
    int32_t failed_slave_result{0};
    uint16_t failed_slave_id{MAX_NUM_SLAVES};
    uint32_t slave_count_calls{0u};
    uint32_t master_state_calls{0u};
    uint32_t slave_state_calls{0u};

    FakeEcatContext() {
        for (uint16_t slave_id = 0u; slave_id < MAX_NUM_SLAVES; ++slave_id) {
            slave_states[slave_id] = zmc::ECAT_STATE_OPERATIONAL;
        }
    }
};

int32_t fake_get_slave_count(void* ctx, uint16_t* slave_count) {
    if (!ctx || !slave_count) {
        return -1;
    }
    FakeEcatContext* fake = static_cast<FakeEcatContext*>(ctx);
    ++fake->slave_count_calls;
    if (fake->slave_count_result != 0) {
        return fake->slave_count_result;
    }
    *slave_count = fake->slave_count;
    return 0;
}

int32_t fake_get_master_state(void* ctx, uint8_t* master_state) {
    if (!ctx || !master_state) {
        return -1;
    }
    FakeEcatContext* fake = static_cast<FakeEcatContext*>(ctx);
    ++fake->master_state_calls;
    if (fake->master_state_result != 0) {
        return fake->master_state_result;
    }
    *master_state = fake->master_state;
    return 0;
}

int32_t fake_get_slave_state(void* ctx, uint16_t slave_id, uint8_t* slave_state) {
    if (!ctx || !slave_state || slave_id >= MAX_NUM_SLAVES) {
        return -1;
    }
    FakeEcatContext* fake = static_cast<FakeEcatContext*>(ctx);
    ++fake->slave_state_calls;
    // Successful queries must populate the state, including after an injected failure.
    if (slave_id == fake->failed_slave_id && fake->failed_slave_result != 0) {
        return fake->failed_slave_result;
    }
    *slave_state = fake->slave_states[slave_id];
    return 0;
}

EcatStateAdapter make_adapter(FakeEcatContext* fake) {
    EcatStateAdapter adapter{};
    adapter.ctx = fake;
    adapter.get_slave_count = fake_get_slave_count;
    adapter.get_slave_state = fake_get_slave_state;
    adapter.get_master_state = fake_get_master_state;
    return adapter;
}

}  // namespace

TEST(EcatState, MasterQueryStoresActiveSlaveCountWithoutTouchingSlaveStates) {
    FakeEcatContext fake;
    fake.master_state =
        static_cast<uint8_t>(zmc::ECAT_STATE_OPERATIONAL | zmc::ECAT_STATE_ERROR_FLAG);
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatState state(&adapter);
    zmc::EcatStateResult result{};
    result.slave_states[0] = 4u;

    EXPECT_EQ(0, state.query_master(&result));

    EXPECT_EQ(fake.master_state, result.master_state);
    EXPECT_EQ(1u, result.master_state_valid);
    EXPECT_EQ(0, result.master_query_error);
    EXPECT_EQ(fake.slave_count, result.slave_count);
    EXPECT_EQ(1u, result.slave_count_valid);
    EXPECT_EQ(4u, result.slave_states[0]);
    EXPECT_EQ(1u, fake.master_state_calls);
    EXPECT_EQ(1u, fake.slave_count_calls);
    EXPECT_EQ(0u, fake.slave_state_calls);
}

TEST(EcatState, MasterQueryNormalizesCallbackFailure) {
    FakeEcatContext fake;
    fake.master_state_result = -44;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatState state(&adapter);
    zmc::EcatStateResult result{};

    EXPECT_EQ(ZMC_ERR_ECAT_GET_MASTER_STATE_FAILED, state.query_master(&result));
    EXPECT_EQ(0u, result.master_state_valid);
    EXPECT_EQ(0u, result.master_state);
    EXPECT_EQ(ZMC_ERR_ECAT_GET_MASTER_STATE_FAILED, result.master_query_error);
    EXPECT_EQ(1u, fake.slave_count_calls);
    EXPECT_EQ(0u, fake.slave_state_calls);
}

TEST(EcatState, SlaveQueryStoresEveryStateBeyondOldFaultListLimit) {
    FakeEcatContext fake;
    fake.slave_count = 18u;
    for (uint16_t slave_id = 0u; slave_id < fake.slave_count; ++slave_id) {
        fake.slave_states[slave_id] = static_cast<uint8_t>((slave_id % 7u) + 1u);
    }
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatState state(&adapter);
    state.set_eni_slave_num(fake.slave_count);
    zmc::EcatStateResult result{};
    result.master_state = 4u;
    result.master_state_valid = 1u;

    EXPECT_EQ(0, state.query_slaves(&result));

    EXPECT_EQ(1u, result.slave_scan_complete);
    EXPECT_EQ(18u, fake.slave_state_calls);
    EXPECT_EQ(0u, fake.slave_count_calls);
    EXPECT_EQ(0u, fake.master_state_calls);
    EXPECT_EQ(4u, result.master_state);
    EXPECT_EQ(1u, result.master_state_valid);
    for (uint16_t slave_id = 0u; slave_id < fake.slave_count; ++slave_id) {
        EXPECT_EQ(fake.slave_states[slave_id], result.slave_states[slave_id]);
        EXPECT_EQ(0, result.slave_query_errors[slave_id]);
    }
}

TEST(EcatState, StateAbnormalitiesAreNotQueryErrors) {
    FakeEcatContext fake;
    fake.slave_count = 3u;
    fake.slave_states[0] = 0u;
    fake.slave_states[1] = 4u;
    fake.slave_states[2] =
        static_cast<uint8_t>(zmc::ECAT_STATE_OPERATIONAL | zmc::ECAT_STATE_ERROR_FLAG);
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatState state(&adapter);
    state.set_eni_slave_num(fake.slave_count);
    zmc::EcatStateResult result{};

    EXPECT_EQ(0, state.query_slaves(&result));
    EXPECT_EQ(0u, result.slave_states[0]);
    EXPECT_EQ(4u, result.slave_states[1]);
    EXPECT_EQ(fake.slave_states[2], result.slave_states[2]);
    EXPECT_EQ(0, result.slave_query_errors[0]);
    EXPECT_EQ(0, result.slave_query_errors[1]);
    EXPECT_EQ(0, result.slave_query_errors[2]);
}

TEST(EcatState, SlaveQueryFailureIsScopedAndDoesNotStopScanning) {
    FakeEcatContext fake;
    fake.slave_count = 3u;
    fake.failed_slave_id = 1u;
    fake.failed_slave_result = -33;
    fake.slave_states[2] = 4u;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatState state(&adapter);
    state.set_eni_slave_num(fake.slave_count);
    zmc::EcatStateResult result{};

    EXPECT_EQ(ZMC_ERR_ECAT_GET_SLAVE_STATE_FAILED, state.query_slaves(&result));
    EXPECT_EQ(3u, fake.slave_state_calls);
    EXPECT_EQ(1u, result.slave_scan_complete);
    EXPECT_EQ(ZMC_ERR_ECAT_GET_SLAVE_STATE_FAILED, result.slave_query_errors[1]);
    EXPECT_EQ(4u, result.slave_states[2]);
}

TEST(EcatState, InvalidSlaveIndexKeepsItsSpecificQueryError) {
    FakeEcatContext fake;
    fake.slave_count = 2u;
    fake.failed_slave_id = 0u;
    fake.failed_slave_result = ECAT_E_INVALID_SLAVE_INDEX;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatState state(&adapter);
    state.set_eni_slave_num(fake.slave_count);
    zmc::EcatStateResult result{};

    EXPECT_EQ(ZMC_ERR_ECAT_INVALID_SLAVE_INDEX, state.query_slaves(&result));
    EXPECT_EQ(ZMC_ERR_ECAT_INVALID_SLAVE_INDEX, result.slave_query_errors[0]);
    EXPECT_EQ(2u, fake.slave_state_calls);
    EXPECT_EQ(1u, result.slave_scan_complete);
}

TEST(EcatState, ActiveSlaveCountFailureIsRecordedByMasterQuery) {
    FakeEcatContext fake;
    fake.slave_count_result = -12;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatState state(&adapter);
    zmc::EcatStateResult result{};

    EXPECT_EQ(ZMC_ERR_ECAT_GET_SLAVE_COUNT_FAILED, state.query_master(&result));
    EXPECT_EQ(0u, result.slave_count_valid);
    EXPECT_EQ(ZMC_ERR_ECAT_GET_SLAVE_COUNT_FAILED, result.slave_count_query_error);
    EXPECT_EQ(0u, fake.slave_state_calls);
    EXPECT_EQ(1u, fake.master_state_calls);
}

TEST(EcatState, ActiveSlaveCountOverflowIsRecordedByMasterQuery) {
    FakeEcatContext fake;
    fake.slave_count = static_cast<uint16_t>(MAX_NUM_SLAVES + 1u);
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatState state(&adapter);
    zmc::EcatStateResult result{};

    EXPECT_EQ(ZMC_ERR_ECAT_SLAVE_COUNT_OVERFLOW, state.query_master(&result));
    EXPECT_EQ(fake.slave_count, result.slave_count);
    EXPECT_EQ(1u, result.slave_count_valid);
    EXPECT_EQ(ZMC_ERR_ECAT_SLAVE_COUNT_OVERFLOW, result.slave_count_query_error);
    EXPECT_EQ(0u, fake.slave_state_calls);
}

TEST(EcatState, MissingCallbacksAreReportedByTheRelevantQuery) {
    zmc::EcatState state;
    zmc::EcatStateResult result{};

    EXPECT_EQ(ZMC_ERR_ECAT_STATE_ADAPTER_INVALID, state.query_master(&result));
    EXPECT_EQ(ZMC_ERR_ECAT_STATE_ADAPTER_INVALID, result.master_query_error);
    EXPECT_EQ(0u, result.master_state_valid);

    EXPECT_EQ(ZMC_ERR_ECAT_STATE_ADAPTER_INVALID, state.query_slaves(&result));
    EXPECT_EQ(ZMC_ERR_ECAT_STATE_ADAPTER_INVALID, result.slave_count_query_error);
    EXPECT_EQ(0u, result.slave_count_valid);
    EXPECT_EQ(0u, result.slave_scan_complete);
}
