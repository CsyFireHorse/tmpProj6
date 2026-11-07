#include "qcmd/MotionCmdSync.hpp"

#include <chrono>
#include <cstring>

#include "fbmEnum.h"
#include "fbt_errorcode_def.h"
#include "impl/fbt_impl_enum_def.h"

// AI modification: GPT-6 (Codex), 2026-10-26. Preserve the existing caller-owned wait lifecycle.
namespace zmc {

uint32_t MotionCmdSync::beginSyncRequest() {
    // Reserve a unique nonzero ID and install its result slot before command submission.
    uint32_t request_id = 0;
    do {
        nextCommandRequestId = (nextCommandRequestId + 1U) & 0x7fffffffU;
        request_id = nextCommandRequestId;
    } while (request_id == 0U || pendingCommands.count(request_id) != 0U);
    pendingCommands.emplace(request_id, PendingCommand());
    return request_id;
}

int MotionCmdSync::waitSyncRequest(uint32_t request_id, int submit_result, uint32_t timeout_ms,
                                   std::unique_lock<std::mutex>& lock) {
    // A failed enqueue cannot produce a confirmation, so return without waiting.
    if (submit_result != 0) return submit_result;

    // The condition-variable wait releases the shared mutex for heartbeat and stop processing.
    const bool completed = commandChanged.wait_for(
        lock, std::chrono::milliseconds(timeout_ms), [this, request_id]() {
            const auto pending = pendingCommands.find(request_id);
            return pending == pendingCommands.end() || pending->second.complete;
        });
    const auto pending = pendingCommands.find(request_id);
    if (pending == pendingCommands.end()) return ZMC_ERR_HOST_NOT_OPEN;
    if (!completed) return ZMC_ERR_HOST_RECV_TIMEOUT;
    // The caller reads this result and removes the request entry.
    return pending->second.result;
}

// Update the shared read timeout while the caller holds the device mutex.
void MotionCmdSync::setSyncReadTimeout(uint32_t timeout_ms) {
    syncReadTimeoutMs = timeout_ms;
}

uint32_t MotionCmdSync::getSyncReadTimeout() const {
    return syncReadTimeoutMs;
}

// Record a stop result for unfinished waits; callers erase their own entries after waking.
void MotionCmdSync::cleanupPendingSyncRequests(int result) {
    bool changed = false;
    // Normal shutdown maps to host-not-open; preserve the error that caused a failed stop.
    const int finish_result = result != 0 ? result : ZMC_ERR_HOST_NOT_OPEN;
    for (auto& pending : pendingCommands) {
        // Preserve the first completion, whether from a confirmation or an earlier stop.
        if (!pending.second.complete) {
            pending.second.result = finish_result;
            pending.second.complete = true;
            changed = true;
        }
    }
    // All request IDs share this condition variable;
    // wake waiters after any incomplete entry changes.
    if (changed) commandChanged.notify_all();
}

int MotionCmdSync::ping(uint32_t timeout_ms, std::unique_lock<std::mutex>& lock) {
    // Register before enqueue so the pump can correlate an immediate confirmation.
    const uint32_t request_id = beginSyncRequest();
    const int result = waitSyncRequest(request_id, syncPing(request_id), timeout_ms, lock);
    // Keep removal with the caller for submission failure, timeout, confirmation, and stop.
    pendingCommands.erase(request_id);
    return result;
}

// Block on the shared condition variable until the axis reaches standstill or ErrorStop.
// GLM-5.3 (opencode), 2026-10-23.
int MotionCmdSync::waitAxisIdleSync(int32_t axis, uint32_t timeout_ms,
                                    std::unique_lock<std::mutex>& lock) {
    if (axis < 0) return ZMC_ERR_MOTCMD_WAIT_INVALID_ARG;
    // Refresh once so the armed tick matches the state observed at registration time.
    updateStatusCache();
    AxisState armed = {};
    const int known = getAxisState(axis, armed);
    if (known != 0) return known;
    const uint32_t request_id = beginSyncRequest();
    auto& pending = pendingCommands.find(request_id)->second;
    // Completion requires a mirror tick beyond the armed snapshot; see processSyncRequests().
    pending.kind = PendingCommand::AXIS_IDLE;
    pending.axis = axis;
    pending.armedTick = armed.tick;
    int result = waitSyncRequest(request_id, 0, timeout_ms, lock);
    // The plain host timeout maps onto the dedicated blocking-wait timeout code.
    if (result == ZMC_ERR_HOST_RECV_TIMEOUT) result = ZMC_ERR_MOTCMD_WAIT_TIMEOUT;
    pendingCommands.erase(request_id);
    return result;
}

// Numeric reads share the existing request and result ownership. GPT-6 Codex.
int MotionCmdSync::readParameter(int32_t axis, int32_t parameter_number, double& value,
                                 std::unique_lock<std::mutex>& lock) {
    const uint32_t request_id = beginSyncRequest();
    const int submitted = requestReadParameter(axis, parameter_number, request_id);
    const int result = waitSyncRequest(request_id, submitted, syncReadTimeoutMs, lock);
    if (result == 0) {
        std::memcpy(&value, pendingCommands.find(request_id)->second.value, sizeof(value));
    }
    pendingCommands.erase(request_id);
    return result;
}

// Writes share the read request's ACK, timeout and cleanup ownership. GPT-6 Codex.
int MotionCmdSync::writeParameter(int32_t axis, int32_t parameter_number, double value,
                                  std::unique_lock<std::mutex>& lock) {
    const uint32_t request_id = beginSyncRequest();
    const int submitted = requestWriteParameter(axis, parameter_number, value, request_id);
    const int result = waitSyncRequest(request_id, submitted, 100U, lock);
    pendingCommands.erase(request_id);
    return result;
}

// Direct reads share ping's registration, confirmation and cleanup ownership.
int MotionCmdSync::readIoSync(uint32_t io, uint16_t type, uint32_t& value,
                              std::unique_lock<std::mutex>& lock) {
    const uint32_t request_id = beginSyncRequest();
    int submitted = ZMC_ERR_MOTCMD_INVALID_ARG;
    switch (type) {
        case FBM_IO_READ_DI: submitted = requestReadDi(io, request_id); break;
        case FBM_IO_READ_DO: submitted = requestReadDo(io, request_id); break;
        case FBM_IO_READ_AI: submitted = requestReadAi(io, request_id); break;
        case FBM_IO_READ_AO: submitted = requestReadAo(io, request_id); break;
    }
    const int result = waitSyncRequest(request_id, submitted, syncReadTimeoutMs, lock);
    if (result == 0) value = pendingCommands.find(request_id)->second.value[0];
    pendingCommands.erase(request_id);
    return result;
}

int MotionCmdSync::readDiSync(uint32_t io, bool& value, std::unique_lock<std::mutex>& lock) {
    uint32_t bits = 0;
    const int ret = readIoSync(io, FBM_IO_READ_DI, bits, lock);
    if (ret == 0) value = bits != 0;
    return ret;
}

int MotionCmdSync::readDoSync(uint32_t io, bool& value, std::unique_lock<std::mutex>& lock) {
    uint32_t bits = 0;
    const int ret = readIoSync(io, FBM_IO_READ_DO, bits, lock);
    if (ret == 0) value = bits != 0;
    return ret;
}

int MotionCmdSync::readAiSync(uint32_t io, uint32_t& value, std::unique_lock<std::mutex>& lock) {
    return readIoSync(io, FBM_IO_READ_AI, value, lock);
}

int MotionCmdSync::readAoSync(uint32_t io, uint32_t& value, std::unique_lock<std::mutex>& lock) {
    return readIoSync(io, FBM_IO_READ_AO, value, lock);
}

void MotionCmdSync::processSyncRequests() {
    // popCmdConfirmation reads only cached data; preserve the prior bulk getter's refresh.
    if (!pendingCommands.empty()) updateStatusCache();
    bool notified = false;
    CmdResult confirm = {};
    // Drain unmatched and late confirmations too, so they do not remain in the shared cache.
    while (popCmdConfirmation(confirm)) {
        const auto pending = pendingCommands.find(confirm.id);
        if (pending == pendingCommands.end() || pending->second.complete) continue;
        // Translate each matching confirmation into the result observed by its waiter.
        pending->second.result = confirm.status == FB_STAT_DONE
                                      ? 0
                                      : (confirm.error_code != 0U ? confirm.error_code
                                                                   : ZMC_ERR_HOST_INTERNAL);
        // Match both unsigned result words before waking the caller. GPT-6 Codex.
        pending->second.value[0] = static_cast<uint32_t>(confirm.data);
        pending->second.value[1] = static_cast<uint32_t>(confirm.data_ext);
        pending->second.complete = true;
        notified = true;
    }
    // Axis-idle waits complete from the mirror instead of a command confirmation.
    for (auto& pending : pendingCommands) {
        if (pending.second.complete ||
            pending.second.kind != PendingCommand::AXIS_IDLE) {
            continue;
        }
        AxisState value = {};
        if (getAxisState(pending.second.axis, value) != 0) continue;
        // Mirror tick must advance past the armed snapshot, so a queued move is observed first.
        if (value.tick <= pending.second.armedTick) continue;
        const uint8_t state = static_cast<uint8_t>(value.axis_state & 0x0FU);
        if (state == mcStandstill) {
            pending.second.result = 0;
        } else if (state == mcErrorStop) {
            // ErrorStop ends the wait with the dedicated axis-error code.
            pending.second.result = ZMC_ERR_MOTCMD_WAIT_AXIS_ERROR;
        } else {
            continue;
        }
        pending.second.complete = true;
        notified = true;
    }
    // Wake all waiters once after the batch; each checks whether its own ID was completed.
    if (notified) commandChanged.notify_all();
}

}  // namespace zmc
