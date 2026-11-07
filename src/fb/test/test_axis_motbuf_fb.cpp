#if 0
/*
 * Axis function-block motbuf tests.
 * AI modification: GPT-5 Codex
 */
#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <type_traits>
#include <vector>

#include "motbuf/AxisMotbufBase.h"
#include "motbuf/AxisMotbufFb.h"
#include "motbuf/AxisWithFb.hpp"
#include "motbuf/FbMoveWrapV3.hpp"
#include "motbuf/fbmEnum.h"
#include "fbt_errorcode_def.h"
#include "impl/fbt_impl_enum_def.h"
#include "pou/AxPou.hpp"
#include "pou/Ctx.h"
#include "qcmd/MotionCmdInterface.hpp"
#include "qcmd/MotionCmdPack.hpp"
#include "axis/AxisRunTmpl.hpp"
#include "axis/axis_coe_channel.h"

static_assert(std::is_base_of<zmc::AxisMotbufBase, zmc::AxisMotbufFb>::value,
              "AxisMotbufFb must be an AxisMotbufBase subclass");
static_assert(!std::is_base_of<zmc::AxisMotbufBase, zmc::AxisWithFb>::value,
              "AxisWithFb must remain a single-axis FB holder");

namespace {
class TestMotionCmdPack : public zmc::MotionCmdPack {
   public:
    explicit TestMotionCmdPack(zmc::FbMoveWrap& wrap) : wrap_(wrap) {}

    int send(const zmc::CmdPacket& packet) override {
        return wrap_.getCmdQueue().push(packet) ? 0 : ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
    }

    bool tryPopStatus(zmc::StatusPacket& packet) override {
        return wrap_.getStatusQueue().pop(packet);
    }

   private:
    zmc::FbMoveWrap& wrap_;
};
}  // namespace

TEST(AxisMotbufFb, BindsAndRebindsAxesFromContext) {
    zmc::AxPouTaskBinderAuto pou(2);
    std::vector<AXIS_REF> axes;
    axes.push_back(pou.axisRef(1));
    axes.push_back(pou.axisRef(0));

    zmc::Ctx ctx;
    ctx.set_axis_arr(&axes);

    zmc::FbMoveWrapWithAxis wrap(0);
    wrap.setCtx(&ctx);
    wrap.axisMotbuf.reserve(4);
    ASSERT_EQ(0, wrap.axisMotbuf.resize(2));
    ASSERT_EQ(static_cast<std::size_t>(2), wrap.axisMotbuf.size());
    EXPECT_EQ(axes[0], wrap.axisMotbuf.axisRef(0));
    EXPECT_EQ(axes[1], wrap.axisMotbuf.axisRef(1));

    axes[0] = pou.axisRef(0);
    axes[1] = pou.axisRef(1);
    wrap.axisMotbuf.setCtx(&ctx);
    EXPECT_EQ(axes[0], wrap.axisMotbuf.axisRef(0));
    EXPECT_EQ(axes[1], wrap.axisMotbuf.axisRef(1));
}

TEST(AxisMotbufFb, RejectsNegativeResizeWithoutChangingAxes) {
    zmc::FbMoveWrapWithAxis wrap(1);
    ASSERT_EQ(static_cast<std::size_t>(1), wrap.axisMotbuf.size());

    EXPECT_EQ(-1, wrap.axisMotbuf.resize(-1));
    EXPECT_EQ(static_cast<std::size_t>(1), wrap.axisMotbuf.size());
}

TEST(AxisMotbufFb, RuntimeResetRebuildsSlotsAndMotionPool) {
    zmc::FbMoveWrapWithAxis wrap(1);
    zmc::CommandStructDefault cmd;
    cmd.cmdType = zmc::FBM_MOVE_RELATIVE;
    cmd.setDouble(1.0);

    wrap.axisMotbuf.axis(0).pushCommand(cmd);
    wrap.axisMotbuf.runCycleTop();
    ASSERT_FALSE(wrap.axisMotbuf.axis(0).fbList.empty());

    wrap.axisMotbuf.reset_runtime();
    EXPECT_EQ(static_cast<std::size_t>(1), wrap.axisMotbuf.size());

    wrap.axisMotbuf.axis(0).pushCommand(cmd);
    wrap.axisMotbuf.runCycleTop();
    EXPECT_FALSE(wrap.axisMotbuf.axis(0).fbList.empty());
}

TEST(FbMoveWrapReset, ClearsQueuesResultsAndRuntimeFields) {
    zmc::FbMoveWrap wrap(0);
    zmc::CmdPacket command;
    zmc::StatusPacket status;
    zmc::CmdResult result = {};
    command.reset();
    status.reset();

    ASSERT_TRUE(command.appendItem(zmc::FBM_EXIT, nullptr, 0));
    ASSERT_TRUE(wrap.getCmdQueue().push(command));
    ASSERT_TRUE(wrap.getStatusQueue().push(status));
    wrap.cmdResultArr.push(result);
    wrap.cnt = 42;
    wrap.exitFlag = true;
    wrap.axSelect = 2;
    wrap.axSelectFlag = 2;
    wrap.axSelectBak = 1;
    wrap.setStateSamplePeriod(4);

    wrap.reset_runtime();

    EXPECT_TRUE(wrap.getCmdQueue().empty());
    EXPECT_TRUE(wrap.getStatusQueue().empty());
    EXPECT_TRUE(wrap.cmdResultArr.empty());
    EXPECT_EQ(0u, wrap.cnt);
    EXPECT_FALSE(wrap.exitFlag);
    EXPECT_EQ(0, wrap.axSelect);
    EXPECT_EQ(0, wrap.axSelectFlag);
    EXPECT_EQ(0, wrap.axSelectBak);
    EXPECT_EQ(512u, wrap.getCurrentPeriod());
}

TEST(CmdConfirmation, ConfigItemsKeepPacketSequenceAndOrdinal) {
    zmc::FbMoveWrap wrap(1);
    zmc::MotionCmdInterface cmd(wrap.getCmdInterface());
    zmc::CmdPacket packet;
    packet.reset();
    const double speed = 12.0;
    const double accel = 34.0;
    FlowCtrlConfig flow = {};
    flow.period = 10;
    ASSERT_TRUE(packet.appendItem(zmc::FBM_SET_SPEED, &speed, sizeof(speed)));
    ASSERT_TRUE(packet.appendItem(zmc::FBM_SET_ACCEL, &accel, sizeof(accel)));
    ASSERT_TRUE(packet.appendItem(zmc::FBM_FLOWCTRL_CFG, &flow, sizeof(flow)));
    ASSERT_TRUE(wrap.getCmdQueue().push(packet));

    wrap.runCycle();
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
    ASSERT_EQ(3u, confirms.size());
    std::sort(confirms.begin(), confirms.end(),
              [](const CmdResult& a, const CmdResult& b) { return a.id < b.id; });
    EXPECT_EQ(0x80000080u, confirms[0].id);
    EXPECT_EQ(0x80000081u, confirms[1].id);
    EXPECT_EQ(0x80000082u, confirms[2].id);
    EXPECT_EQ(zmc::FBM_SET_SPEED, confirms[0].command_type);
    EXPECT_EQ(zmc::FBM_SET_ACCEL, confirms[1].command_type);
    EXPECT_EQ(zmc::FBM_FLOWCTRL_CFG, confirms[2].command_type);
    EXPECT_EQ(zmc::FB_STAT_DONE, confirms[0].status);
    EXPECT_EQ(zmc::FB_STAT_DONE, confirms[1].status);
    EXPECT_EQ(zmc::FB_STAT_DONE, confirms[2].status);
    EXPECT_DOUBLE_EQ(speed, wrap.axisMotbuf.axis(0).cmdVel);
    EXPECT_DOUBLE_EQ(accel, wrap.axisMotbuf.axis(0).cmdAcc);
    EXPECT_EQ(10u, wrap.getCurrentPeriod());
    ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
    EXPECT_TRUE(confirms.empty());
}

TEST(CmdConfirmation, PopCachedConfirmationsInFifoOrder) {
    // Send real configuration commands through the wrap before draining confirmations.
    zmc::FbMoveWrap wrap(1);
    TestMotionCmdPack cmd(wrap);
    zmc::CmdPacket packet;
    packet.reset();
    const double speed = 12.0;
    const double accel = 34.0;
    ASSERT_TRUE(packet.appendItem(zmc::FBM_SET_SPEED, &speed, sizeof(speed)));
    ASSERT_TRUE(packet.appendItem(zmc::FBM_SET_ACCEL, &accel, sizeof(accel)));
    ASSERT_EQ(0, cmd.sendPacket(packet));

    wrap.runCycle();
    cmd.updateStatusCache();

    CmdResult confirm = {};
    ASSERT_TRUE(cmd.popCmdConfirmation(confirm));
    EXPECT_EQ(zmc::FBM_SET_SPEED, confirm.command_type);
    ASSERT_TRUE(cmd.popCmdConfirmation(confirm));
    EXPECT_EQ(zmc::FBM_SET_ACCEL, confirm.command_type);
    EXPECT_FALSE(cmd.popCmdConfirmation(confirm));
}

TEST(CmdConfirmation, BatchesConfirmationsAcrossStatusPackets) {
    zmc::FbMoveWrap wrap(1);
    zmc::CmdPacket command;
    command.reset();
    for (int i = 0; i < 32; ++i) {
        const double speed = static_cast<double>(i + 1);
        ASSERT_TRUE(command.appendItem(zmc::FBM_SET_SPEED, &speed, sizeof(speed)));
    }
    ASSERT_TRUE(wrap.getCmdQueue().push(command));

    wrap.runCycle();
    EXPECT_DOUBLE_EQ(32.0, wrap.axisMotbuf.axis(0).cmdVel);

    unsigned int confirmed = 0;
    const unsigned int first_packet_limit = static_cast<unsigned int>(
        zmc::MAX_PAYLOAD_SIZE / (sizeof(CmdResult) + 2 * sizeof(uint16_t)));
    for (unsigned int packetIndex = 0; packetIndex < 2; ++packetIndex) {
        zmc::StatusPacket status;
        ASSERT_TRUE(wrap.getStatusQueue().pop(status));
        zmc::StatusItemIterator it(status.payload, status.payload_len);
        uint16_t type = 0;
        uint16_t length = 0;
        const uint8_t* data = nullptr;
        unsigned int inPacket = 0;
        while (it.next(type, length, data)) {
            ASSERT_EQ(zmc::STAT_CMD_CONFIRM, type);
            ASSERT_EQ(sizeof(CmdResult), length);
            CmdResult confirm = {};
            std::memcpy(&confirm, data, sizeof(confirm));
            EXPECT_EQ(0x80000080u + confirmed, confirm.id);
            EXPECT_EQ(zmc::FB_STAT_DONE, confirm.status);
            ++confirmed;
            ++inPacket;
        }
        EXPECT_EQ(packetIndex == 0 ? first_packet_limit : 32u - first_packet_limit, inPacket);
    }
    EXPECT_EQ(32u, confirmed);
}

TEST(CmdConfirmation, ChangeRatioHasAcceptanceAndMatchingTerminalResult) {
    zmc::AxPouTaskBinderAuto pou(1);
    std::vector<AXIS_REF> axes(1, pou.axisRef(0));
    zmc::Ctx ctx;
    ctx.set_axis_arr(&axes);
    zmc::FbMoveWrap wrap(1);
    wrap.setCtx(&ctx);
    zmc::MotionCmdInterface cmd(wrap.getCmdInterface());
    const int ratio[2] = {2, 1};
    // The shared terminal payload must retain the unsigned command-binding cycle.
    wrap.cnt = 0x80000000u;
    zmc::CmdPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::FBM_SET_NUM_DEN, ratio, sizeof(ratio)));
    ASSERT_TRUE(wrap.getCmdQueue().push(packet));

    wrap.runCycle();
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
    ASSERT_EQ(1u, confirms.size());
    EXPECT_EQ(zmc::FB_STAT_BUSY_ACTIVE, confirms[0].status);
    EXPECT_EQ(zmc::FBM_SET_NUM_DEN, confirms[0].command_type);

    std::vector<zmc::CmdResultInfo> results;
    for (int cycle = 0; cycle < 8; ++cycle) {
        wrap.runCycle();
        cmd.updateStatusCache();
        if (cmd.getCmdResults(results) == 0) break;
    }
    ASSERT_FALSE(results.empty());
    EXPECT_EQ(confirms[0].id, results.back().id);
    EXPECT_EQ(zmc::FB_STAT_DONE, results.back().status);
    EXPECT_EQ(zmc::FBM_SET_NUM_DEN, results.back().command_type);
    EXPECT_EQ(CMD_TARGET_AXIS, results.back().target_kind);
    EXPECT_EQ(confirms[0].tick, cmd_result_data_u32(&results.back()));
}

// Both Home variants share one FB implementation but retain their submitted command types.
TEST(CmdConfirmation, HomeVariantsProduceMatchingTerminalRecordsThroughTheAxisCycle) {
    for (bool sdo : {false, true}) {
        zmc::AxisRunTmpl<> runtime;
        auto& cfg = runtime.get_config_ref(0);
        cfg.pdo_flag.enable_modes_of_operation = true;
        cfg.pdo_flag.enable_modes_of_operation_display = true;
        cfg.debug.servo_fak_pos = -100;
        cfg.cycle_time = 10000;
        runtime.reload_config(0);
        std::vector<AXIS_REF> axes(1, runtime.axisRef(0));
        zmc::Ctx ctx;
        ctx.set_axis_arr(&axes);
        zmc::FbMoveWrap wrap(1);
        wrap.setCtx(&ctx);
        zmc::MotionCmdInterface cmd(wrap.getCmdInterface());
        zmc::CmdPacket packet;
        packet.reset();
        const double enabled = 1;
        ASSERT_TRUE(packet.appendItem(zmc::FBM_POWER, &enabled, sizeof(enabled)));
        ASSERT_TRUE(wrap.getCmdQueue().push(packet));
        for (int cycle = 0; cycle < 8; ++cycle) {
            wrap.runCycle();
            runtime.axisRunCycle();
        }
        std::vector<CmdResult> confirms;
        ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
        packet.reset();
        const uint16_t type = sdo ? zmc::FBM_HOME_SDO : zmc::FBM_HOME;
        const HomeSdoConfig home = {-5, 15, 200, 20, 1000, 1000};
        const double position = 40;
        ASSERT_TRUE(packet.appendItem(type, sdo ? static_cast<const void*>(&home) : &position,
                                     sdo ? sizeof(home) : sizeof(position)));
        ASSERT_TRUE(wrap.getCmdQueue().push(packet));
        for (int cycle = 0; cycle < 80; ++cycle) {
            wrap.runCycle();
            runtime.axisRunCycle();
            // The fixture acts as the CoE bridge and completes the existing axis-owned request.
            zmc::AxisCoeRequest request = {};
            if (zmc::axis_coe_req_head(axes[0], &request) == 0) {
                zmc::axis_coe_result_push(axes[0], request.tag, 0, request.index,
                                         request.subindex, request.len,
                                         static_cast<uint32_t>(request.value));
                zmc::axis_coe_req_pop(axes[0]);
            }
        }
        ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
        ASSERT_EQ(1u, confirms.size());
        EXPECT_EQ(zmc::FB_STAT_BUSY_ACTIVE, confirms[0].status);
        std::vector<CmdResult> results;
        ASSERT_EQ(0, cmd.getCmdResults(results));
        ASSERT_EQ(1u, results.size());
        EXPECT_EQ(type, results[0].command_type);
        EXPECT_EQ(confirms[0].id, results[0].id);
        EXPECT_EQ(zmc::FB_STAT_DONE, results[0].status);
        EXPECT_EQ(0u, results[0].error_code);
        EXPECT_EQ(confirms[0].tick, cmd_result_data_u32(&results[0]));
        int status, error;
        ASSERT_EQ(0, cmd.getHomeStatus(0, status, error));
        EXPECT_EQ(zmc::FB_STAT_DONE, status);
        EXPECT_EQ(0, error);
    }
}

TEST(CmdConfirmation, PowerHasOneConfirmationAndNoTerminalResult) {
    zmc::AxPouTaskBinderAuto pou(1);
    std::vector<AXIS_REF> axes(1, pou.axisRef(0));
    zmc::Ctx ctx;
    ctx.set_axis_arr(&axes);
    zmc::FbMoveWrap wrap(1);
    wrap.setCtx(&ctx);
    zmc::MotionCmdInterface cmd(wrap.getCmdInterface());
    const double on = 1.0;
    zmc::CmdPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::FBM_POWER, &on, sizeof(on)));
    ASSERT_TRUE(wrap.getCmdQueue().push(packet));

    wrap.runCycle();
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
    ASSERT_EQ(1u, confirms.size());
    EXPECT_EQ(zmc::FB_STAT_BUSY_ACTIVE, confirms[0].status);
    for (int cycle = 0; cycle < 4; ++cycle) wrap.runCycle();
    ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
    EXPECT_TRUE(confirms.empty());
    std::vector<zmc::CmdResultInfo> results;
    EXPECT_NE(0, cmd.getCmdResults(results));
}

TEST(CmdConfirmation, GroupConfigurationAndMoveConfirmAtExecution) {
    zmc::AxPouTaskBinderAuto pou(2);
    std::vector<AXIS_REF> axes;
    axes.push_back(pou.axisRef(0));
    axes.push_back(pou.axisRef(1));
    zmc::Ctx ctx;
    ctx.set_axis_arr(&axes);
    zmc::FbMoveWrap wrap(2);
    wrap.setCtx(&ctx);
    zmc::MotionCmdInterface cmd(wrap.getCmdInterface());
    const int groupIndex = 0;
    const int groupAxes[2] = {0, 1};
    const double target[2] = {1.0, 2.0};
    zmc::CmdPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::FBM_GROUP_SELECT, &groupIndex, sizeof(groupIndex)));
    ASSERT_TRUE(packet.appendItem(zmc::FBM_AXIS_GROUP, groupAxes, sizeof(groupAxes)));
    ASSERT_TRUE(packet.appendItem(zmc::FBM_MULTI_MOVE, target, sizeof(target)));
    ASSERT_TRUE(wrap.getCmdQueue().push(packet));

    wrap.runCycle();
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
    ASSERT_EQ(2u, confirms.size());
    EXPECT_EQ(0x80000081u, confirms[0].id);
    EXPECT_EQ(0x80000082u, confirms[1].id);
    EXPECT_EQ(zmc::FB_STAT_DONE, confirms[0].status);
    EXPECT_EQ(zmc::FB_STAT_BUSY_ACTIVE, confirms[1].status);
    EXPECT_EQ(CMD_TARGET_GROUP, confirms[0].target_kind);
    EXPECT_EQ(2, wrap.zmotMotbuf.group(0)->axNum());
    std::vector<zmc::CmdResultInfo> results;
    EXPECT_NE(0, cmd.getCmdResults(results));
}

TEST(CmdConfirmation, StatusQueuePressureRetainsConfirmation) {
    zmc::FbMoveWrap wrap(1);
    zmc::MotionCmdInterface cmd(wrap.getCmdInterface());
    zmc::StatusPacket filler;
    filler.reset();
    while (wrap.getStatusQueue().push(filler)) {}

    const double speed = 9.0;
    zmc::CmdPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::FBM_SET_SPEED, &speed, sizeof(speed)));
    ASSERT_TRUE(wrap.getCmdQueue().push(packet));
    wrap.runCycle();
    EXPECT_DOUBLE_EQ(speed, wrap.axisMotbuf.axis(0).cmdVel);
    EXPECT_EQ(1u, wrap.cmdConfirmArr.size());

    cmd.updateStatusCache();
    wrap.runCycle();
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
    ASSERT_EQ(1u, confirms.size());
    EXPECT_EQ(0x80000080u, confirms[0].id);
    EXPECT_EQ(zmc::FB_STAT_DONE, confirms[0].status);
}

TEST(CmdConfirmation, BatchesResultsAfterStatusQueuePressure) {
    zmc::FbMoveWrap wrap(0);
    zmc::StatusPacket filler;
    filler.reset();
    while (wrap.getStatusQueue().push(filler)) {}

    zmc::CmdResult first = {};
    first.id = 1;
    first.target_kind = CMD_TARGET_AXIS;
    zmc::CmdResult second = {};
    second.id = 2;
    second.target_kind = CMD_TARGET_AXIS;
    wrap.cmdResultArr.push(first);
    wrap.cmdResultArr.push(second);
    wrap.runCycle();
    EXPECT_EQ(2u, wrap.cmdResultArr.size());

    zmc::StatusPacket status;
    while (wrap.getStatusQueue().pop(status)) {}
    wrap.runCycle();
    ASSERT_TRUE(wrap.getStatusQueue().pop(status));
    zmc::StatusItemIterator it(status.payload, status.payload_len);
    uint16_t type = 0;
    uint16_t length = 0;
    const uint8_t* data = nullptr;
    for (uint16_t index = 1; index <= 2; ++index) {
        ASSERT_TRUE(it.next(type, length, data));
        ASSERT_EQ(zmc::STAT_CMDRESULT, type);
        ASSERT_EQ(sizeof(zmc::CmdResult), length);
        zmc::CmdResult result = {};
        std::memcpy(&result, data, sizeof(result));
        EXPECT_EQ(index, result.id);
    }
    EXPECT_FALSE(it.next(type, length, data));
    EXPECT_TRUE(wrap.cmdResultArr.empty());
}

TEST(CmdConfirmation, PacketSequenceWrapAdvancesIdEpoch) {
    zmc::FbMoveWrap wrap(1);
    zmc::MotionCmdInterface cmd(wrap.getCmdInterface());
    zmc::CmdPacket skip;
    skip.reset();
    ASSERT_TRUE(skip.appendItem(zmc::FBM_NOP, nullptr, 0));
    zmc::CmdPacket popped;
    for (uint32_t number = 0; number < 65534u; ++number) {
        ASSERT_TRUE(wrap.getCmdQueue().push(skip));
        ASSERT_TRUE(wrap.getCmdQueue().pop(popped));
    }

    const double speed = 5.0;
    zmc::CmdPacket packet;
    packet.reset();
    ASSERT_TRUE(packet.appendItem(zmc::FBM_SET_SPEED, &speed, sizeof(speed)));
    ASSERT_TRUE(wrap.getCmdQueue().push(packet));
    wrap.runCycle();
    ASSERT_TRUE(wrap.getCmdQueue().push(packet));
    wrap.runCycle();

    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, cmd.getCmdConfirmations(confirms));
    ASSERT_EQ(2u, confirms.size());
    EXPECT_EQ(0x807fff80u, confirms[0].id);
    EXPECT_EQ(0x80800080u, confirms[1].id);
}

// new add from test_cmdWrap2_generated

TEST(MotionCmdAxisInfo, returnsAxisWithFbParametersAndPulseRatio) {
    FbMoveWrap wrap(1);
    AxisOpaque axis;
    AxisCiaConfig cfg;
    AxisCiaConfig_default_init(&cfg);
    cfg.is_axis_virtual = true;
    axis.set_config(&cfg);
    wrap.axisMotbuf.bindAxis(axis.axis(), 0);
    MotionCmdInterface cmdIf(wrap.getCmdInterface());

    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setSpeed(0, 12.5));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setAccel(0, 23.5));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setDecel(0, 34.5));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setJerk(0, 45.5));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.setUnitsNumDen(0, 5, 2));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.requestAxisInfo(0));
    EXPECT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.advanceScript());

    wrap.runCycle();
    cmdIf.updateStatusCache();

    AxisInfo info = {};
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), cmdIf.getAxisInfo(0, info));
    EXPECT_EQ(0, info.error_code);
    EXPECT_DOUBLE_EQ(12.5, info.velocity);
    EXPECT_DOUBLE_EQ(23.5, info.acceleration);
    EXPECT_DOUBLE_EQ(34.5, info.deceleration);
    EXPECT_DOUBLE_EQ(45.5, info.jerk);
    EXPECT_EQ(5, info.pulses_per_unit_num);
    EXPECT_EQ(2u, info.pulses_per_unit_den);
}

// Numeric reads retain both result words and their explicit targets. GPT-6 Codex.
TEST(MotionCmdReadParameter, ReadsEachAxisThroughExistingConfirmations) {
    FbMoveWrap wrap(2);
    AxisOpaque axes[2];
    MotionCmdInterface command(wrap.getCmdInterface());
    for (int i = 0; i < 2; ++i) {
        AxisCiaConfig cfg;
        AxisCiaConfig_default_init(&cfg);
        cfg.is_axis_virtual = true;
        cfg.finite.software_limits.negative = -12.3456789 - i;
        axes[i].set_config(&cfg);
        wrap.axisMotbuf.bindAxis(axes[i].axis(), i);
        ASSERT_EQ(0, command.requestReadParameter(i, PARA_fSWLimitNegative, 41u + i));
    }
    // Each request occupies one ordinary packet and returns one ordinary confirmation.
    for (int i = 0; i < 2; ++i) wrap.runCycle();
    command.updateStatusCache();
    for (int i = 0; i < 2; ++i) {
        ::CmdResult confirm = {};
        ASSERT_TRUE(command.popCmdConfirmation(confirm));
        EXPECT_EQ(41u + i, confirm.id);
        EXPECT_EQ(FBM_READ_PARAMETER, confirm.command_type);
        EXPECT_EQ(CMD_TARGET_AXIS, confirm.target_kind);
        EXPECT_EQ(i, confirm.target_index);
        ASSERT_EQ(FB_STAT_DONE, confirm.status);
        EXPECT_EQ(0, confirm.error_code);
        const int32_t words[2] = {confirm.data, confirm.data_ext};
        double value = 0;
        std::memcpy(&value, words, sizeof(value));
        EXPECT_DOUBLE_EQ(-12.3456789 - i, value);
        EXPECT_NE(0, confirm.data);
        EXPECT_NE(0, confirm.data_ext);
    }
}


// Exercise feedback and snapshots through real execution cycles. GPT-6 Codex, 2026-10-12.
class AxisParameterMirrorTest : public testing::Test {
   protected:
    AxisParameterMirrorTest() : wrap_(1), command_(wrap_.getCmdInterface()) {}
    void SetUp() override {
        AxisCiaConfig cfg;
        AxisCiaConfig_default_init(&cfg);
        cfg.is_axis_virtual = true;
        cfg.linear.increments_units_in_application_num = 3;
        cfg.linear.increments_units_in_application_den = 2;
        axis_.set_config(&cfg);
        wrap_.axisMotbuf.bindAxis(axis_.axis(), 0);
        command_.initAxisInfo(1);
    }
    void cycle() {
        wrap_.runCycle();
        command_.updateStatusCache();
    }
    void expectParameter(uint16_t type, double expected) {
        double value = -1;
        ASSERT_EQ(0, command_.getAxisParameter(0, type, value));
        EXPECT_DOUBLE_EQ(expected, value);
    }
    void preheat() {
        command_.updateStatusCache();
        cycle();
    }
    FbMoveWrap wrap_;
    AxisOpaque axis_;
    MotionCmdInterface command_;
};

TEST_F(AxisParameterMirrorTest, DefaultsPreheatAndCacheRebuild) {
    expectParameter(FBM_SET_NUM_DEN, 1);
    expectParameter(FBM_SET_SPEED, 5);
    expectParameter(FBM_SET_ACCEL, 20);
    EXPECT_TRUE(wrap_.getCmdQueue().empty());  // Getters do not submit a query.
    preheat();
    expectParameter(FBM_SET_NUM_DEN, 1.5);
    command_.resetCache();
    double value = 71;
    EXPECT_EQ(ZMC_ERR_MOTCMD_AXIS_INFO_UNAVAILABLE,
              command_.getAxisParameter(0, FBM_SET_SPEED, value));
    EXPECT_DOUBLE_EQ(71, value);
    command_.initAxisInfo(1);
    expectParameter(FBM_SET_NUM_DEN, 1);
    preheat();
    expectParameter(FBM_SET_NUM_DEN, 1.5);
    ASSERT_EQ(0, command_.requestAxisInfo(3));
    ASSERT_EQ(0, command_.advanceScript());
    cycle();
    value = 71;
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, command_.getAxisParameter(3, FBM_SET_NUM_DEN, value));
    EXPECT_DOUBLE_EQ(71, value);  // The real query's error is not a parameter value.
}

TEST_F(AxisParameterMirrorTest, MergesTextAndPacketFeedbackDuringScriptWait) {
    preheat();
    // reset() appends a reset command; initialize the wire storage before the first append.
    MotionCmdPacket packet = {};
    packet.reset();
    ASSERT_EQ(0, packet.setAxis(0));
    ASSERT_EQ(0, packet.setSpeed(12.5));
    ASSERT_EQ(0, packet.setAccel(23.5));
    ASSERT_EQ(0, command_.pushPacket(packet));
    ASSERT_EQ(0, command_.executeCommand("speed 13.5"));
    ASSERT_EQ(0, command_.waitTicks(1000, 0));
    ASSERT_EQ(0, command_.advanceScript());
    EXPECT_TRUE(command_.isScriptBusy());
    expectParameter(FBM_SET_SPEED, 5);  // Recording requests does not change the mirror.
    cycle();
    EXPECT_EQ(1u, wrap_.getCmdQueue().size());  // All axis-0 feedback shares one query.
    expectParameter(FBM_SET_SPEED, 5);          // Feedback itself carries no parameter value.
    cycle();
    expectParameter(FBM_SET_SPEED, 13.5);
    expectParameter(FBM_SET_ACCEL, 23.5);
    EXPECT_TRUE(command_.isScriptBusy());
}

TEST_F(AxisParameterMirrorTest, UnitsRefreshOnlyAfterTerminalResult) {
    preheat();
    ASSERT_EQ(0, command_.setUnitsNumDen(0, 5, 2));
    ASSERT_EQ(0, command_.advanceScript());
    cycle();
    ::CmdResult confirm = {};
    ASSERT_TRUE(command_.popCmdConfirmation(confirm));
    EXPECT_EQ(FBM_SET_NUM_DEN, confirm.command_type);
    EXPECT_EQ(FB_STAT_BUSY_ACTIVE, confirm.status);
    EXPECT_TRUE(wrap_.getCmdQueue().empty());
    expectParameter(FBM_SET_NUM_DEN, 1.5);
    cycle();
    EXPECT_EQ(1u, wrap_.getCmdQueue().size());
    expectParameter(FBM_SET_NUM_DEN, 1.5);
    cycle();
    expectParameter(FBM_SET_NUM_DEN, 2.5);
    // The first ratio executes; the second is rejected while its function block is active.
    ASSERT_EQ(0, command_.setUnitsNumDen(0, 7, 2));
    ASSERT_EQ(0, command_.setUnitsNumDen(0, 9, 2));
    ASSERT_EQ(0, command_.advanceScript());
    cycle();
    ASSERT_TRUE(command_.popCmdConfirmation(confirm));
    EXPECT_EQ(FB_STAT_BUSY_ACTIVE, confirm.status);
    ASSERT_TRUE(command_.popCmdConfirmation(confirm));
    EXPECT_EQ(FB_STAT_ERROR, confirm.status);
    EXPECT_EQ(1u, wrap_.getCmdQueue().size());  // Acceptance error also requests actual values.
    expectParameter(FBM_SET_NUM_DEN, 2.5);
    cycle();
    expectParameter(FBM_SET_NUM_DEN, 3.5);
    cycle();
    expectParameter(FBM_SET_NUM_DEN, 3.5);
}

TEST_F(AxisParameterMirrorTest, RetriesRefreshAfterQueueBackpressure) {
    preheat();
    ASSERT_EQ(0, command_.setSpeed(0, 17.25));
    ASSERT_EQ(0, command_.advanceScript());
    wrap_.runCycle();
    while (command_._heartbeat() == 0) {
    }
    ASSERT_TRUE(wrap_.getCmdQueue().full());
    command_.updateStatusCache();
    expectParameter(FBM_SET_SPEED, 5);
    // Drain ordinary heartbeat packets and retry the retained refresh on later cycles.
    for (int i = 0; i < 20; ++i) cycle();
    expectParameter(FBM_SET_SPEED, 17.25);
}
#endif