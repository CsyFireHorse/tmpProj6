/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-19
@ArtifactStatusAuthority GPT-5 Codex
*/
#include <array>

#include <gtest/gtest.h>

#include "AxisCiaConfig.hpp"
#include "axis/fbt_axis_opaque.h"
#include "common/ecat/Cia402Easy.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "ecat/EcatErrorManager.hpp"
#include "fb/fb_cpp_def.h"
#include "fb/mc_parameter_enum_def.h"
#include "impl/fbt_impl_enum_def.h"
#include "pou/AxisFbGraph.hpp"
#include "source/axgrp/AxisGrpZmotImpl.h"

namespace {

const char kTwoPhysicalAxesIni[] = R"(
[base]
axis_num = 2
[axis_0]
is_axis_virtual = false
[axis_1]
is_axis_virtual = false
)";

const char kOnePhysicalAxisIni[] = R"(
[base]
axis_num = 1
[axis_0]
is_axis_virtual = false
)";

struct FakeEcatContext {
    uint16_t slave_count{2u};
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

int32_t get_slave_count(void* ctx, uint16_t* slave_count) {
    FakeEcatContext* fake = static_cast<FakeEcatContext*>(ctx);
    ++fake->slave_count_calls;
    if (fake->slave_count_result != 0) {
        return fake->slave_count_result;
    }
    *slave_count = fake->slave_count;
    return 0;
}

int32_t get_master_state(void* ctx, uint8_t* master_state) {
    FakeEcatContext* fake = static_cast<FakeEcatContext*>(ctx);
    ++fake->master_state_calls;
    if (fake->master_state_result != 0) {
        return fake->master_state_result;
    }
    *master_state = fake->master_state;
    return 0;
}

int32_t get_slave_state(void* ctx, uint16_t slave_id, uint8_t* slave_state) {
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
    adapter.get_slave_count = get_slave_count;
    adapter.get_slave_state = get_slave_state;
    adapter.get_master_state = get_master_state;
    return adapter;
}

void configure_graph(zmc::AxisFbGraph& graph) {
    ASSERT_EQ(0, graph.configure("", kTwoPhysicalAxesIni, 2u));
    ASSERT_EQ(0, graph.bindAddress(0, 1u));
    ASSERT_EQ(0, graph.bindAddress(1, 2u));
}

void bind_manager(zmc::EcatErrorManager& manager, zmc::AxisFbGraph& graph,
                  std::array<uint16_t, MC_MOTION_MAX_AXES>& axis_slave_ids, uint16_t first_slave,
                  uint16_t second_slave) {
    axis_slave_ids.fill(0u);
    axis_slave_ids[0] = static_cast<uint16_t>(first_slave + 1u);
    axis_slave_ids[1] = static_cast<uint16_t>(second_slave + 1u);
    manager.bind(zmc::AxisFbBinding{&graph.get_axis_ref_arr(), &axis_slave_ids, 2u});
}

bool communication_error(zmc::AxisFbGraph& graph, int axis_idx) {
    return zmc::axisGetCommunError(graph.axisRef(axis_idx));
}

bool axis_error_stop(AXIS_REF axis) {
    zmc::FbtReadStatus status;
    status.Axis = axis;
    status.Enable = true;
    status.runCycle();
    return status.ErrorStop;
}

uint32_t axis_error_id(AXIS_REF axis) {
    zmc::FbtReadAxisError error;
    error.Axis = axis;
    error.Enable = true;
    error.runCycle();
    return static_cast<uint32_t>(error.AxisErrorID);
}

uint32_t power_error_id(AXIS_REF axis) {
    zmc::FbtPower power;
    power.Axis = axis;
    power.Enable = true;
    power.bRegulatorOn = true;
    power.bDriveStart = true;
    power.runCycle();
    EXPECT_TRUE(power.Error);
    return power.ErrorID;
}

struct AxisSetValues {
    double position;
    double velocity;
    double acceleration;
};

AxisSetValues axis_set_values(AXIS_REF axis) {
    zmc::FbtReadValues values;
    values.Axis = axis;
    values.Enable = true;
    values.Source = zmc::ZMC_VALUESELECT_SET;
    values.runCycle();
    return {values.Position, values.Velocity, values.Acceleration};
}

void write_axis_parameter(AXIS_REF axis, int parameter, double value) {
    zmc::FbtWriteParameter writer;
    writer.Axis = axis;
    writer.Execute = true;
    writer.ParameterNumber = parameter;
    writer.Value = value;
    writer.runCycle();
    ASSERT_TRUE(writer.Done);
    ASSERT_FALSE(writer.Error);
}

void run_manager_cycle(zmc::EcatErrorManager& manager, zmc::AxisFbGraph&) {
    manager.update();
    manager.apply();
}

}  // namespace

TEST(EcatErrorManager, BindingDoesNotQueryOnTheBindingThread) {
    FakeEcatContext fake;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;

    ASSERT_EQ(0, manager.bind_adapter(&adapter));

    EXPECT_EQ(0u, fake.master_state_calls);
    EXPECT_EQ(0u, fake.slave_count_calls);
    EXPECT_EQ(0u, fake.slave_state_calls);
}

TEST(EcatErrorManager, RejectsIncompleteAdapterWithoutEnablingQueries) {
    zmc::EcatErrorManager manager;
    EcatStateAdapter adapter{};

    EXPECT_EQ(ZMC_ERR_BRIDGE_ECAT_STATE_ADAPTER_INVALID, manager.bind_adapter(&adapter));
}

TEST(EcatErrorManager, FirstCycleChecksMasterOnceAndSlavesEveryCycle) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);
    run_manager_cycle(manager, graph);

    EXPECT_EQ(1u, fake.master_state_calls);
    EXPECT_EQ(1u, fake.slave_count_calls);
    EXPECT_EQ(4u, fake.slave_state_calls);
    EXPECT_FALSE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
}

TEST(EcatErrorManager, MasterStateAbnormalityDoesNotCreateAxisErrors) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.master_state = 4u;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);

    EXPECT_EQ(1u, fake.master_state_calls);
    EXPECT_FALSE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
}

TEST(EcatErrorManager, MasterQueryFailureDoesNotCreateAxisErrors) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.master_state_result = -44;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);

    EXPECT_FALSE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
}

TEST(EcatErrorManager, QueriedFaultStatesAffectOnlyTheirBoundAxis) {
    const uint8_t states[] = {
        0u, 4u, static_cast<uint8_t>(zmc::ECAT_STATE_OPERATIONAL | zmc::ECAT_STATE_ERROR_FLAG)};
    for (uint8_t state : states) {
        SCOPED_TRACE(static_cast<int>(state));
        // Isolate each sample so a prior latched error cannot mask a classification failure.
        zmc::AxisFbGraph graph;
        configure_graph(graph);
        std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
        FakeEcatContext fake;
        fake.slave_states[1] = state;
        const EcatStateAdapter adapter = make_adapter(&fake);
        zmc::EcatErrorManager manager;
        ASSERT_EQ(0, manager.bind_adapter(&adapter));
        bind_manager(manager, graph, axis_slave_ids, 0u, 1u);
        run_manager_cycle(manager, graph);
        graph.run_once();
        EXPECT_FALSE(communication_error(graph, 0));
        EXPECT_EQ(state == 0u, communication_error(graph, 1));
        EXPECT_FALSE(axis_error_stop(graph.axisRef(0)));
        EXPECT_EQ(ZMC_NO_ERROR, axis_error_id(graph.axisRef(0)));
        EXPECT_TRUE(axis_error_stop(graph.axisRef(1)));
        EXPECT_EQ(state == 0u ? ZMC_DI_GENERAL_COMMUNICATION_ERROR : ZMC_AXIS_IS_NOT_OPERATIONAL,
                  axis_error_id(graph.axisRef(1)));
    }
}

TEST(EcatErrorManager, SlaveQueryFailureIsScopedAndDoesNotStopTheScan) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.failed_slave_id = 0u;
    fake.failed_slave_result = -33;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);

    EXPECT_EQ(2u, fake.slave_state_calls);
    EXPECT_TRUE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
    graph.run_once();
    EXPECT_EQ(ZMC_DI_GENERAL_COMMUNICATION_ERROR, axis_error_id(graph.axisRef(0)));
    EXPECT_FALSE(axis_error_stop(graph.axisRef(1)));
    EXPECT_EQ(ZMC_NO_ERROR, axis_error_id(graph.axisRef(1)));
}

TEST(EcatErrorManager, MultipleAxesOnOneSlaveReceiveTheSameError) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.slave_states[1] =
        static_cast<uint8_t>(zmc::ECAT_STATE_OPERATIONAL | zmc::ECAT_STATE_ERROR_FLAG);
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 1u, 1u);

    run_manager_cycle(manager, graph);

    EXPECT_FALSE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
    graph.run_once();
    EXPECT_EQ(ZMC_AXIS_IS_NOT_OPERATIONAL, axis_error_id(graph.axisRef(0)));
    EXPECT_EQ(ZMC_AXIS_IS_NOT_OPERATIONAL, axis_error_id(graph.axisRef(1)));
}

TEST(EcatErrorManager, ActiveSlaveCountFailureDoesNotCreateAxisErrors) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.slave_count_result = -1;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);
    EXPECT_FALSE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
}

TEST(EcatErrorManager, ActiveSlaveCountDoesNotLimitConfiguredSlaveScan) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.slave_count = 1u;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);

    EXPECT_FALSE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
    EXPECT_EQ(2u, fake.slave_state_calls);
}

TEST(EcatErrorManager, HealthySlaveStateClearsPreviousCommunicationError) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.failed_slave_id = 0u;
    fake.failed_slave_result = -33;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);
    ASSERT_TRUE(communication_error(graph, 0));

    fake.failed_slave_result = 0;
    run_manager_cycle(manager, graph);
    EXPECT_FALSE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
}

TEST(EcatErrorManager, VirtualAndDummyAxesDoNotReceiveSlaveErrors) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    graph.axisRef(0)->get_config_addr()->is_axis_virtual = true;
    graph.axisRef(1)->get_config_addr()->debug.axis_dtype |= 2u;
    zmc::axisSetCommunError(graph.axisRef(0), true);
    zmc::axisSetCommunError(graph.axisRef(1), true);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.failed_slave_id = 0u;
    fake.failed_slave_result = -1;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);

    EXPECT_TRUE(communication_error(graph, 0));
    EXPECT_TRUE(communication_error(graph, 1));
}

TEST(EcatErrorManager, UnmappedAxisKeepsPreviousCommunicationError) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);
    axis_slave_ids[0] = 0u;
    zmc::axisSetCommunError(graph.axisRef(0), true);

    run_manager_cycle(manager, graph);

    EXPECT_TRUE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
}

TEST(EcatErrorManager, UnboundAdapterKeepsPreviousCommunicationError) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    zmc::EcatErrorManager manager;
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);
    zmc::axisSetCommunError(graph.axisRef(0), true);

    run_manager_cycle(manager, graph);

    EXPECT_TRUE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
}

TEST(EcatErrorManager, ReconfigureToFewerAxesDropsOldBinding) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    ASSERT_EQ(0, graph.configure("", kOnePhysicalAxisIni, 1u));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);
    run_manager_cycle(manager, graph);

    ASSERT_EQ(1u, graph.axis_num());
    EXPECT_FALSE(communication_error(graph, 0));
}

TEST(EcatErrorManager, ResetStopsProjectionAndRequiresRebind) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.failed_slave_id = 0u;
    fake.failed_slave_result = -33;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);
    run_manager_cycle(manager, graph);
    ASSERT_TRUE(communication_error(graph, 0));
    const uint32_t count_calls = fake.slave_count_calls;

    manager.reset();
    run_manager_cycle(manager, graph);

    EXPECT_EQ(count_calls, fake.slave_count_calls);
    EXPECT_TRUE(communication_error(graph, 0));
    EXPECT_FALSE(communication_error(graph, 1));
}

TEST(EcatErrorManager, FieldbusFaultsHoldSetPositionAndPowerOffWithoutConsumingActualPva) {
    const bool query_failures[] = {false, true};
    for (bool query_failed : query_failures) {
        SCOPED_TRACE(query_failed);
        zmc::AxisFbGraph graph;
        configure_graph(graph);
        std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
        FakeEcatContext fake;
        fake.slave_states[0] = 4u;
        fake.failed_slave_id = 0u;
        fake.failed_slave_result = query_failed ? -33 : 0;
        const EcatStateAdapter adapter = make_adapter(&fake);
        zmc::EcatErrorManager manager;
        ASSERT_EQ(0, manager.bind_adapter(&adapter));
        bind_manager(manager, graph, axis_slave_ids, 0u, 1u);
        AXIS_REF axis = graph.axisRef(0);
        write_axis_parameter(axis, PARA_fSavePosition, 12.5);
        write_axis_parameter(axis, PARA_fSaveVelocity, 7.0);
        write_axis_parameter(axis, PARA_fSaveAcceleration, 3.0);
        write_axis_parameter(axis, PARA_fActPosition, 900.0);
        graph.oupArr[0].control_word = zmc::Cia402Easy::sm_cmd_enable_operation;

        run_manager_cycle(manager, graph);
        graph.run_once();

        const AxisSetValues values = axis_set_values(axis);
        EXPECT_EQ(query_failed, communication_error(graph, 0));
        EXPECT_TRUE(axis_error_stop(axis));
        EXPECT_EQ(query_failed ? ZMC_DI_GENERAL_COMMUNICATION_ERROR : ZMC_AXIS_IS_NOT_OPERATIONAL,
                  axis_error_id(axis));
        EXPECT_DOUBLE_EQ(12.5, values.position);
        EXPECT_DOUBLE_EQ(0.0, values.velocity);
        EXPECT_DOUBLE_EQ(0.0, values.acceleration);
        EXPECT_EQ(zmc::Cia402Easy::sm_cmd_disable_voltage, graph.oupArr[0].control_word);
    }
}

TEST(EcatErrorManager, HealthySnapshotRequiresExplicitResetBeforePowerCanSucceed) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);
    graph.run_once();
    AXIS_REF axis = graph.axisRef(0);
    ASSERT_FALSE(axis_error_stop(axis));
    ASSERT_EQ(ZMC_NO_ERROR, axis_error_id(axis));

    fake.slave_states[0] = 4u;
    run_manager_cycle(manager, graph);
    graph.run_once();
    ASSERT_TRUE(axis_error_stop(graph.axisRef(0)));
    ASSERT_EQ(ZMC_AXIS_IS_NOT_OPERATIONAL, axis_error_id(axis));

    fake.slave_states[0] = zmc::ECAT_STATE_OPERATIONAL;
    run_manager_cycle(manager, graph);
    graph.run_once();

    EXPECT_FALSE(communication_error(graph, 0));
    EXPECT_TRUE(axis_error_stop(graph.axisRef(0)));
    EXPECT_EQ(ZMC_AXIS_IS_NOT_OPERATIONAL, axis_error_id(axis));
    EXPECT_EQ(ZMC_AXIS_IS_NOT_OPERATIONAL, power_error_id(axis));

    zmc::FbtReset reset;
    reset.Axis = axis;
    reset.Execute = true;
    // Reset must let the axis clear its latched error before reporting completion.
    for (int cycle = 0; cycle < 4 && !reset.Done && !reset.Error; ++cycle) {
        reset.runCycle();
        run_manager_cycle(manager, graph);
        graph.run_once();
    }
    ASSERT_TRUE(reset.Done);
    ASSERT_FALSE(reset.Error);
    EXPECT_FALSE(axis_error_stop(axis));
    EXPECT_EQ(ZMC_NO_ERROR, axis_error_id(axis));

    zmc::FbtPower power;
    power.Axis = axis;
    power.Enable = true;
    power.bRegulatorOn = true;
    power.bDriveStart = true;
    graph.inpArr[0].status_word = zmc::Cia402Easy::STATWORD_OPERATION_ENABLED;
    for (int cycle = 0; cycle < 2; ++cycle) {
        run_manager_cycle(manager, graph);
        graph.run_once();
        power.runCycle();
    }
    EXPECT_TRUE(power.Status);
    EXPECT_FALSE(power.Error);
    EXPECT_EQ(ZMC_NO_ERROR, power.ErrorID);
    EXPECT_FALSE(axis_error_stop(axis));
}

TEST(EcatErrorManager, FaultCategoryChangesKeepFirstAxisErrorAndRejectResetUntilHealthy) {
    const bool first_query_failures[] = {false, true};
    for (bool first_query_failed : first_query_failures) {
        SCOPED_TRACE(first_query_failed);
        zmc::AxisFbGraph graph;
        configure_graph(graph);
        std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
        FakeEcatContext fake;
        fake.slave_states[0] = 4u;
        fake.failed_slave_id = 0u;
        const EcatStateAdapter adapter = make_adapter(&fake);
        zmc::EcatErrorManager manager;
        ASSERT_EQ(0, manager.bind_adapter(&adapter));
        bind_manager(manager, graph, axis_slave_ids, 0u, 1u);
        AXIS_REF axis = graph.axisRef(0);
        const uint32_t first_error = first_query_failed ? ZMC_DI_GENERAL_COMMUNICATION_ERROR
                                                      : ZMC_AXIS_IS_NOT_OPERATIONAL;

        for (int phase = 0; phase < 2; ++phase) {
            const bool query_failed = phase == 0 ? first_query_failed : !first_query_failed;
            fake.failed_slave_result = query_failed ? -33 : 0;
            run_manager_cycle(manager, graph);
            graph.run_once();
            EXPECT_EQ(query_failed, communication_error(graph, 0));
            EXPECT_EQ(first_error, axis_error_id(axis));
            EXPECT_EQ(query_failed ? ZMC_DI_GENERAL_COMMUNICATION_ERROR
                                   : ZMC_AXIS_IS_NOT_OPERATIONAL,
                      power_error_id(axis));

            zmc::FbtReset reset;
            reset.Axis = axis;
            reset.Execute = true;
            reset.runCycle();
            reset.runCycle();
            EXPECT_TRUE(reset.Error);
            EXPECT_EQ(SMC_R_CANNOT_RESET_COMMUNICATION_ERROR, reset.ErrorID);
            EXPECT_TRUE(axis_error_stop(axis));
            EXPECT_EQ(first_error, axis_error_id(axis));
        }
    }
}

TEST(EcatErrorManager, GroupObservesAxisErrorStopOnTheFollowingGraphCycle) {
    zmc::AxisFbGraph graph;
    configure_graph(graph);
    AXIS_REF grouped_axis = graph.axisRef(0);
    zmc::AxisGrpZmotWrap* group = graph.fbGrp.zmotMotbuf.group(0);
    ASSERT_NE(nullptr, group);
    ASSERT_EQ(0, group->setAxisRefArr(&grouped_axis, 1u));
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids{};
    FakeEcatContext fake;
    fake.slave_states[0] = 4u;
    const EcatStateAdapter adapter = make_adapter(&fake);
    zmc::EcatErrorManager manager;
    ASSERT_EQ(0, manager.bind_adapter(&adapter));
    bind_manager(manager, graph, axis_slave_ids, 0u, 1u);

    run_manager_cycle(manager, graph);
    graph.run_once();
    EXPECT_TRUE(axis_error_stop(graph.axisRef(0)));
    EXPECT_NE(zmc::ErrorStop, zmc::get_grp_inner(*group).groupState);

    graph.run_once();
    EXPECT_EQ(zmc::ErrorStop, zmc::get_grp_inner(*group).groupState);
}
