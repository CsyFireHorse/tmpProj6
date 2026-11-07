#include "srv/MotSrv.hpp"
#include "SrvState.hpp"
#include <chrono>
#include <cstdio>
#include <system_error>

// AI modification: GPT-6 (Codex), 2026-10-26. Finish qcmd-owned waits before shutdown.
// GPT-6 (Codex), 2026-10-27. Fixed pump groups share no device command state.
// Model signature: GPT-6. This service creates, drives and releases the core runtime.
// Parameter preheat added by GPT-6 Codex, 2026-10-12.
// GPT-6 (Codex), 2026-10-29. Protocols and optional startup belong to the application.
namespace zmc {
namespace srv {
MotSrv::MotSrv() {}
MotSrv::~MotSrv() { (void)shutdown(); }

int MotSrv::initialize(const std::vector<std::string>& args) {
    // No core-specific CLI flags exist yet; optional business receives these values later.
    (void)args;
    {
        std::lock_guard<std::mutex> lock(impl_->runtimeMutex);
        if (impl_->runtimeState != Impl::STOPPED) return ZMC_ERR_HOST_HANDLE_ALREADY_OPEN;
        impl_->runtimeState = Impl::STARTING;
        impl_->runtimeError = 0;
    }
    for (auto& slot : impl_->deviceSlot) {
        std::lock_guard<std::mutex> lock(slot.mutex);
        slot.sessions.fill(RpcSession());
    }

    int ret = initializeFiles();
    if (ret == 0) {
        Impl::DeviceSlot& slot = impl_->deviceSlot[0];
        std::lock_guard<std::mutex> lock(slot.mutex);
        // Reset previous CLI defaults without reading or running optional business.
        ret = slot.motion.prepare_config({});
        if (ret == 0) ret = slot.initialize();
    }
    if (ret == 0) {
        std::lock_guard<std::mutex> lock(impl_->runtimeMutex);
        if (impl_->runtimeState != Impl::STARTING) return 0;
        impl_->runtimeState = Impl::RUNNING;
        // std::thread can throw; shutdown must join any pumps already created.
        try {
            for (size_t pumpIdx = 0; pumpIdx < impl_->pumpThread.size(); ++pumpIdx)
                impl_->pumpThread[pumpIdx] = std::thread(&MotSrv::pump, this, pumpIdx);
        } catch (const std::system_error& error) {
            ret = error.code().value();
        }
    }

    if (ret != 0) (void)shutdown();
    return ret;
}

void MotSrv::pump(size_t pumpIdx) {
    while (!exitRequested()) {
        for (size_t devIdx = pumpIdx; devIdx < impl_->deviceSlot.size(); devIdx += Impl::P) {
            const int ret = runOnce(devIdx);
            if (ret != 0 && ret != ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED) {
                // runOnce releases its device lock before stop visits every slot.
                stop(ret);
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void MotSrv::stop(int result) {
    // The core owns exit/error mutation; protocols never touch native state or synchronization.
    // Never retain the service lock while waiting for a device command lock.
    {
        std::lock_guard<std::mutex> lock(impl_->runtimeMutex);
        if (impl_->runtimeError == 0) impl_->runtimeError = result;
        result = impl_->runtimeError;
        if (impl_->runtimeState != Impl::STOPPED) impl_->runtimeState = Impl::STOPPING;
    }
    for (auto& slot : impl_->deviceSlot) {
        std::lock_guard<std::mutex> lock(slot.mutex);
        slot.motion.beginClose();
        if (!slot.cmdIf) continue;
        slot.cmdIf->sendExitFlag();
        // Callers read the stop result and remove their own pending entry after waking.
        slot.cmdIf->cleanupPendingSyncRequests(result);
    }
}
void MotSrv::requestExit() { stop(0); }
bool MotSrv::exitRequested() const {
    std::lock_guard<std::mutex> lock(impl_->runtimeMutex);
    return impl_->runtimeState == Impl::STOPPED || impl_->runtimeState == Impl::STOPPING;
}

int MotSrv::shutdown() {
    requestExit();
    for (auto& thread : impl_->pumpThread)
        if (thread.joinable()) thread.join();
    int cleanup = 0;
    for (auto& slot : impl_->deviceSlot) {
        std::lock_guard<std::mutex> lock(slot.mutex);
        const int script_cleanup = slot.cmdIf ? slot.cmdIf->scriptSweep() : 0;
        if (slot.cmdIf) slot.cmdIf->resetCache();
        const int motion_cleanup = slot.motion.closeAll();
        const int ret = motion_cleanup != 0 ? motion_cleanup : script_cleanup;
        if (cleanup == 0) cleanup = ret;
        slot.sessions.fill(RpcSession());
        slot.cmdIf.reset();
    }
    std::lock_guard<std::mutex> lock(impl_->runtimeMutex);
    impl_->runtimeState = Impl::STOPPED;
    if (cleanup != 0) std::fprintf(stderr, "Core cleanup failed: %d\n", cleanup);
    return impl_->runtimeError != 0 ? impl_->runtimeError : cleanup;
}
}  // namespace srv
}  // namespace zmc
