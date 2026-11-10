#ifndef ZMC_MOTION_CMD_FLOW_HPP_
#define ZMC_MOTION_CMD_FLOW_HPP_

#include <cstdint>

// Drafted by GPT-6 (Codex), 2026-11-09. Caller-serialized heartbeat flow control.
namespace zmc {

class MotionCmdFlow {
   public:
    enum class State { Paused, Running, Stalled };

    // Advance one round using one send sequence snapshot; return whether to send a heartbeat.
    bool step(uint16_t sendSeq, uint16_t rcvSeq) {
        const State previous = state_;
        if (sendSeq == 0) {
            // Invalidate the old baseline once; repeated zero snapshots keep the retry interval.
            if (rcvSeq_ != 0 || stallCount_ != 0) probeCount_ = kLowProbeInterval;
            rcvSeq_ = 0;
            stallCount_ = 0;
            state_ = State::Paused;
        } else {
            const uint32_t distance =
                (static_cast<uint32_t>(rcvSeq) + 65535U - rcvSeq_) % 65535U;
            const bool progress = rcvSeq != 0 &&
                (rcvSeq_ == 0 || (distance != 0 && distance <= 32767U));
            if (progress) rcvSeq_ = rcvSeq;
            if (progress) stallCount_ = 0;
            else if (stallCount_ <= kAckStallLimit) ++stallCount_;
            const uint32_t gap =
                (static_cast<uint32_t>(sendSeq) + 65535U - rcvSeq_) % 65535U;
            if (stallCount_ > kAckStallLimit) state_ = State::Stalled;
            else if (rcvSeq_ != 0 && gap <= kFlowLagLimit &&
                     (previous == State::Running || progress)) state_ = State::Running;
            else state_ = State::Paused;
        }
        if (state_ == State::Running) return true;
        if (previous == State::Running) probeCount_ = kLowProbeInterval;
        if (probeCount_ < kLowProbeInterval) ++probeCount_;
        if (probeCount_ < kLowProbeInterval) return false;
        probeCount_ = 0;
        return true;
    }

    // Return the current flow state without advancing its counters.
    State state() const { return state_; }

    // Restore the initial state and schedule an immediate heartbeat.
    void reset() {
        state_ = State::Paused;
        rcvSeq_ = 0;
        stallCount_ = 0;
        probeCount_ = kLowProbeInterval;
    }

   private:
    // Units: packets, heartbeat rounds, heartbeat rounds.
    enum { kFlowLagLimit = 40, kAckStallLimit = 40, kLowProbeInterval = 20 };
    State state_ = State::Paused;
    uint16_t rcvSeq_ = 0;
    unsigned int stallCount_ = 0;
    unsigned int probeCount_ = kLowProbeInterval;
};

}  // namespace zmc
#endif
