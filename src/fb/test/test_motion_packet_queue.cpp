// GPT-6 (Codex), 2026-11-04. Nonzero packet sequences and runtime progress.
#include <gtest/gtest.h>

#include <vector>
#include <cstring>

#include "common/container/FbCmdQueue.hpp"
#include "motbuf/FbMoveWrapV3.hpp"
#include "motbuf/fbmEnum.h"
#include "qcmd/MotionCmdMirror.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "impl/fbt_impl_enum_def.h"
#include "pou/AxPou.hpp"
#include "pou/Ctx.h"

namespace {

// Read progress through the real status queue and mirror, including command confirmations.
void refreshStatus(zmc::FbMoveWrap2Queue& wrap, zmc::MotionStatusMirror& mirror) {
    zmc::StatusPacket packet;
    while (wrap.getStatusQueue().pop(packet)) mirror.applyPacket(packet);
}

TEST(MotionPacketQueue, StartsAtOneAndRestartsAfterClear) {
    zmc::CmdQueue<4> queue;
    zmc::CmdPacket packet;
    packet.reset();
    EXPECT_EQ(3u, queue.available());
    ASSERT_TRUE(packet.appendItem(zmc::FBM_NOP, nullptr, 0));
    for (uint16_t seq = 1; seq <= 3; ++seq) {
        ASSERT_TRUE(queue.push(packet));
        EXPECT_EQ(3u - seq, queue.available());
    }
    EXPECT_FALSE(queue.push(packet));
    for (uint16_t seq = 1; seq <= 3; ++seq) {
        ASSERT_TRUE(queue.pop(packet));
        EXPECT_EQ(seq, packet.seq);
    }
    ASSERT_TRUE(queue.push(packet));
    ASSERT_TRUE(queue.pop(packet));
    EXPECT_EQ(4u, packet.seq);

    // Clearing pending packets also restarts successful numbering at one.
    ASSERT_TRUE(queue.push(packet));
    queue.clear();
    ASSERT_TRUE(queue.push(packet));
    ASSERT_TRUE(queue.pop(packet));
    EXPECT_EQ(1u, packet.seq);
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(3u, queue.available());
}

// Build transport fragments without depending on app/qcmd from the fb module.
zmc::CmdPacket fragmentPacket(uint16_t type, const void* data, uint16_t total,
                              uint16_t offset, uint16_t bytes) {
    uint8_t param[MOTION_CMD_SINGLE_PARAM_MAX];
    MotionCmdFragmentHeader header = {type, total, offset};
    std::memcpy(param, &header, sizeof(header));
    std::memcpy(param + sizeof(header), static_cast<const uint8_t*>(data) + offset, bytes);
    zmc::CmdPacket packet;
    packet.reset();
    EXPECT_TRUE(packet.appendItem(zmc::FBM_FRAG, param, sizeof(header) + bytes));
    HeartbeatAck ack = {};
    EXPECT_TRUE(packet.appendItem(zmc::FBM_HEARTBEAT, &ack, sizeof(ack)));
    return packet;
}

TEST(MotionPacketFragments, UnsupportedCommandIsNotDispatchedAndHeartbeatIsProcessed) {
    zmc::FbMoveWrap2Queue wrap(0);
    zmc::MotionStatusMirror mirror;
    uint8_t data[512] = {};
    FlowCtrlConfig config = {};
    config.period = 1;
    std::memcpy(data, &config, sizeof(config));
    zmc::CmdPacket first = fragmentPacket(zmc::FBM_FLOWCTRL_CFG, data, sizeof(data), 0, 486);
    ASSERT_TRUE(wrap.getCmdQueue().push(first));
    wrap.runCycle();
    refreshStatus(wrap, mirror);
    EXPECT_EQ(512u, wrap.getCurrentPeriod());
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
    EXPECT_TRUE(confirms.empty());
    // Idle cycles do not discard the partial command or execute its available prefix.
    wrap.runCycle();
    EXPECT_EQ(512u, wrap.getCurrentPeriod());
    zmc::CmdPacket last = fragmentPacket(zmc::FBM_FLOWCTRL_CFG, data, sizeof(data), 486, 26);
    ASSERT_TRUE(wrap.getCmdQueue().push(last));
    wrap.runCycle();
    refreshStatus(wrap, mirror);
    EXPECT_EQ(512u, wrap.getCurrentPeriod());
    ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
    EXPECT_TRUE(confirms.empty());
    EXPECT_EQ(3u, wrap.getLastLatency());
}

TEST(MotionPacketFragments, FinalMultiMoveFragmentReachesTheGroupInTheSameCycle) {
    zmc::AxPouTaskBinderAuto pou(2);
    std::vector<AXIS_REF> axes = {pou.axisRef(0), pou.axisRef(1)};
    zmc::Ctx ctx;
    ctx.set_axis_arr(&axes);
    zmc::FbMoveWrap2Queue wrap(2);
    wrap.setCtx(&ctx);
    zmc::MotionStatusMirror mirror;
    const int indices[2] = {0, 1};
    zmc::CmdPacket config;
    config.reset();
    ASSERT_TRUE(config.appendItem(zmc::FBM_AXIS_GROUP, indices, sizeof(indices)));
    ASSERT_TRUE(wrap.getCmdQueue().push(config));
    wrap.runCycle();
    refreshStatus(wrap, mirror);
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
    ASSERT_EQ(2, wrap.zmotMotbuf.group(0)->axNum());
    const double positions[2] = {10.0, 20.0};
    for (uint16_t type : {zmc::FBM_MULTI_MOVE, zmc::FBM_MULTI_MOVE_REL}) {
        const uint16_t seq = type == zmc::FBM_MULTI_MOVE ? 2 : 4;
        zmc::CmdPacket first = fragmentPacket(type, positions, sizeof(positions), 0, 8);
        // An ordinary item after the heartbeat does not discard the partial command.
        ASSERT_TRUE(first.appendItem(zmc::FBM_NOP, nullptr, 0));
        ASSERT_TRUE(wrap.getCmdQueue().push(first));
        wrap.runCycle();
        refreshStatus(wrap, mirror);
        ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
        EXPECT_TRUE(confirms.empty());
        ASSERT_TRUE(wrap.getCmdQueue().push(
            fragmentPacket(type, positions, sizeof(positions), 8, 8)));
        wrap.runCycle();
        refreshStatus(wrap, mirror);
        ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
        ASSERT_EQ(1u, confirms.size());
        EXPECT_EQ(0x80000000u | (static_cast<uint32_t>(seq) << 7), confirms[0].id);
        EXPECT_EQ(type, confirms[0].command_type);
        EXPECT_EQ(zmc::FB_STAT_BUSY_ACTIVE, confirms[0].status);
        EXPECT_EQ(CMD_TARGET_GROUP, confirms[0].target_kind);
        EXPECT_EQ(static_cast<uint32_t>(wrap.cnt), confirms[0].tick);
    }
}

TEST(MotionPacketFragments, OutOfOrderOrResetNeverDispatchesAnIncompleteCommand) {
    zmc::AxPouTaskBinderAuto pou(2);
    std::vector<AXIS_REF> axes = {pou.axisRef(0), pou.axisRef(1)};
    zmc::Ctx ctx;
    ctx.set_axis_arr(&axes);
    zmc::FbMoveWrap2Queue wrap(2);
    wrap.setCtx(&ctx);
    zmc::MotionStatusMirror mirror;
    const int indices[2] = {0, 1};
    zmc::CmdPacket config;
    config.reset();
    ASSERT_TRUE(config.appendItem(zmc::FBM_AXIS_GROUP, indices, sizeof(indices)));
    ASSERT_TRUE(wrap.getCmdQueue().push(config));
    wrap.runCycle();
    refreshStatus(wrap, mirror);
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
    const double positions[2] = {10.0, 20.0};
    const zmc::CmdPacket first =
        fragmentPacket(zmc::FBM_MULTI_MOVE, positions, sizeof(positions), 0, 8);
    const zmc::CmdPacket last =
        fragmentPacket(zmc::FBM_MULTI_MOVE, positions, sizeof(positions), 8, 8);
    ASSERT_TRUE(wrap.getCmdQueue().push(first));
    wrap.runCycle();
    ASSERT_TRUE(wrap.getCmdQueue().push(
        fragmentPacket(zmc::FBM_MULTI_MOVE, positions, sizeof(positions), 7, 9)));
    wrap.runCycle();
    refreshStatus(wrap, mirror);
    ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
    ASSERT_EQ(1u, confirms.size());
    EXPECT_EQ(0x80000100u, confirms[0].id);
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, confirms[0].error_code);
    ASSERT_TRUE(wrap.getCmdQueue().push(first));
    wrap.runCycle();
    wrap.reset_runtime();
    mirror.resetCache();
    ASSERT_TRUE(wrap.getCmdQueue().push(config));
    wrap.runCycle();
    ASSERT_TRUE(wrap.getCmdQueue().push(last));
    wrap.runCycle();
    refreshStatus(wrap, mirror);
    ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
    // A new complete group must succeed after both discard paths.
    ASSERT_TRUE(wrap.getCmdQueue().push(first));
    ASSERT_TRUE(wrap.getCmdQueue().push(last));
    wrap.runCycle();
    wrap.runCycle();
    refreshStatus(wrap, mirror);
    ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
    ASSERT_EQ(1u, confirms.size());
    EXPECT_EQ(0x80000180u, confirms[0].id);
    EXPECT_EQ(zmc::FBM_MULTI_MOVE, confirms[0].command_type);
    EXPECT_EQ(zmc::FB_STAT_BUSY_ACTIVE, confirms[0].status);
}

TEST(MotionPacketProgress, StartsAtZeroAndTracksAcceptedPacketsAcrossReset) {
    zmc::FbMoveWrap2Queue wrap(0);
    zmc::MotionStatusMirror mirror;
    FlowCtrlConfig config = {};
    config.period = 1;
    zmc::CmdPacket command;
    command.reset();
    ASSERT_TRUE(command.appendItem(zmc::FBM_FLOWCTRL_CFG, &config, sizeof(config)));
    FlowCtrlStatus flow = {};

    for (unsigned int runtime = 0; runtime < 2; ++runtime) {
        if (runtime != 0) {
            wrap.reset_runtime();
            mirror.resetCache();
        }
        ASSERT_EQ(0, mirror.getFlowStatus(flow));
        EXPECT_EQ(0u, flow.processed_seq);
        wrap.runCycle();
        refreshStatus(wrap, mirror);
        ASSERT_EQ(0, mirror.getFlowStatus(flow));
        EXPECT_EQ(0u, flow.processed_seq);
        EXPECT_EQ(1u, mirror.ackSeq());

        for (uint16_t seq = 1; seq <= 2; ++seq) {
            ASSERT_TRUE(wrap.getCmdQueue().push(command));
            // Enqueue alone must not advance the accepted-packet progress.
            refreshStatus(wrap, mirror);
            ASSERT_EQ(0, mirror.getFlowStatus(flow));
            EXPECT_EQ(seq - 1u, flow.processed_seq);
            wrap.runCycle();
            refreshStatus(wrap, mirror);
            ASSERT_EQ(0, mirror.getFlowStatus(flow));
            EXPECT_EQ(seq, flow.processed_seq);
            std::vector<CmdResult> confirms;
            ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
            ASSERT_EQ(1u, confirms.size());
            EXPECT_EQ(0x80000000u | (static_cast<uint32_t>(seq) << 7), confirms[0].id);
        }
    }
}

TEST(MotionPacketProgress, WrapSkipsZeroAndAdvancesCommandIdEpoch) {
    zmc::FbMoveWrap2Queue wrap(0);
    zmc::MotionStatusMirror mirror;
    zmc::CmdPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::FBM_NOP, nullptr, 0));
    // Advance the producer to the last sequence without executing unrelated commands.
    for (uint32_t seq = 1; seq < 65535u; ++seq) {
        ASSERT_TRUE(wrap.getCmdQueue().push(packet));
        ASSERT_TRUE(wrap.getCmdQueue().pop(packet));
        ASSERT_EQ(seq, packet.seq);
    }
    FlowCtrlConfig config = {};
    config.period = 1;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::FBM_FLOWCTRL_CFG, &config, sizeof(config)));
    const uint16_t sequences[] = {65535u, 1u, 2u};
    const uint32_t ids[] = {0x807fff80u, 0x80800080u, 0x80800100u};
    for (unsigned int index = 0; index < 3; ++index) {
        ASSERT_TRUE(wrap.getCmdQueue().push(packet));
        wrap.runCycle();
        refreshStatus(wrap, mirror);
        FlowCtrlStatus flow = {};
        ASSERT_EQ(0, mirror.getFlowStatus(flow));
        EXPECT_EQ(sequences[index], flow.processed_seq);
        std::vector<CmdResult> confirms;
        ASSERT_EQ(0, mirror.getCmdConfirmations(confirms));
        ASSERT_EQ(1u, confirms.size());
        EXPECT_EQ(ids[index], confirms[0].id);
    }
}

}  // namespace
