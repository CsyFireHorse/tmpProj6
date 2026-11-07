#ifndef _ZMC_MOTION_CMD_INTERFACE_EXTRA_HPP_
#define _ZMC_MOTION_CMD_INTERFACE_EXTRA_HPP_

#include "qcmd/MotionCmdInterface.hpp"
#include "motbuf/MotionCmdQueueBase.hpp"

namespace zmc {

class MotionCmdInterface : public MotionCmdHerit {
   private:
    CmdQueueType& cmdQueue_;
    StatusQueueType& statusQueue_;
    bool& exitFlag_;

   public:
    // Bind directly to the legacy queue and exit references.
    MotionCmdInterface(CmdQueueType& cmdQueue, StatusQueueType& statusQueue, bool& exitFlag);
    // Bind to the queues and exit flag exposed by the runtime command base.
    explicit MotionCmdInterface(MotionCmdInterfaceBase& base);
    // Send one packet and convert queue backpressure to a motion-command error code.
    int send(const CmdPacket& packet) override;
    // Preflight the whole group before publishing any packet; no additional producer lock.
    int sendPackets(const CmdPacket* packets, size_t count) override;
    uint16_t lastSentSequence() const override;
    // Pop one status packet for MotionCmdPack status-cache draining.
    bool tryPopStatus(StatusPacket& packet) override;
    // Request termination through the original shared exit flag.
    void sendExitFlag() { exitFlag_ = true; }
    // Return the current shared exit-flag value.
    bool exitFlag() const { return exitFlag_; }
};

}  // namespace zmc
#endif
