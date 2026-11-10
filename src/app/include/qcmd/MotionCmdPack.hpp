#ifndef _ZMC_MOTION_CMD_PACK_HPP_
#define _ZMC_MOTION_CMD_PACK_HPP_
// AI modification: GPT-6 Codex, 2026-10-12. Low-frequency parameter mirror refresh.
// Immediate send + status mirror; no script queue. Not for FbMoveWrapV3.hpp / AxisFbGraph.hpp.
#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

#include "common/container/FbCmdQueue.hpp"
#include "motbuf/MotCmd.hpp"
#include "motbuf/MotionCmdTypes.hpp"
#include "qcmd/MotionCmdMirror.hpp"
#include "qcmd/MotionCmdFlow.hpp"

namespace zmc {

class MotionCmdPort {
   public:
    virtual ~MotionCmdPort() {}

    // Sends one complete command packet and propagates the transport result.
    virtual int send(const CmdPacket& packet) = 0;

    // Send a complete consecutive group under the caller's existing producer serialization.
    virtual int sendPackets(const CmdPacket* packets, size_t count) = 0;

    virtual uint16_t lastSentSequence() const = 0;

    // Optionally produces one status packet.
    virtual bool tryPopStatus(StatusPacket& packet) {
        (void)packet;
        return false;
    }
};

// MotionCmdPack: immediate send + status mirror cache; no script queue.
class MotionCmdPack : public MotionCmdPort {
   protected:
    MotionStatusMirror mirror_;

    // GPT-6 (Codex), 2026-11-09. Flow uses the caller's existing serialization.
    MotionCmdFlow flow_;
    int probeResult_ = 0;
    bool flowPaused() const { return flow_.state() != MotionCmdFlow::State::Running; }

    // Shared bounded packetization for immediate requests and deferred script items.
    int buildItemPackets(uint16_t type, const void* param, uint16_t param_len, CmdPacket* packets,
                         size_t capacity, size_t& count);
    int sendPacketGroup(CmdPacket* packets, size_t count);
    int pumpHeartbeat();

   public:
    MotionCmdPack();
    // Immediate send path (user-facing; no script queue).
    int sendPacket(CmdPacket pkt);
    int submitItem(uint16_t type, const void* param, uint16_t param_len);
    int _heartbeat();
    int setFlowConfig(const FlowCtrlConfig& cfg);
    int flush();
    int autoFlush();
    void setFlushLevel(size_t level);
    void setAutoFlushEnabled(bool enabled);
    bool isAutoFlushEnabled() const;
    int getAxisState(int axis_id, AxisState& state);
    int getAxisState2(int axis_id, AxisState2& state);
    int getAxisInfo(int axis_id, AxisInfo& info);
    void initAxisInfo(int axis_count);
    int getAxisParameter(int axis_id, uint16_t command_type, double& value) const;
    int getCmdResults(std::vector<CmdResultInfo>& results);
    int getCmdConfirmations(std::vector<CmdResult>& confirms);
    // Consume one cached confirmation in arrival order without refreshing status.
    bool popCmdConfirmation(CmdResult& confirm);
    uint32_t getCmdConfirmationDropCount() const { return mirror_.cmdConfirmationDropCount(); }
    // Returns the current or latest terminal drive-home function-block status.
    int getHomeStatus(int axis_id, int& status, int& error_id);
    // Copy the current deduplicated IO result snapshot without consuming it.
    int getIoResults(std::vector<CmdResult>& results);
    int getFlowStatus(FlowCtrlStatus& status) const;
    int getFlowConfig(FlowCtrlConfig& config) const;
    // Apply one externally supplied status packet.
    void applyStatusPacket(const StatusPacket& packet);
    void updateStatusCache();
    // Clear only the cached data returned by dashboard-related HTTP queries.
    void clearDashboardCache();
    // Clear mirrored status and ACK state.
    void resetCache();
    // Reset flow after the command queue sequence is restarted.
    void resetFlow();
    uint32_t getAckSequence() const;
    uint32_t getAckTick() const;
};

}  // namespace zmc
#endif
