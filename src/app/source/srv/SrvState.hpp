#ifndef ZMC_SRV_STATE_HPP_
#define ZMC_SRV_STATE_HPP_
#include <mutex>
#include <thread>
#include <array>
#include <memory>
#include "srv/SrvBase.hpp"
#include "srv/HttpFileManager.hpp"
#include "EcatMotionFacade.hpp"
#include "qcmd/MotionCmdInterface.hpp"
#include "fbt_errorcode_def.h"
namespace zmc {
namespace srv {
// Embed the device and session slot in the existing scalar credential.
inline RpcSession makeSession(uint64_t serial, size_t device, size_t index) {
    return (serial << 7) | (static_cast<uint64_t>(index) << 4) | device;
}
inline size_t sessionDevice(RpcSession session) { return static_cast<size_t>(session & 15U); }
inline size_t sessionIndex(RpcSession session) { return static_cast<size_t>((session >> 4) & 7U); }
// AI modification: GPT-6 (Codex), 2026-10-27. Fixed slots own device command state.
// Source-only fixed state owns each backend and its single command interface.
// Model signature: GPT-6. Only srv serializes registry and command access.
struct SrvBase::Impl {
    enum RuntimeState { STOPPED, STARTING, RUNNING, STOPPING };
    enum { N = 10, P = 2, SESSION_COUNT = 8 };
    static_assert(P > 0 && P <= N && N <= 16, "Invalid device/pump capacity");

    // Session revocation leaves this device's queues and command adapter alive.
    typedef zmc::DeviceState DeviceState;
    struct DeviceSlot {
        EcatMotionFacade motion;
        std::array<RpcSession, SESSION_COUNT> sessions = {};
        uint64_t nextSerial = 0;
        std::mutex mutex;
        std::unique_ptr<MotionCmdInterface> cmdIf;

        // Bind the existing device queues once, before publishing sessions.
        int initialize();
    };

    std::array<DeviceSlot, N> deviceSlot;
    std::array<std::thread, P> pumpThread;
    std::mutex runtimeMutex;
    RuntimeState runtimeState = STOPPED;
    int runtimeError = 0;
    HttpFileManager files{"server_files"};

    // Hold the device lock through validation and the caller's complete operation.
    int lockSession(const RpcSession& session, DeviceSlot*& slot,
                    std::unique_lock<std::mutex>& lock);
};
}  // namespace srv
}  // namespace zmc
#endif
