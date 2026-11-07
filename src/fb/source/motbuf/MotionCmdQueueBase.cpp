#include "motbuf/MotionCmdQueueBase.hpp"

namespace zmc {

MotionCmdQueueBase::MotionCmdQueueBase(CmdQueueType& cmdQueue, StatusQueueType& statusQueue,
                                       bool& exitFlag)
    : cmdQueue_(cmdQueue), statusQueue_(statusQueue), exitFlag_(exitFlag) {}

bool MotionCmdQueueBase::push_cmd_packet(const CmdPacket& packet) {
    return cmdQueue_.push(packet);
}

bool MotionCmdQueueBase::pop_status_packet(StatusPacket& packet) {
    return statusQueue_.pop(packet);
}

void MotionCmdQueueBase::request_exit() { exitFlag_ = true; }

bool MotionCmdQueueBase::exit_requested() const { return exitFlag_; }

bool MotionCmdQueueBase::consume_exit_flag() {
    if (!exitFlag_) {
        return false;
    }
    exitFlag_ = false;
    return true;
}

void MotionCmdQueueBase::clear() {
    cmdQueue_.clear();
    statusQueue_.clear();
    exitFlag_ = false;
}

}  // namespace zmc