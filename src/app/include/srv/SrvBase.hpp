#ifndef ZMC_SRV_BASE_HPP_
#define ZMC_SRV_BASE_HPP_

#include <memory>
#include "SrvTypes.hpp"
// Model signature: GPT-6.
namespace zmc {
namespace srv {
class SrvFileService;
class SrvQcmd;
class SrvCommand;
class MotSrv;
class SrvBase {
   public:
    // Open or join a device and publish a session.
    int open(const std::string& device, RpcSession& session);
    // Revoke a session without closing the device.
    int close(RpcSession session);
    // Prepare this device from command-line values without starting it.
    // GPT-6 (Codex), 2026-10-29. Use the existing device lock and owned motion runtime.
    int prepareConfig(RpcSession session, const std::vector<std::string>& args);
    // Validate a registered session.
    int checkSession(RpcSession session) const;
    // Echo through the owning motion service. GPT-6 Codex.
    int mailboxPing(RpcSession s, uint32_t pingValue, uint32_t& pongValue);
    // Copy device and build information.
    int getIdentity(RpcSession s, SrvIdentity& out);
    // Read the open state of one logical device without requiring a session.
    int getDeviceOpen(const std::string& device, bool& opened);
    // List existing logical devices without requiring a session.
    int scanDevice(std::vector<SrvDeviceEntry>& out);
    // Preserve the channel compatibility operation.
    int openChannel(RpcSession s, const std::string& netname);
    // Preserve the channel compatibility operation.
    int closeChannel(RpcSession s);
    // Read bus running state.
    int busIsRunning(RpcSession s, bool& running);
    // Request a master state.
    int requestMasterState(RpcSession s, uint8_t state);
    // Read requested master state.
    int getMasterReqState(RpcSession s, uint8_t& state);
    // Read actual master state.
    int getMasterState(RpcSession s, uint8_t& state);
    // Set process-image enable state.
    int piEnableSet(RpcSession s, bool enable);
    // Read process-image enable state.
    int piEnableGet(RpcSession s, bool& enable);
    // Read configured slave count.
    int getSlaveCount(RpcSession s, uint16_t& count);
    // Read active slave count.
    int getActiveSlaveCount(RpcSession s, uint16_t& count);
    // Read one slave state.
    int getSlaveState(RpcSession s, uint16_t slave_id, uint8_t& state);
    // Clear one slave state error.
    int clearSlaveStateError(RpcSession s, uint16_t slave_id);

   private:
    // Construct only through the derived chain.
    SrvBase();
    // Release this service's private context.
    ~SrvBase();

   private:
    struct Impl;
    // GPT-6 (Codex), 2026-10-29. Each service owns its device slots and workers.
    std::unique_ptr<Impl> impl_;
    friend class SrvFileService;
    friend class SrvQcmd;
    friend class SrvCommand;
    friend class MotSrv;
    // Service-local fixture binds independent queues without starting protocol listeners.
    friend class SrvDeviceSlotTest;
};
}  // namespace srv
}  // namespace zmc
#endif
