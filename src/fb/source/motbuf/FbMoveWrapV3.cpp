#include "motbuf/FbMoveWrapV3.hpp"

#include <algorithm>
#include <cstring>

#include "common/container/FbCmdQueue.hpp"
#include "common/utils/double2frac.hpp"
#include "fbt_errorcode_def.h"
#include "impl/fbt_impl_enum_def.h"
#include "mc_parameter_enum_def.h"
#include "motbuf/MotionCmdTypes.hpp"

namespace zmc {

namespace {
constexpr uint32_t kAutomaticIdFlag = 0x80000000u;
constexpr uint32_t kEpochMask = 0xffu;
constexpr uint32_t kPacketSeqMask = 0xffffu;
constexpr uint32_t kItemOrdinalMask = 0x7fu;

uint32_t makeAutomaticCommandId(uint8_t epoch, uint16_t seq, uint8_t ordinal) {
    return kAutomaticIdFlag | ((static_cast<uint32_t>(epoch) & kEpochMask) << 23) |
           ((static_cast<uint32_t>(seq) & kPacketSeqMask) << 7) |
           (static_cast<uint32_t>(ordinal) & kItemOrdinalMask);
}
}  // namespace

FbMoveWrap::FbMoveWrap(int axNum)
    : FbMoveWrap2Queue(axNum), cmdIf_(getCmdQueue(), getStatusQueue(), exitFlag) {}

void FbMoveWrapWithAxis::pushCmdResult(const CommandMidDefault& cmdMid, uint8_t fbStatus,
                                       void* pFb) {
    CmdResult cmdInfo = {};
    cmdInfo.id = cmdMid.id;
    cmdInfo.command_type = cmdMid.cmdType;
    cmdInfo.target_kind = CMD_TARGET_AXIS;
    cmdInfo.target_index = static_cast<uint8_t>(cmdMid.axisId);
    cmdInfo.tick = cnt;
    cmdInfo.status = fbStatus;
    cmdInfo.error_code = getFbImplErrorCode(pFb);
    // Keep the original command-binding cycle in the shared payload.
    std::memcpy(&cmdInfo.data, &cmdMid.tickStarted, sizeof(cmdInfo.data));

    cmdResultArr.push(cmdInfo);  // RingBuffer auto-overwrites oldest when full
}

bool FbMoveWrapWithAxis::pushCmdConfirm(const CmdResult& confirm) {
    if (cmdConfirmArr.full()) return false;
    return cmdConfirmArr.push(confirm);
}

void FbMoveWrapWithAxis::printCmdResultInfo() const {
    printf("cmdResultArr.size() = %zu\n", cmdResultArr.size());
}

FbMoveWrapWithAxis::FbMoveWrapWithAxis(int axNum) : axisMotbuf(this, axNum), ctx_(nullptr) {}

FbMoveWrapWithAxis::~FbMoveWrapWithAxis() {}

void FbMoveWrapWithAxis::runCycle() { _runCycle(); }

void FbMoveWrapWithAxis::_runCycle() {
    cnt++;
    axisMotbuf.runCycle();
}

void FbMoveWrapWithAxis::reset_runtime() {
    axisMotbuf.reset_runtime();
    cmdResultArr.clear();
    cmdConfirmArr.clear();
    cnt = 0;
    exitFlag = false;
    axSelect = 0;
    axSelectFlag = 0;
    axSelectBak = 0;
}

void FbMoveWrapWithZmot::runCycle() { _runCycle(); }

void FbMoveWrapWithZmot::_runCycle() {
    cnt++;
    _runCycleTop();
    _runCycleBottom();
}

void FbMoveWrapWithZmot::_runCycleTop() {
    axisMotbuf.runCycleTop();
    zmotMotbuf.runCycleTop();
}
void FbMoveWrapWithZmot::_runCycleBottom() {
    axisMotbuf.runCycleBottom();
    zmotMotbuf.runCycleBottom();
}

void FbMoveWrapWithZmot::reset_runtime() {
    FbMoveWrap2::reset_runtime();
    zmotMotbuf.reset_runtime();
}

void FbMoveWrapWithRobot::runCycle() { _runCycle(); }

void FbMoveWrapWithRobot::_runCycle() {
    cnt++;
    _runCycleTop();
    _runCycleBottom();
}

void FbMoveWrapWithRobot::_runCycleTop() {
    FbMoveWrapWithZmot::_runCycleTop();
    robotMotbuf.runCycleTop();
}
void FbMoveWrapWithRobot::_runCycleBottom() {
    FbMoveWrapWithZmot::_runCycleBottom();
    robotMotbuf.runCycleBottom();
}

void FbMoveWrapWithRobot::reset_runtime() {
    FbMoveWrapWithZmot::reset_runtime();
    robotMotbuf.reset_runtime();
}

void FbMoveWrap2Queue::processIoCommand(uint16_t type, const uint8_t* paramPtr,
                                      uint16_t paramLen, CmdResult& result) {
    result.command_type = type;
    result.tick = cnt;
    result.target_kind = CMD_TARGET_INVALID;
    result.status = FB_STAT_ERROR;
    if (paramLen < sizeof(IoCommandParam)) {
        result.error_code = ZMC_ERR_MOTCMD_INVALID_ARG;
        return;
    }

    IoCommandParam param;
    std::memcpy(&param, paramPtr, sizeof(param));
    const bool write = type == FBM_IO_WRITE_DO || type == FBM_IO_WRITE_DO_MULTI ||
                       type == FBM_IO_WRITE_AO || type == FBM_IO_WRITE_VIRTUAL_DI ||
                       type == FBM_IO_WRITE_VIRTUAL_AI;
    uint32_t resultValue = write ? param.value : 0u;
    // The command selects the IO kind; the starting number selects one of its four groups.
    const uint8_t kind = type == FBM_IO_READ_DO || type == FBM_IO_READ_DO_MULTI ||
                                 type == FBM_IO_WRITE_DO || type == FBM_IO_WRITE_DO_MULTI
                             ? CMD_TARGET_DO
                         : type == FBM_IO_READ_AI || type == FBM_IO_WRITE_VIRTUAL_AI
                             ? CMD_TARGET_AI
                         : type == FBM_IO_READ_AO || type == FBM_IO_WRITE_AO
                             ? CMD_TARGET_AO : CMD_TARGET_DI;
    if (param.start_io_number < 1024u) {
        result.target_kind = kind + (param.start_io_number >> 8);
        result.target_index = static_cast<uint8_t>(param.start_io_number);
        if (param.start_io_number <= param.end_io_number && param.end_io_number < 1024u &&
            param.end_io_number - param.start_io_number < 32u) {
            result.io_count = static_cast<uint8_t>(param.end_io_number - param.start_io_number + 1);
        }
    }

    Ctx* runtimeCtx = ctx();
    int errorCode = ZMC_ERR_IO_INVALID_ARG;
    if (runtimeCtx) {
        IoManager& io = runtimeCtx->io();
        switch (type) {
            case FBM_IO_READ_DI: {
                bool value = false;
                errorCode = io.read_di(static_cast<uint32_t>(param.start_io_number), &value);
                resultValue = errorCode == ZMC_NO_ERROR && value ? 1u : 0u;
                break;
            }
            case FBM_IO_READ_DI_MULTI: {
                uint32_t value = 0;
                errorCode = io.read_di_multi(static_cast<uint32_t>(param.start_io_number),
                                             static_cast<uint32_t>(param.end_io_number), &value);
                resultValue = errorCode == ZMC_NO_ERROR ? value : 0u;
                break;
            }
            case FBM_IO_READ_DO: {
                bool value = false;
                errorCode = io.read_do(static_cast<uint32_t>(param.start_io_number), &value);
                resultValue = errorCode == ZMC_NO_ERROR && value ? 1u : 0u;
                break;
            }
            case FBM_IO_READ_DO_MULTI: {
                uint32_t value = 0;
                errorCode = io.read_do_multi(static_cast<uint32_t>(param.start_io_number),
                                             static_cast<uint32_t>(param.end_io_number), &value);
                resultValue = errorCode == ZMC_NO_ERROR ? value : 0u;
                break;
            }
            case FBM_IO_WRITE_DO:
                errorCode =
                    io.write_do(static_cast<uint32_t>(param.start_io_number), param.value != 0u);
                break;
            case FBM_IO_WRITE_DO_MULTI:
                errorCode = io.write_do_multi(static_cast<uint32_t>(param.start_io_number),
                                              static_cast<uint32_t>(param.end_io_number), param.value);
                break;
            case FBM_IO_READ_AI: {
                uint32_t value = 0;
                errorCode = io.read_ai(static_cast<uint32_t>(param.start_io_number), &value);
                resultValue = errorCode == ZMC_NO_ERROR ? value : 0u;
                break;
            }
            case FBM_IO_READ_AO: {
                uint32_t value = 0;
                errorCode = io.read_ao(static_cast<uint32_t>(param.start_io_number), &value);
                resultValue = errorCode == ZMC_NO_ERROR ? value : 0u;
                break;
            }
            case FBM_IO_WRITE_AO:
                errorCode = io.write_ao(static_cast<uint32_t>(param.start_io_number), param.value);
                break;
            case FBM_IO_WRITE_VIRTUAL_DI:
                errorCode = io.write_virtual_di(static_cast<uint32_t>(param.start_io_number),
                                                param.value != 0u);
                break;
            case FBM_IO_WRITE_VIRTUAL_AI:
                errorCode =
                    io.write_virtual_ai(static_cast<uint32_t>(param.start_io_number), param.value);
                break;
            default:
                errorCode = ZMC_ERR_MOTCMD_UNKNOWN_COMMAND;
                break;
        }
    }
    result.error_code = static_cast<uint16_t>(errorCode);
    result.status = errorCode ? FB_STAT_ERROR : FB_STAT_DONE;
    // Reads retain zero on error; writes report their submitted value, not a readback.
    std::memcpy(&result.data, &resultValue, sizeof(result.data));
}

void FbMoveWrap2Queue::confirm(uint32_t id, uint16_t type, CmdTargetKind targetKind,
                               uint16_t targetIndex, uint8_t status, uint16_t errorCode) {
    CmdResult result = {};
    result.id = id;
    result.command_type = type;
    result.target_kind = static_cast<uint8_t>(targetKind);
    result.target_index = static_cast<uint8_t>(targetIndex);
    result.status = status;
    result.error_code = errorCode;
    result.tick = cnt;
    pushCmdConfirm(result);
}

void FbMoveWrap2Queue::flushCmdConfirmations() {
    while (!cmdConfirmArr.empty()) {
        StatusPacket packet;
        packet.reset();
        unsigned int count = 0;
        while (count < cmdConfirmArr.size() &&
               packet.appendItem(STAT_CMD_CONFIRM, &cmdConfirmArr.at(count),
                                 sizeof(CmdResult))) {
            ++count;
        }
        if (count == 0 || !statusQueue_.push(packet)) return;
        for (unsigned int i = 0; i < count; ++i) cmdConfirmArr.pop_front();
    }
}

void FbMoveWrap2Queue::flushCmdResults() {
    while (!cmdResultArr.empty()) {
        StatusPacket packet;
        packet.reset();
        unsigned int count = 0;
        while (count < cmdResultArr.size() &&
               packet.appendItem(STAT_CMDRESULT, &cmdResultArr.at(count), sizeof(CmdResult))) {
            ++count;
        }
        if (count == 0 || !statusQueue_.push(packet)) return;
        for (unsigned int i = 0; i < count; ++i) cmdResultArr.pop_front();
    }
}

void FbMoveWrap2Queue::flushAxisInfoResult() {
    if (pendingAxisInfoMask_ == 0) {
        return;
    }

    for (uint32_t axisId = 0; axisId < 64; ++axisId) {
        const uint64_t axisMask = uint64_t(1) << axisId;
        if ((pendingAxisInfoMask_ & axisMask) == 0) {
            continue;
        }

        AxisInfo info = {};
        info.axis_id = axisId;
        if (axisId >= axisMotbuf.size()) {
            info.error_code = ZMC_ERR_MOTCMD_INVALID_ARG;
        } else {
            info.error_code = axisMotbuf.axis(static_cast<int>(axisId)).readAxisInfo(info);
        }

        StatusPacket statusPkt;
        statusPkt.reset();
        if (statusPkt.appendItem(STAT_AXIS_INFO, &info, sizeof(info)) && statusQueue_.push(statusPkt)) {
            pendingAxisInfoMask_ &= ~axisMask;
        }
        return;
    }
}

// GPT-6 Codex, 2026-11-04. Discard only the RT-owned assembly, with one logical error.
void FbMoveWrap2Queue::resetFragment(uint16_t errorCode) {
    if (errorCode != 0 && fragmentCommandId_ != 0) {
        confirm(fragmentCommandId_, fragment_.command_type, CMD_TARGET_CONTROLLER, 0,
                FB_STAT_ERROR, errorCode);
    }
    fragment_ = MotionCmdFragmentHeader{};
    fragmentCommandId_ = 0;
}

bool FbMoveWrap2Queue::assembleFragment(uint16_t& type, uint16_t& paramLen,
                                      const uint8_t*& param, uint32_t& id) {
    if (paramLen < sizeof(MotionCmdFragmentHeader)) {
        resetFragment(ZMC_ERR_MOTCMD_INVALID_ARG);
        return false;
    }
    MotionCmdFragmentHeader header;
    std::memcpy(&header, param, sizeof(header));
    const size_t bytes = paramLen - sizeof(header);
    if (fragmentCommandId_ == 0) {
        fragment_.command_type = header.command_type;
        fragment_.total_len = header.total_len;
        fragmentCommandId_ = id;
    }
    if (header.command_type == FBM_FRAG || header.command_type >= FBM_ROBOT_BEGIN ||
        header.command_type != fragment_.command_type || header.total_len == 0 ||
        header.total_len > sizeof(fragmentData_) || header.total_len != fragment_.total_len ||
        header.offset != fragment_.offset || bytes == 0 ||
        header.offset + bytes > header.total_len) {
        resetFragment(ZMC_ERR_MOTCMD_INVALID_ARG);
        return false;
    }
    std::memcpy(fragmentData_ + header.offset, param + sizeof(header), bytes);
    fragment_.offset = static_cast<uint16_t>(fragment_.offset + bytes);
    if (fragment_.offset != fragment_.total_len) return false;

    // Return the assembled item before this cycle's motion FBs run.
    type = fragment_.command_type;
    paramLen = fragment_.total_len;
    param = fragmentData_;
    id = fragmentCommandId_;
    resetFragment();
    return true;
}

void FbMoveWrap2Queue::processFragment(uint16_t paramLen, const uint8_t* param, uint32_t id) {
    uint16_t type = FBM_FRAG;
    if (!assembleFragment(type, paramLen, param, id)) return;

    switch (type) {
        case FBM_TEST_LONG_PACKET:
            confirm(id, type, CMD_TARGET_CONTROLLER, 0, FB_STAT_DONE);
            break;
        case FBM_MULTI_MOVE:
        case FBM_MULTI_MOVE_REL: {
            auto cmd = createCommandFromPacket<MemoryAllocator>(type, param, paramLen);
            cmd.id = id;
            if (cmd.paramLen != paramLen) {
                confirm(id, type, CMD_TARGET_CONTROLLER, 0, FB_STAT_ERROR,
                        ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED);
                break;
            }
            const int error = zmotMotbuf.pushCommand(cmd);
            if (error) {
                confirm(id, type, CMD_TARGET_GROUP, static_cast<uint16_t>(zmotMotbuf.groupSelect()),
                        FB_STAT_ERROR, static_cast<uint16_t>(error));
            }
            break;
        }
        default:
            break;
    }
}

// Implementation of command packet processing for FbMoveWrap2Queue
void FbMoveWrap2Queue::processCmdPacket(const CmdPacket& packet) {
    uint16_t type;
    uint16_t param_len;
    const uint8_t* param_ptr;

    // A taken packet must be fully consumed in this cycle; the FIFO may stay non-empty.
    uint16_t offset = 0;
    uint8_t ordinal = 0;
    while (offset < packet.payload_len) {
        CmdItemIterator it(packet.payload + offset, packet.payload_len - offset);
        if (!it.next(type, param_len, param_ptr)) break;
        uint32_t id = makeAutomaticCommandId(commandEpoch_, packet.seq, ordinal);
        const bool confirmable = type != FBM_NOP && type != FBM_BASE && type != FBM_AXIS &&
            type != FBM_GROUP_SELECT && type != FBM_HEARTBEAT &&
            type != FBM_AXIS_INFO_QUERY;
        if (!confirmable) id = 0;
        bool immediateConfirm = false;
        CmdTargetKind targetKind = CMD_TARGET_CONTROLLER;
        uint16_t targetIndex = 0;
        int immediateError = 0;
        // Parse command based on type
        switch (type) {
            case FBM_FRAG: {
                processFragment(param_len, param_ptr, id);
                break;
            }
            case FBM_TEST_LONG_PACKET: {
                immediateConfirm = true;
                break;
            }
            case FBM_HEARTBEAT: {
                // Extract HeartbeatAck and update flow control
                if (param_len >= sizeof(HeartbeatAck)) {
                    HeartbeatAck ack;
                    std::memcpy(&ack, param_ptr, sizeof(HeartbeatAck));
                    updateFlowControl(ack);
                    // printf("ackSeq=%u, ackTick=%u\n", ack.ackSeq, ack.ackTick); // XXX
                }
                break;
            }
            case FBM_FLOWCTRL_CFG: {
                // Process flow control configuration
                if (param_len >= sizeof(FlowCtrlConfig)) {
                    FlowCtrlConfig cfg;
                    std::memcpy(&cfg, param_ptr, sizeof(FlowCtrlConfig));
                    // Apply configuration
                    if (cfg.period > 0) {
                        currentPeriod_ = cfg.period;
                    }
                    if (cfg.batchSize > 0) {
                        // Note: STATE_BATCH_SIZE is constexpr, but we can use cfg.batchSize as
                        // limit For now, we just store verboseFlag
                    }
                    verboseFlag_ = cfg.verboseFlag;
                } else immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                immediateConfirm = true;
                break;
            }
            case FBM_SYNC_PING: {
                if (param_len < sizeof(SyncPingParam)) {
                    immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                } else {
                    SyncPingParam param = {};
                    std::memcpy(&param, param_ptr, sizeof(param));
                    id = param.request_id;
                }
                immediateConfirm = true;
                break;
            }
            // Preserve axis queue order without changing selection. GPT-6 Codex.
            case FBM_READ_PARAMETER:
            case FBM_SET_PARAMETER: {
                const bool write = type == FBM_SET_PARAMETER;
                const size_t required = write ? sizeof(WriteParameterParam)
                                              : sizeof(ReadParameterParam);
                if (param_len < required) {
                    immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                    immediateConfirm = true;
                    break;
                }
                ReadParameterParam param;
                std::memcpy(&param, param_ptr, sizeof(param));
                id = param.request_id;
                targetKind = CMD_TARGET_AXIS;
                targetIndex = static_cast<uint16_t>(param.axis_id);
                if (param.axis_id < 0 || static_cast<size_t>(param.axis_id) >= axisMotbuf.size()) {
                    immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                    immediateConfirm = true;
                    break;
                }
                // Keep writes in inline storage and preserve the full double. GPT-6 Codex.
                CommandStructDefault cmd = write
                    ? createCommandFromPacket<MemoryAllocator>(type, param_ptr,
                                                               sizeof(WriteParameterParam))
                    : CommandStructDefault();
                cmd.cmdType = type;
                cmd.id = id;
                if (!write) cmd.setInt(param.parameter_number);
                axisMotbuf.axis(param.axis_id).pushCommand(cmd);
                break;
            }
            case FBM_AXIS_INFO_QUERY: {
                if (param_len >= sizeof(AxisInfoQuery)) {
                    AxisInfoQuery query;
                    std::memcpy(&query, param_ptr, sizeof(query));
                    if (query.axis_id < 64) {
                        pendingAxisInfoMask_ |= uint64_t(1) << query.axis_id;
                    }
                }
                break;
            }
            case FBM_IO_READ_DI:
            case FBM_IO_READ_DI_MULTI:
            case FBM_IO_READ_DO:
            case FBM_IO_READ_DO_MULTI:
            case FBM_IO_WRITE_DO:
            case FBM_IO_WRITE_DO_MULTI:
            case FBM_IO_READ_AI:
            case FBM_IO_READ_AO:
            case FBM_IO_WRITE_AO:
            case FBM_IO_WRITE_VIRTUAL_DI:
            case FBM_IO_WRITE_VIRTUAL_AI: {
                if (param_len >= sizeof(IoCommandParam)) {
                    IoCommandParam param = {};
                    std::memcpy(&param, param_ptr, sizeof(param));
                    // Single-point reads reuse the unused value slot for synchronous IDs.
                    if ((type == FBM_IO_READ_DI || type == FBM_IO_READ_DO ||
                         type == FBM_IO_READ_AI || type == FBM_IO_READ_AO) && param.value != 0u) {
                        id = param.value;
                    }
                }
                CmdResult result = {};
                result.id = id;
                processIoCommand(type, param_ptr, param_len, result);
                pushCmdConfirm(result);
                break;
            }
            case FBM_BASE: {
                int val = static_cast<const int*>(static_cast<const void*>(param_ptr))[0];
                if (val >= 0 && static_cast<size_t>(val) < axisMotbuf.size()) {
                    axSelect = val;
                    cmdTarget_ = CMD_TARGET_AXIS;
                } else {
                    // TODO: fill to cmdResultArr
                }
                break;
            }
            case FBM_AXIS: {
                int val = static_cast<const int*>(static_cast<const void*>(param_ptr))[0];
                if (val >= 0 && static_cast<size_t>(val) < axisMotbuf.size()) {
                    backupAndSwitchAxis(val);
                    cmdTarget_ = CMD_TARGET_AXIS;
                } else {
                    // TODO: fill to cmdResultArr
                }
                break;
            }
            case FBM_SIMU_MANUAL: {
                if (param_len >= sizeof(uint8_t)) {
                    uint8_t enable = param_ptr[0];
                    setManualMode(enable != 0);
                } else immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                immediateConfirm = true;
                break;
            }
            case FBM_SIMU_PAUSE: {
                setPauseFlag(true);
                immediateConfirm = true;
                break;
            }
            case FBM_SIMU_RESUME: {
                setPauseFlag(false);
                immediateConfirm = true;
                break;
            }
            case FBM_STEPS: {
                if (param_len >= sizeof(int32_t)) {
                    int32_t steps;
                    std::memcpy(&steps, param_ptr, sizeof(int32_t));
                    sendSteps(steps);
                } else immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                immediateConfirm = true;
                break;
            }
            case FBM_TICK_TO: {
                if (param_len >= sizeof(int32_t)) {
                    int32_t tick;
                    std::memcpy(&tick, param_ptr, sizeof(int32_t));
                    sendTickTo(tick);
                } else immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                immediateConfirm = true;
                break;
            }
            case FBM_TIME_TO: {
                if (param_len >= sizeof(double)) {
                    double time;
                    std::memcpy(&time, param_ptr, sizeof(double));
                    sendTimeTo(time);
                } else immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                immediateConfirm = true;
                break;
            }
            case FBM_DELAY: {
                if (param_len >= sizeof(double)) {
                    double time;
                    std::memcpy(&time, param_ptr, sizeof(double));
                    sendDelay(time);
                } else immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                immediateConfirm = true;
                break;
            }
            case FBM_BLOCK_TICKS: {
                if (param_len >= sizeof(int32_t)) {
                    int32_t n = 0;
                    std::memcpy(&n, param_ptr, sizeof(int32_t));
                    if (n > 5000) n = 5000;
                    blockTicksRemain_ = (n > 0) ? n : 0;
                } else immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                immediateConfirm = true;
                break;
            }
            case FBM_EXIT: {
                sendExitFlag();
                immediateConfirm = true;
                break;
            }
            default: {
                const uint8_t* cmd_param = param_ptr;
                auto cmd = createCommandFromPacket<MemoryAllocator>(type, cmd_param, param_len);
                cmd.id = id;
                if (cmd.paramLen != param_len) {
                    immediateError = ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
                    immediateConfirm = true;
                    break;
                }
                if (cmd.cmdType == FBM_GROUP_SELECT) {
                    cmdTarget_ = CMD_TARGET_GROUP;
                    zmotMotbuf.pushCommand(cmd);
                } else if (cmd.cmdType >= FBM_AXIS_GROUP && cmd.cmdType <= FBM_GROUP_SET_JERK) {
                    const int error = zmotMotbuf.pushCommand(cmd);
                    if (error) {
                        immediateError = error;
                        immediateConfirm = true;
                        targetKind = CMD_TARGET_GROUP;
                        targetIndex = static_cast<uint16_t>(zmotMotbuf.groupSelect());
                    }
                } else if ((cmd.cmdType == FBM_SET_SPEED || cmd.cmdType == FBM_SET_ACCEL ||
                            cmd.cmdType == FBM_SET_DECEL || cmd.cmdType == FBM_SET_JERK) &&
                           cmdTarget_ == CMD_TARGET_GROUP) {
                    const int error = zmotMotbuf.pushCommand(cmd);
                    if (error) {
                        immediateError = error;
                        immediateConfirm = true;
                        targetKind = CMD_TARGET_GROUP;
                        targetIndex = static_cast<uint16_t>(zmotMotbuf.groupSelect());
                    }
                } else if (cmd.cmdType < FBM_ROBOT_BEGIN) {
                    // An IO-only POU has no default axis for single-axis commands.
                    if (axisMotbuf.size() == 0 || axSelect < 0 ||
                        static_cast<size_t>(axSelect) >= axisMotbuf.size()) {
                        immediateError = ZMC_ERR_MOTCMD_INVALID_ARG;
                        immediateConfirm = true;
                        targetKind = CMD_TARGET_AXIS;
                        targetIndex = static_cast<uint16_t>(axSelect);
                        break;
                    }
                    // printf("cmd=%d,%d\n", type, param_len);
                    axisMotbuf.axis(axSelect).pushCommand(cmd);
                    targetKind = CMD_TARGET_AXIS;
                    targetIndex = static_cast<uint16_t>(axSelect);
                    tryRestoreAxisSel();
                } else if (cmd.cmdType <= FBM_ROBOT_END) {
                    immediateError = ZMC_ERR_MOTCMD_UNKNOWN_COMMAND;
                    immediateConfirm = true;
                } else {
                    immediateError = ZMC_ERR_MOTCMD_UNKNOWN_COMMAND;
                    immediateConfirm = true;
                }

                break;
            }
        }
        if (immediateConfirm && id != 0) {
            confirm(id, type, targetKind, targetIndex,
                    immediateError ? FB_STAT_ERROR : FB_STAT_DONE,
                    static_cast<uint16_t>(immediateError));
        }
        offset += static_cast<uint16_t>(it.getOffset());
        ++ordinal;
    }
}

// Update flow control based on HeartbeatAck
void FbMoveWrap2Queue::updateFlowControl(const HeartbeatAck& ack) {
    lastAckSeq_ = ack.ackSeq;
    auto ackTick = ack.ackTick;
    // Calculate latency (ticks since ack'd packet was sent)
    if (cnt >= ackTick) {
        lastLatency_ = cnt - ackTick;
    } else {
        lastLatency_ = 0;  // Wrap-around or invalid
    }

    // Adjust period based on latency
    // if (lastLatency_ < lowLatencyThreshold_) {
    //     // Low latency: can speed up
    //     if (currentPeriod_ > minPeriod_) {
    //         currentPeriod_--;
    //     }
    // } else if (lastLatency_ > highLatencyThreshold_) {
    //     // High latency: slow down (double period, cap at maxPeriod_)
    //     currentPeriod_ = std::min(currentPeriod_ * 2, maxPeriod_);
    // }
    // else: latency in normal range, keep current period
}

// Implementation of status packet generation for FbMoveWrap2Queue
void FbMoveWrap2Queue::generateStatusPackets() {
    // ===== Step 1: Check if it's time to send =====
    if (currentPeriod_ > 1 && cnt > 1 && (cnt % currentPeriod_) != 0) {
        return;  // Not this cycle
    }

    const std::size_t axisCount = axisMotbuf.size();
    const std::size_t stateCount = axisCount < STATE_BATCH_SIZE ? axisCount : STATE_BATCH_SIZE;
    const std::size_t firstAxis = axisCount == 0 ? 0 : stateNextAxis_ % axisCount;

    // The base state and flow status fit together for the selected 16 axes.
    StatusPacket statusPkt;
    statusPkt.reset();
    statusPkt.seq = cnt;
    for (std::size_t offset = 0; offset < stateCount; ++offset) {
        const std::size_t i = (firstAxis + offset) % axisCount;
        const AxisWithFb& axisFb = axisMotbuf.axis(static_cast<int>(i));
        AxisState param;

        param.axis_id = static_cast<uint8_t>(i);
        param.axis_state = static_cast<uint8_t>(axisFb.getAxisState());
        param.error_id = static_cast<uint16_t>(axisFb.fbRdError.AxisErrorID);
        param.tick = static_cast<uint32_t>(cnt);
        param.act_pos = axisFb.fbRdActualValues.Position;
        param.set_pos = axisFb.fbRdSetValues.Position;

        if (!statusPkt.appendItem(STAT_STATE, &param, sizeof(param))) break;
    }

    // Append flow control status
    {
        FlowCtrlStatus fs = {};
        fs.period = static_cast<uint16_t>(currentPeriod_);
        fs.batchSize = STATE_BATCH_SIZE;
        fs.verboseFlag = verboseFlag_;
        fs.latency = static_cast<uint16_t>(lastLatency_);
        fs.axis_num = static_cast<uint16_t>(axisMotbuf.size());
        fs.cycleTime = static_cast<uint32_t>(cycleTime * 1e6);
        // Packet-acceptance progress rides the flow status, decoupled from command confirms.
        fs.processed_seq = processedCmdSeq_;
        if (!statusPkt.appendItem(STAT_FLOWCTRL_STATUS, &fs, sizeof(fs))) ++statusDropCount_;
    }

    if (statusPkt.payload_len > 0 && !statusQueue_.push(statusPkt)) ++statusDropCount_;

    // Keep the verbose stream on its existing switch and split 16 axes into 9- and 7-item packets.
    if (verboseFlag_ != 0) {
        StatusPacket extendedPacket;
        extendedPacket.reset();
        extendedPacket.seq = cnt;
        unsigned int extendedInPacket = 0;
        for (std::size_t offset = 0; offset < stateCount; ++offset) {
            const std::size_t i = (firstAxis + offset) % axisCount;
            const AxisWithFb& axisFb = axisMotbuf.axis(static_cast<int>(i));
            AxisState2 param;
            param.axis_id = static_cast<uint8_t>(i);
            param.axis_state = static_cast<uint8_t>(axisFb.getAxisState());
            param.error_id = static_cast<uint16_t>(axisFb.fbRdError.AxisErrorID);
            param.tick = static_cast<uint32_t>(cnt);
            param.act_pos = axisFb.fbRdActualValues.Position;
            param.set_pos = axisFb.fbRdSetValues.Position;
            param.set_vel = axisFb.fbRdSetValues.Velocity;
            param.set_acc = axisFb.fbRdSetValues.Acceleration;
            param.set_position = axisFb.fbRdPowerParam.SetPosition;
            param.act_position = axisFb.fbRdPowerParam.ActPosition;

            if (extendedInPacket == EXTENDED_STATE_ITEMS_PER_PACKET) {
                if (!statusQueue_.push(extendedPacket)) ++statusDropCount_;
                extendedPacket.reset();
                extendedPacket.seq = cnt;
                extendedInPacket = 0;
            }
            if (!extendedPacket.appendItem(STAT_STATE2, &param, sizeof(param))) {
                ++statusDropCount_;
                continue;
            }
            ++extendedInPacket;
        }
        if (extendedInPacket != 0 && !statusQueue_.push(extendedPacket)) ++statusDropCount_;
    }

    if (axisCount != 0) stateNextAxis_ = (firstAxis + stateCount) % axisCount;
}

void FbMoveWrap2Queue::runCycle() {
    // static auto last_time = std::chrono::steady_clock::now();
    // auto now = std::chrono::steady_clock::now();
    // auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now -
    // last_time).count();
    // // if (elapsed < 1000)
    // //     return;
    // if (elapsed < 1000) {
    //     auto sleep_us = 1000 - elapsed;
    //     if (sleep_us > 0) {
    //         std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
    //     }
    // }
    // last_time = std::chrono::steady_clock::now();
    _runCycle();
}

void FbMoveWrap2Queue::dispatchCmdPackets() {
    if (blockTicksRemain_ > 0) {
        --blockTicksRemain_;
        return;
    }
    // No confirmation-capacity gate: dispatch straight to the axes.
    CmdPacket packet;
    if (!cmdpackQueue_.pop(packet)) return;
    // The producer owns the sequence counter; a backward step marks a new ID epoch.
    if (hasLastCommandSeq_ && packet.seq < lastCommandSeq_) ++commandEpoch_;
    lastCommandSeq_ = packet.seq;
    hasLastCommandSeq_ = true;
    processCmdPacket(packet);
    // Packet-acceptance progress counts on full consumption, independent of per-command
    // confirms, which keep reporting their own asynchronous statuses.
    processedCmdSeq_ = packet.seq;
}

void FbMoveWrap2Queue::_runCycle() {
    cnt++;
    dispatchCmdPackets();

    // Run axis cycles
    _runCycleTop();
    _runCycleBottom();
    flushCmdConfirmations();
    flushCmdResults();
    flushAxisInfoResult();

    // Generate status packets
    generateStatusPackets();
}

void FbMoveWrap2Queue::reset_runtime() {
    FbMoveWrapWithRobot::reset_runtime();
    cmdpackQueue_.clear();
    statusQueue_.clear();
    pendingAxisInfoMask_ = 0;
    cmdTarget_ = CMD_TARGET_AXIS;
    currentPeriod_ = 512;
    stateSamplePeriod_ = 1;
    stateNextAxis_ = 0;
    lastAckSeq_ = 0;
    lastLatency_ = 0;
    verboseFlag_ = 0;
    statusDropCount_ = 0;
    blockTicksRemain_ = 0;
    processedCmdSeq_ = 0;
    commandEpoch_ = 0;
    lastCommandSeq_ = 0;
    hasLastCommandSeq_ = false;
    resetFragment();
}

void FbMoveWrap::reset_runtime() {
    FbMoveWrap2Queue::reset_runtime();
    cmdIf_.clear();
}

}  // namespace zmc
