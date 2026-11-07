#ifndef _ZMC_MOTION_CMD_MIRROR_HPP_
#define _ZMC_MOTION_CMD_MIRROR_HPP_

#include <cstdint>
#include <deque>
#include <vector>

#include "common/container/FbCmdQueue.hpp"
#include "motbuf/MotCmd.hpp"
#include "motbuf/MotionCmdTypes.hpp"

namespace zmc {

// Status mirror drained from StatusQueue; each axis keeps one tagged state representation.
class MotionStatusMirror {
   private:
    enum AxisStateCacheKind {
        AXIS_STATE_CACHE_NONE,
        AXIS_STATE_CACHE_BASIC,
        AXIS_STATE_CACHE_EXTENDED
    };
    struct AxisStateCache {
        // The discriminator selects the one active representation for this axis.
        AxisState2 state;
        AxisStateCacheKind kind;
        AxisStateCache() : state(), kind(AXIS_STATE_CACHE_NONE) {}
    };

    std::vector<AxisStateCache> axisStates_;
    std::vector<AxisInfo> axisInfos_;
    std::uint64_t axisInfoRefresh_ = 0;
    std::deque<CmdResultInfo> cmdResults_;
    std::deque<CmdResult> cmdConfirms_;
    uint32_t cmdConfirmDropCount_ = 0;
    std::deque<CmdResult> ioResults_;
    uint32_t lastRecvSeq_ = 0;
    uint32_t lastRecvTick_ = 0;
    FlowCtrlStatus flowStatus_{};

    // Keep one refresh bit per axis until the query is submitted.
    void requestAxisInfoRefresh(uint32_t axis_id);

   public:
    MotionStatusMirror();
    // Apply one status packet; return whether it contained a valid flow status.
    bool applyPacket(const StatusPacket& pkt);
    // Clear only the data exposed by dashboard queries while preserving IO and acknowledgements.
    void clearDashboardCache();
    // Reset every mirror cache entry, including IO results and heartbeat acknowledgements.
    void resetCache();
    // Seed configured axes and schedule their parameter preheat.
    void initAxisInfo(int axis_count);
    bool nextAxisInfoRefresh(uint32_t& axis_id) const;
    void finishAxisInfoRefresh(uint32_t axis_id);
    int getAxisState(int axis_id, AxisState& state) const;
    int getAxisState2(int axis_id, AxisState2& state) const;
    int getAxisInfo(int axis_id, AxisInfo& info) const;
    // Read only the current parameter mirror; command types reuse the existing encoding.
    int getAxisParameter(int axis_id, uint16_t command_type, double& value) const;
    int getCmdResults(std::vector<CmdResultInfo>& results) const;
    // Consume confirmations in arrival order; persistent result queries remain independent.
    int getCmdConfirmations(std::vector<CmdResult>& confirms);
    // Consume one cached confirmation in arrival order without refreshing status.
    bool popCmdConfirmation(CmdResult& confirm);
    uint32_t cmdConfirmationDropCount() const { return cmdConfirmDropCount_; }
    // Returns the current or latest terminal drive-home function-block status.
    int getHomeStatus(int axis_id, int& status, int& error_id) const;
    // Copy the current deduplicated IO result snapshot without consuming it.
    int getIoResults(std::vector<CmdResult>& results);
    int getFlowStatus(FlowCtrlStatus& status) const;
    int getFlowConfig(FlowCtrlConfig& config) const;
    uint32_t currentTickForAxis(int axis) const;
    uint8_t currentAxisState(int axis, bool& hasState) const;
    uint32_t ackSeq() const { return lastRecvSeq_; }
    uint32_t ackTick() const { return lastRecvTick_; }
};

}  // namespace zmc
#endif
