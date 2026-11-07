#include "qcmd/MotionCmdScript.hpp"

#include <cstring>
#include <deque>

#include "fbmEnum.h"
#include "fbt_errorcode_def.h"
#include "impl/fbt_impl_enum_def.h"
#include "qcmd/MotionCmdPacket.hpp"

namespace zmc {

namespace {

inline int motcmdRet(MC_ERROR_CODE code) { return static_cast<int>(code); }
const uint16_t kLocalWaitType = 0xFFFEu;

struct LocalWaitParam {
    uint8_t gateType;
    int axis;
    uint32_t nTick;
};

bool getFirstPacketItem(const CmdPacket& packet, uint16_t& type, uint16_t& paramLen,
                        const uint8_t*& paramPtr) {
    CmdItemIterator it(packet.payload, packet.payload_len);
    return it.next(type, paramLen, paramPtr);
}

}  // namespace

int MotionCmdScript::pushPacket(const CmdPacket& pkt) {
    scriptQueue_.push_back(pkt);
    return motcmdRet(ZMC_NO_ERROR);
}

// Script-first: flow bypass; sweep-only clear; wait arms in advanceScript.
// appendOrDeferItem: merge into non-wait tail when canAppend, else new packet.
int MotionCmdScript::appendOrDeferItem(uint16_t type, const void* param, uint16_t param_len) {
    if (param_len > MOTION_CMD_PARAM_MAX || (param_len != 0 && param == nullptr) ||
        type == FBM_FRAG) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    if (!scriptQueue_.empty()) {
        CmdPacket& tail = scriptQueue_.back();
        if (tail.firstType() != kLocalWaitType && tail.firstType() != FBM_FRAG) {
            MotionCmdPacket& pack = static_cast<MotionCmdPacket&>(tail);
            if (pack.canAppend(param_len) &&
                tail.payload_len + MOTION_CMD_ITEM_HEADER_SIZE + param_len +
                    MOTION_CMD_HEARTBEAT_SIZE <= MAX_PAYLOAD_SIZE &&
                pack.appendItem(type, param, param_len)) {
                return motcmdRet(ZMC_NO_ERROR);
            }
        }
    }
    CmdPacket packets[MOTION_CMD_FRAGMENT_PACKETS_MAX];
    size_t count = 0;
    const int ret = buildItemPackets(type, param, param_len, packets,
                                     MOTION_CMD_FRAGMENT_PACKETS_MAX, count);
    if (ret != 0) return ret;
    for (size_t i = 0; i < count; ++i) scriptQueue_.push_back(packets[i]);
    return motcmdRet(ZMC_NO_ERROR);
}

int MotionCmdScript::priorityAppendItem(uint16_t type, const void* param, uint16_t param_len) {
    return appendOrDeferItem(type, param, param_len);
}

int MotionCmdScript::heartbeat() {
    int ret = pumpHeartbeat();
    int scriptRet = flowPaused() ? 0 : advanceScript();
    return ret != 0 ? ret : scriptRet;
}

int MotionCmdScript::syncPing(uint32_t request_id) {
    SyncPingParam param = {};
    param.request_id = request_id;
    return submitItem(FBM_SYNC_PING, &param, sizeof(param));
}

int MotionCmdScript::testLongPacket(const void* data, uint16_t length) {
    CmdPacket packets[MOTION_CMD_FRAGMENT_GROUP_MAX];
    size_t count = 0;
    const int ret = buildItemPackets(FBM_TEST_LONG_PACKET, data, length, packets,
                                     MOTION_CMD_FRAGMENT_GROUP_MAX, count);
    if (ret != 0) return ret;
    for (size_t i = 0; i < count; ++i) scriptQueue_.push_back(packets[i]);
    return motcmdRet(ZMC_NO_ERROR);
}

bool MotionCmdScript::isScriptBusy() const {
    return !scriptQueue_.empty() || scriptGateType_ != ScriptGateType::None;
}

void MotionCmdScript::armScriptTickGate(ScriptGateType gateType, uint32_t waitTicks, int axis) {
    scriptGateType_ = gateType;
    scriptGateAxis_ = axis;
    const uint32_t nowTick = mirror_.currentTickForAxis(axis);
    scriptGateTargetTick_ = (gateType == ScriptGateType::WaitTick) ? (nowTick + waitTicks) : 0;
    scriptGateArmedTick_ = nowTick;
}

int MotionCmdScript::enqueueExclusivePacket(uint16_t type, const void* param, uint16_t param_len) {
    CmdPacket pkt;
    pkt.reset();
    if (!pkt.appendItem(type, param, param_len)) {
        return motcmdRet(ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED);
    }
    scriptQueue_.push_back(pkt);
    return motcmdRet(ZMC_NO_ERROR);
}

// Wait command entry: always enqueue; gate arms only in advanceScript.
int MotionCmdScript::enqueueWaitItem(ScriptGateType gateType, uint32_t waitTicks, int axis) {
    LocalWaitParam p = {};
    p.gateType = static_cast<uint8_t>(gateType);
    p.axis = axis;
    p.nTick = waitTicks;
    return enqueueExclusivePacket(kLocalWaitType, &p, sizeof(p));
}

void MotionCmdScript::abortScript() {
    while (!scriptQueue_.empty()) {
        scriptQueue_.pop_front();
    }
    scriptGateType_ = ScriptGateType::None;
}

int MotionCmdScript::scriptSweep() {
    abortScript();
    return motcmdRet(ZMC_NO_ERROR);
}

int MotionCmdScript::advanceScript() {
    if (flowPaused()) return 0;
    if (scriptGateType_ != ScriptGateType::None) {
        const uint32_t nowTick = mirror_.currentTickForAxis(scriptGateAxis_);
        bool gateOpen = false;
        // check gate condition: WaitTick or WaitIdle
        if (scriptGateType_ == ScriptGateType::WaitTick) {
            gateOpen = (nowTick >= scriptGateTargetTick_);
        } else if (scriptGateType_ == ScriptGateType::WaitIdle) {
            bool hasState = false;
            const uint8_t st = mirror_.currentAxisState(scriptGateAxis_, hasState);
            gateOpen = (nowTick > scriptGateArmedTick_) && hasState &&
                       (st <= mcStandstill || st >= mcErrorStop);
        }
        if (!gateOpen) {
            return 0;
        }
        scriptGateType_ = ScriptGateType::None;
    }

    while (scriptGateType_ == ScriptGateType::None && !scriptQueue_.empty()) {
        CmdPacket pkt = scriptQueue_.front();

        uint16_t type = 0;
        uint16_t param_len = 0;
        const uint8_t* param_ptr = nullptr;
        if (!getFirstPacketItem(pkt, type, param_len, param_ptr)) {
            return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
        }

        int ret = 0;
        size_t count = 1;
        if (type == kLocalWaitType) {
            LocalWaitParam p = {};
            if (param_len < sizeof(p)) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            std::memcpy(&p, param_ptr, sizeof(p));
            armScriptTickGate(static_cast<ScriptGateType>(p.gateType), p.nTick, p.axis);
        } else if (type == FBM_FRAG) {
            MotionCmdFragmentHeader header;
            if (param_len < sizeof(header)) return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            std::memcpy(&header, param_ptr, sizeof(header));
            count = (header.total_len + MOTION_CMD_FRAGMENT_DATA_MAX - 1) /
                MOTION_CMD_FRAGMENT_DATA_MAX;
            if (header.offset != 0 || count == 0 || count > MOTION_CMD_FRAGMENT_GROUP_MAX ||
                scriptQueue_.size() < count) {
                return motcmdRet(ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED);
            }
            // Copy only the bounded group; retain the original FIFO until all sends succeed.
            CmdPacket packets[MOTION_CMD_FRAGMENT_GROUP_MAX];
            for (size_t i = 0; i < count; ++i) packets[i] = scriptQueue_[i];
            ret = sendPacketGroup(packets, count);
        } else {
            ret = sendPacket(pkt);
        }

        if (ret != 0) {
            return ret;
        }
        for (size_t i = 0; i < count; ++i) scriptQueue_.pop_front();
    }

    return 0;
}

int MotionCmdScript::waitTicks(uint32_t nTick, int axis, uint32_t timeoutMs, uint32_t pollMs) {
    (void)timeoutMs;
    (void)pollMs;
    if (axis < 0) {
        return static_cast<int>(ZMC_ERR_MOTCMD_WAIT_INVALID_ARG);
    }
    if (nTick == 0) {
        return static_cast<int>(ZMC_NO_ERROR);
    }
    return enqueueWaitItem(ScriptGateType::WaitTick, nTick, axis);
}

int MotionCmdScript::waitSeconds(double seconds, int axis, uint32_t timeoutMs, uint32_t pollMs) {
    (void)timeoutMs;
    (void)pollMs;
    if (axis < 0 || seconds < 0.0) {
        return static_cast<int>(ZMC_ERR_MOTCMD_WAIT_INVALID_ARG);
    }
    if (seconds <= 0.0) {
        return static_cast<int>(ZMC_NO_ERROR);
    }

    FlowCtrlStatus st;
    int ret = getFlowStatus(st);
    if (ret != static_cast<int>(ZMC_NO_ERROR)) {
        return ret;
    }
    uint32_t waitN = 0;
    if (st.cycleTime > 0) {
        const double ticks = (seconds * 1000000.0) / static_cast<double>(st.cycleTime);
        waitN = static_cast<uint32_t>(ticks);
        if (ticks > static_cast<double>(waitN)) {
            ++waitN;
        }
    } else {
        // Rough fallback: 1 tick ~= 20ms
        waitN = static_cast<uint32_t>(seconds * 50.0);
    }
    if (waitN == 0) {
        waitN = 1;
    }
    return enqueueWaitItem(ScriptGateType::WaitTick, waitN, axis);
}

int MotionCmdScript::waitAxisIdle(int axis, uint32_t timeoutMs, uint32_t pollMs,
                                  bool errorAsFail) {
    (void)timeoutMs;
    (void)pollMs;
    (void)errorAsFail;
    if (axis < 0) {
        return static_cast<int>(ZMC_ERR_MOTCMD_WAIT_INVALID_ARG);
    }
    return enqueueWaitItem(ScriptGateType::WaitIdle, 0, axis);
}

}  // namespace zmc
