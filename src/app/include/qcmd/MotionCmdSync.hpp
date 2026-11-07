#ifndef _ZMC_MOTION_CMD_SYNC_HPP_
#define _ZMC_MOTION_CMD_SYNC_HPP_

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <unordered_map>

#include "qcmd/MotionCmdIo.hpp"

// Drafted by GPT-6 (Codex), 2026-10-26.
namespace zmc {

// Callers serialize access with one shared mutex.
// Blocking calls receive its owned lock; ping and idle waits require a positive timeout.
class MotionCmdSync : public MotionCmdIo {
   public:
    // Wait for the existing motion-cycle ping confirmation.
    int ping(uint32_t timeout_ms, std::unique_lock<std::mutex>& lock);

    // Read one numeric parameter through the existing two-word confirmation.
    int readParameter(int32_t axis, int32_t parameter_number, double& value,
                      std::unique_lock<std::mutex>& lock);

    // Wait for the existing numeric parameter write confirmation.
    int writeParameter(int32_t axis, int32_t parameter_number, double value,
                       std::unique_lock<std::mutex>& lock);

    // Read one digital input through a directly submitted command.
    int readDiSync(uint32_t io, bool& value, std::unique_lock<std::mutex>& lock);

    // Read one digital output through a directly submitted command.
    int readDoSync(uint32_t io, bool& value, std::unique_lock<std::mutex>& lock);

    // Read one analog input while preserving all unsigned result bits.
    int readAiSync(uint32_t io, uint32_t& value, std::unique_lock<std::mutex>& lock);

    // Read one analog output while preserving all unsigned result bits.
    int readAoSync(uint32_t io, uint32_t& value, std::unique_lock<std::mutex>& lock);

    // Wait for a newer axis snapshot to report standstill or ErrorStop.
    int waitAxisIdleSync(int32_t axis, uint32_t timeout_ms,
                         std::unique_lock<std::mutex>& lock);

    // Complete requests from confirmations and axis state under the shared mutex.
    void processSyncRequests();

    // Record the stop result and wake pending callers; caller holds the shared mutex.
    void cleanupPendingSyncRequests(int result);

    // Configure the shared timeout for numeric and IO reads; caller holds the shared mutex.
    void setSyncReadTimeout(uint32_t timeout_ms);

    // Return the configured timeout for synchronous reads.
    uint32_t getSyncReadTimeout() const;

   private:
    // Each synchronous caller removes its entry after reading the completion result.
    struct PendingCommand {
        // Command waits complete from confirmations;
        // axis-idle waits complete from the state mirror.
        enum Kind { COMMAND = 0, AXIS_IDLE };
        bool complete = false;
        int result = 0;
        // IO uses the first word; numeric parameters use both. GPT-6 Codex.
        uint32_t value[2] = {};
        // Axis-idle wait descriptor: mirror tick lags must exceed armedTick before completion.
        uint8_t kind = COMMAND;
        int32_t axis = 0;
        uint32_t armedTick = 0;
    };

    // Move the existing request state from SrvBase::Impl.
    std::condition_variable commandChanged;
    std::unordered_map<uint32_t, PendingCommand> pendingCommands;
    uint32_t syncReadTimeoutMs = 500U;
    uint32_t nextCommandRequestId = 0;

    // Register a request while the shared mutex is held, before publishing its command.
    uint32_t beginSyncRequest();

    // Consume submission failure, confirmation, timeout, or stop under the shared mutex.
    int waitSyncRequest(uint32_t request_id, int submit_result, uint32_t timeout_ms,
                        std::unique_lock<std::mutex>& lock);
    // Share request ownership and use the shared read timeout across typed IO reads.
    int readIoSync(uint32_t io, uint16_t type, uint32_t& value,
                   std::unique_lock<std::mutex>& lock);
};

}  // namespace zmc

#endif  // _ZMC_MOTION_CMD_SYNC_HPP_
