#ifndef _ZMC_MOTION_CMD_SCRIPT_HPP_
#define _ZMC_MOTION_CMD_SCRIPT_HPP_
// Deferred script queue + gate; typed API compact-enqueues via appendOrDeferItem.
#include <cstdint>
#include <deque>

#include "qcmd/MotionCmdPack.hpp"

namespace zmc {

// MotionCmdScript: deferred script queue + gate; tail-merge compact enqueue.
class MotionCmdScript : public MotionCmdPack {
   protected:
    enum class ScriptGateType : uint8_t { None = 0, WaitTick, WaitIdle };
    std::deque<CmdPacket> scriptQueue_;
    ScriptGateType scriptGateType_ = ScriptGateType::None;
    int scriptGateAxis_ = 0;
    uint32_t scriptGateTargetTick_ = 0;
    uint32_t scriptGateArmedTick_ = 0;

    // Compact enqueue: merge into non-wait tail when canAppend, else new packet.
    int appendOrDeferItem(uint16_t type, const void* param, uint16_t param_len);
    void enqueueAxisInfoRefreshes();
    int priorityAppendItem(uint16_t type, const void* param, uint16_t param_len);

    void armScriptTickGate(ScriptGateType gateType, uint32_t waitTicks, int axis);
    int enqueueExclusivePacket(uint16_t type, const void* param, uint16_t param_len);
    int enqueueWaitItem(ScriptGateType gateType, uint32_t waitTicks, int axis);
    void abortScript();

   public:
    MotionCmdScript() : MotionCmdPack() {}
    // Push a caller-built packet as-is (no cross-packet merge).
    int pushPacket(const CmdPacket& pkt);
    int scriptSweep();
    int heartbeat();
    int advanceScript();
    // Send a no-op command immediately with a caller-owned confirmation ID.
    int syncPing(uint32_t request_id);
    // Queue a long-packet test command, up to eight transport packets.
    int testLongPacket(const void* data, uint16_t length);
    bool isScriptBusy() const;
    int waitTicks(uint32_t nTick, int axis = 0, uint32_t timeoutMs = 0, uint32_t pollMs = 2);
    int waitSeconds(double seconds, int axis = 0, uint32_t timeoutMs = 0, uint32_t pollMs = 2);
    int waitAxisIdle(int axis = 0, uint32_t timeoutMs = 0, uint32_t pollMs = 2,
                     bool errorAsFail = true);
};

}  // namespace zmc
#endif
