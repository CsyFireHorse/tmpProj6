// GPT-6 (Codex), 2026-11-09. Exercise heartbeat flow through real command/status queues.
#include <gtest/gtest.h>
#include "common/zmc/fbt_errorcode_def.h"
#include "impl/fbt_impl_enum_def.h"
#include "motbuf/FbMoveWrapV3.hpp"
#include "motbuf/fbmEnum.h"
#include "qcmd/MotionCmdInterface.hpp"

namespace {
class FlowCommands : public zmc::MotionCmdInterface {
 public:
    explicit FlowCommands(zmc::FbMoveWrap& wrap) : MotionCmdInterface(wrap.getCmdInterface()) {}
    using MotionCmdPack::pumpHeartbeat;
    bool paused() const { return flowPaused(); }
    int probeResult() const { return probeResult_; }
};

// Consume configuration and heartbeat before enabling sends; leave scripts for the caller.
void ready(zmc::FbMoveWrap& wrap, FlowCommands& command) {
    FlowCtrlConfig config = {};
    config.period = 1;
    ASSERT_EQ(0, command.setFlowConfig(config));
    ASSERT_EQ(0, command.heartbeat());
    wrap.runCycle();
    wrap.runCycle();
    command.updateStatusCache();
    EXPECT_TRUE(command.paused());
    ASSERT_EQ(0, command.pumpHeartbeat());
    ASSERT_FALSE(command.paused());
    wrap.runCycle();
    std::vector<CmdResult> confirmations;
    command.getCmdConfirmations(confirmations);
}

TEST(MotionHeartbeatFlow, CachedCommandExecutesAndResumesAfterLag) {
    zmc::FbMoveWrap wrap(0);
    FlowCommands command(wrap);
    const uint8_t data[] = {1, 2, 3};
    ASSERT_EQ(0, command.testLongPacket(data, sizeof(data)));
    EXPECT_EQ(ZMC_ERR_MOTCMD_FLOW_PAUSED, command.syncPing(1));
    ASSERT_EQ(0, command.advanceScript());
    EXPECT_TRUE(wrap.getCmdQueue().empty());
    ASSERT_NO_FATAL_FAILURE(ready(wrap, command));
    ASSERT_EQ(0, command.heartbeat());
    wrap.runCycle();
    wrap.runCycle();
    std::vector<CmdResult> results;
    command.getCmdConfirmations(results);
    ASSERT_EQ(1u, results.size());
    EXPECT_EQ(zmc::FBM_TEST_LONG_PACKET, results[0].command_type);
    EXPECT_EQ(zmc::FB_STAT_DONE, results[0].status);

    // Queries retain the latest feedback; duplicate ACKs cannot postpone the timeout.
    command.heartbeat();
    FlowCtrlStatus flow = {};
    command.getFlowStatus(flow);
    zmc::StatusPacket duplicate;
    duplicate.reset();
    ASSERT_TRUE(duplicate.appendItem(zmc::STAT_FLOWCTRL_STATUS, &flow, sizeof(flow)));
    for (int i = 0; i < 40; ++i) {
        command.applyStatusPacket(duplicate);
        command.heartbeat();
    }
    EXPECT_FALSE(command.paused());
    command.heartbeat();
    ASSERT_TRUE(command.paused());
    ASSERT_EQ(0, command.testLongPacket(data, sizeof(data)));
    // Consuming the backlog can recover flow without covering a designated heartbeat.
    for (int i = 0; i < 16; ++i) wrap.runCycle();
    command.updateStatusCache();
    EXPECT_TRUE(command.paused());
    ASSERT_EQ(0, command.heartbeat());
    ASSERT_FALSE(command.paused());
    EXPECT_EQ(0, command.probeResult());
    wrap.runCycle();
    wrap.runCycle();
    command.getCmdConfirmations(results);
    ASSERT_EQ(1u, results.size());
    EXPECT_EQ(zmc::FB_STAT_DONE, results[0].status);
    EXPECT_FALSE(command.isScriptBusy());
}

TEST(MotionHeartbeatFlow, TimeoutWaitsForProgressAndCacheClearsDoNotRecover) {
    zmc::FbMoveWrap wrap(0);
    FlowCommands command(wrap);
    ASSERT_EQ(0, command.heartbeat());
    ASSERT_EQ(1u, wrap.getCmdQueue().size());
    for (int i = 0; i < 40; ++i) ASSERT_EQ(0, command.heartbeat());
    EXPECT_EQ(3u, wrap.getCmdQueue().size());
    EXPECT_EQ(0, command.probeResult());
    ASSERT_EQ(0, command.heartbeat());
    EXPECT_EQ(3u, wrap.getCmdQueue().size());
    EXPECT_EQ(ZMC_ERR_HOST_RECV_TIMEOUT, command.probeResult());
    command.clearDashboardCache();
    command.resetCache();
    EXPECT_TRUE(command.paused());
    ASSERT_EQ(0, command.heartbeat());
    EXPECT_EQ(ZMC_ERR_HOST_RECV_TIMEOUT, command.probeResult());
    EXPECT_TRUE(command.paused());
    FlowCtrlConfig config = {};
    config.period = 1;
    ASSERT_EQ(0, command.setFlowConfig(config));
    for (int i = 0; i < 4; ++i) wrap.runCycle();
    command.updateStatusCache();
    EXPECT_TRUE(command.paused());
    ASSERT_EQ(0, command.heartbeat());
    EXPECT_FALSE(command.paused());
    EXPECT_EQ(0, command.probeResult());
}

TEST(MotionHeartbeatFlow, FailedSendIsSpacedAndQueueResetIsDetected) {
    zmc::FbMoveWrap wrap(0);
    FlowCommands command(wrap);
    zmc::CmdPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::FBM_NOP, nullptr, 0));
    while (wrap.getCmdQueue().push(packet)) {}
    EXPECT_EQ(ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED, command.heartbeat());
    EXPECT_EQ(ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED, command.probeResult());
    ASSERT_TRUE(wrap.getCmdQueue().pop(packet));
    for (int i = 0; i < 19; ++i) ASSERT_EQ(0, command.heartbeat());
    EXPECT_FALSE(wrap.getCmdQueue().full());
    ASSERT_EQ(0, command.heartbeat());
    EXPECT_TRUE(wrap.getCmdQueue().full());
    wrap.reset_runtime();
    ASSERT_EQ(0, command.heartbeat());
    ASSERT_NO_FATAL_FAILURE(ready(wrap, command));
    wrap.reset_runtime();
    command.resetCache();  // Match the service stop path's mirror reset.
    command.resetFlow();
    ASSERT_EQ(0, command._heartbeat());  // A new sequence can appear before the next pump.
    command.updateStatusCache();
    ASSERT_EQ(0, command.heartbeat());
    EXPECT_TRUE(command.paused());
    ASSERT_NO_FATAL_FAILURE(ready(wrap, command));
}

TEST(MotionHeartbeatFlow, HeartbeatAndAcknowledgementsCrossSequenceWrap) {
    zmc::FbMoveWrap wrap(0);
    FlowCommands command(wrap);
    zmc::CmdPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::FBM_NOP, nullptr, 0));
    for (unsigned int i = 0; i < 65534; ++i) {
        ASSERT_TRUE(wrap.getCmdQueue().push(packet));
        ASSERT_TRUE(wrap.getCmdQueue().pop(packet));
    }
    ASSERT_EQ(0, command.heartbeat());
    FlowCtrlConfig config = {};
    config.period = 1;
    ASSERT_EQ(0, command.setFlowConfig(config));
    wrap.runCycle();
    command.updateStatusCache();
    ASSERT_EQ(0, command.pumpHeartbeat());
    ASSERT_FALSE(command.paused());
    wrap.runCycle();
    command.updateStatusCache();
    command.heartbeat();
    EXPECT_FALSE(command.paused());
}

TEST(MotionHeartbeatFlow, WindowProgressAndZeroSnapshotsConverge) {
    zmc::MotionCmdFlow flow;
    typedef zmc::MotionCmdFlow::State State;
    ASSERT_TRUE(flow.step(0, 0));
    for (int i = 0; i < 19; ++i) EXPECT_FALSE(flow.step(0, 0));
    EXPECT_TRUE(flow.step(0, 0));
    ASSERT_TRUE(flow.step(100, 98));
    ASSERT_EQ(State::Running, flow.state());
    EXPECT_TRUE(flow.step(145, 105));  // A progressing ACK may trail by exactly 40 packets.
    EXPECT_EQ(State::Running, flow.state());
    EXPECT_TRUE(flow.step(147, 106));
    EXPECT_EQ(State::Paused, flow.state());
    EXPECT_FALSE(flow.step(147, 106));
    EXPECT_TRUE(flow.step(147, 107));
    ASSERT_EQ(State::Running, flow.state());
    EXPECT_TRUE(flow.step(147, 0));  // Zero alone does not revoke the accepted baseline.
    for (int i = 0; i < 39; ++i) EXPECT_TRUE(flow.step(147, 107));
    EXPECT_EQ(State::Running, flow.state());
    EXPECT_TRUE(flow.step(147, 107));
    EXPECT_EQ(State::Stalled, flow.state());
    EXPECT_FALSE(flow.step(147, 107));
    EXPECT_TRUE(flow.step(148, 108));
    EXPECT_EQ(State::Running, flow.state());
    EXPECT_TRUE(flow.step(0, 108));
    EXPECT_EQ(State::Paused, flow.state());
    EXPECT_FALSE(flow.step(0, 108));
}

TEST(MotionHeartbeatFlow, ResetRestoresInitialFlow) {
    zmc::MotionCmdFlow flow;
    typedef zmc::MotionCmdFlow::State State;
    ASSERT_TRUE(flow.step(100, 100));
    for (int i = 0; i < 41; ++i) flow.step(100, 100);
    ASSERT_EQ(State::Stalled, flow.state());
    flow.reset();
    EXPECT_EQ(State::Paused, flow.state());
    EXPECT_TRUE(flow.step(1, 1));
    EXPECT_EQ(State::Running, flow.state());
    flow.reset();
    EXPECT_TRUE(flow.step(0, 0));
    EXPECT_FALSE(flow.step(0, 0));
}
}  // namespace
