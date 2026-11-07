// GPT-6 Codex, 2026-11-04. Exercise packetization and the real serialized queue adapter.
#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include "common/zmc/fbt_errorcode_def.h"
#include "impl/fbt_impl_enum_def.h"
#include "motbuf/FbMoveWrapV3.hpp"
#include "pou/AxPou.hpp"
#include "pou/Ctx.h"
#include "qcmd/MotionCmdInterface.hpp"

namespace {

class FragmentCommands : public zmc::MotionCmdInterface {
   public:
    FragmentCommands(zmc::CmdQueueType& commands, zmc::StatusQueueType& status, bool& exit)
        : MotionCmdInterface(commands, status, exit) {
        // Complete one probe over the actual queues before testing packet serialization.
        acknowledgeProbe(commands, status);
    }
    using MotionCmdScript::appendOrDeferItem;
    void acknowledgeProbe(zmc::CmdQueueType& commands, zmc::StatusQueueType& status) {
        ASSERT_EQ(0, pumpHeartbeat());
        zmc::CmdPacket probe;
        ASSERT_TRUE(commands.pop(probe));
        FlowCtrlStatus flow = {};
        flow.processed_seq = probe.seq;
        zmc::StatusPacket reply;
        reply.reset();
        ASSERT_TRUE(reply.appendItem(zmc::STAT_FLOWCTRL_STATUS, &flow, sizeof(flow)));
        ASSERT_TRUE(status.push(reply));
        updateStatusCache();
        ASSERT_FALSE(flowPaused());
    }
};

// Runtime-backed tests must consume the probe in the motion cycle as well.
void ready(zmc::FbMoveWrap& wrap, zmc::MotionCmdInterface& commands) {
    FlowCtrlConfig config = {};
    config.period = 1;
    ASSERT_EQ(0, commands.setFlowConfig(config));
    ASSERT_EQ(0, commands.heartbeat());
    wrap.runCycle();
    wrap.runCycle();
    commands.updateStatusCache();
    std::vector<CmdResult> results;
    commands.getCmdConfirmations(results);
}

// Decode exactly the transmitted group, including each packet's heartbeat trailer.
void expectFragments(zmc::CmdQueueType& queue, uint16_t commandType, const void* data,
                     size_t length, uint16_t firstSeq = 2) {
    std::vector<uint8_t> received;
    while (received.size() < length) {
        zmc::CmdPacket packet;
        ASSERT_TRUE(queue.pop(packet));
        EXPECT_EQ(firstSeq++, packet.seq);
        zmc::CmdItemIterator it(packet.payload, packet.payload_len);
        uint16_t type, bytes;
        const uint8_t* param;
        ASSERT_TRUE(it.next(type, bytes, param));
        ASSERT_EQ(zmc::FBM_FRAG, type);
        ASSERT_GE(bytes, sizeof(MotionCmdFragmentHeader));
        MotionCmdFragmentHeader header;
        std::memcpy(&header, param, sizeof(header));
        EXPECT_EQ(commandType, header.command_type);
        EXPECT_EQ(length, header.total_len);
        EXPECT_EQ(received.size(), header.offset);
        const size_t remaining = length - received.size();
        EXPECT_EQ(remaining < MOTION_CMD_FRAGMENT_DATA_MAX
                      ? remaining : MOTION_CMD_FRAGMENT_DATA_MAX, bytes - sizeof(header));
        received.insert(received.end(), param + sizeof(header), param + bytes);
        ASSERT_TRUE(it.next(type, bytes, param));
        EXPECT_EQ(zmc::FBM_HEARTBEAT, type);
        EXPECT_EQ(sizeof(HeartbeatAck), bytes);
        EXPECT_FALSE(it.next(type, bytes, param));
    }
    ASSERT_EQ(length, received.size());
    EXPECT_EQ(0, std::memcmp(data, received.data(), length));
}

TEST(QcmdFragments, ImmediateMultiMovePreservesAll512Bytes) {
    double positions[64];
    for (int i = 0; i < 64; ++i) positions[i] = i * 1.25 - 40.0;
    const uint16_t types[] = {zmc::FBM_MULTI_MOVE, zmc::FBM_MULTI_MOVE_REL};
    for (uint16_t type : types) {
        zmc::CmdQueueType queue;
        zmc::StatusQueueType status;
        bool exit = false;
        FragmentCommands commands(queue, status, exit);
        ASSERT_EQ(0, commands.submitItem(type, positions, sizeof(positions)));
        ASSERT_EQ(2u, queue.size());
        expectFragments(queue, type, positions, sizeof(positions));
        EXPECT_TRUE(queue.empty());
    }
}

TEST(QcmdFragments, LongPacketTestSendsEightPacketsAndRejectsMore) {
    zmc::CmdQueueType queue;
    zmc::StatusQueueType status;
    bool exit = false;
    FragmentCommands commands(queue, status, exit);
    std::vector<uint8_t> data(MOTION_CMD_FRAGMENT_PARAM_MAX + 1);
    for (size_t i = 0; i < data.size(); ++i) data[i] = static_cast<uint8_t>(i);

    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG,
              commands.testLongPacket(data.data(), static_cast<uint16_t>(data.size())));
    EXPECT_FALSE(commands.isScriptBusy());
    ASSERT_EQ(0, commands.testLongPacket(data.data(), MOTION_CMD_FRAGMENT_PARAM_MAX));
    ASSERT_EQ(0, commands.advanceScript());
    ASSERT_EQ(8u, queue.size());
    expectFragments(queue, zmc::FBM_TEST_LONG_PACKET, data.data(), MOTION_CMD_FRAGMENT_PARAM_MAX);
    EXPECT_TRUE(queue.empty());
}

TEST(QcmdFragments, LongPacketTestConfirmsOnlyAfterFinalFragment) {
    zmc::FbMoveWrap wrap(0);
    zmc::MotionCmdInterface commands(wrap.getCmdInterface());
    ASSERT_NO_FATAL_FAILURE(ready(wrap, commands));
    std::vector<uint8_t> data(MOTION_CMD_FRAGMENT_PARAM_MAX, 0x5a);
    ASSERT_EQ(0, commands.testLongPacket(data.data(), static_cast<uint16_t>(data.size())));
    ASSERT_EQ(0, commands.advanceScript());
    std::vector<CmdResult> confirms;
    for (size_t i = 0; i < MOTION_CMD_FRAGMENT_GROUP_MAX; ++i) {
        wrap.runCycle();
        ASSERT_EQ(0, commands.getCmdConfirmations(confirms));
        if (i + 1 < MOTION_CMD_FRAGMENT_GROUP_MAX) {
            EXPECT_TRUE(confirms.empty());
        } else {
            ASSERT_EQ(1u, confirms.size());
            EXPECT_EQ(0x80000180u, confirms[0].id);
            EXPECT_EQ(zmc::FBM_TEST_LONG_PACKET, confirms[0].command_type);
            EXPECT_EQ(CMD_TARGET_CONTROLLER, confirms[0].target_kind);
            EXPECT_EQ(zmc::FB_STAT_DONE, confirms[0].status);
        }
    }

    // A test command that fits in one packet follows the ordinary item switch.
    ASSERT_EQ(0, commands.testLongPacket(data.data(), 16));
    ASSERT_EQ(0, commands.advanceScript());
    wrap.runCycle();
    ASSERT_EQ(0, commands.getCmdConfirmations(confirms));
    ASSERT_EQ(1u, confirms.size());
    EXPECT_EQ(zmc::FBM_TEST_LONG_PACKET, confirms[0].command_type);
    EXPECT_EQ(zmc::FB_STAT_DONE, confirms[0].status);
}

TEST(QcmdFragments, TypedMultiMoveKeepsAxisConfigurationBeforeTheGroup) {
    int axes[64];
    double positions[64];
    for (int i = 0; i < 64; ++i) { axes[i] = i; positions[i] = i + 0.5; }
    for (int relative = 0; relative < 2; ++relative) {
        zmc::CmdQueueType queue;
        zmc::StatusQueueType status;
        bool exit = false;
        FragmentCommands commands(queue, status, exit);
        ASSERT_EQ(0, relative ? commands.multiMoveRel(64, axes, positions)
                              : commands.multiMoveAbs(64, axes, positions));
        ASSERT_EQ(0, commands.advanceScript());
        ASSERT_EQ(3u, queue.size());
        zmc::CmdPacket config;
        ASSERT_TRUE(queue.pop(config));
        zmc::CmdItemIterator it(config.payload, config.payload_len);
        uint16_t type, bytes;
        const uint8_t* param;
        ASSERT_TRUE(it.next(type, bytes, param));
        EXPECT_EQ(zmc::FBM_AXIS_GROUP, type);
        ASSERT_EQ(sizeof(axes), bytes);
        EXPECT_EQ(0, std::memcmp(axes, param, bytes));
        expectFragments(queue, relative ? zmc::FBM_MULTI_MOVE_REL : zmc::FBM_MULTI_MOVE,
                        positions, sizeof(positions), 3);
        EXPECT_TRUE(queue.empty());
    }
}

TEST(QcmdFragments, WholeGroupWaitsForSpaceWithoutConsumingSequenceNumbers) {
    zmc::CmdQueueType queue;
    zmc::StatusQueueType status;
    bool exit = false;
    FragmentCommands commands(queue, status, exit);
    zmc::CmdPacket filler;
    filler.reset();
    ASSERT_TRUE(filler.appendItem(zmc::FBM_NOP, nullptr, 0));
    while (queue.available() > 1) ASSERT_TRUE(queue.push(filler));
    const size_t queued = queue.size();
    double positions[64] = {};
    ASSERT_EQ(0, commands.appendOrDeferItem(zmc::FBM_MULTI_MOVE, positions, sizeof(positions)));
    EXPECT_EQ(ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED, commands.advanceScript());
    EXPECT_EQ(queued, queue.size());
    EXPECT_TRUE(commands.isScriptBusy());
    EXPECT_EQ(ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED,
              commands.submitItem(zmc::FBM_MULTI_MOVE_REL, positions, sizeof(positions)));
    EXPECT_EQ(queued, queue.size());
    ASSERT_TRUE(queue.pop(filler));
    ASSERT_EQ(2u, queue.available());
    ASSERT_EQ(0, commands.advanceScript());
    EXPECT_FALSE(commands.isScriptBusy());
    for (size_t i = 1; i < queued; ++i) ASSERT_TRUE(queue.pop(filler));
    expectFragments(queue, zmc::FBM_MULTI_MOVE, positions, sizeof(positions),
                    static_cast<uint16_t>(queued + 2));
    EXPECT_TRUE(queue.empty());
}

TEST(QcmdFragments, BoundaryReservesHeartbeatAndStartsFragmentingAt493Bytes) {
    uint8_t data[493];
    for (size_t i = 0; i < sizeof(data); ++i) data[i] = static_cast<uint8_t>(i);
    zmc::CmdQueueType queue;
    zmc::StatusQueueType status;
    bool exit = false;
    FragmentCommands commands(queue, status, exit);
    ASSERT_EQ(0, commands.submitItem(zmc::FBM_NOP, data, 492));
    zmc::CmdPacket packet;
    ASSERT_TRUE(queue.pop(packet));
    EXPECT_EQ(508u, packet.payload_len);
    zmc::CmdItemIterator it(packet.payload, packet.payload_len);
    uint16_t type, bytes;
    const uint8_t* param;
    ASSERT_TRUE(it.next(type, bytes, param));
    EXPECT_EQ(zmc::FBM_NOP, type);
    ASSERT_EQ(492u, bytes);
    EXPECT_EQ(0, std::memcmp(data, param, bytes));
    ASSERT_TRUE(it.next(type, bytes, param));
    EXPECT_EQ(zmc::FBM_HEARTBEAT, type);
    ASSERT_EQ(0, commands.submitItem(zmc::FBM_NOP, data, sizeof(data)));
    expectFragments(queue, zmc::FBM_NOP, data, sizeof(data), 3);
}

TEST(QcmdFragments, ScriptMergingReservesHeartbeatAndWaitStillGatesLaterItems) {
    zmc::CmdQueueType queue;
    zmc::StatusQueueType status;
    bool exit = false;
    FragmentCommands commands(queue, status, exit);
    uint8_t data[450] = {};
    ASSERT_EQ(0, commands.appendOrDeferItem(zmc::FBM_NOP, data, sizeof(data)));
    ASSERT_EQ(0, commands.appendOrDeferItem(zmc::FBM_NOP, data, 40));
    ASSERT_EQ(0, commands.waitTicks(1));
    ASSERT_EQ(0, commands.setSpeed(0, 5.0));
    ASSERT_EQ(0, commands.advanceScript());
    ASSERT_EQ(2u, queue.size());
    EXPECT_TRUE(commands.isScriptBusy());
    zmc::CmdPacket packet;
    ASSERT_TRUE(queue.pop(packet));
    EXPECT_EQ(466u, packet.payload_len);
    ASSERT_TRUE(queue.pop(packet));
    EXPECT_EQ(56u, packet.payload_len);
    ASSERT_EQ(0, commands.advanceScript());
    EXPECT_TRUE(queue.empty());
    AxisState state = {};
    state.tick = 1;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::STAT_STATE, &state, sizeof(state)));
    commands.applyStatusPacket(packet);
    ASSERT_EQ(0, commands.advanceScript());
    EXPECT_FALSE(commands.isScriptBusy());
    ASSERT_TRUE(queue.pop(packet));
    zmc::CmdItemIterator it(packet.payload, packet.payload_len);
    uint16_t type, bytes;
    const uint8_t* param;
    ASSERT_TRUE(it.next(type, bytes, param));
    EXPECT_EQ(zmc::FBM_AXIS, type);
    ASSERT_TRUE(it.next(type, bytes, param));
    EXPECT_EQ(zmc::FBM_SET_SPEED, type);
}

TEST(QcmdFragments, ExistingTwoAxisMultiMoveStillReachesGroupExecution) {
    zmc::AxPouTaskBinderAuto pou(2);
    std::vector<AXIS_REF> axes = {pou.axisRef(0), pou.axisRef(1)};
    zmc::Ctx ctx;
    ctx.set_axis_arr(&axes);
    zmc::FbMoveWrap wrap(2);
    wrap.setCtx(&ctx);
    zmc::MotionCmdInterface commands(wrap.getCmdInterface());
    ASSERT_NO_FATAL_FAILURE(ready(wrap, commands));
    const int indices[2] = {0, 1};
    const double positions[2] = {1.0, 2.0};
    ASSERT_EQ(0, commands.multiMoveAbs(2, indices, positions));
    ASSERT_EQ(0, commands.advanceScript());
    wrap.runCycle();
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, commands.getCmdConfirmations(confirms));
    ASSERT_EQ(2u, confirms.size());
    EXPECT_EQ(zmc::FBM_MULTI_MOVE, confirms[1].command_type);
    EXPECT_EQ(zmc::FB_STAT_BUSY_ACTIVE, confirms[1].status);
    EXPECT_EQ(CMD_TARGET_GROUP, confirms[1].target_kind);
    EXPECT_EQ(2, wrap.zmotMotbuf.group(0)->axNum());
}

TEST(QcmdFragments, OversizedMultiMoveDoesNotLeaveAnAxisConfiguration) {
    zmc::CmdQueueType queue;
    zmc::StatusQueueType status;
    bool exit = false;
    FragmentCommands commands(queue, status, exit);
    int axes[65] = {};
    double positions[65] = {};
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, commands.multiMoveAbs(65, axes, positions));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, commands.multiMoveRel(65, axes, positions));
    EXPECT_FALSE(commands.isScriptBusy());
    EXPECT_TRUE(queue.empty());
}

}  // namespace
