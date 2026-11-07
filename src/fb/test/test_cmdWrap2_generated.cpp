#if 0
#include <gtest/gtest.h>

#include "AxisRunTmpl.hpp"
#include "axis/fbt_axis_opaque.h"
#include "common/container/FbCmdQueue.hpp"
#include "fbt_errorcode_def.h"
#include "fb/mc_parameter_enum_def.h"
#include "motbuf/FbMoveWrapV3.hpp"
#include "motbuf/MotionCmdTypes.hpp"
#include "motbuf/fbmEnum.h"
#include "qcmd/MotionCmdInterface.hpp"
#include "qcmd/MotionCmdPacket.hpp"
#include "traj/include/TrajEnumDef.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <thread>
#include <vector>

using namespace zmc;

namespace {

bool popFirstItem(CmdQueueType& queue, uint16_t& type, uint16_t& paramLen,
                  const uint8_t*& paramPtr, CmdPacket& packet) {
    if (!queue.pop(packet)) {
        return false;
    }
    CmdItemIterator it(packet.payload, packet.payload_len);
    return it.next(type, paramLen, paramPtr);
}

// Pop one wire packet and read successive non-heartbeat items (sendPacket trails HEARTBEAT).
bool popCompactItems(CmdQueueType& queue, CmdPacket& packet, std::vector<uint16_t>& types,
                     std::vector<uint16_t>& paramLens, std::vector<const uint8_t*>& paramPtrs) {
    types.clear();
    paramLens.clear();
    paramPtrs.clear();
    if (!queue.pop(packet)) {
        return false;
    }
    CmdItemIterator it(packet.payload, packet.payload_len);
    uint16_t type = 0;
    uint16_t paramLen = 0;
    const uint8_t* paramPtr = nullptr;
    while (it.next(type, paramLen, paramPtr)) {
        if (type == static_cast<uint16_t>(FBM_HEARTBEAT)) {
            break;
        }
        types.push_back(type);
        paramLens.push_back(paramLen);
        paramPtrs.push_back(paramPtr);
    }
    return !types.empty();
}

}  // namespace

// TODO: Concurrent producers may need mutex protection before entering the SPSC queue.
TEST(CmdQueueSerialized, concurrentPushAssignsMonotonicSeq) {
    CmdQueueType queue;
    std::vector<std::thread> producers;

    for (int producer = 0; producer < 3; ++producer) {
        producers.push_back(std::thread([&queue, producer]() {
            for (int item = 0; item < 4; ++item) {
                CmdPacket packet;
                packet.reset();
                packet.payload_len = 1;
                packet.payload[0] = static_cast<uint8_t>(producer * 4 + item);
                EXPECT_TRUE(queue.push(packet));
            }
        }));
    }

    for (size_t i = 0; i < producers.size(); ++i) {
        producers[i].join();
    }

    std::vector<uint16_t> seqs;
    CmdPacket packet;
    while (queue.pop(packet)) {
        seqs.push_back(packet.seq);
    }

    ASSERT_EQ(12u, seqs.size());
    for (size_t i = 0; i < seqs.size(); ++i) {
        EXPECT_EQ(static_cast<uint16_t>(i + 1u), seqs[i]);
    }
}

TEST(CmdQueueSerialized, capacityFourQueueAcceptsThreePackets) {
    SerializedCmdQueue<4> queue;
    CmdPacket packet;
    packet.reset();
    packet.payload_len = 1;

    EXPECT_TRUE(queue.push(packet));
    EXPECT_TRUE(queue.push(packet));
    EXPECT_TRUE(queue.push(packet));
    EXPECT_FALSE(queue.push(packet));
    CmdPacket received;
    for (uint16_t seq = 1; seq <= 3; ++seq) {
        ASSERT_TRUE(queue.pop(received));
        EXPECT_EQ(seq, received.seq);
    }
    ASSERT_TRUE(queue.push(packet));
    ASSERT_TRUE(queue.pop(received));
    EXPECT_EQ(4u, received.seq);
}

TEST(CmdQueueSerialized, sequenceWrapsAfterSuccessfulPackets) {
    CmdQueueType queue;
    CmdPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(FBM_NOP, nullptr, 0));
    CmdPacket received;
    for (uint32_t number = 0; number <= 65536u; ++number) {
        ASSERT_TRUE(queue.push(packet));
        ASSERT_TRUE(queue.pop(received));
        EXPECT_EQ(static_cast<uint16_t>(number % 65535u + 1u), received.seq);
    }
}

TEST(MotionCmdInterfaceScript, FailedSendLeavesPacketForRetry) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmd(cmdQueue, statusQueue, exitFlag);
    CmdPacket filler;
    filler.reset();
    ASSERT_TRUE(filler.appendItem(FBM_NOP, nullptr, 0));
    while (cmdQueue.push(filler)) {
    }

    ASSERT_EQ(0, cmd.setSpeed(0, 7.0));
    EXPECT_EQ(ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED, cmd.advanceScript());
    EXPECT_TRUE(cmd.isScriptBusy());
    CmdPacket received;
    while (cmdQueue.pop(received)) {
    }
    ASSERT_EQ(0, cmd.advanceScript());
    EXPECT_FALSE(cmd.isScriptBusy());
    ASSERT_TRUE(cmdQueue.pop(received));
    EXPECT_EQ(16u, received.seq);
    CmdItemIterator it(received.payload, received.payload_len);
    uint16_t type = 0;
    uint16_t paramLen = 0;
    const uint8_t* paramPtr = nullptr;
    ASSERT_TRUE(it.next(type, paramLen, paramPtr));
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), type);
    ASSERT_EQ(sizeof(int), paramLen);
    int axis = -1;
    std::memcpy(&axis, paramPtr, sizeof(axis));
    EXPECT_EQ(0, axis);
    ASSERT_TRUE(it.next(type, paramLen, paramPtr));
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED), type);
}

TEST(MotionCmdInterfaceTransport, baseConstructorBindsCommandAndStatusQueues) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterfaceBase base(cmdQueue, statusQueue, exitFlag);
    MotionCmdInterface cmdIf(base);

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setSpeed(0, 2.0));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket commandPacket;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, commandPacket, types, paramLens, paramPtrs));
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED), types[1]);
    ASSERT_EQ(sizeof(int), paramLens[0]);
    int axis = -1;
    std::memcpy(&axis, paramPtrs[0], sizeof(axis));
    EXPECT_EQ(0, axis);
    ASSERT_EQ(sizeof(double), paramLens[1]);

    StatusPacket statusPacket;
    statusPacket.reset();
    AxisState inputState = {};
    inputState.axis_id = 0;
    inputState.axis_state = static_cast<uint8_t>(mcStandstill);
    inputState.tick = 7;
    ASSERT_TRUE(statusPacket.appendItem(STAT_STATE, &inputState, sizeof(inputState)));
    ASSERT_TRUE(statusQueue.push(statusPacket));

    cmdIf.updateStatusCache();
    AxisState outputState = {};
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getAxisState(0, outputState));
    EXPECT_EQ(7u, outputState.tick);
}

TEST(MotionCmdInterfaceTransport, AxisStateCacheSwitchesBetweenBasicAndExtended) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);
    StatusPacket packet;
    AxisState basic = {};
    basic.axis_id = 3;
    basic.axis_state = static_cast<uint8_t>(mcStandstill);
    basic.error_id = 5;
    basic.tick = 7;
    basic.act_pos = 2.5;
    basic.set_pos = 3.5;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(STAT_STATE, &basic, sizeof(basic)));
    ASSERT_TRUE(statusQueue.push(packet));
    cmdIf.updateStatusCache();

    AxisState outputBasic = {};
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getAxisState(3, outputBasic));
    EXPECT_EQ(7u, outputBasic.tick);
    AxisState2 outputExtended = {};
    EXPECT_EQ(ZMC_ERR_MOTCMD_AXIS_STATE2_INVALID_AXIS, cmdIf.getAxisState2(3, outputExtended));
    EXPECT_EQ(ZMC_ERR_MOTCMD_AXIS_STATE_INVALID_AXIS, cmdIf.getAxisState(0, outputBasic));

    AxisState2 extended = {};
    extended.axis_id = 3;
    extended.axis_state = static_cast<uint8_t>(mcStandstill);
    extended.error_id = 6;
    extended.tick = 8;
    extended.act_pos = 4.5;
    extended.set_pos = 5.5;
    extended.set_vel = 6.5;
    extended.set_acc = 7.5;
    extended.set_position = 800;
    extended.act_position = 700;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(STAT_STATE2, &extended, sizeof(extended)));
    ASSERT_TRUE(statusQueue.push(packet));
    cmdIf.updateStatusCache();

    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getAxisState2(3, outputExtended));
    EXPECT_EQ(8u, outputExtended.tick);
    EXPECT_DOUBLE_EQ(6.5, outputExtended.set_vel);
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getAxisState(3, outputBasic));
    EXPECT_EQ(8u, outputBasic.tick);
    EXPECT_DOUBLE_EQ(4.5, outputBasic.act_pos);

    basic.tick = 9;
    basic.act_pos = 8.5;
    basic.set_pos = 9.5;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(STAT_STATE, &basic, sizeof(basic)));
    ASSERT_TRUE(statusQueue.push(packet));
    cmdIf.updateStatusCache();

    EXPECT_EQ(ZMC_ERR_MOTCMD_AXIS_STATE2_INVALID_AXIS, cmdIf.getAxisState2(3, outputExtended));
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getAxisState(3, outputBasic));
    EXPECT_EQ(9u, outputBasic.tick);
    EXPECT_DOUBLE_EQ(8.5, outputBasic.act_pos);
}

TEST(MotionCmdInterfaceHomeSdo, serializesConfigurationWithoutPosition) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);
    HomeSdoConfig config = {};
    config.mode = 35;
    config.offset = 1.25;
    config.velocity_fast = 20.0;
    config.velocity_slow = 5.0;
    config.acceleration = 100.0;
    config.timeout_ms = 800;

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.homeSdo(0, config));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_HOME_SDO), types[1]);
    ASSERT_EQ(sizeof(int), paramLens[0]);
    int axis = -1;
    std::memcpy(&axis, paramPtrs[0], sizeof(axis));
    EXPECT_EQ(0, axis);
    ASSERT_EQ(sizeof(HomeSdoConfig), paramLens[1]);
    HomeSdoConfig serialized = {};
    std::memcpy(&serialized, paramPtrs[1], sizeof(serialized));
    EXPECT_EQ(config.mode, serialized.mode);
    EXPECT_DOUBLE_EQ(config.offset, serialized.offset);
    EXPECT_DOUBLE_EQ(config.velocity_fast, serialized.velocity_fast);
    EXPECT_DOUBLE_EQ(config.velocity_slow, serialized.velocity_slow);
    EXPECT_DOUBLE_EQ(config.acceleration, serialized.acceleration);
    EXPECT_EQ(config.timeout_ms, serialized.timeout_ms);

    MotionCmdPacket directPacket;
    static_cast<MotionPacket&>(directPacket).reset();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), directPacket.homeSdo(config));
    EXPECT_EQ(static_cast<uint16_t>(FBM_HOME_SDO), directPacket.firstType());
}

TEST(MotionCmdInterfaceHomeStatus, readsAxisAndCommandResultMirror) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);
    int status = -1;
    int errorId = -1;

    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getHomeStatus(0, status, errorId));
    EXPECT_EQ(FB_STAT_IDLE, status);
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), errorId);

    StatusPacket busyPacket;
    busyPacket.reset();
    AxisState busyState = {};
    busyState.axis_id = 0;
    busyState.axis_state = static_cast<uint8_t>(mcHomeEx);
    ASSERT_TRUE(busyPacket.appendItem(STAT_STATE, &busyState, sizeof(busyState)));
    ASSERT_TRUE(statusQueue.push(busyPacket));
    cmdIf.updateStatusCache();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getHomeStatus(0, status, errorId));
    EXPECT_EQ(FB_STAT_BUSY_ACTIVE, status);
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), errorId);

    StatusPacket resultPacket;
    resultPacket.reset();
    AxisState idleState = busyState;
    idleState.axis_state = static_cast<uint8_t>(mcStandstill);
    CmdResultInfo result = {};
    result.target_index = 0;
    result.target_kind = CMD_TARGET_AXIS;
    result.command_type = static_cast<uint16_t>(FBM_HOME_SDO);
    result.status = FB_STAT_DONE;
    ASSERT_TRUE(resultPacket.appendItem(STAT_STATE, &idleState, sizeof(idleState)));
    ASSERT_TRUE(resultPacket.appendItem(STAT_CMDRESULT, &result, sizeof(result)));
    ASSERT_TRUE(statusQueue.push(resultPacket));
    cmdIf.updateStatusCache();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getHomeStatus(0, status, errorId));
    EXPECT_EQ(FB_STAT_DONE, status);
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), errorId);

    StatusPacket errorPacket;
    errorPacket.reset();
    result.status = FB_STAT_ERROR;
    result.error_code = 172;
    ASSERT_TRUE(errorPacket.appendItem(STAT_CMDRESULT, &result, sizeof(result)));
    ASSERT_TRUE(statusQueue.push(errorPacket));
    cmdIf.updateStatusCache();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getHomeStatus(0, status, errorId));
    EXPECT_EQ(FB_STAT_ERROR, status);
    EXPECT_EQ(172, errorId);

    StatusPacket abortedPacket;
    abortedPacket.reset();
    result.status = FB_STAT_ABORTED;
    result.error_code = 0;
    ASSERT_TRUE(abortedPacket.appendItem(STAT_CMDRESULT, &result, sizeof(result)));
    ASSERT_TRUE(statusQueue.push(abortedPacket));
    cmdIf.updateStatusCache();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getHomeStatus(0, status, errorId));
    EXPECT_EQ(FB_STAT_ABORTED, status);
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), errorId);
}

TEST(MotionCmdAxisInfo, requestUsesTypedPayloadAndRejectsNegativeAxis) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG), cmdIf.requestAxisInfo(-1));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.requestAxisInfo(3));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    uint16_t type = 0;
    uint16_t paramLen = 0;
    const uint8_t* paramPtr = nullptr;
    ASSERT_TRUE(popFirstItem(cmdQueue, type, paramLen, paramPtr, packet));
    ASSERT_EQ(static_cast<uint16_t>(FBM_AXIS_INFO_QUERY), type);
    ASSERT_EQ(sizeof(AxisInfoQuery), paramLen);

    AxisInfoQuery query = {};
    std::memcpy(&query, paramPtr, sizeof(query));
    EXPECT_EQ(3u, query.axis_id);

    AxisInfo info = {};
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_AXIS_INFO_UNAVAILABLE), cmdIf.getAxisInfo(3, info));
}

TEST(MotionCmdInterfaceTransport, exitAccessorsUseSharedFlag) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_FALSE(cmdIf.exitFlag());
    cmdIf.sendExitFlag();
    EXPECT_TRUE(exitFlag);
    EXPECT_TRUE(cmdIf.exitFlag());
}

TEST(MotionCmdInterfaceTransport, fullCommandQueueReturnsExistingErrorCode) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);
    CmdPacket packet;
    packet.reset();
    packet.payload_len = 1;

    while (cmdQueue.push(packet)) {
        // Fill the bounded queue to exercise transport backpressure.
    }

    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED), cmdIf.send(packet));
}

TEST(MotionCmdInterfaceScript, apiBusyQueuesDeferredPacket) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("speedratio 0.5"));
    double pos = 10.0;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));
    EXPECT_TRUE(cmdIf.isScriptBusy());

    CmdPacket packet;
    EXPECT_FALSE(cmdQueue.pop(packet));

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    // Compact: consecutive items share one wire packet (plus HEARTBEAT trailer).
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(4u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED_RATIO), types[1]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[2]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[3]);
    ASSERT_EQ(sizeof(int), paramLens[0]);
    int axis = -1;
    std::memcpy(&axis, paramPtrs[0], sizeof(axis));
    EXPECT_EQ(0, axis);
    ASSERT_EQ(sizeof(double), paramLens[3]);
    double queuedPos = 0.0;
    std::memcpy(&queuedPos, paramPtrs[3], sizeof(queuedPos));
    EXPECT_DOUBLE_EQ(10.0, queuedPos);
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceScript, flowConfigSendsImmediatelyWhileScriptBusy) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("speedratio 0.5"));

    FlowCtrlConfig cfg;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getFlowConfig(cfg));
    cfg.period = 100;
    cfg.batchSize = 3;
    cfg.verboseFlag = 1;

    // setFlowConfig is the only typed API that sends immediately.
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setFlowConfig(cfg));

    FlowCtrlConfig cached;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getFlowConfig(cached));
    EXPECT_EQ(512, cached.period);
    EXPECT_EQ(0u, cached.batchSize);
    EXPECT_EQ(0u, static_cast<unsigned>(cached.verboseFlag));

    FlowCtrlStatus mirrored = {};
    mirrored.period = cfg.period;
    mirrored.batchSize = cfg.batchSize;
    mirrored.verboseFlag = cfg.verboseFlag;
    StatusPacket statusPacket;
    statusPacket.reset();
    ASSERT_TRUE(statusPacket.appendItem(STAT_FLOWCTRL_STATUS, &mirrored, sizeof(FlowCtrlStatus)));
    ASSERT_TRUE(statusQueue.push(statusPacket));
    cmdIf.updateStatusCache();

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getFlowConfig(cached));
    EXPECT_EQ(100u, cached.period);
    EXPECT_EQ(3u, cached.batchSize);
    EXPECT_EQ(1u, static_cast<unsigned>(cached.verboseFlag));
    EXPECT_TRUE(cmdIf.isScriptBusy());

    CmdPacket packet;
    uint16_t type = 0;
    uint16_t paramLen = 0;
    const uint8_t* paramPtr = nullptr;
    ASSERT_TRUE(popFirstItem(cmdQueue, type, paramLen, paramPtr, packet));
    EXPECT_EQ(static_cast<uint16_t>(FBM_FLOWCTRL_CFG), type);

    double pos = 10.0;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(4u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED_RATIO), types[1]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[2]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[3]);
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceScript, stopQueuesWithoutSweep) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("speedratio 0.5"));
    double pos = 10.0;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.stop(0));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));
    EXPECT_TRUE(cmdIf.isScriptBusy());

    CmdPacket packet;
    EXPECT_FALSE(cmdQueue.pop(packet));

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(8u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED_RATIO), types[1]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[2]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[3]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[4]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_STOP), types[5]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[6]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[7]);
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceScript, axisEnableQueuesWithoutSweep) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("speedratio 0.5"));
    double pos = 10.0;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.axisEnable(0, true));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));
    EXPECT_TRUE(cmdIf.isScriptBusy());

    CmdPacket packet;
    EXPECT_FALSE(cmdQueue.pop(packet));

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(8u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED_RATIO), types[1]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[2]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[3]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[4]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_POWER), types[5]);
    ASSERT_EQ(sizeof(double), paramLens[5]);
    double enableVal = 0.0;
    std::memcpy(&enableVal, paramPtrs[5], sizeof(enableVal));
    EXPECT_DOUBLE_EQ(1.0, enableVal);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[6]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[7]);
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceScript, scriptSweepClearsPendingScript) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("speedratio 0.5"));
    double pos = 10.0;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));
    EXPECT_TRUE(cmdIf.isScriptBusy());
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.scriptSweep());
    EXPECT_FALSE(cmdIf.isScriptBusy());

    CmdPacket packet;
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceSpeedRatio, typedApiQueuesSpeedRatioUntilAdvance) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setSpeedRatio(0, 0.5));
    EXPECT_TRUE(cmdIf.isScriptBusy());

    CmdPacket packet;
    EXPECT_FALSE(cmdQueue.pop(packet));

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED_RATIO), types[1]);
    ASSERT_EQ(sizeof(int), paramLens[0]);
    int axis = -1;
    std::memcpy(&axis, paramPtrs[0], sizeof(axis));
    EXPECT_EQ(0, axis);
    ASSERT_EQ(sizeof(double), paramLens[1]);
    double ratio = 0.0;
    std::memcpy(&ratio, paramPtrs[1], sizeof(ratio));
    EXPECT_DOUBLE_EQ(0.5, ratio);
}

TEST(MotionCmdInterfaceAxis, eachTypedCommandQueuesItsExplicitAxis) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setSpeed(2, 12.5));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(1, 4.0));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(4u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED), types[1]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[2]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[3]);

    int axis = -1;
    ASSERT_EQ(sizeof(int), paramLens[0]);
    std::memcpy(&axis, paramPtrs[0], sizeof(axis));
    EXPECT_EQ(2, axis);
    ASSERT_EQ(sizeof(double), paramLens[1]);
    double speed = 0.0;
    std::memcpy(&speed, paramPtrs[1], sizeof(speed));
    EXPECT_DOUBLE_EQ(12.5, speed);

    ASSERT_EQ(sizeof(int), paramLens[2]);
    std::memcpy(&axis, paramPtrs[2], sizeof(axis));
    EXPECT_EQ(1, axis);
    ASSERT_EQ(sizeof(double), paramLens[3]);
    double distance = 0.0;
    std::memcpy(&distance, paramPtrs[3], sizeof(distance));
    EXPECT_DOUBLE_EQ(4.0, distance);
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceAxis, scriptCommandWithoutAxisUsesZero) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("axis 1\ndefpos 3.5"));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(3u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[1]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_POSITION), types[2]);

    int axis = -1;
    ASSERT_EQ(sizeof(int), paramLens[0]);
    std::memcpy(&axis, paramPtrs[0], sizeof(axis));
    EXPECT_EQ(1, axis);
    ASSERT_EQ(sizeof(int), paramLens[1]);
    std::memcpy(&axis, paramPtrs[1], sizeof(axis));
    EXPECT_EQ(0, axis);
    ASSERT_EQ(sizeof(double), paramLens[2]);
    double pos = 0.0;
    std::memcpy(&pos, paramPtrs[2], sizeof(pos));
    EXPECT_DOUBLE_EQ(3.5, pos);
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceAxis, scriptAxisSuffixTargetsOnlyItsCommand) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("movesgabs 12.34 axis 1\nmovesgrel -4.5 axis 3\nmovesg 5"));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(6u, types.size());
    const uint16_t expectedTypes[] = {FBM_AXIS,          FBM_MOVE_ABSOLUTE, FBM_AXIS,
                                      FBM_MOVE_RELATIVE, FBM_AXIS,          FBM_MOVE_RELATIVE};
    const int expectedAxes[] = {1, 3, 0};
    for (size_t i = 0; i < 6; ++i) EXPECT_EQ(expectedTypes[i], types[i]);
    for (size_t i = 0; i < 3; ++i) {
        int axis = -1;
        ASSERT_EQ(sizeof(int), paramLens[i * 2]);
        std::memcpy(&axis, paramPtrs[i * 2], sizeof(axis));
        EXPECT_EQ(expectedAxes[i], axis);
    }
    const double expectedPositions[] = {12.34, -4.5, 5.0};
    for (size_t i = 0; i < 3; ++i) {
        double pos = 0.0;
        ASSERT_EQ(sizeof(double), paramLens[i * 2 + 1]);
        std::memcpy(&pos, paramPtrs[i * 2 + 1], sizeof(pos));
        EXPECT_DOUBLE_EQ(expectedPositions[i], pos);
    }
}

TEST(MotionCmdInterfaceAxisGroup, eachOperationQueuesItsOwnAxisList) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);
    const int axesA[] = {2, 5};
    const int axesB[] = {1, 4};
    const double absPositions[] = {10.5, 20.5};
    const double relPositions[] = {-1.5, -2.5};

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.multiMoveAbs(2, axesA, absPositions));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.multiMoveRel(2, axesB, relPositions));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.groupEnable(2, axesA));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.groupDisable(2, axesB));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.groupReset(2, axesA));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    const uint16_t expectedTypes[] = {
        FBM_AXIS_GROUP,   FBM_MULTI_MOVE, FBM_AXIS_GROUP,    FBM_MULTI_MOVE_REL, FBM_AXIS_GROUP,
        FBM_GROUP_ENABLE, FBM_AXIS_GROUP, FBM_GROUP_DISABLE, FBM_AXIS_GROUP,     FBM_GROUP_RESET};
    ASSERT_EQ(sizeof(expectedTypes) / sizeof(expectedTypes[0]), types.size());
    for (size_t i = 0; i < types.size(); ++i) EXPECT_EQ(expectedTypes[i], types[i]);

    const int* expectedAxes[] = {axesA, axesB, axesA, axesB, axesA};
    const size_t groupItemIndices[] = {0, 2, 4, 6, 8};
    for (size_t i = 0; i < 5; ++i) {
        const size_t item = groupItemIndices[i];
        ASSERT_EQ(2 * sizeof(int), paramLens[item]);
        int actualAxes[2] = {};
        std::memcpy(actualAxes, paramPtrs[item], sizeof(actualAxes));
        EXPECT_EQ(expectedAxes[i][0], actualAxes[0]);
        EXPECT_EQ(expectedAxes[i][1], actualAxes[1]);
    }
    ASSERT_EQ(2 * sizeof(double), paramLens[1]);
    ASSERT_EQ(2 * sizeof(double), paramLens[3]);
    double queuedPositions[2] = {};
    std::memcpy(queuedPositions, paramPtrs[1], sizeof(queuedPositions));
    EXPECT_DOUBLE_EQ(absPositions[1], queuedPositions[1]);
    std::memcpy(queuedPositions, paramPtrs[3], sizeof(queuedPositions));
    EXPECT_DOUBLE_EQ(relPositions[1], queuedPositions[1]);
}

TEST(MotionCmdInterfaceAxisGroup, scriptActionsReuseConfiguredAxesExplicitly) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("group 2 5\ngrp_en\nmoveabs 10 20"));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    const uint16_t expectedTypes[] = {FBM_AXIS_GROUP, FBM_AXIS_GROUP, FBM_GROUP_ENABLE,
                                      FBM_AXIS_GROUP, FBM_MULTI_MOVE};
    ASSERT_EQ(sizeof(expectedTypes) / sizeof(expectedTypes[0]), types.size());
    for (size_t i = 0; i < types.size(); ++i) EXPECT_EQ(expectedTypes[i], types[i]);

    const size_t groupItemIndices[] = {0, 1, 3};
    for (size_t i = 0; i < 3; ++i) {
        int axes[2] = {};
        ASSERT_EQ(sizeof(axes), paramLens[groupItemIndices[i]]);
        std::memcpy(axes, paramPtrs[groupItemIndices[i]], sizeof(axes));
        EXPECT_EQ(2, axes[0]);
        EXPECT_EQ(5, axes[1]);
    }
}

TEST(MotionCmdInterfaceAxisGroup, scriptAxgrpSuffixIsOneTimeAndDefaultsByPositionCount) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("moveabs 1 2\nmove 3 4 axgrp 5 7\ngrp_en axgrp 8 9\n"
                                      "moverel 11 12 axgrp 10 11\nmoveabs 13 14"));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    const uint16_t expectedTypes[] = {
        FBM_AXIS_GROUP,   FBM_MULTI_MOVE, FBM_AXIS_GROUP,     FBM_MULTI_MOVE_REL, FBM_AXIS_GROUP,
        FBM_GROUP_ENABLE, FBM_AXIS_GROUP, FBM_MULTI_MOVE_REL, FBM_AXIS_GROUP,     FBM_MULTI_MOVE};
    ASSERT_EQ(sizeof(expectedTypes) / sizeof(expectedTypes[0]), types.size());
    for (size_t i = 0; i < types.size(); ++i) EXPECT_EQ(expectedTypes[i], types[i]);

    const int expectedAxes[][2] = {{0, 1}, {5, 7}, {8, 9}, {10, 11}, {0, 1}};
    const size_t groupItemIndices[] = {0, 2, 4, 6, 8};
    for (size_t i = 0; i < 5; ++i) {
        int axes[2] = {};
        ASSERT_EQ(sizeof(axes), paramLens[groupItemIndices[i]]);
        std::memcpy(axes, paramPtrs[groupItemIndices[i]], sizeof(axes));
        EXPECT_EQ(expectedAxes[i][0], axes[0]);
        EXPECT_EQ(expectedAxes[i][1], axes[1]);
    }
}

TEST(MotionCmdInterfaceAxis, singleAxisMoveUsesScalarPayload) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveAbs(0, 6.25));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_ABSOLUTE), types[1]);
    ASSERT_EQ(sizeof(int), paramLens[0]);
    int axis = -1;
    std::memcpy(&axis, paramPtrs[0], sizeof(axis));
    EXPECT_EQ(0, axis);
    ASSERT_EQ(sizeof(double), paramLens[1]);
    double pos = 0.0;
    std::memcpy(&pos, paramPtrs[1], sizeof(pos));
    EXPECT_DOUBLE_EQ(6.25, pos);
}

TEST(MotionCmdInterfaceSpeedRatio, rejectsInvalidRatioWithoutQueueing) {
    const double invalidValues[] = {
        0.0,
        -0.1,
        1.1,
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(),
    };

    for (size_t i = 0; i < sizeof(invalidValues) / sizeof(invalidValues[0]); ++i) {
        CmdQueueType cmdQueue;
        StatusQueueType statusQueue;
        bool exitFlag = false;
        MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

        EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG),
                  cmdIf.setSpeedRatio(0, invalidValues[i]));
        CmdPacket packet;
        EXPECT_FALSE(cmdQueue.pop(packet));
    }
}

TEST(MotionCmdInterfaceSpeedRatio, scriptCommandQueuesSpeedRatio) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("speedratio 0.5"));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED_RATIO), types[1]);
    ASSERT_EQ(sizeof(int), paramLens[0]);
    int axis = -1;
    std::memcpy(&axis, paramPtrs[0], sizeof(axis));
    EXPECT_EQ(0, axis);
    ASSERT_EQ(sizeof(double), paramLens[1]);
    double ratio = 0.0;
    std::memcpy(&ratio, paramPtrs[1], sizeof(ratio));
    EXPECT_DOUBLE_EQ(0.5, ratio);
}

TEST(MotionCmdInterfaceScript, scriptQueuePreservesCommandOrder) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("speedratio 0.5\nmoveadd 2.0"));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    // Compact packet: both items + HEARTBEAT trailer; order preserved.
    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(4u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED_RATIO), types[1]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[2]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_ADDITIVE), types[3]);
    ASSERT_EQ(sizeof(double), paramLens[3]);
    double addPos = 0.0;
    std::memcpy(&addPos, paramPtrs[3], sizeof(addPos));
    EXPECT_DOUBLE_EQ(2.0, addPos);
    {
        CmdItemIterator it(packet.payload, packet.payload_len);
        uint16_t type = 0;
        uint16_t paramLen = 0;
        const uint8_t* paramPtr = nullptr;
        ASSERT_TRUE(it.next(type, paramLen, paramPtr));
        ASSERT_TRUE(it.next(type, paramLen, paramPtr));
        ASSERT_TRUE(it.next(type, paramLen, paramPtr));
        ASSERT_TRUE(it.next(type, paramLen, paramPtr));
        ASSERT_TRUE(it.next(type, paramLen, paramPtr));
        EXPECT_EQ(static_cast<uint16_t>(FBM_HEARTBEAT), type);
    }
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceCircular, scriptCommandsQueueExpectedCircularParameters) {
    struct CircularCase {
        const char* command;
        uint16_t cmdType;
        double aux1;
        double aux2;
        double end1;
        double end2;
        uint16_t pathChoice;
    };
    const CircularCase cases[] = {
        {"movecirc 10.5 20.5 1.5 2.5 0 axgrp 4 5", static_cast<uint16_t>(FBM_MULTI_CIRCULAR), 1.5,
         2.5, 10.5, 20.5, 1},
        {"movecircabs 11.5 21.5 3.5 4.5 1", static_cast<uint16_t>(FBM_MULTI_CIRCULAR_ABS), 3.5,
         4.5, 11.5, 21.5, 0},
        {"movecirc2 5.5 6.5 12.5 22.5 axgrp 6 7", static_cast<uint16_t>(FBM_MULTI_CIRCULAR2), 5.5,
         6.5, 12.5, 22.5, 0},
        {"movecirc2abs 7.5 8.5 13.5 23.5", static_cast<uint16_t>(FBM_MULTI_CIRCULAR2_ABS), 7.5,
         8.5, 13.5, 23.5, 0},
    };

    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("group 2 3"));
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        EXPECT_EQ(0, cmdIf.executeCommand(cases[i].command));
    }
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(1u + 2u * (sizeof(cases) / sizeof(cases[0])), types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS_GROUP), types[0]);
    int configuredAxes[2] = {};
    ASSERT_EQ(sizeof(configuredAxes), paramLens[0]);
    std::memcpy(configuredAxes, paramPtrs[0], sizeof(configuredAxes));
    EXPECT_EQ(2, configuredAxes[0]);
    EXPECT_EQ(3, configuredAxes[1]);
    const int expectedAxes[][2] = {{4, 5}, {2, 3}, {6, 7}, {2, 3}};
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        const size_t axisItem = 1 + i * 2;
        const size_t motionItem = axisItem + 1;
        EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS_GROUP), types[axisItem]);
        int axes[2] = {};
        ASSERT_EQ(sizeof(axes), paramLens[axisItem]);
        std::memcpy(axes, paramPtrs[axisItem], sizeof(axes));
        EXPECT_EQ(expectedAxes[i][0], axes[0]);
        EXPECT_EQ(expectedAxes[i][1], axes[1]);
        ASSERT_EQ(cases[i].cmdType, types[motionItem]);
        ASSERT_EQ(sizeof(MotionCmdCircularParam), paramLens[motionItem]);

        MotionCmdCircularParam param = {};
        std::memcpy(&param, paramPtrs[motionItem], sizeof(param));
        EXPECT_DOUBLE_EQ(cases[i].aux1, param.aux[0]);
        EXPECT_DOUBLE_EQ(cases[i].aux2, param.aux[1]);
        EXPECT_DOUBLE_EQ(cases[i].end1, param.end[0]);
        EXPECT_DOUBLE_EQ(cases[i].end2, param.end[1]);
        EXPECT_EQ(cases[i].pathChoice, param.pathChoice);
    }
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceCircular, typedInterfacesPreserveDoubleCoordinates) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    const int axes[] = {4, 7};
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR),
              cmdIf.moveCirc(2, axes, 0.125, -0.25, 0.375, -0.5, 0));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR),
              cmdIf.moveCircAbs(2, axes, 1.25, 2.25, 3.25, 4.25, 1));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveCirc2(2, axes, 5.25, 6.25, 7.25, 8.25));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR),
              cmdIf.moveCirc2Abs(2, axes, 9.25, 10.25, 11.25, 12.25));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    const uint16_t expectedTypes[] = {
        FBM_AXIS_GROUP, FBM_MULTI_CIRCULAR,  FBM_AXIS_GROUP, FBM_MULTI_CIRCULAR_ABS,
        FBM_AXIS_GROUP, FBM_MULTI_CIRCULAR2, FBM_AXIS_GROUP, FBM_MULTI_CIRCULAR2_ABS,
    };
    ASSERT_EQ(sizeof(expectedTypes) / sizeof(expectedTypes[0]), types.size());
    for (size_t i = 0; i < types.size(); ++i) EXPECT_EQ(expectedTypes[i], types[i]);

    const double expectedAux[][2] = {{0.375, -0.5}, {3.25, 4.25}, {5.25, 6.25}, {9.25, 10.25}};
    const double expectedEnd[][2] = {{0.125, -0.25}, {1.25, 2.25}, {7.25, 8.25}, {11.25, 12.25}};
    const uint16_t expectedPathChoice[] = {1, 0, 0, 0};
    for (size_t i = 0; i < 4; ++i) {
        int queuedAxes[2] = {};
        ASSERT_EQ(sizeof(axes), paramLens[i * 2]);
        std::memcpy(queuedAxes, paramPtrs[i * 2], sizeof(queuedAxes));
        EXPECT_EQ(axes[0], queuedAxes[0]);
        EXPECT_EQ(axes[1], queuedAxes[1]);

        MotionCmdCircularParam param = {};
        ASSERT_EQ(sizeof(param), paramLens[i * 2 + 1]);
        std::memcpy(&param, paramPtrs[i * 2 + 1], sizeof(param));
        EXPECT_DOUBLE_EQ(expectedAux[i][0], param.aux[0]);
        EXPECT_DOUBLE_EQ(expectedAux[i][1], param.aux[1]);
        EXPECT_DOUBLE_EQ(expectedEnd[i][0], param.end[0]);
        EXPECT_DOUBLE_EQ(expectedEnd[i][1], param.end[1]);
        EXPECT_EQ(expectedPathChoice[i], param.pathChoice);
    }
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceCircular, scriptUsesDefaultAxesAndOneTimeSuffix) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("movecirc2 1 2 3 4\n"
                                      "movecirc2abs 5 6 7 8 axgrp 3 4\n"
                                      "movecirc2 9 10 11 12"));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    const uint16_t expectedTypes[] = {FBM_AXIS_GROUP, FBM_MULTI_CIRCULAR2,
                                      FBM_AXIS_GROUP, FBM_MULTI_CIRCULAR2_ABS,
                                      FBM_AXIS_GROUP, FBM_MULTI_CIRCULAR2};
    ASSERT_EQ(sizeof(expectedTypes) / sizeof(expectedTypes[0]), types.size());
    for (size_t i = 0; i < types.size(); ++i) EXPECT_EQ(expectedTypes[i], types[i]);

    const int expectedAxes[][2] = {{0, 1}, {3, 4}, {0, 1}};
    const size_t axisItemIndices[] = {0, 2, 4};
    for (size_t i = 0; i < 3; ++i) {
        int queuedAxes[2] = {};
        ASSERT_EQ(sizeof(queuedAxes), paramLens[axisItemIndices[i]]);
        std::memcpy(queuedAxes, paramPtrs[axisItemIndices[i]], sizeof(queuedAxes));
        EXPECT_EQ(expectedAxes[i][0], queuedAxes[0]);
        EXPECT_EQ(expectedAxes[i][1], queuedAxes[1]);
    }
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceCircular, rejectsInvalidAxesAndCoordinatesWithoutQueueing) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);
    const int axes[] = {2, 3};

    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG),
              cmdIf.moveCirc(1, axes, 1.0, 2.0, 3.0, 4.0, 0));
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG),
              cmdIf.moveCirc(2, nullptr, 1.0, 2.0, 3.0, 4.0, 0));
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG),
              cmdIf.moveCirc(2, axes, 1.0, 2.0, 3.0, 4.0, 2));
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG),
              cmdIf.moveCirc2(3, axes, 1.0, 2.0, 3.0, 4.0));
    EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG),
              cmdIf.moveCirc2Abs(2, axes, std::numeric_limits<double>::infinity(), 2.0, 3.0, 4.0));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceCircular, rejectsMalformedScriptArguments) {
    const char* invalidCommands[] = {
        "movecirc 1 2 3 4",    "movecirc 1 2 3 4 2",         "movecirc 1 2 3 4 0 extra",
        "movecirc2 1 2 bad 4", "movecirc2abs 1 2 3 4 extra",
    };

    for (size_t i = 0; i < sizeof(invalidCommands) / sizeof(invalidCommands[0]); ++i) {
        CmdQueueType cmdQueue;
        StatusQueueType statusQueue;
        bool exitFlag = false;
        MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

        EXPECT_EQ(static_cast<int>(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED),
                  cmdIf.executeCommand(invalidCommands[i]));
        CmdPacket packet;
        EXPECT_FALSE(cmdQueue.pop(packet));
    }
}

TEST(MotionCmdInterfaceScript, deferredCommandsShareCompactPacket) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("speedratio 0.5"));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setSpeed(0, 1.0));
    double pos = 10.0;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));

    CmdPacket packet;
    EXPECT_FALSE(cmdQueue.pop(packet));

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    // Script + deferred API items merge into one compact packet; order preserved.
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(6u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED_RATIO), types[1]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[2]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_SET_SPEED), types[3]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[4]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[5]);
    ASSERT_EQ(sizeof(double), paramLens[5]);
    double queuedPos = 0.0;
    std::memcpy(&queuedPos, paramPtrs[5], sizeof(queuedPos));
    EXPECT_DOUBLE_EQ(10.0, queuedPos);
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceScript, waitSplitsCompactPackets) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    double pos = 1.0;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.waitTicks(5, 0));
    pos = 2.0;
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.moveRel(0, pos));

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());
    EXPECT_TRUE(cmdIf.isScriptBusy());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[1]);
    ASSERT_EQ(sizeof(double), paramLens[1]);
    double firstPos = 0.0;
    std::memcpy(&firstPos, paramPtrs[1], sizeof(firstPos));
    EXPECT_DOUBLE_EQ(1.0, firstPos);
    EXPECT_FALSE(cmdQueue.pop(packet));

    StatusPacket statusPkt;
    statusPkt.reset();
    AxisState st = {};
    st.axis_id = 0;
    st.axis_state = static_cast<uint8_t>(mcStandstill);
    st.tick = 5;
    ASSERT_TRUE(statusPkt.appendItem(STAT_STATE, &st, sizeof(st)));
    ASSERT_TRUE(statusQueue.push(statusPkt));
    cmdIf.updateStatusCache();
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    ASSERT_EQ(2u, types.size());
    EXPECT_EQ(static_cast<uint16_t>(FBM_AXIS), types[0]);
    EXPECT_EQ(static_cast<uint16_t>(FBM_MOVE_RELATIVE), types[1]);
    double secondPos = 0.0;
    std::memcpy(&secondPos, paramPtrs[1], sizeof(secondPos));
    EXPECT_DOUBLE_EQ(2.0, secondPos);
    EXPECT_FALSE(cmdQueue.pop(packet));
    EXPECT_FALSE(cmdIf.isScriptBusy());
}

TEST(MotionCmdInterfaceScript, nearlyFullPacketForcesNewPacket) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    // Each command appends an 8-byte axis selection and a 12-byte speed item.
    // The 24th command starts a new packet at the 90% merge threshold.
    const int kCommands = 40;
    for (int i = 0; i < kCommands; ++i) {
        EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setSpeed(0, static_cast<double>(i)));
    }
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    CmdPacket packet;
    std::vector<uint16_t> types;
    std::vector<uint16_t> paramLens;
    std::vector<const uint8_t*> paramPtrs;
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    EXPECT_EQ(46u, types.size());
    ASSERT_TRUE(popCompactItems(cmdQueue, packet, types, paramLens, paramPtrs));
    EXPECT_EQ(34u, types.size());
    EXPECT_FALSE(cmdQueue.pop(packet));
}

TEST(MotionCmdInterfaceScript, waitTicksOutsideScriptEnqueuesAndArmsLater) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.waitTicks(10, 0));
    EXPECT_TRUE(cmdIf.isScriptBusy());
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());
    EXPECT_TRUE(cmdIf.isScriptBusy());

    StatusPacket statusPkt;
    statusPkt.reset();
    AxisState st = {};
    st.axis_id = 0;
    st.axis_state = static_cast<uint8_t>(mcStandstill);
    st.tick = 10;
    ASSERT_TRUE(statusPkt.appendItem(STAT_STATE, &st, sizeof(st)));
    ASSERT_TRUE(statusQueue.push(statusPkt));
    cmdIf.updateStatusCache();
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());
    EXPECT_FALSE(cmdIf.isScriptBusy());
}

TEST(MotionCmdInterfaceScript, waitAxisIdleOutsideScriptEnqueuesAndArms) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.waitAxisIdle(0));
    EXPECT_TRUE(cmdIf.isScriptBusy());
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());
    EXPECT_TRUE(cmdIf.isScriptBusy());

    StatusPacket statusPkt;
    statusPkt.reset();
    AxisState st = {};
    st.axis_id = 0;
    st.axis_state = static_cast<uint8_t>(mcStandstill);
    st.tick = 1;
    ASSERT_TRUE(statusPkt.appendItem(STAT_STATE, &st, sizeof(st)));
    ASSERT_TRUE(statusQueue.push(statusPkt));
    cmdIf.updateStatusCache();
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());
    EXPECT_FALSE(cmdIf.isScriptBusy());
}

TEST(MotionCmdInterfaceScript, waitTicksInsideScriptEnqueuesBeforeGateArms) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    EXPECT_EQ(0, cmdIf.executeCommand("wait_ticks 10"));
    EXPECT_TRUE(cmdIf.isScriptBusy());
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());
    EXPECT_TRUE(cmdIf.isScriptBusy());

    StatusPacket statusPkt;
    statusPkt.reset();
    AxisState st = {};
    st.axis_id = 0;
    st.axis_state = static_cast<uint8_t>(mcStandstill);
    st.tick = 10;
    ASSERT_TRUE(statusPkt.appendItem(STAT_STATE, &st, sizeof(st)));
    ASSERT_TRUE(statusQueue.push(statusPkt));
    cmdIf.updateStatusCache();
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());
    EXPECT_FALSE(cmdIf.isScriptBusy());
}
#endif