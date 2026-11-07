/*
 * Bridge reset regression tests for power and single-axis motion commands.
 * AI modification signature: GPT-5 Codex
 */
#include <algorithm>
#include <array>
#include <cstring>

#include <gtest/gtest.h>

#include "bridge/AxisFbBridge.h"
#include "common/container/FbCmdQueue.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "impl/fbt_axis_base.h"
#include "motbuf/MotionCmdTypes.hpp"
#include "motbuf/MotionCmdQueueBase.hpp"
#include "motbuf/fbmEnum.h"
#include "pou/AxPou.hpp"
#include "qcmd/MotionCmdPacket.hpp"

namespace {

const char kBridgeOneAxisIni[] = R"(
[base]
axis_num = 1
[axis_0]
is_axis_virtual = true
)";

struct BridgeCycleBuffers {
    std::array<uint8_t, 16> input{};
    std::array<uint8_t, 16> output{};
};

ZmcPioOffsetLayout make_one_axis_layout() {
    ZmcPioOffsetLayout layout;
    std::memset(&layout, 0, sizeof(layout));
    layout.slot_count = 1;
    layout.descriptor_pod.tx_cap = 16;
    layout.descriptor_pod.rx_cap = 16;
    layout.descriptor_pod.slave_count = 1;
    layout.slots[0].slave_idx = 1;
    layout.slots[0].slave_channel_idx = 0;
    layout.slots[0].po_ctrl_word_offset = 0;
    layout.slots[0].po_p_offset = 2;
    layout.slots[0].po_mode_offset = 6;
    layout.slots[0].po_v_offset = -1;
    layout.slots[0].po_t_offset = -1;
    layout.slots[0].pi_stat_offset = 0;
    layout.slots[0].pi_p_offset = 2;
    layout.slots[0].pi_mode_offset = -1;
    layout.slots[0].pi_error_offset = -1;
    layout.slots[0].pi_v_offset = -1;
    layout.slots[0].pi_t_offset = -1;
    return layout;
}

int configure_one_axis_bridge(zmc::AxisFbBridge& bridge) {
    const int configure_ret = bridge.configure(nullptr, kBridgeOneAxisIni, 1u);
    if (configure_ret != 0) {
        return configure_ret;
    }
    const int bind_ret = bridge.auto_bind(0, 1);
    if (bind_ret != 0) {
        return bind_ret;
    }
    const ZmcPioOffsetLayout layout = make_one_axis_layout();
    return bridge.apply_pio_layout(&layout);
}

int run_cycles(zmc::AxisFbBridge& bridge, BridgeCycleBuffers& buffers, int count) {
    for (int i = 0; i < count; ++i) {
        std::fill(buffers.input.begin(), buffers.input.end(), 0);
        std::fill(buffers.output.begin(), buffers.output.end(), 0);
        const int ret = bridge.run_cycle(buffers.input.data(), buffers.output.data());
        if (ret != 0) {
            return ret;
        }
    }
    return 0;
}

bool pop_latest_axis_state(zmc::MotionCmdQueueBase& command_interface, AxisState& state) {
    bool found = false;
    zmc::StatusPacket packet;
    while (command_interface.pop_status_packet(packet)) {
        uint16_t item_type = 0;
        uint16_t item_length = 0;
        const uint8_t* item_data = nullptr;
        zmc::CmdItemIterator iterator(packet.payload, packet.payload_len);
        while (iterator.next(item_type, item_length, item_data)) {
            if (item_type != zmc::STAT_STATE || item_length < sizeof(AxisState)) {
                continue;
            }
            AxisState candidate = {};
            std::memcpy(&candidate, item_data, sizeof(candidate));
            if (candidate.axis_id == 0) {
                state = candidate;
                found = true;
            }
        }
    }
    return found;
}

bool push_power_and_move(zmc::MotionCmdQueueBase& command_interface, double target) {
    zmc::MotionCmdPacket packet;
    static_cast<zmc::MotionPacket&>(packet).reset();
    if (packet.powerOn() != static_cast<int>(ZMC_NO_ERROR)) {
        return false;
    }
    if (packet.setSpeed(10.0) != static_cast<int>(ZMC_NO_ERROR)) {
        return false;
    }
    if (packet.setAccel(20.0) != static_cast<int>(ZMC_NO_ERROR)) {
        return false;
    }
    if (packet.setDecel(20.0) != static_cast<int>(ZMC_NO_ERROR)) {
        return false;
    }
    if (packet.moveAbs(1, &target) != static_cast<int>(ZMC_NO_ERROR)) {
        return false;
    }
    return command_interface.push_cmd_packet(packet);
}

bool push_flow_config_and_power(zmc::MotionCmdQueueBase& command_interface) {
    zmc::MotionCmdPacket packet;
    static_cast<zmc::MotionPacket&>(packet).reset();
    FlowCtrlConfig config = {};
    config.period = 1;
    config.batchSize = 20;
    if (packet.setFlowConfig(config) != static_cast<int>(ZMC_NO_ERROR)) {
        return false;
    }
    if (packet.powerOn() != static_cast<int>(ZMC_NO_ERROR)) {
        return false;
    }
    return command_interface.push_cmd_packet(packet);
}

}  // namespace

TEST(AxisFbBridgeResetPowerMove, ResetDiscardsQueuedPowerAndMove) {
    zmc::AxisFbBridge bridge;
    ASSERT_EQ(0, configure_one_axis_bridge(bridge));

    ASSERT_TRUE(push_power_and_move(bridge.command_interface(), 10.0));
    ASSERT_EQ(0, bridge.reset());
    ASSERT_EQ(0, configure_one_axis_bridge(bridge));

    BridgeCycleBuffers buffers;
    ASSERT_EQ(0, run_cycles(bridge, buffers, 4));

    AXIS_REF axis = zmc::AxPouTaskBinderAuto::getAxisByIndex(0);
    ASSERT_NE(nullptr, axis);
    zmc::AxisImplBase& axis_impl = zmc::get_axis_impl_ref(axis);
    EXPECT_EQ(zmc::mcDisabled, axis_impl.axisStat);
    EXPECT_FALSE(axis_impl.isPowerOn());
    EXPECT_DOUBLE_EQ(0.0, axis_impl.fSetPosition);
}

TEST(AxisFbBridgeResetPowerMove, ResetStopsActivePowerAndMove) {
    zmc::AxisFbBridge bridge;
    ASSERT_EQ(0, configure_one_axis_bridge(bridge));

    zmc::MotionCmdPacket power_packet;
    static_cast<zmc::MotionPacket&>(power_packet).reset();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), power_packet.powerOn());
    ASSERT_TRUE(bridge.command_interface().push_cmd_packet(power_packet));

    BridgeCycleBuffers buffers;
    ASSERT_EQ(0, run_cycles(bridge, buffers, 8));

    AXIS_REF axis = zmc::AxPouTaskBinderAuto::getAxisByIndex(0);
    ASSERT_NE(nullptr, axis);
    zmc::AxisImplBase& axis_impl = zmc::get_axis_impl_ref(axis);
    ASSERT_TRUE(axis_impl.isPowerOn());

    double target = 10.0;
    zmc::MotionCmdPacket move_packet;
    static_cast<zmc::MotionPacket&>(move_packet).reset();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), move_packet.setSpeed(10.0));
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), move_packet.moveAbs(1, &target));
    ASSERT_TRUE(bridge.command_interface().push_cmd_packet(move_packet));
    ASSERT_EQ(0, run_cycles(bridge, buffers, 2));
    EXPECT_EQ(zmc::mcDiscreteMotion, axis_impl.axisStat);
    EXPECT_GT(axis_impl.fSetPosition, 0.0);

    ASSERT_EQ(0, bridge.reset());

    EXPECT_EQ(zmc::mcDisabled, axis_impl.axisStat);
    EXPECT_FALSE(axis_impl.bRegulatorOn);
    EXPECT_FALSE(axis_impl.bDriveStart);
    EXPECT_DOUBLE_EQ(0.0, axis_impl.fSetPosition);
    EXPECT_EQ(0u, axis_impl.m_cycleCounter);

    ASSERT_EQ(0, configure_one_axis_bridge(bridge));
    EXPECT_EQ(axis, zmc::AxPouTaskBinderAuto::getAxisByIndex(0));
}

TEST(AxisFbBridgeResetPowerMove, ResetAllowsFreshPowerAndMove) {
    zmc::AxisFbBridge bridge;
    ASSERT_EQ(0, configure_one_axis_bridge(bridge));

    BridgeCycleBuffers buffers;
    ASSERT_TRUE(push_flow_config_and_power(bridge.command_interface()));
    ASSERT_EQ(0, run_cycles(bridge, buffers, 8));

    AXIS_REF axis = zmc::AxPouTaskBinderAuto::getAxisByIndex(0);
    ASSERT_NE(nullptr, axis);
    zmc::AxisImplBase& axis_impl = zmc::get_axis_impl_ref(axis);
    ASSERT_TRUE(axis_impl.isPowerOn());

    double first_target = 10.0;
    zmc::MotionCmdPacket first_move_packet;
    static_cast<zmc::MotionPacket&>(first_move_packet).reset();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), first_move_packet.setSpeed(10.0));
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), first_move_packet.moveAbs(1, &first_target));
    ASSERT_TRUE(bridge.command_interface().push_cmd_packet(first_move_packet));
    ASSERT_EQ(0, run_cycles(bridge, buffers, 2));
    ASSERT_EQ(zmc::mcDiscreteMotion, axis_impl.axisStat);
    ASSERT_GT(axis_impl.fSetPosition, 0.0);

    ASSERT_EQ(0, bridge.reset());
    ASSERT_EQ(0, configure_one_axis_bridge(bridge));
    EXPECT_EQ(axis, zmc::AxPouTaskBinderAuto::getAxisByIndex(0));

    AxisState state = {};
    zmc::AxisImplBase& reset_axis_impl = zmc::get_axis_impl_ref(axis);
    EXPECT_EQ(zmc::mcDisabled, reset_axis_impl.axisStat);
    EXPECT_FALSE(reset_axis_impl.isPowerOn());

    ASSERT_TRUE(push_flow_config_and_power(bridge.command_interface()));
    ASSERT_EQ(0, run_cycles(bridge, buffers, 8));
    ASSERT_TRUE(reset_axis_impl.isPowerOn());

    ASSERT_TRUE(pop_latest_axis_state(bridge.command_interface(), state));
    EXPECT_EQ(static_cast<uint8_t>(zmc::mcStandstill), state.axis_state);

    double target = 10.0;
    zmc::MotionCmdPacket move_packet;
    static_cast<zmc::MotionPacket&>(move_packet).reset();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), move_packet.setSpeed(10.0));
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), move_packet.moveAbs(1, &target));
    ASSERT_TRUE(bridge.command_interface().push_cmd_packet(move_packet));
    ASSERT_EQ(0, run_cycles(bridge, buffers, 2));

    ASSERT_TRUE(pop_latest_axis_state(bridge.command_interface(), state));
    EXPECT_EQ(static_cast<uint8_t>(zmc::mcDiscreteMotion), state.axis_state);
    EXPECT_EQ(zmc::mcDiscreteMotion, axis_impl.axisStat);
    EXPECT_GT(axis_impl.fSetPosition, 0.0);
    ASSERT_EQ(0, run_cycles(bridge, buffers, 1000));
    EXPECT_EQ(zmc::mcDiscreteMotion, axis_impl.axisStat);
    ASSERT_EQ(0, run_cycles(bridge, buffers, 1000));
    EXPECT_EQ(zmc::mcStandstill, axis_impl.axisStat);
    EXPECT_NEAR(target, axis_impl.fSetPosition, 1e-6);
    EXPECT_NEAR(target, axis_impl.fActPosition, 1e-6);
}
