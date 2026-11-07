#include "qcmd/MotionCmdMirror.hpp"

#include <cstring>

#include "fbmEnum.h"
#include "impl/fbt_impl_enum_def.h"
#include "mnode/FbTypeDef.h"

// AI modification: GPT-6 Codex, 2026-10-12. Feedback schedules reads, never input-value commits.

#include "fbmEnum.h"
#include "fbt_errorcode_def.h"
#include "impl/fbt_impl_enum_def.h"

namespace zmc {

namespace {

inline void FlowCtrlStatusInit(FlowCtrlStatus& flowStatus) {
    flowStatus.period = 512;
    flowStatus.batchSize = 0;
    flowStatus.verboseFlag = 0;
    flowStatus.reserved = 0;
    flowStatus.processed_seq = 0;
    flowStatus.latency = 0;
    // No status packet means that no runtime axes are known yet.
    flowStatus.axis_num = 0;
    flowStatus.cycleTime = 1000;
}

}  // namespace

MotionStatusMirror::MotionStatusMirror() { FlowCtrlStatusInit(flowStatus_); }

void MotionStatusMirror::requestAxisInfoRefresh(uint32_t axis_id) {
    axisInfoRefresh_ |= std::uint64_t(1) << axis_id;
}

void MotionStatusMirror::initAxisInfo(int axis_count) {
    axisInfos_.clear();
    axisInfoRefresh_ = 0;
    for (int axis = 0; axis < axis_count; ++axis) {
        AxisInfo info = {};
        info.axis_id = static_cast<uint32_t>(axis);
        info.velocity = 5.0;
        info.acceleration = 20.0;
        info.deceleration = 20.0;
        info.pulses_per_unit_num = 1;
        info.pulses_per_unit_den = 1;
        axisInfos_.push_back(info);
        requestAxisInfoRefresh(info.axis_id);
    }
}

bool MotionStatusMirror::nextAxisInfoRefresh(uint32_t& axis_id) const {
    for (uint32_t axis = 0; axis < sizeof(axisInfoRefresh_) * 8; ++axis) {
        if ((axisInfoRefresh_ & (std::uint64_t(1) << axis)) != 0) {
            axis_id = axis;
            return true;
        }
    }
    return false;
}

void MotionStatusMirror::finishAxisInfoRefresh(uint32_t axis_id) {
    axisInfoRefresh_ &= ~(std::uint64_t(1) << axis_id);
}

bool MotionStatusMirror::applyPacket(const StatusPacket& statusPkt) {
    bool hasFlow = false;
    // Record last received seq for flow control ack
    lastRecvSeq_ = statusPkt.seq;

    StatusItemIterator it(statusPkt.payload, statusPkt.payload_len);
    uint16_t type;
    uint16_t param_len;
    const uint8_t* param_ptr;

    while (it.next(type, param_len, param_ptr)) {
        if (type == STAT_STATE) {
            if (param_len >= sizeof(AxisState)) {
                AxisState state;
                std::memcpy(&state, param_ptr, sizeof(AxisState));
                // tick is already correctly read from payload (32-bit uint32_t)
                // Do NOT overwrite with statusPkt.seq (16-bit uint16_t)
                if (state.axis_id >= axisStates_.size()) axisStates_.resize(state.axis_id + 1);
                AxisState2 cachedState = {};
                cachedState.axis_id = state.axis_id;
                cachedState.axis_state = state.axis_state;
                cachedState.error_id = state.error_id;
                cachedState.tick = state.tick;
                cachedState.act_pos = state.act_pos;
                cachedState.set_pos = state.set_pos;
                axisStates_[state.axis_id].state = cachedState;
                axisStates_[state.axis_id].kind = AXIS_STATE_CACHE_BASIC;
                // Update last received tick
                if (state.tick > lastRecvTick_) {
                    lastRecvTick_ = state.tick;
                }
            }
        } else if (type == STAT_STATE2) {
            if (param_len >= sizeof(AxisState2)) {
                AxisState2 state2;
                std::memcpy(&state2, param_ptr, sizeof(AxisState2));
                if (state2.axis_id >= axisStates_.size()) axisStates_.resize(state2.axis_id + 1);
                axisStates_[state2.axis_id].state = state2;
                axisStates_[state2.axis_id].kind = AXIS_STATE_CACHE_EXTENDED;
                // Keep the last received tick monotonic.
                if (state2.tick > lastRecvTick_) {
                    lastRecvTick_ = state2.tick;
                }
            }
        } else if (type == STAT_AXIS_INFO) {
            if (param_len >= sizeof(AxisInfo)) {
                AxisInfo info;
                std::memcpy(&info, param_ptr, sizeof(info));
                bool replaced = false;
                for (std::size_t index = 0; index < axisInfos_.size(); ++index) {
                    if (axisInfos_[index].axis_id == info.axis_id) {
                        axisInfos_[index] = info;
                        replaced = true;
                        break;
                    }
                }
                if (!replaced) {
                    axisInfos_.push_back(info);
                }
            }
        } else if (type == STAT_FLOWCTRL_STATUS) {
            if (param_len >= sizeof(FlowCtrlStatus)) {
                FlowCtrlStatus st;
                std::memcpy(&st, param_ptr, sizeof(FlowCtrlStatus));
                flowStatus_ = st;
                hasFlow = true;
            }
        } else if (type == STAT_CMDRESULT) {
            if (param_len >= sizeof(CmdResultInfo)) {
                CmdResultInfo result;
                std::memcpy(&result, param_ptr, sizeof(CmdResultInfo));
                if (result.target_kind != CMD_TARGET_AXIS) continue;
                // Ratio changes report their applied state through the existing terminal result.
                if (result.command_type == FBM_SET_NUM_DEN &&
                    (result.status == FB_STAT_DONE || result.status == FB_STAT_ERROR ||
                     result.status == FB_STAT_ABORTED)) {
                    requestAxisInfoRefresh(result.target_index);
                }
                cmdResults_.push_back(result);
                // Limit result queue size
                if (cmdResults_.size() > 100) {
                    cmdResults_.pop_front();
                }
            }
        } else if (type == STAT_CMD_CONFIRM) {
            if (param_len >= sizeof(CmdResult)) {
                CmdResult confirm = {};
                std::memcpy(&confirm, param_ptr, sizeof(confirm));
                const bool scalar = confirm.command_type == FBM_SET_SPEED ||
                                    confirm.command_type == FBM_SET_ACCEL ||
                                    confirm.command_type == FBM_SET_DECEL ||
                                    confirm.command_type == FBM_SET_JERK;
                const bool completed =
                    confirm.status == FB_STAT_DONE || confirm.status == FB_STAT_ERROR;
                if (confirm.target_kind == static_cast<uint8_t>(CMD_TARGET_AXIS) &&
                    ((scalar && completed) || (confirm.command_type == FBM_SET_NUM_DEN &&
                                               confirm.status == FB_STAT_ERROR))) {
                    requestAxisInfoRefresh(confirm.target_index);
                }
                // One IO record updates its snapshot before its confirmation can be consumed.
                if (confirm.command_type >= FBM_IO_READ_DI &&
                    confirm.command_type <= FBM_IO_WRITE_VIRTUAL_AI) {
                    for (auto existing = ioResults_.begin(); existing != ioResults_.end();) {
                        if (confirm.io_count != 0 &&
                            existing->command_type == confirm.command_type &&
                            existing->target_kind == confirm.target_kind &&
                            existing->target_index == confirm.target_index &&
                            existing->io_count == confirm.io_count) {
                            existing = ioResults_.erase(existing);
                        } else ++existing;
                    }
                    ioResults_.push_back(confirm);
                    if (ioResults_.size() > 100) ioResults_.pop_front();
                }
                if (confirm.id == 0) continue;
                if (cmdConfirms_.size() == 256) {
                    cmdConfirms_.pop_front();
                    ++cmdConfirmDropCount_;
                }
                cmdConfirms_.push_back(confirm);
            }

        }
    }
    return hasFlow;
}

// Clear data surfaced by dashboard endpoints without disrupting IO or heartbeat ACK progress.
void MotionStatusMirror::clearDashboardCache() {
    axisStates_.clear();
    cmdResults_.clear();
    FlowCtrlStatusInit(flowStatus_);
}

// Restore the mirror to its initial state, including the IO snapshot and ACK counters.
void MotionStatusMirror::resetCache() {
    clearDashboardCache();
    axisInfos_.clear();
    axisInfoRefresh_ = 0;
    ioResults_.clear();
    cmdConfirms_.clear();
    cmdConfirmDropCount_ = 0;
    lastRecvSeq_ = 0;
    lastRecvTick_ = 0;
}

int MotionStatusMirror::getAxisState(int axis_id, AxisState& state) const {
    if (axis_id >= 0 && axis_id < static_cast<int>(axisStates_.size()) &&
        axisStates_[axis_id].kind != AXIS_STATE_CACHE_NONE) {
        const AxisState2& cached = axisStates_[axis_id].state;
        state = AxisState{cached.axis_id, cached.axis_state, cached.error_id, cached.tick,
                          cached.act_pos, cached.set_pos};
        return static_cast<int>(ZMC_NO_ERROR);
    }
    return static_cast<int>(ZMC_ERR_MOTCMD_AXIS_STATE_INVALID_AXIS);
}

int MotionStatusMirror::getAxisState2(int axis_id, AxisState2& state) const {
    if (axis_id >= 0 && axis_id < static_cast<int>(axisStates_.size()) &&
        axisStates_[axis_id].kind == AXIS_STATE_CACHE_EXTENDED) {
        state = axisStates_[axis_id].state;
        return static_cast<int>(ZMC_NO_ERROR);
    }
    return static_cast<int>(ZMC_ERR_MOTCMD_AXIS_STATE2_INVALID_AXIS);
}

int MotionStatusMirror::getAxisInfo(int axis_id, AxisInfo& info) const {
    if (axis_id < 0) {
        return static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    for (std::size_t index = 0; index < axisInfos_.size(); ++index) {
        if (axisInfos_[index].axis_id == static_cast<uint32_t>(axis_id)) {
            info = axisInfos_[index];
            return static_cast<int>(ZMC_NO_ERROR);
        }
    }
    return static_cast<int>(ZMC_ERR_MOTCMD_AXIS_INFO_UNAVAILABLE);
}

int MotionStatusMirror::getAxisParameter(int axis_id, uint16_t command_type, double& value) const {
    AxisInfo info = {};
    const int ret = getAxisInfo(axis_id, info);
    if (ret != 0) return ret;
    if (info.error_code != 0) return info.error_code;
    switch (command_type) {
        case FBM_SET_NUM_DEN:
            value = static_cast<double>(info.pulses_per_unit_num) / info.pulses_per_unit_den;
            break;
        case FBM_SET_SPEED:
            value = info.velocity;
            break;
        case FBM_SET_ACCEL:
            value = info.acceleration;
            break;
        case FBM_SET_DECEL:
            value = info.deceleration;
            break;
        case FBM_SET_JERK:
            value = info.jerk;
            break;
        default:
            return ZMC_ERR_MOTCMD_INVALID_ARG;
    }
    return 0;
}

int MotionStatusMirror::getCmdResults(std::vector<CmdResultInfo>& results) const {
    results.assign(cmdResults_.begin(), cmdResults_.end());
    if (results.empty()) {
        return static_cast<int>(ZMC_ERR_MOTCMD_CMD_RESULTS_EMPTY);
    }
    return static_cast<int>(ZMC_NO_ERROR);
}

int MotionStatusMirror::getCmdConfirmations(std::vector<CmdResult>& confirms) {
    confirms.assign(cmdConfirms_.begin(), cmdConfirms_.end());
    cmdConfirms_.clear();
    return static_cast<int>(ZMC_NO_ERROR);
}

// Consume the oldest cached confirmation without refreshing the status queue.
bool MotionStatusMirror::popCmdConfirmation(CmdResult& confirm) {
    if (cmdConfirms_.empty()) return false;
    confirm = cmdConfirms_.front();
    cmdConfirms_.pop_front();
    return true;
}

int MotionStatusMirror::getHomeStatus(int axis_id, int& status, int& error_id) const {
    status = FB_STAT_IDLE;
    error_id = static_cast<int>(ZMC_NO_ERROR);
    if (axis_id < 0) return static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG);

    if (axis_id < static_cast<int>(axisStates_.size()) &&
        axisStates_[axis_id].kind != AXIS_STATE_CACHE_NONE &&
        axisStates_[axis_id].state.axis_state == mcHomeEx) {
        status = FB_STAT_BUSY_ACTIVE;
        return static_cast<int>(ZMC_NO_ERROR);
    }

    for (std::deque<CmdResultInfo>::const_reverse_iterator it = cmdResults_.rbegin();
         it != cmdResults_.rend(); ++it) {
        if (it->target_kind != CMD_TARGET_AXIS || it->target_index != static_cast<uint8_t>(axis_id) ||
            (it->command_type != FBM_HOME && it->command_type != FBM_HOME_SDO)) {
            continue;
        }
        status = static_cast<int>(it->status);
        if (it->status == FB_STAT_ERROR) error_id = static_cast<int>(it->error_code);
        return static_cast<int>(ZMC_NO_ERROR);
    }
    return static_cast<int>(ZMC_NO_ERROR);
}

int MotionStatusMirror::getIoResults(std::vector<CmdResult>& results) {
    results.assign(ioResults_.begin(), ioResults_.end());
    if (results.empty()) {
        return static_cast<int>(ZMC_ERR_MOTCMD_IO_RESULTS_EMPTY);
    }
    return static_cast<int>(ZMC_NO_ERROR);
}

int MotionStatusMirror::getFlowStatus(FlowCtrlStatus& status) const {
    status = flowStatus_;
    return static_cast<int>(ZMC_NO_ERROR);
}
int MotionStatusMirror::getFlowConfig(FlowCtrlConfig& config) const {
    config.period = flowStatus_.period;
    config.batchSize = flowStatus_.batchSize;
    config.verboseFlag = flowStatus_.verboseFlag;
    // Preserve the original byte-for-byte echo of the former reserved[0..2] bytes.
    config.reserved[0] = flowStatus_.reserved;
    std::memcpy(&config.reserved[1], &flowStatus_.processed_seq, sizeof(uint16_t));
    return static_cast<int>(ZMC_NO_ERROR);
}
uint32_t MotionStatusMirror::currentTickForAxis(int axis) const {
    if (axis >= 0 && axis < static_cast<int>(axisStates_.size()) &&
        axisStates_[axis].kind != AXIS_STATE_CACHE_NONE) {
        return axisStates_[axis].state.tick;
    }
    return lastRecvTick_;
}

uint8_t MotionStatusMirror::currentAxisState(int axis, bool& hasState) const {
    hasState = false;
    if (axis >= 0 && axis < static_cast<int>(axisStates_.size()) &&
        axisStates_[axis].kind != AXIS_STATE_CACHE_NONE) {
        hasState = true;
        return axisStates_[axis].state.axis_state;
    }
    return 0;
}

}  // namespace zmc
