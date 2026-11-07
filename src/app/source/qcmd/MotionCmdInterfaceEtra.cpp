#include "qcmd/MotionCmdInterfaceExtra.hpp"

#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

#include "fbt_errorcode_def.h"

namespace zmc {
inline int motcmdRet(MC_ERROR_CODE code) { return static_cast<int>(code); }

MotionCmdInterface::MotionCmdInterface(CmdQueueType& cmdQueue, StatusQueueType& statusQueue,
                                       bool& exitFlag)
    : MotionCmdHerit(), cmdQueue_(cmdQueue), statusQueue_(statusQueue), exitFlag_(exitFlag) {}

MotionCmdInterface::MotionCmdInterface(MotionCmdInterfaceBase& base)
    : MotionCmdInterface(base.cmdQueue_, base.statusQueue_, base.exitFlag_) {}

// Convert the queue transport result to the public motion-command error contract.
int MotionCmdInterface::send(const CmdPacket& packet) {
    return cmdQueue_.push(packet) ? motcmdRet(ZMC_NO_ERROR)
                                  : motcmdRet(ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED);
}

// The caller serializes producers; the consumer can only increase available space.
int MotionCmdInterface::sendPackets(const CmdPacket* packets, size_t count) {
    if (cmdQueue_.available() < count) return motcmdRet(ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED);
    for (size_t i = 0; i < count; ++i) {
        const int ret = send(packets[i]);
        if (ret != 0) return ret;
    }
    return motcmdRet(ZMC_NO_ERROR);
}

// Supply one status packet to the transport-independent mirror update path.
bool MotionCmdInterface::tryPopStatus(StatusPacket& packet) { return statusQueue_.pop(packet); }

uint16_t MotionCmdInterface::lastSentSequence() const { return cmdQueue_.lastSequence(); }

}  // namespace zmc
