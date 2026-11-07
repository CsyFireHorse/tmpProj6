#include <gtest/gtest.h>

#include <cstring>

#include "axis/fbt_axis_opaque.h"
#include "bridge/AxisFbBridge.h"
#include "bridge/AxisFbBridgeWithThread.h"
#include "common/container/FbCmdQueue.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "motbuf/MotionCmdQueueBase.hpp"
#include "motbuf/fbmEnum.h"
#include "pou/AxPou.hpp"

namespace {

ZmcPioOffsetLayout make_bridge_test_layout() {
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

const char kBridgeOneAxisIni[] = R"(
[base]
axis_num = 1
[axis_0]
is_axis_virtual = true
)";

}  // namespace

TEST(AxisFbBridgePdoLayoutStatus, DefaultConstructIsNotReady) {
    zmc::AxisFbBridge bridge;

    EXPECT_FALSE(bridge.pdo_layout_ready());
}

TEST(AxisFbBridgePdoLayoutStatus, ApplyPioLayoutUpdatesReady) {
    zmc::AxisFbBridge bridge;
    ZmcPioOffsetLayout layout = make_bridge_test_layout();

    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));

    EXPECT_TRUE(bridge.pdo_layout_ready());
}

TEST(AxisFbBridgePdoLayoutStatus, ApplyPioLayoutRejectsInvalidArgs) {
    zmc::AxisFbBridge bridge;
    ZmcPioOffsetLayout empty_layout;
    std::memset(&empty_layout, 0, sizeof(empty_layout));

    EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_ARG, bridge.apply_pio_layout(nullptr));
    EXPECT_EQ(ZMC_ERR_BRIDGE_PDO_EMPTY_LAYOUT, bridge.apply_pio_layout(&empty_layout));
    EXPECT_FALSE(bridge.pdo_layout_ready());
}

TEST(AxisFbBridgeReset, RestoresUnusedStateAndKeepsRuntimePointers) {
    zmc::AxisFbBridge bridge;
    zmc::SdoRt* native_sdo = bridge.native_sdo();
    zmc::MotionCmdQueueBase* cmd_if = &bridge.command_interface();

    ASSERT_EQ(0, bridge.configure(nullptr, kBridgeOneAxisIni, 1u));
    ASSERT_EQ(0, bridge.auto_bind(0, 1));
    ZmcPioOffsetLayout layout = make_bridge_test_layout();
    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));

    char inp[16] = {0};
    char oup[16] = {0};
    ASSERT_EQ(0, bridge.run_cycle(inp, oup));
    AXIS_REF axis = zmc::AxPouTaskBinderAuto::getAxisByIndex(0);
    ASSERT_NE(nullptr, axis);
    EXPECT_GT(static_cast<zmc::AxisOpaque*>(axis)->get_count(), 0u);

    zmc::CmdPacket exit_packet;
    exit_packet.reset();
    ASSERT_TRUE(exit_packet.appendItem(zmc::FBM_EXIT, nullptr, 0));
    ASSERT_TRUE(cmd_if->push_cmd_packet(exit_packet));

    ASSERT_EQ(0, bridge.reset());

    EXPECT_EQ(native_sdo, bridge.native_sdo());
    EXPECT_EQ(cmd_if, &bridge.command_interface());
    EXPECT_EQ(0, bridge.axis_count());
    EXPECT_EQ(0, bridge.servo_count());
    EXPECT_FALSE(bridge.pdo_layout_ready());
    // EXPECT_EQ(0, zmc::AxPouTaskBinderAuto::getAxisCount());
    EXPECT_EQ(ZMC_ERR_BRIDGE_PDO_UNCONFIGURED, bridge.register_opmode_watches(0));
    EXPECT_EQ(0u, static_cast<zmc::AxisOpaque*>(axis)->get_count());

    ASSERT_EQ(0, bridge.configure(nullptr, kBridgeOneAxisIni, 1u));
    EXPECT_EQ(1, bridge.axis_count());
    EXPECT_FALSE(bridge.pdo_layout_ready());
    EXPECT_EQ(native_sdo, bridge.native_sdo());
    EXPECT_EQ(cmd_if, &bridge.command_interface());

    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));
    ASSERT_EQ(0, bridge.run_cycle(inp, oup));
    EXPECT_FALSE(cmd_if->exit_requested());
}

TEST(AxisFbBridgeReset, DirectBridgeResetPausesRunningSdoWorker) {
    zmc::AxisFbBridgeWithThread bridge_with_thread;

    ASSERT_EQ(0, bridge_with_thread.start_sdo());
    EXPECT_TRUE(bridge_with_thread.sdo_started());
    EXPECT_EQ(0, bridge_with_thread.bridge().reset());
    EXPECT_TRUE(bridge_with_thread.sdo_started());

    EXPECT_EQ(0, bridge_with_thread.stop_sdo());
    EXPECT_FALSE(bridge_with_thread.sdo_started());
}
