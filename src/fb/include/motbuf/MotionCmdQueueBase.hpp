#ifndef _ZMC_MOTION_CMD_QUEUE_BASE_HPP_
#define _ZMC_MOTION_CMD_QUEUE_BASE_HPP_

// Minimal MC <-> Server/RPC comm handle (queue refs + exitFlag). Not raw queue typedefs alone.

#include "common/container/FbCmdQueue.hpp"

namespace zmc {

class MotionCmdInterface;

class MotionCmdQueueBase {
    friend class MotionCmdInterface;

   protected:
    CmdQueueType& cmdQueue_;
    StatusQueueType& statusQueue_;
    bool& exitFlag_;

   public:
    MotionCmdQueueBase(CmdQueueType& cmdQueue, StatusQueueType& statusQueue, bool& exitFlag);
    virtual ~MotionCmdQueueBase() = default;

    MotionCmdQueueBase(const MotionCmdQueueBase&) = delete;
    MotionCmdQueueBase& operator=(const MotionCmdQueueBase&) = delete;

    bool push_cmd_packet(const CmdPacket& packet);
    bool pop_status_packet(StatusPacket& packet);
    void request_exit();
    bool exit_requested() const;
    bool consume_exit_flag();
    void clear();
};

using MotionCmdInterfaceBase = MotionCmdQueueBase;

}  // namespace zmc

#endif
