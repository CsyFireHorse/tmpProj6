// GPT-6.1 Sol, 2026-11-04. Exercise self-probing through real command/status queues.
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
    bool paused() const { return flowPaused(); }
    unsigned int lag() const { return lagCount_; }
    int probeResult() const { return probeResult_; }
};

// The configuration and probe are consumed by the real runtime, not a forced ready flag.
void ready(zmc::FbMoveWrap& wrap, FlowCommands& command) {
    FlowCtrlConfig config = {};
    config.period = 1;
    ASSERT_EQ(0, command.setFlowConfig(config));
    ASSERT_EQ(0, command.heartbeat());
    wrap.runCycle();
    command.updateStatusCache();
    EXPECT_TRUE(command.paused());
    wrap.runCycle();
    command.updateStatusCache();
    ASSERT_FALSE(command.paused());
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

    // Queries can consume progress before the next heartbeat; duplicates cannot clear lag.
    command.heartbeat();
    EXPECT_EQ(0u, command.lag());
    FlowCtrlStatus flow = {};
    command.getFlowStatus(flow);
    zmc::StatusPacket duplicate;
    duplicate.reset();
    ASSERT_TRUE(duplicate.appendItem(zmc::STAT_FLOWCTRL_STATUS, &flow, sizeof(flow)));
    for (int i = 0; i < 40; ++i) {
        command.applyStatusPacket(duplicate);
        command.heartbeat();
    }
    EXPECT_EQ(40u, command.lag());
    EXPECT_FALSE(command.paused());
    command.heartbeat();
    ASSERT_TRUE(command.paused());
    ASSERT_EQ(0, command.testLongPacket(data, sizeof(data)));
    // Drain already published work. Its replies alone must not recover a failed fresh probe.
    for (int i = 0; i < 16; ++i) wrap.runCycle();
    command.updateStatusCache();
    EXPECT_TRUE(command.paused());
    for (int i = 0; i < 20; ++i) command.heartbeat();
    wrap.runCycle();
    command.updateStatusCache();
    ASSERT_FALSE(command.paused());
    command.heartbeat();
    wrap.runCycle();
    wrap.runCycle();
    command.getCmdConfirmations(results);
    ASSERT_EQ(1u, results.size());
    EXPECT_EQ(zmc::FB_STAT_DONE, results[0].status);
    EXPECT_FALSE(command.isScriptBusy());
}

TEST(MotionHeartbeatFlow, TimeoutRejectsOldReplyAndCacheClearsDoNotRecover) {
    zmc::FbMoveWrap wrap(0);
    FlowCommands command(wrap);
    ASSERT_EQ(0, command.heartbeat());
    ASSERT_EQ(1u, wrap.getCmdQueue().size());
    for (int i = 0; i < 39; ++i) ASSERT_EQ(0, command.heartbeat());
    EXPECT_EQ(1u, wrap.getCmdQueue().size());
    ASSERT_EQ(0, command.heartbeat());
    EXPECT_EQ(2u, wrap.getCmdQueue().size());
    EXPECT_EQ(ZMC_ERR_HOST_RECV_TIMEOUT, command.probeResult());
    command.clearDashboardCache();
    command.resetCache();
    EXPECT_TRUE(command.paused());
    wrap.runCycle();
    command.updateStatusCache();
    EXPECT_TRUE(command.paused());
    FlowCtrlConfig config = {};
    config.period = 1;
    ASSERT_EQ(0, command.setFlowConfig(config));
    wrap.runCycle();
    wrap.runCycle();
    command.updateStatusCache();
    EXPECT_FALSE(command.paused());
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
    command.updateStatusCache();
    ASSERT_NO_FATAL_FAILURE(ready(wrap, command));
    wrap.reset_runtime();
    command.updateStatusCache();
    EXPECT_TRUE(command.paused());
    ASSERT_NO_FATAL_FAILURE(ready(wrap, command));
}

TEST(MotionHeartbeatFlow, ProbeAndAcknowledgementsCrossSequenceWrap) {
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
    ASSERT_FALSE(command.paused());
    wrap.runCycle();
    command.updateStatusCache();
    command.heartbeat();
    EXPECT_EQ(0u, command.lag());
    EXPECT_FALSE(command.paused());
}
}  // namespace
