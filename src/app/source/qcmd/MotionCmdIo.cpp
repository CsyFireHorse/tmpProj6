#include "qcmd/MotionCmdIo.hpp"

#include "common/zmc/zmc_predef.h"
#include "fbt_errorcode_def.h"
#include "fbmEnum.h"

namespace zmc {

namespace {

// Build the common wire payload used by every alias-based IO command.
IoCommandParam makeIoCommandParam(uint32_t startIoNumber, uint32_t endIoNumber,
                                  uint32_t value) {
    IoCommandParam param;
    param.start_io_number = static_cast<uint16_t>(startIoNumber);
    param.end_io_number = static_cast<uint16_t>(endIoNumber);
    param.value = value;
    return param;
}

// Check that a public IO number is within the configured logical-port capacity.
bool isWireIoNumber(uint32_t ioNumber) {
    return ioNumber < static_cast<uint32_t>(MC_IO_MAX_COUNT);
}

}  // namespace

int MotionCmdIo::requestAxisInfo(int axisId) {
    if (axisId < 0) return ZMC_ERR_MOTCMD_INVALID_ARG;
    AxisInfoQuery query = {};
    query.axis_id = static_cast<uint32_t>(axisId);
    return appendOrDeferItem(FBM_AXIS_INFO_QUERY, &query, sizeof(query));
}

// Use the same immediate submission as synchronous IO. Model signature: GPT-6 Codex.
int MotionCmdIo::requestReadParameter(int32_t axisId, int32_t parameterNumber,
                                      uint32_t requestId) {
    if (axisId < 0) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const ReadParameterParam param = {requestId, axisId, parameterNumber};
    return submitItem(FBM_READ_PARAMETER, &param, sizeof(param));
}

// Reuse the synchronous read submission path and explicit request ID. GPT-6 Codex.
int MotionCmdIo::requestWriteParameter(int32_t axisId, int32_t parameterNumber, double value,
                                       uint32_t requestId) {
    if (axisId < 0) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const WriteParameterParam param = {requestId, axisId, parameterNumber, value};
    return submitItem(FBM_SET_PARAMETER, &param, sizeof(param));
}

int MotionCmdIo::requestReadDi(uint32_t ioNumber, uint32_t requestId) {
    if (!isWireIoNumber(ioNumber)) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const IoCommandParam param = makeIoCommandParam(ioNumber, ioNumber, requestId);
    if (requestId != 0u) return submitItem(FBM_IO_READ_DI, &param, sizeof(param));
    return appendOrDeferItem(FBM_IO_READ_DI, &param, sizeof(param));
}

int MotionCmdIo::requestReadDiMulti(uint32_t startIoNumber, uint32_t endIoNumber) {
    if (!isWireIoNumber(startIoNumber) || !isWireIoNumber(endIoNumber)) {
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }
    const IoCommandParam param = makeIoCommandParam(startIoNumber, endIoNumber, 0u);
    return appendOrDeferItem(FBM_IO_READ_DI_MULTI, &param, sizeof(param));
}

int MotionCmdIo::requestReadDo(uint32_t ioNumber, uint32_t requestId) {
    if (!isWireIoNumber(ioNumber)) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const IoCommandParam param = makeIoCommandParam(ioNumber, ioNumber, requestId);
    if (requestId != 0u) return submitItem(FBM_IO_READ_DO, &param, sizeof(param));
    return appendOrDeferItem(FBM_IO_READ_DO, &param, sizeof(param));
}

int MotionCmdIo::requestReadDoMulti(uint32_t startIoNumber, uint32_t endIoNumber) {
    if (!isWireIoNumber(startIoNumber) || !isWireIoNumber(endIoNumber)) {
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }
    const IoCommandParam param = makeIoCommandParam(startIoNumber, endIoNumber, 0u);
    return appendOrDeferItem(FBM_IO_READ_DO_MULTI, &param, sizeof(param));
}

int MotionCmdIo::writeDo(uint32_t ioNumber, bool value) {
    if (!isWireIoNumber(ioNumber)) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const IoCommandParam param = makeIoCommandParam(ioNumber, ioNumber, value ? 1u : 0u);
    return appendOrDeferItem(FBM_IO_WRITE_DO, &param, sizeof(param));
}

int MotionCmdIo::writeDoMulti(uint32_t startIoNumber, uint32_t endIoNumber,
                              uint32_t value) {
    if (!isWireIoNumber(startIoNumber) || !isWireIoNumber(endIoNumber)) {
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }
    const IoCommandParam param = makeIoCommandParam(startIoNumber, endIoNumber, value);
    return appendOrDeferItem(FBM_IO_WRITE_DO_MULTI, &param, sizeof(param));
}

int MotionCmdIo::requestReadAi(uint32_t ioNumber, uint32_t requestId) {
    if (!isWireIoNumber(ioNumber)) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const IoCommandParam param = makeIoCommandParam(ioNumber, ioNumber, requestId);
    if (requestId != 0u) return submitItem(FBM_IO_READ_AI, &param, sizeof(param));
    return appendOrDeferItem(FBM_IO_READ_AI, &param, sizeof(param));
}

int MotionCmdIo::requestReadAo(uint32_t ioNumber, uint32_t requestId) {
    if (!isWireIoNumber(ioNumber)) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const IoCommandParam param = makeIoCommandParam(ioNumber, ioNumber, requestId);
    if (requestId != 0u) return submitItem(FBM_IO_READ_AO, &param, sizeof(param));
    return appendOrDeferItem(FBM_IO_READ_AO, &param, sizeof(param));
}

int MotionCmdIo::writeAo(uint32_t ioNumber, uint32_t value) {
    if (!isWireIoNumber(ioNumber)) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const IoCommandParam param = makeIoCommandParam(ioNumber, ioNumber, value);
    return appendOrDeferItem(FBM_IO_WRITE_AO, &param, sizeof(param));
}

int MotionCmdIo::writeVirtualDi(uint32_t ioNumber, bool value) {
    if (!isWireIoNumber(ioNumber)) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const IoCommandParam param = makeIoCommandParam(ioNumber, ioNumber, value ? 1u : 0u);
    return appendOrDeferItem(FBM_IO_WRITE_VIRTUAL_DI, &param, sizeof(param));
}

int MotionCmdIo::writeVirtualAi(uint32_t ioNumber, uint32_t value) {
    if (!isWireIoNumber(ioNumber)) return ZMC_ERR_MOTCMD_INVALID_ARG;
    const IoCommandParam param = makeIoCommandParam(ioNumber, ioNumber, value);
    return appendOrDeferItem(FBM_IO_WRITE_VIRTUAL_AI, &param, sizeof(param));
}

}  // namespace zmc
