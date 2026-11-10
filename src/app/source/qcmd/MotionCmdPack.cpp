#include "qcmd/MotionCmdPack.hpp"

#include "fbmEnum.h"
#include "fbt_errorcode_def.h"

// AI modification: GPT-6 Codex, 2026-10-12. Refresh queries bypass local script waits.

namespace zmc {

namespace {

inline int motcmdRet(MC_ERROR_CODE code) { return static_cast<int>(code); }

}  // namespace

MotionCmdPack::MotionCmdPack() {}

// GPT-6 (Codex), 2026-11-09. Decide flow once before publishing this round's packets.
int MotionCmdPack::pumpHeartbeat() {
    updateStatusCache();
    FlowCtrlStatus flow;
    mirror_.getFlowStatus(flow);
    const uint16_t sendSeq = lastSentSequence();
    const MotionCmdFlow::State previous = flow_.state();
    const bool heartbeat = flow_.step(sendSeq, flow.processed_seq);
    if (flow_.state() == MotionCmdFlow::State::Stalled && previous != flow_.state()) {
        probeResult_ = ZMC_ERR_HOST_RECV_TIMEOUT;
    }
    if (!flowPaused()) probeResult_ = 0;
    if (!heartbeat) return 0;
    const int ret = _heartbeat();
    if (flowPaused() && ret != 0) probeResult_ = ret;
    return ret;
}

// GPT-6 Codex, 2026-11-04. Reserve heartbeat space before splitting logical parameters.
int MotionCmdPack::buildItemPackets(uint16_t type, const void* param, uint16_t param_len,
                                    CmdPacket* packets, size_t capacity, size_t& count) {
    if (param_len > capacity * MOTION_CMD_FRAGMENT_DATA_MAX ||
        (param_len != 0 && param == nullptr) || type == FBM_FRAG) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    count = 1;
    packets[0].reset();
    if (param_len <= MOTION_CMD_SINGLE_PARAM_MAX) {
        return packets[0].appendItem(type, param, param_len)
                   ? motcmdRet(ZMC_NO_ERROR)
                   : motcmdRet(ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED);
    }
    count = 0;
    for (size_t offset = 0; offset < param_len;) {
        const size_t remaining = param_len - offset;
        const size_t bytes =
            remaining < MOTION_CMD_FRAGMENT_DATA_MAX ? remaining : MOTION_CMD_FRAGMENT_DATA_MAX;
        MotionCmdFragmentHeader header = {type, param_len, static_cast<uint16_t>(offset)};
        uint8_t fragment[MOTION_CMD_SINGLE_PARAM_MAX];
        std::memcpy(fragment, &header, sizeof(header));
        std::memcpy(fragment + sizeof(header), static_cast<const uint8_t*>(param) + offset, bytes);
        CmdPacket& packet = packets[count++];
        packet.reset();
        if (!packet.appendItem(FBM_FRAG, fragment,
                               static_cast<uint16_t>(sizeof(header) + bytes))) {
            return motcmdRet(ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED);
        }
        offset += bytes;
    }
    return motcmdRet(ZMC_NO_ERROR);
}

// Prepare every trailer before transport preflight; a failed group is never partly published.
int MotionCmdPack::sendPacketGroup(CmdPacket* packets, size_t count) {
    if (flowPaused()) return motcmdRet(ZMC_ERR_MOTCMD_FLOW_PAUSED);
    HeartbeatAck ack = {mirror_.ackSeq(), mirror_.ackTick()};
    for (size_t i = 0; i < count; ++i) {
        if (!packets[i].appendItem(FBM_HEARTBEAT, &ack, sizeof(ack))) {
            return motcmdRet(ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED);
        }
    }
    return count == 1 ? send(packets[0]) : sendPackets(packets, count);
}

// Append heartbeat trailer and push; send path has no script awareness.
int MotionCmdPack::sendPacket(CmdPacket pkt) {
    if (pkt.payload_len == 0) {
        return motcmdRet(ZMC_NO_ERROR);
    }
    return sendPacketGroup(&pkt, 1);
}

// Immediate send: packetize once, then publish the complete logical item consecutively.
int MotionCmdPack::submitItem(uint16_t type, const void* param, uint16_t param_len) {
    if (param_len > MOTION_CMD_PARAM_MAX) return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    CmdPacket packets[MOTION_CMD_FRAGMENT_PACKETS_MAX];
    size_t count = 0;
    const int ret =
        buildItemPackets(type, param, param_len, packets, MOTION_CMD_FRAGMENT_PACKETS_MAX, count);
    return ret != 0 ? ret : sendPacketGroup(packets, count);
}

// Heartbeat-only packet: cannot use sendPacket(empty) which is a no-op.
int MotionCmdPack::_heartbeat() {
    CmdPacket pkt;
    pkt.reset();
    HeartbeatAck ack;
    ack.ackSeq = mirror_.ackSeq();
    ack.ackTick = mirror_.ackTick();
    if (!pkt.appendItem(FBM_HEARTBEAT, &ack, sizeof(HeartbeatAck))) {
        return motcmdRet(ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED);
    }
    return send(pkt);
}

// Flow control: send immediately;
int MotionCmdPack::setFlowConfig(const FlowCtrlConfig& cfg) {
    CmdPacket pkt;
    pkt.reset();
    pkt.appendItem(FBM_FLOWCTRL_CFG, &cfg, sizeof(FlowCtrlConfig));
    return send(pkt);
}

int MotionCmdPack::flush() { return motcmdRet(ZMC_NO_ERROR); }

int MotionCmdPack::autoFlush() { return motcmdRet(ZMC_NO_ERROR); }

void MotionCmdPack::setAutoFlushEnabled(bool /*enabled*/) {}

bool MotionCmdPack::isAutoFlushEnabled() const { return true; }

void MotionCmdPack::setFlushLevel(size_t /*level*/) {}

int MotionCmdPack::getAxisState(int axis_id, AxisState& state) {
    return mirror_.getAxisState(axis_id, state);
}

int MotionCmdPack::getAxisState2(int axis_id, AxisState2& state) {
    return mirror_.getAxisState2(axis_id, state);
}

int MotionCmdPack::getAxisInfo(int axis_id, AxisInfo& info) {
    return mirror_.getAxisInfo(axis_id, info);
}

void MotionCmdPack::initAxisInfo(int axis_count) { mirror_.initAxisInfo(axis_count); }

int MotionCmdPack::getAxisParameter(int axis_id, uint16_t command_type, double& value) const {
    return mirror_.getAxisParameter(axis_id, command_type, value);
}

int MotionCmdPack::getCmdResults(std::vector<CmdResultInfo>& results) {
    return mirror_.getCmdResults(results);
}

int MotionCmdPack::getCmdConfirmations(std::vector<CmdResult>& confirms) {
    updateStatusCache();
    return mirror_.getCmdConfirmations(confirms);
}

// Delegate one-item consumption to the existing status mirror cache.
bool MotionCmdPack::popCmdConfirmation(CmdResult& confirm) {
    return mirror_.popCmdConfirmation(confirm);
}

int MotionCmdPack::getHomeStatus(int axis_id, int& status, int& error_id) {
    return mirror_.getHomeStatus(axis_id, status, error_id);
}

int MotionCmdPack::getIoResults(std::vector<CmdResult>& results) {
    return mirror_.getIoResults(results);
}

int MotionCmdPack::getFlowStatus(FlowCtrlStatus& status) const {
    return mirror_.getFlowStatus(status);
}

int MotionCmdPack::getFlowConfig(FlowCtrlConfig& config) const {
    return mirror_.getFlowConfig(config);
}

void MotionCmdPack::applyStatusPacket(const StatusPacket& packet) {
    mirror_.applyPacket(packet);
}

void MotionCmdPack::updateStatusCache() {
    StatusPacket packet;
    while (tryPopStatus(packet)) {
        applyStatusPacket(packet);
    }
}

// Delegate dashboard-only cache clearing to the status mirror.
void MotionCmdPack::clearDashboardCache() { mirror_.clearDashboardCache(); }

// Delegate full mirror cache reset to the status mirror.
void MotionCmdPack::resetCache() { mirror_.resetCache(); }

// Invalidate the prior queue's feedback and timeout when its sequence is restarted.
void MotionCmdPack::resetFlow() {
    flow_.reset();
    probeResult_ = 0;
}

uint32_t MotionCmdPack::getAckSequence() const { return mirror_.ackSeq(); }

uint32_t MotionCmdPack::getAckTick() const { return mirror_.ackTick(); }

}  // namespace zmc
