#include "srv/SrvBase.hpp"
#include "SrvState.hpp"
#include <new>

// Model signature: GPT-6. Business state stays inside the service implementation.
// GPT-6 (Codex), 2026-10-27. Resolve each session under its owning device lock.
namespace zmc {
namespace srv {
namespace {
// A bounded decimal device index cannot overflow while parsing.
int deviceIndex(const std::string& device, size_t limit, size_t& index) {
    if (device.empty()) return ZMC_ERR_HOST_INVALID_ARG;
    index = 0;
    for (char digit : device) {
        if (digit < '0' || digit > '9') return ZMC_ERR_HOST_INVALID_ARG;
        index = index * 10 + static_cast<size_t>(digit - '0');
        if (index >= limit) return ZMC_ERR_HOST_INVALID_ARG;
    }
    return 0;
}
}  // namespace

// Each service owns its existing private runtime. GPT-6 (Codex), 2026-10-29.
SrvBase::SrvBase() : impl_(new Impl()) {}
SrvBase::~SrvBase() = default;

int SrvBase::Impl::DeviceSlot::initialize() {
    if (cmdIf) return 0;
    cmdIf.reset(new (std::nothrow) MotionCmdInterface(motion.command_interface()));
    if (!cmdIf) return ZMC_ERR_HOST_MALLOC_FAILED;
    cmdIf->initAxisInfo(motion.axis_count());
    return 0;
}

int SrvBase::Impl::lockSession(const RpcSession& session, DeviceSlot*& slot,
                              std::unique_lock<std::mutex>& lock) {
    if (session == 0 || sessionDevice(session) >= N)
        return ZMC_ERR_HOST_INVALID_HANDLE;
    DeviceSlot& device = deviceSlot[sessionDevice(session)];
    lock = std::unique_lock<std::mutex>(device.mutex);
    if (device.motion.deviceState() != DeviceState::Open ||
        device.sessions[sessionIndex(session)] != session)
        return ZMC_ERR_HOST_INVALID_HANDLE;
    slot = &device;
    return 0;
}

// Publish only after device open succeeds; joining preserves the runtime.
int SrvBase::open(const std::string& device, RpcSession& session) {
    session = RpcSession();
    size_t devIdx = 0;
    const int parsed = deviceIndex(device, Impl::N, devIdx);
    if (parsed != 0) return parsed;
    Impl::DeviceSlot& slot = impl_->deviceSlot[devIdx];
    std::lock_guard<std::mutex> lock(slot.mutex);
    {
        std::lock_guard<std::mutex> runtimeLock(impl_->runtimeMutex);
        if (impl_->runtimeState != Impl::RUNNING) return ZMC_ERR_HOST_NOT_OPEN;
    }
    size_t index = 0;
    while (index < slot.sessions.size() && slot.sessions[index] != 0) ++index;
    if (index == slot.sessions.size()) return ZMC_ERR_HOST_SESSION_CAPACITY;
    if (!slot.motion.is_device_open()) {
        const int ret = slot.motion.openDevice(device);
        if (ret != 0) return ret;
    }
    const int ret = slot.initialize();
    if (ret != 0) return ret;
    slot.nextSerial = (slot.nextSerial + 1) & (UINT64_MAX >> 7);
    if (slot.nextSerial == 0) ++slot.nextSerial;
    session = makeSession(slot.nextSerial, devIdx, index);
    slot.sessions[index] = session;
    return 0;
}

// GPT-6 (Codex), 2026-10-29. Configuration never escapes its device ownership boundary.
int SrvBase::prepareConfig(RpcSession session, const std::vector<std::string>& args) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(session, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.prepare_config(args);
}

// Session revocation never changes device, queue or mirror state.
int SrvBase::close(RpcSession session) {
    if (session == 0 || sessionDevice(session) >= Impl::N)
        return ZMC_ERR_HOST_INVALID_HANDLE;
    Impl::DeviceSlot& slot = impl_->deviceSlot[sessionDevice(session)];
    std::lock_guard<std::mutex> lock(slot.mutex);
    RpcSession& registered = slot.sessions[sessionIndex(session)];
    if (registered != session) return ZMC_ERR_HOST_INVALID_HANDLE;
    registered = RpcSession();
    return 0;
}

int SrvBase::checkSession(RpcSession session) const {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    return impl_->lockSession(session, slot, lock);
}

// Reuse session validation; the echo only writes caller storage. GPT-6 Codex.
int SrvBase::mailboxPing(RpcSession s, uint32_t pingValue, uint32_t& pongValue) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.mailbox_ping(pingValue, &pongValue);
}

int SrvBase::getIdentity(RpcSession s, SrvIdentity& out) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;

    EcatMotionFacade& m = slot->motion;
    out.version = m.version();
    out.author = m.about_author();
    out.description = m.about_description();
    out.code_date = m.about_code_date();
    out.device_name = m.device_name();
    out.channel_name = m.channel_name();
    out.eni_name = m.eni_name();
    out.ini_name = m.ini_name();
    out.eni_hash = m.current_eni_hash();
    out.ini_hash = m.current_ini_hash();
    out.selected_eni_path = m.selected_eni_path();
    out.selected_eni_hash = m.selected_eni_hash();
    out.selected_ini_path = m.selected_ini_path();
    out.selected_ini_hash = m.selected_ini_hash();
    out.controller_id = "ZML-RPC-0001";
    out.available_channel_names = m.available_channel_names();
    out.device_open = m.is_device_open();
    out.channel_open = m.is_channel_open();
    out.ecat_running = m.is_ecat_running();
    return 0;
}

// Inspect one logical slot without changing its device or session state.
int SrvBase::getDeviceOpen(const std::string& device, bool& opened) {
    size_t devIdx = 0;
    opened = false;
    const int ret = deviceIndex(device, Impl::N, devIdx);
    if (ret != 0) return ret;
    Impl::DeviceSlot& slot = impl_->deviceSlot[devIdx];
    std::lock_guard<std::mutex> lock(slot.mutex);
    opened = slot.motion.is_device_open();
    return 0;
}

// Enumerate fixed logical slots without probing hardware or opening devices.
int SrvBase::scanDevice(std::vector<SrvDeviceEntry>& out) {
    out.clear();
    for (size_t devIdx = 0; devIdx < Impl::N; ++devIdx) {
        Impl::DeviceSlot& slot = impl_->deviceSlot[devIdx];
        std::lock_guard<std::mutex> lock(slot.mutex);
        out.push_back(SrvDeviceEntry{std::to_string(devIdx), slot.motion.device_name()});
    }
    return 0;
}

// Preserve the channel compatibility operation.
int SrvBase::openChannel(RpcSession s, const std::string& netname) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.openChannel(netname);
}

// Preserve the channel compatibility operation.
int SrvBase::closeChannel(RpcSession s) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.closeChannel();
}

// Read bus running state.
int SrvBase::busIsRunning(RpcSession s, bool& running) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.bus_is_running(&running);
}

// Request a master state.
int SrvBase::requestMasterState(RpcSession s, uint8_t state) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.request_master_state(state);
}

// Read requested master state.
int SrvBase::getMasterReqState(RpcSession s, uint8_t& state) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.get_master_req_state(&state);
}

// Read actual master state.
int SrvBase::getMasterState(RpcSession s, uint8_t& state) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.get_master_state(&state);
}

// Set process-image enable state.
int SrvBase::piEnableSet(RpcSession s, bool enable) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.pi_enable_set(enable);
}

// Read process-image enable state.
int SrvBase::piEnableGet(RpcSession s, bool& enable) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.pi_enable_get(&enable);
}

// Read configured slave count.
int SrvBase::getSlaveCount(RpcSession s, uint16_t& count) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.get_slave_count(&count);
}

// Read active slave count.
int SrvBase::getActiveSlaveCount(RpcSession s, uint16_t& count) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.get_active_slave_count(&count);
}

// Read one slave state.
int SrvBase::getSlaveState(RpcSession s, uint16_t slave_id, uint8_t& state) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.get_slave_state(slave_id, &state);
}

// Clear one slave state error.
int SrvBase::clearSlaveStateError(RpcSession s, uint16_t slave_id) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.clear_slave_state_error(slave_id);
}

}  // namespace srv
}  // namespace zmc
