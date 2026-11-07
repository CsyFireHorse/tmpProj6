/* Unit tests for axfb command-packet and status apply C APIs. */
#include <gtest/gtest.h>

#include <cstring>

#include "axfb_command_packet_api.h"
#include "axfb_status_api.h"
#include "common/container/FbCmdQueue.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "motbuf/MotionCmdTypes.hpp"
#include "motbuf/fbmEnum.h"
#include "impl/fbt_impl_enum_def.h"

TEST(AxfbCommandPacketApi, LayoutMatchesRuntimePacket) {
    static_assert(sizeof(MCCmdPacket) == sizeof(zmc::CmdPacket),
                  "MCCmdPacket must match runtime CmdPacket");
    static_assert(sizeof(MCStatusPacket) == sizeof(zmc::StatusPacket),
                  "MCStatusPacket must match runtime StatusPacket");
}

TEST(AxfbCommandPacketApi, InitClearsAndResetAppends) {
    MCCmdPacket packet = {};
    packet.payload_len = 99;
    packet.payload[0] = 7;

    EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_ARG, axfb_cmd_packet_init(nullptr));
    EXPECT_EQ(ZMC_NO_ERROR, axfb_cmd_packet_init(&packet));
    EXPECT_EQ(0u, packet.payload_len);
    EXPECT_EQ(0u, packet.payload[0]);

    EXPECT_EQ(ZMC_NO_ERROR, axfb_cmd_packet_set_axis(&packet, 0));
    EXPECT_EQ(ZMC_NO_ERROR, axfb_cmd_packet_move_abs(&packet, -12.5));
    EXPECT_GT(packet.payload_len, 0u);

    const uint16_t before_reset = packet.payload_len;
    EXPECT_EQ(ZMC_NO_ERROR, axfb_cmd_packet_reset(&packet));
    EXPECT_GT(packet.payload_len, before_reset);

    EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_ARG, axfb_cmd_packet_set_axis(nullptr, 0));
    EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_ARG, axfb_cmd_packet_move_abs(nullptr, 1.0));
}

TEST(AxfbCommandPacketApi, MultiAndGroupHelpers) {
    MCCmdPacket packet = {};
    ASSERT_EQ(ZMC_NO_ERROR, axfb_cmd_packet_init(&packet));

    EXPECT_EQ(ZMC_NO_ERROR, axfb_cmd_packet_set_units(&packet, 1.0));
    const double positions[] = {1.0, 2.0};
    EXPECT_EQ(ZMC_NO_ERROR, axfb_cmd_packet_multi_move_abs(&packet, 2, positions));
    const int axes[] = {0, 1};
    EXPECT_EQ(ZMC_NO_ERROR, axfb_cmd_packet_group_set_axes(&packet, 2, axes));
    EXPECT_GT(packet.payload_len, 0u);

    EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_ARG, axfb_cmd_packet_multi_move_abs(&packet, 0, positions));
    EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_ARG, axfb_cmd_packet_group_set_axes(&packet, 2, nullptr));
}

TEST(AxfbStatusApi, ApplyPacketRejectsNullArgs) {
    axfb_status_handle_t handle = nullptr;
    ASSERT_EQ(ZMC_NO_ERROR, axfb_status_create(&handle));
    ASSERT_NE(nullptr, handle);

    MCStatusPacket packet = {};
    EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_ARG, axfb_status_apply_packet(nullptr, &packet));
    EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_ARG, axfb_status_apply_packet(handle, nullptr));
    EXPECT_EQ(ZMC_NO_ERROR, axfb_status_apply_packet(handle, &packet));

    axfb_status_destroy(handle);
}

TEST(AxfbStatusApi, EmptyResultsAndAck) {
    axfb_status_handle_t handle = nullptr;
    ASSERT_EQ(ZMC_NO_ERROR, axfb_status_create(&handle));
    ASSERT_NE(nullptr, handle);

    CmdResult cmd_buf[4];
    uint32_t cmd_len = 4;
    EXPECT_EQ(ZMC_ERR_MOTCMD_CMD_RESULTS_EMPTY,
              axfb_status_get_cmd_results(handle, cmd_buf, &cmd_len));
    EXPECT_EQ(0u, cmd_len);

    CmdResult io_buf[4];
    uint32_t io_len = 4;
    EXPECT_EQ(ZMC_ERR_MOTCMD_IO_RESULTS_EMPTY,
              axfb_status_get_io_results(handle, io_buf, &io_len));
    EXPECT_EQ(0u, io_len);

    uint32_t seq = 99;
    uint32_t tick = 99;
    EXPECT_EQ(ZMC_NO_ERROR, axfb_status_ack_seq(handle, &seq));
    EXPECT_EQ(0u, seq);
    EXPECT_EQ(ZMC_NO_ERROR, axfb_status_ack_tick(handle, &tick));
    EXPECT_EQ(0u, tick);

    FlowCtrlStatus flow = {};
    EXPECT_EQ(ZMC_NO_ERROR, axfb_status_get_flow_status(handle, &flow));

    uint32_t zero_len = 0;
    EXPECT_EQ(ZMC_ERR_MOTCMD_CMD_RESULTS_EMPTY,
              axfb_status_get_cmd_results(handle, nullptr, &zero_len));

    axfb_status_destroy(handle);
}

// The public C API reads the same records from axis history and persistent IO snapshots.
TEST(AxfbStatusApi, ReadsSharedAxisAndIoResultsWithoutConsumingSnapshots) {
    axfb_status_handle_t handle = nullptr;
    ASSERT_EQ(0, axfb_status_create(&handle));
    CmdResult axis = {};
    axis.id = 11;
    axis.command_type = zmc::FBM_MOVE_ABSOLUTE;
    axis.target_kind = CMD_TARGET_AXIS;
    axis.status = zmc::FB_STAT_DONE;
    axis.tick = 20;
    const uint32_t start = 0x80000000u;
    std::memcpy(&axis.data, &start, sizeof(start));
    CmdResult io = {};
    io.id = 12;
    io.command_type = zmc::FBM_IO_READ_AI;
    io.target_kind = CMD_TARGET_AI + 3;
    io.target_index = 255;
    io.io_count = 1;
    io.status = zmc::FB_STAT_DONE;
    const uint32_t value = 0xffffffffu;
    std::memcpy(&io.data, &value, sizeof(value));
    zmc::StatusPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::STAT_CMDRESULT, &axis, sizeof(axis)));
    ASSERT_TRUE(packet.appendItem(zmc::STAT_CMD_CONFIRM, &io, sizeof(io)));
    MCStatusPacket wire;
    std::memcpy(&wire, &packet, sizeof(wire));
    ASSERT_EQ(0, axfb_status_apply_packet(handle, &wire));
    for (int query = 0; query < 2; ++query) {
        CmdResult result;
        uint32_t count = 1;
        ASSERT_EQ(0, axfb_status_get_cmd_results(handle, &result, &count));
        EXPECT_EQ(axis.id, result.id);
        EXPECT_EQ(start, cmd_result_data_u32(&result));
        count = 1;
        ASSERT_EQ(0, axfb_status_get_io_results(handle, &result, &count));
        EXPECT_EQ(io.id, result.id);
        EXPECT_EQ(1023u, cmd_result_io_start(&result));
        EXPECT_EQ(value, cmd_result_data_u32(&result));
    }
    axfb_status_destroy(handle);
}
