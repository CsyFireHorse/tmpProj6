#ifndef _ZMC_MOTION_CMD_IO_HPP_
#define _ZMC_MOTION_CMD_IO_HPP_

// Typed asynchronous alias-based IO commands.

#include <cstdint>

#include "qcmd/MotionCmdScript.hpp"

namespace zmc {

class MotionCmdIo : public MotionCmdScript {
   public:
    MotionCmdIo() : MotionCmdScript() {}

    // Enqueue one AxisWithFb motion-parameter query.
    int requestAxisInfo(int axisId);
    // Submit one synchronous numeric axis read. Model signature: GPT-6 Codex.
    int requestReadParameter(int32_t axisId, int32_t parameterNumber, uint32_t requestId);
    // Submit one numeric write with an execution ACK. GPT-6 Codex.
    int requestWriteParameter(int32_t axisId, int32_t parameterNumber, double value,
                              uint32_t requestId);
    // Enqueue one digital-input read; a nonzero request ID sends it immediately.
    int requestReadDi(uint32_t ioNumber, uint32_t requestId = 0);
    // Enqueue one bit-packed digital-input range read request.
    int requestReadDiMulti(uint32_t startIoNumber, uint32_t endIoNumber);
    // Enqueue one digital-output read; a nonzero request ID sends it immediately.
    int requestReadDo(uint32_t ioNumber, uint32_t requestId = 0);
    // Enqueue one bit-packed digital-output range read request.
    int requestReadDoMulti(uint32_t startIoNumber, uint32_t endIoNumber);
    // Enqueue one digital-output write request.
    int writeDo(uint32_t ioNumber, bool value);
    // Enqueue one bit-packed digital-output range write request.
    int writeDoMulti(uint32_t startIoNumber, uint32_t endIoNumber, uint32_t value);
    // Enqueue one analog-input read; a nonzero request ID sends it immediately.
    int requestReadAi(uint32_t ioNumber, uint32_t requestId = 0);
    // Enqueue one analog-output read; a nonzero request ID sends it immediately.
    int requestReadAo(uint32_t ioNumber, uint32_t requestId = 0);
    // Enqueue one analog-output write request.
    int writeAo(uint32_t ioNumber, uint32_t value);
    // Enqueue one virtual digital-input injection request.
    int writeVirtualDi(uint32_t ioNumber, bool value);
    // Enqueue one virtual analog-input injection request.
    int writeVirtualAi(uint32_t ioNumber, uint32_t value);
};

}  // namespace zmc

#endif  // _ZMC_MOTION_CMD_IO_HPP_
