#if 0
#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include "common/zmc/fbt_errorcode_def.h"
#include "motbuf/FbMoveWrapV3.hpp"
#include "motbuf/fbmEnum.h"
#include "pou/Ctx.h"
#include "qcmd/MotionCmdInterface.hpp"
#include "impl/fbt_impl_enum_def.h"

namespace {

using zmc::CmdItemIterator;
using zmc::CmdPacket;
using zmc::CmdQueueType;
using zmc::Ctx;
using zmc::FbMoveWrap2Queue;
using zmc::IoObjectKey;
using zmc::MotionCmdInterface;
using zmc::StatusPacket;
using zmc::StatusQueueType;

struct ExpectedIoCommand {
    uint16_t type;
    IoCommandParam param;
};

// Compare queued IO commands against the fixed wire payload contract.
// Compact packets may merge many items into one CmdPacket (script R4);
// sendPacket always appends an FBM_HEARTBEAT trailer that packing checks skip.
void expectIoCommands(CmdQueueType& queue, const std::vector<ExpectedIoCommand>& expected) {
    std::vector<ExpectedIoCommand> actual;
    CmdPacket packet;
    while (queue.pop(packet)) {
        uint16_t type = 0;
        uint16_t paramLen = 0;
        const uint8_t* paramPtr = nullptr;
        CmdItemIterator it(packet.payload, packet.payload_len);
        while (it.next(type, paramLen, paramPtr)) {
            if (type == zmc::FBM_HEARTBEAT) {
                continue;
            }
            ASSERT_EQ(sizeof(IoCommandParam), paramLen);
            IoCommandParam param = {};
            std::memcpy(&param, paramPtr, sizeof(param));
            ExpectedIoCommand item = {};
            item.type = type;
            item.param = param;
            actual.push_back(item);
        }
    }
    ASSERT_EQ(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(expected[i].type, actual[i].type);
        EXPECT_EQ(expected[i].param.start_io_number, actual[i].param.start_io_number);
        EXPECT_EQ(expected[i].param.end_io_number, actual[i].param.end_io_number);
        EXPECT_EQ(expected[i].param.value, actual[i].param.value);
    }
}

// Build a compact layout containing three digital points plus one analog point per direction.
ZmcPioOffsetLayout makeIoCommandLayout() {
    ZmcPioOffsetLayout layout;
    std::memset(&layout, 0, sizeof(layout));
    layout.slot_count = 1;
    layout.descriptor_pod.tx_cap = 8;
    layout.descriptor_pod.rx_cap = 8;
    layout.descriptor_pod.slave_count = 1;
    layout.descriptor_pod.pdo_cell_count = 8;

    for (uint16_t i = 0; i < 3; ++i) {
        RtBusPdoCell& input = layout.descriptor_pod.pdo_cells[i];
        input.index = static_cast<uint16_t>(0x6100u + i);
        input.bit_offset = static_cast<uint16_t>(i * 8u);
        input.sub_index = 1;
        input.bit_size = 1;
        input.is_input = 1;
        input.slave_idx = 0;

        RtBusPdoCell& output = layout.descriptor_pod.pdo_cells[3 + i];
        output.index = static_cast<uint16_t>(0x7100u + i);
        output.bit_offset = static_cast<uint16_t>(i * 8u);
        output.sub_index = 1;
        output.bit_size = 1;
        output.is_input = 0;
        output.slave_idx = 0;
    }

    RtBusPdoCell& analogInput = layout.descriptor_pod.pdo_cells[6];
    analogInput.index = 0x6200;
    analogInput.bit_offset = 32; /* byte 4 */
    analogInput.sub_index = 1;
    analogInput.bit_size = 16;
    analogInput.is_input = 1;
    analogInput.slave_idx = 0;

    RtBusPdoCell& analogOutput = layout.descriptor_pod.pdo_cells[7];
    analogOutput.index = 0x7200;
    analogOutput.bit_offset = 32; /* byte 4 */
    analogOutput.sub_index = 1;
    analogOutput.bit_size = 16;
    analogOutput.is_input = 0;
    analogOutput.slave_idx = 0;
    return layout;
}

class IoCommandRuntimeTest : public testing::Test {
   protected:
    IoCommandRuntimeTest()
        : wrap_(0),
          cmdIf_(wrap_.getCmdQueue(), wrap_.getStatusQueue(), wrap_.exitFlag) {}

    void SetUp() override {
        std::memset(input_, 0, sizeof(input_));
        std::memset(output_, 0, sizeof(output_));
        const ZmcPioOffsetLayout layout = makeIoCommandLayout();
        ASSERT_EQ(0, ctx_.io().apply_pio_layout(&layout));
        for (uint32_t i = 0; i < 3; ++i) {
            ASSERT_EQ(0, ctx_.io().register_di_alias(
                             10u + i, IoObjectKey{0, static_cast<uint16_t>(0x6100u + i), 1}));
            ASSERT_EQ(0, ctx_.io().register_do_alias(
                             10u + i, IoObjectKey{0, static_cast<uint16_t>(0x7100u + i), 1}));
        }
        ASSERT_EQ(0, ctx_.io().register_ai_alias(20u, IoObjectKey{0, 0x6200, 1}));
        ASSERT_EQ(0, ctx_.io().register_ao_alias(20u, IoObjectKey{0, 0x7200, 1}));
        ctx_.io().bind_process_image(input_, output_);
        wrap_.setCtx(&ctx_);
    }

    Ctx ctx_;
    FbMoveWrap2Queue wrap_;
    MotionCmdInterface cmdIf_;
    uint8_t input_[8];
    uint8_t output_[8];
};

TEST(MotionCmdIoPacking, TypedApisUseFixedCommandPayloads) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    ASSERT_EQ(0, cmdIf.requestReadDi(10u));
    ASSERT_EQ(0, cmdIf.requestReadDiMulti(10u, 12u));
    ASSERT_EQ(0, cmdIf.requestReadDo(10u));
    ASSERT_EQ(0, cmdIf.requestReadDoMulti(10u, 12u));
    ASSERT_EQ(0, cmdIf.writeDo(10u, true));
    ASSERT_EQ(0, cmdIf.writeDoMulti(10u, 12u, 5u));
    ASSERT_EQ(0, cmdIf.requestReadAi(20u));
    ASSERT_EQ(0, cmdIf.requestReadAo(20u));
    ASSERT_EQ(0, cmdIf.writeAo(20u, 0x1234u));
    ASSERT_EQ(0, cmdIf.writeVirtualDi(30u, true));
    ASSERT_EQ(0, cmdIf.writeVirtualAi(40u, 0x12345678u));
    ASSERT_EQ(0, cmdIf.advanceScript());

    const std::vector<ExpectedIoCommand> expected = {
        {zmc::FBM_IO_READ_DI, {10u, 10u, 0u}},
        {zmc::FBM_IO_READ_DI_MULTI, {10u, 12u, 0u}},
        {zmc::FBM_IO_READ_DO, {10u, 10u, 0u}},
        {zmc::FBM_IO_READ_DO_MULTI, {10u, 12u, 0u}},
        {zmc::FBM_IO_WRITE_DO, {10u, 10u, 1u}},
        {zmc::FBM_IO_WRITE_DO_MULTI, {10u, 12u, 5u}},
        {zmc::FBM_IO_READ_AI, {20u, 20u, 0u}},
        {zmc::FBM_IO_READ_AO, {20u, 20u, 0u}},
        {zmc::FBM_IO_WRITE_AO, {20u, 20u, 0x1234u}},
        {zmc::FBM_IO_WRITE_VIRTUAL_DI, {30u, 30u, 1u}},
        {zmc::FBM_IO_WRITE_VIRTUAL_AI, {40u, 40u, 0x12345678u}},
    };
    expectIoCommands(cmdQueue, expected);
}

TEST(MotionCmdIoPacking, PreservesIoCommandWireValues) {
    EXPECT_EQ(0x0070, static_cast<int>(zmc::FBM_IO_READ_DI));
    EXPECT_EQ(0x0078, static_cast<int>(zmc::FBM_IO_READ_AO));
    EXPECT_EQ(0x0079, static_cast<int>(zmc::FBM_IO_WRITE_VIRTUAL_DI));
    EXPECT_EQ(0x007A, static_cast<int>(zmc::FBM_IO_WRITE_VIRTUAL_AI));
}

TEST(MotionCmdIoPacking, TypedApisRejectIoNumbersOverLogicalCapacity) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    ASSERT_EQ(0, cmdIf.requestReadDi(1023u));
    ASSERT_EQ(0, cmdIf.writeVirtualDi(1023u, true));
    ASSERT_EQ(0, cmdIf.writeVirtualAi(1023u, 0xFFFFFFFFu));
    ASSERT_EQ(0, cmdIf.advanceScript());
    const std::vector<ExpectedIoCommand> boundary = {
        {zmc::FBM_IO_READ_DI, {1023u, 1023u, 0u}},
        {zmc::FBM_IO_WRITE_VIRTUAL_DI, {1023u, 1023u, 1u}},
        {zmc::FBM_IO_WRITE_VIRTUAL_AI, {1023u, 1023u, 0xFFFFFFFFu}},
    };
    expectIoCommands(cmdQueue, boundary);

    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.requestReadDi(1024u));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.requestReadDiMulti(0u, 1024u));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.requestReadDo(1024u));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.requestReadDoMulti(1024u, 1024u));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.writeDo(1024u, true));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.writeDoMulti(0u, 1024u, 0u));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.requestReadAi(1024u));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.requestReadAo(1024u));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.writeAo(1024u, 0u));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.writeVirtualDi(1024u, true));
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, cmdIf.writeVirtualAi(1024u, 0u));
}

TEST(MotionCmdIoPacking, TextCommandsAcceptFixedDecimalAndHexArguments) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    ASSERT_EQ(0, cmdIf.executeCommand("read_di 0x3ff"));
    ASSERT_EQ(0, cmdIf.executeCommand("read_di_multi 32 35"));
    ASSERT_EQ(0, cmdIf.executeCommand("read_do 33"));
    ASSERT_EQ(0, cmdIf.executeCommand("read_do_multi 32 35"));
    ASSERT_EQ(0, cmdIf.executeCommand("write_do 34 1"));
    ASSERT_EQ(0, cmdIf.executeCommand("write_do_multi 32 35 0xa"));
    ASSERT_EQ(0, cmdIf.executeCommand("read_ai 40"));
    ASSERT_EQ(0, cmdIf.executeCommand("write_ao 40 0x1234"));
    ASSERT_EQ(0, cmdIf.advanceScript());

    const std::vector<ExpectedIoCommand> expected = {
        {zmc::FBM_IO_READ_DI, {1023u, 1023u, 0u}},
        {zmc::FBM_IO_READ_DI_MULTI, {32u, 35u, 0u}},
        {zmc::FBM_IO_READ_DO, {33u, 33u, 0u}},
        {zmc::FBM_IO_READ_DO_MULTI, {32u, 35u, 0u}},
        {zmc::FBM_IO_WRITE_DO, {34u, 34u, 1u}},
        {zmc::FBM_IO_WRITE_DO_MULTI, {32u, 35u, 10u}},
        {zmc::FBM_IO_READ_AI, {40u, 40u, 0u}},
        {zmc::FBM_IO_WRITE_AO, {40u, 40u, 0x1234u}},
    };
    expectIoCommands(cmdQueue, expected);

    EXPECT_EQ(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG, cmdIf.executeCommand("read_di"));
    EXPECT_EQ(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED, cmdIf.executeCommand("read_di 1024"));
    EXPECT_EQ(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED, cmdIf.executeCommand("read_di 1 2"));
    EXPECT_EQ(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED, cmdIf.executeCommand("write_do 1 2"));
    EXPECT_EQ(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED,
              cmdIf.executeCommand("read_ai 1 extra"));
    EXPECT_EQ(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED,
              cmdIf.executeCommand("write_ao 1 0x100000000"));
    EXPECT_EQ(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED,
              cmdIf.executeCommand("write_ao 1024 1"));
}

// Direct reads bypass a queued script gate while preserving the existing payload layout.
TEST(MotionCmdIoPacking, DirectReadsBypassScriptAndReportBackpressure) {
    CmdQueueType queue;
    StatusQueueType status;
    bool exit = false;
    MotionCmdInterface cmd(queue, status, exit);
    ASSERT_EQ(0, cmd.waitTicks(1000u));
    ASSERT_EQ(0, cmd.writeDo(10u, true));
    ASSERT_EQ(0, cmd.requestReadDi(10u, 101u));
    ASSERT_EQ(0, cmd.requestReadDo(11u, 102u));
    ASSERT_EQ(0, cmd.requestReadAi(20u, 103u));
    ASSERT_EQ(0, cmd.requestReadAo(21u, 104u));
    expectIoCommands(queue, {{zmc::FBM_IO_READ_DI, {10u, 10u, 101u}},
                             {zmc::FBM_IO_READ_DO, {11u, 11u, 102u}},
                             {zmc::FBM_IO_READ_AI, {20u, 20u, 103u}},
                             {zmc::FBM_IO_READ_AO, {21u, 21u, 104u}}});
    EXPECT_TRUE(cmd.isScriptBusy());
    while (cmd.requestReadDi(10u, 105u) == 0) {}
    EXPECT_EQ(ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED, cmd.requestReadDo(11u, 106u));
    zmc::CmdPacket packet;
    ASSERT_TRUE(queue.pop(packet));
    EXPECT_EQ(0, cmd.requestReadDo(11u, 106u));
}

// Each confirmation carries the value of its own read, including the complete analog bits.
TEST_F(IoCommandRuntimeTest, DirectReadsConfirmDigitalAndFullWidthAnalogValues) {
    ASSERT_EQ(0, ctx_.io().register_virtual_ai(30u));
    ASSERT_EQ(0, ctx_.io().register_virtual_ao(31u));
    const uint32_t values[] = {0x1234u, 0x80000000u, 0xffffffffu};
    for (size_t n = 0; n < 3; ++n) {
        input_[0] = static_cast<uint8_t>(n % 2);
        ctx_.io().bind_process_image(input_, output_);
        ASSERT_EQ(0, ctx_.io().write_virtual_ai(30u, values[n]));
        ASSERT_EQ(0, cmdIf_.writeDo(10u, n % 2 != 0));
        ASSERT_EQ(0, cmdIf_.writeAo(31u, values[n]));
        ASSERT_EQ(0, cmdIf_.advanceScript());
        wrap_.runCycle();
        std::vector<CmdResult> confirms;
        ASSERT_EQ(0, cmdIf_.getCmdConfirmations(confirms));

        const uint32_t firstId = 100u + static_cast<uint32_t>(n) * 4u;
        ASSERT_EQ(0, cmdIf_.requestReadDi(10u, firstId));
        ASSERT_EQ(0, cmdIf_.requestReadDo(10u, firstId + 1u));
        ASSERT_EQ(0, cmdIf_.requestReadAi(30u, firstId + 2u));
        ASSERT_EQ(0, cmdIf_.requestReadAo(31u, firstId + 3u));
        for (int i = 0; i < 4; ++i) wrap_.runCycle();
        ASSERT_EQ(0, cmdIf_.getCmdConfirmations(confirms));
        ASSERT_EQ(4u, confirms.size());
        const uint32_t expected[] = {static_cast<uint32_t>(n % 2),
                                     static_cast<uint32_t>(n % 2), values[n], values[n]};
        for (size_t i = 0; i < confirms.size(); ++i) {
            EXPECT_EQ(firstId + i, confirms[i].id);
            EXPECT_EQ(zmc::FB_STAT_DONE, confirms[i].status);
            EXPECT_EQ(0, confirms[i].error_code);
            uint32_t bits = 0;
            std::memcpy(&bits, &confirms[i].data, sizeof(bits));
            EXPECT_EQ(expected[i], bits);
        }
    }
    ASSERT_EQ(0, cmdIf_.requestReadAi(500u, 200u));
    wrap_.runCycle();
    std::vector<CmdResult> confirms;
    ASSERT_EQ(0, cmdIf_.getCmdConfirmations(confirms));
    ASSERT_EQ(1u, confirms.size());
    EXPECT_EQ(200u, confirms[0].id);
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, confirms[0].error_code);
}

TEST_F(IoCommandRuntimeTest, ExecutesAllAliasCommandsAndReturnsOrderedResults) {
    input_[0] = 1u;
    input_[1] = 0u;
    input_[2] = 1u;
    input_[4] = 0x34u;
    input_[5] = 0x12u;
    output_[0] = 0u;
    output_[1] = 1u;
    output_[2] = 1u;
    output_[4] = 0x78u;
    output_[5] = 0x56u;
    ctx_.io().bind_process_image(input_, output_);

    ASSERT_EQ(0, cmdIf_.requestReadDi(10u));
    ASSERT_EQ(0, cmdIf_.requestReadDiMulti(10u, 12u));
    ASSERT_EQ(0, cmdIf_.requestReadDo(11u));
    ASSERT_EQ(0, cmdIf_.requestReadDoMulti(10u, 12u));
    ASSERT_EQ(0, cmdIf_.writeDo(10u, false));
    ASSERT_EQ(0, cmdIf_.writeDoMulti(10u, 12u, 5u));
    ASSERT_EQ(0, cmdIf_.requestReadAi(20u));
    ASSERT_EQ(0, cmdIf_.requestReadAo(20u));
    ASSERT_EQ(0, cmdIf_.writeAo(20u, 0xA55Au));
    ASSERT_EQ(0, cmdIf_.advanceScript());

    wrap_.runCycle();
    cmdIf_.updateStatusCache();
    std::vector<CmdResult> results;
    ASSERT_EQ(0, cmdIf_.getIoResults(results));
    ASSERT_EQ(9u, results.size());

    const uint32_t expectedValues[] = {
        1u, 5u, 0u, 0u, 0u, 5u, 0x1234u, 0u, 0xA55Au,
    };
    const uint16_t expectedTypes[] = {
        zmc::FBM_IO_READ_DI,       zmc::FBM_IO_READ_DI_MULTI, zmc::FBM_IO_READ_DO,
        zmc::FBM_IO_READ_DO_MULTI, zmc::FBM_IO_WRITE_DO,      zmc::FBM_IO_WRITE_DO_MULTI,
        zmc::FBM_IO_READ_AI,       zmc::FBM_IO_READ_AO,       zmc::FBM_IO_WRITE_AO,
    };
    const uint16_t expectedStarts[] = {10u, 10u, 11u, 10u, 10u, 10u, 20u, 20u, 20u};
    const uint16_t expectedEnds[] = {10u, 12u, 11u, 12u, 10u, 12u, 20u, 20u, 20u};
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_EQ(0, results[i].error_code);
        EXPECT_EQ(expectedValues[i], cmd_result_data_u32(&results[i]));
        EXPECT_EQ(1u, results[i].tick);
        EXPECT_EQ(expectedTypes[i], results[i].command_type);
        EXPECT_EQ(expectedStarts[i], cmd_result_io_start(&results[i]));
        EXPECT_EQ(expectedEnds[i], cmd_result_io_end(&results[i]));
    }
    // This fixture runs the command wrapper without the bridge cycle that flushes PDO output.
    ctx_.io().restore_retained_outputs();
    EXPECT_EQ(1u, output_[0]);
    EXPECT_EQ(0u, output_[1]);
    EXPECT_EQ(1u, output_[2]);
    EXPECT_EQ(0x5Au, output_[4]);
    EXPECT_EQ(0xA5u, output_[5]);
    ASSERT_EQ(0, cmdIf_.getIoResults(results));
    ASSERT_EQ(9u, results.size());
    EXPECT_EQ(zmc::FBM_IO_READ_DI, results.front().command_type);
    EXPECT_EQ(zmc::FBM_IO_WRITE_AO, results.back().command_type);
}

TEST_F(IoCommandRuntimeTest, ExecutesVirtualInputInjectionCommands) {
    ASSERT_EQ(0, ctx_.io().register_virtual_di(30u));
    ASSERT_EQ(0, ctx_.io().register_virtual_ai(40u));

    ASSERT_EQ(0, cmdIf_.writeVirtualDi(30u, true));
    ASSERT_EQ(0, cmdIf_.writeVirtualAi(40u, 0x12345678u));
    ASSERT_EQ(0, cmdIf_.advanceScript());
    wrap_.runCycle();
    cmdIf_.updateStatusCache();

    bool digitalValue = false;
    uint32_t analogValue = 0u;
    ASSERT_EQ(0, ctx_.io().read_di(30u, &digitalValue));
    ASSERT_EQ(0, ctx_.io().read_ai(40u, &analogValue));
    EXPECT_TRUE(digitalValue);
    EXPECT_EQ(0x12345678u, analogValue);

    std::vector<CmdResult> results;
    ASSERT_EQ(0, cmdIf_.getIoResults(results));
    ASSERT_EQ(2u, results.size());
    EXPECT_EQ(0u, results[0].error_code);
    EXPECT_EQ(1u, cmd_result_data_u32(&results[0]));
    EXPECT_EQ(1u, results[0].tick);
    EXPECT_EQ(zmc::FBM_IO_WRITE_VIRTUAL_DI, results[0].command_type);
    EXPECT_EQ(0u, results[1].error_code);
    EXPECT_EQ(0x12345678u, cmd_result_data_u32(&results[1]));
    EXPECT_EQ(1u, results[1].tick);
    EXPECT_EQ(zmc::FBM_IO_WRITE_VIRTUAL_AI, results[1].command_type);
}

// One wire record per operation covers all four IO kinds and all four groups.
TEST_F(IoCommandRuntimeTest, UnifiedResultsCoverEveryIoGroupAndSurviveConfirmationConsumption) {
    const uint16_t types[] = {zmc::FBM_IO_READ_DI, zmc::FBM_IO_READ_DO,
                              zmc::FBM_IO_READ_AI, zmc::FBM_IO_READ_AO};
    const uint32_t values[] = {1u, 1u, 0xffffffffu, 0x80000000u};
    for (uint32_t group = 0; group < 4; ++group) {
        const uint32_t io = group * 256 + 200;
        ASSERT_EQ(0, ctx_.io().register_virtual_di(io));
        ASSERT_EQ(0, ctx_.io().register_virtual_do(io));
        ASSERT_EQ(0, ctx_.io().register_virtual_ai(io));
        ASSERT_EQ(0, ctx_.io().register_virtual_ao(io));
        ASSERT_EQ(0, ctx_.io().write_virtual_di(io, true));
        ASSERT_EQ(0, ctx_.io().write_do(io, true));
        ASSERT_EQ(0, ctx_.io().write_virtual_ai(io, values[2]));
        ASSERT_EQ(0, ctx_.io().write_ao(io, values[3]));
        ASSERT_EQ(0, cmdIf_.requestReadDi(io));
        ASSERT_EQ(0, cmdIf_.requestReadDo(io));
        ASSERT_EQ(0, cmdIf_.requestReadAi(io));
        ASSERT_EQ(0, cmdIf_.requestReadAo(io));
    }
    ASSERT_EQ(0, cmdIf_.advanceScript());
    wrap_.runCycle();
    unsigned int records = 0;
    StatusPacket packet;
    while (wrap_.getStatusQueue().pop(packet)) {
        zmc::StatusItemIterator it(packet.payload, packet.payload_len);
        uint16_t type, length;
        const uint8_t* payload;
        while (it.next(type, length, payload)) {
            EXPECT_NE(zmc::STAT_IO_RESULT, type);
            if (type != zmc::STAT_CMD_CONFIRM) continue;
            ASSERT_EQ(sizeof(CmdResult), length);
            CmdResult result;
            std::memcpy(&result, payload, sizeof(result));
            const unsigned int kind = records % 4;
            const unsigned int group = records / 4;
            ASSERT_LT(group, 4u);
            EXPECT_EQ(types[kind], result.command_type);
            EXPECT_EQ(CMD_TARGET_DI + kind * 4 + group, result.target_kind);
            EXPECT_EQ(200u, result.target_index);
            EXPECT_EQ(group * 256 + 200, cmd_result_io_start(&result));
            EXPECT_EQ(1u, result.io_count);
            EXPECT_EQ(zmc::FB_STAT_DONE, result.status);
            EXPECT_EQ(values[kind], cmd_result_data_u32(&result));
            ++records;
        }
        cmdIf_.applyStatusPacket(packet);
    }
    ASSERT_EQ(16u, records);
    std::vector<CmdResult> confirms, results;
    ASSERT_EQ(0, cmdIf_.getCmdConfirmations(confirms));
    ASSERT_EQ(16u, confirms.size());
    ASSERT_EQ(0, cmdIf_.getIoResults(results));
    ASSERT_EQ(16u, results.size());
    ASSERT_EQ(0, cmdIf_.getCmdConfirmations(confirms));
    EXPECT_TRUE(confirms.empty());
    ASSERT_EQ(0, cmdIf_.getIoResults(results));
    EXPECT_EQ(16u, results.size());
}

// A count preserves a range crossing the boundary between two 256-point groups.
TEST_F(IoCommandRuntimeTest, UnifiedRangeResultsPreserveCrossGroupAndThirtyTwoPoints) {
    for (uint32_t io = 250; io <= 281; ++io) {
        ASSERT_EQ(0, ctx_.io().register_virtual_do(io));
    }
    ASSERT_EQ(0, cmdIf_.writeDoMulti(250, 281, 0xffffffffu));
    ASSERT_EQ(0, cmdIf_.requestReadDoMulti(250, 270));
    ASSERT_EQ(0, cmdIf_.advanceScript());
    wrap_.runCycle();
    std::vector<CmdResult> confirms, results;
    ASSERT_EQ(0, cmdIf_.getCmdConfirmations(confirms));
    ASSERT_EQ(2u, confirms.size());
    ASSERT_EQ(0, cmdIf_.getIoResults(results));
    ASSERT_EQ(2u, results.size());
    EXPECT_EQ(32u, results[0].io_count);
    EXPECT_EQ(281u, cmd_result_io_end(&results[0]));
    EXPECT_EQ(0xffffffffu, cmd_result_data_u32(&results[0]));
    EXPECT_EQ(21u, results[1].io_count);
    EXPECT_EQ(CMD_TARGET_DO, results[1].target_kind);
    EXPECT_EQ(250u, cmd_result_io_start(&results[1]));
    EXPECT_EQ(270u, cmd_result_io_end(&results[1]));
    EXPECT_EQ(0x1fffffu, cmd_result_data_u32(&results[1]));
}

TEST_F(IoCommandRuntimeTest, PreservesIoManagerErrorsAndRetriesAfterBackpressure) {
    ASSERT_EQ(0, cmdIf_.requestReadDi(500u));
    ASSERT_EQ(0, cmdIf_.requestReadDiMulti(12u, 10u));
    ASSERT_EQ(0, cmdIf_.advanceScript());
    wrap_.runCycle();
    cmdIf_.updateStatusCache();

    std::vector<CmdResult> results;
    ASSERT_EQ(0, cmdIf_.getIoResults(results));
    ASSERT_EQ(2u, results.size());
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, results[0].error_code);
    EXPECT_EQ(0u, cmd_result_data_u32(&results[0]));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, results[1].error_code);
    EXPECT_EQ(0u, cmd_result_data_u32(&results[1]));
    EXPECT_EQ(1u, results[0].io_count);
    EXPECT_EQ(0u, results[1].io_count);
    EXPECT_EQ(zmc::FB_STAT_ERROR, results[1].status);

    StatusPacket filler;
    filler.reset();
    while (wrap_.getStatusQueue().push(filler)) {
        // Fill the bounded status queue to force the runtime retry path.
    }
    ASSERT_EQ(0, cmdIf_.requestReadDi(10u));
    ASSERT_EQ(0, cmdIf_.advanceScript());
    wrap_.runCycle();
    cmdIf_.updateStatusCache();
    ASSERT_EQ(0, cmdIf_.getIoResults(results));
    ASSERT_EQ(2u, results.size());
    EXPECT_EQ(500u, cmd_result_io_start(&results[0]));
    EXPECT_EQ(12u, cmd_result_io_start(&results[1]));

    wrap_.runCycle();
    cmdIf_.updateStatusCache();
    ASSERT_EQ(0, cmdIf_.getIoResults(results));
    ASSERT_EQ(3u, results.size());
    EXPECT_EQ(10u, cmd_result_io_start(&results[2]));
    EXPECT_EQ(zmc::FBM_IO_READ_DI, results[2].command_type);
}

TEST(MotionStatusMirrorIo, ReplacesMatchingKeyAndMovesLatestResultToBack) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    CmdResult first = {};
    first.target_kind = CMD_TARGET_DI;
    first.target_index = 10u;
    first.io_count = 3u;
    first.data = 1u;
    first.tick = 1u;
    first.error_code = ZMC_ERR_IO_OBJECT_NOT_FOUND;
    first.status = zmc::FB_STAT_ERROR;
    first.command_type = zmc::FBM_IO_READ_DI_MULTI;

    CmdResult otherType = first;
    otherType.data = 2u;
    otherType.tick = 2u;
    otherType.error_code = 0u;
    otherType.status = zmc::FB_STAT_DONE;
    otherType.command_type = zmc::FBM_IO_READ_DO_MULTI;
    otherType.target_kind = CMD_TARGET_DO;

    CmdResult replacement = first;
    replacement.data = 3u;
    replacement.tick = 3u;
    replacement.error_code = ZMC_ERR_IO_INVALID_ARG;

    const CmdResult input[] = {first, otherType, replacement};
    for (size_t i = 0; i < sizeof(input) / sizeof(input[0]); ++i) {
        StatusPacket packet;
        packet.reset();
        ASSERT_TRUE(packet.appendItem(zmc::STAT_CMD_CONFIRM, &input[i], sizeof(input[i])));
        ASSERT_TRUE(statusQueue.push(packet));
    }

    cmdIf.updateStatusCache();
    std::vector<CmdResult> results;
    ASSERT_EQ(0, cmdIf.getIoResults(results));
    ASSERT_EQ(2u, results.size());
    EXPECT_EQ(zmc::FBM_IO_READ_DO_MULTI, results[0].command_type);
    EXPECT_EQ(2u, cmd_result_data_u32(&results[0]));
    EXPECT_EQ(2u, results[0].tick);
    EXPECT_EQ(0u, results[0].error_code);
    EXPECT_EQ(zmc::FBM_IO_READ_DI_MULTI, results[1].command_type);
    EXPECT_EQ(3u, cmd_result_data_u32(&results[1]));
    EXPECT_EQ(3u, results[1].tick);
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, results[1].error_code);
}

TEST(MotionStatusMirrorIo, KeepsNewestHundredUniqueResultsWithoutConsuming) {
    CmdQueueType cmdQueue;
    StatusQueueType statusQueue;
    bool exitFlag = false;
    MotionCmdInterface cmdIf(cmdQueue, statusQueue, exitFlag);

    for (uint32_t i = 0; i < 101u; ++i) {
        CmdResult result = {};
        result.target_kind = CMD_TARGET_DI;
        result.target_index = static_cast<uint8_t>(i);
        result.io_count = 1;
        result.status = zmc::FB_STAT_DONE;
        result.tick = i;
        result.command_type = zmc::FBM_IO_READ_DI;
        StatusPacket packet;
        packet.reset();
        ASSERT_TRUE(packet.appendItem(zmc::STAT_CMD_CONFIRM, &result, sizeof(result)));
        ASSERT_TRUE(statusQueue.push(packet));
    }

    cmdIf.updateStatusCache();
    std::vector<CmdResult> results;
    ASSERT_EQ(0, cmdIf.getIoResults(results));
    ASSERT_EQ(100u, results.size());
    EXPECT_EQ(1u, cmd_result_io_start(&results.front()));
    EXPECT_EQ(1u, results.front().tick);
    EXPECT_EQ(100u, cmd_result_io_start(&results.back()));
    EXPECT_EQ(100u, results.back().tick);
    ASSERT_EQ(0, cmdIf.getIoResults(results));
    ASSERT_EQ(100u, results.size());
    EXPECT_EQ(1u, cmd_result_io_start(&results.front()));
    EXPECT_EQ(100u, cmd_result_io_start(&results.back()));
}

}  // namespace
#endif