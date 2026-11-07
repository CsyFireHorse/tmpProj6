#include "srv/SrvCommand.hpp"
#include "SrvState.hpp"
#include <cstdio>
#include <cstring>

// GPT-6 (Codex), 2026-10-27. Device configuration and mirrors follow the session slot.
// Model signature: GPT-6. Business state stays inside the service implementation.
// Parameter preheat added by GPT-6 Codex, 2026-10-12.
namespace zmc {
namespace srv {
SrvCommand::SrvCommand() {}
SrvCommand::~SrvCommand() {}

// Select the configuration using the owning motion service.
int SrvCommand::eni_select(RpcSession s, const std::string& path) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.eni_select(path);
}

// Select the configuration using the owning motion service.
int SrvCommand::ini_select(RpcSession s, const std::string& path) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.ini_select(path);
}

// Start and bind PDO in one critical section; roll back a failed binding.
int SrvCommand::startup(RpcSession s, const std::string& path) { return start(s, path, false); }

// GPT-6 (Codex), 2026-10-29. Reuse the existing startup, PDO binding and rollback path.
int SrvCommand::startup(RpcSession s, const std::string& eni_path,
                        const std::string& ini_path) {
    return start(s, eni_path, false, ini_path);
}

// Start and bind PDO in one critical section; roll back a failed binding.
int SrvCommand::startup_ini(RpcSession s, const std::string& path) { return start(s, path, true); }

// Start and bind PDO in one critical section; roll back a failed binding.
int SrvCommand::ecat_start(RpcSession s) { return start(s, std::string(), false); }

// Keep authentication, startup and rollback in their original order.
int SrvCommand::start(RpcSession s, const std::string& path, bool use_ini,
                      const std::string& ini_path) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    const int ret = use_ini ? slot->motion.startupIni(path)
                           : slot->motion.startup(path, ini_path);
    if (ret != 0) return ret;
    const int bind_ret = slot->motion.bind_motion_pdo();
    if (bind_ret != 0) {
        const int stop_ret = slot->motion.stop();
        if (stop_ret != 0) std::fprintf(stderr, "Startup rollback failed: %d\n", stop_ret);
        slot->cmdIf->resetCache();
    } else {
        // The configured axis count seeds defaults; heartbeat sends the existing queries.
        slot->cmdIf->initAxisInfo(slot->motion.axis_count());
    }
    return bind_ret;
}

// Cancel local commands before the bus resets its queues; the pump holds the same lock.
int SrvCommand::ecat_stop(RpcSession s) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    const int sweep_ret = slot->cmdIf->scriptSweep();
    if (sweep_ret != 0) return sweep_ret;
    const int ret = slot->motion.stop();
    if (ret == 0) slot->cmdIf->resetCache();
    return ret;
}

// Use RPC integer/size semantics with complete access zero and timeout 1000 ms.
int SrvCommand::ecat_coe_sdo_download(RpcSession s, uint16_t slave_id, uint16_t index,
                                      uint8_t subindex, uint32_t size, uint32_t data) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    if (size != 1U && size != 2U && size != 4U) return ZMC_ERR_RPC_INVALID_ARG;
    return slot->motion.coe_sdo_download(slave_id, index, subindex, 0, &data, size, 1000);
}

// Use RPC integer/size semantics with complete access zero and timeout 1000 ms.
int SrvCommand::ecat_coe_sdo_upload(RpcSession s, uint16_t slave_id, uint16_t index,
                                    uint8_t subindex, uint32_t size, uint32_t& data) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    if (size != 1U && size != 2U && size != 4U) return ZMC_ERR_RPC_INVALID_ARG;
    data = 0;
    uint32_t length = size;
    return slot->motion.coe_sdo_upload(slave_id, index, subindex, 0, &length, &data, 1000);
}

// Read configuration content through its owning service.
int SrvCommand::eni_json(RpcSession s, std::string& text) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.read_eni_json_text(text);
}

// Read configuration content through its owning service.
int SrvCommand::eni_download(RpcSession s, std::string& text) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.read_eni_file(text);
}

// Read configuration content through its owning service.
int SrvCommand::ini_content(RpcSession s, std::string& text) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.read_ini_file(text);
}

// Return value metadata without exposing the IO runtime.
int SrvCommand::io_metadata(RpcSession s, std::vector<IoMetadataEntry>& out, bool& layout_ready) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->motion.get_io_metadata(out, layout_ready);
}

int SrvCommand::session_status(RpcSession s, SrvIdentity& out) {
    const int ret = getIdentity(s, out);
    if (ret != 0) return ret;
    if (!out.device_open) {
        out.device_name.clear();
        out.channel_name.clear();
        out.eni_name.clear();
        out.eni_hash.clear();
        out.available_channel_names.clear();
        out.channel_open = false;
        out.ecat_running = false;
    }
    return 0;
}

// Read the aggregate bus snapshot without nested service locking.
int SrvCommand::ecat_status(RpcSession s, SrvEcatStatus& out) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    EcatMotionFacade& m = slot->motion;
    int ret = m.bus_is_running(&out.bus_is_running);
    if (ret == 0) ret = m.get_master_req_state(&out.master_req_state);
    if (ret == 0) ret = m.get_master_state(&out.master_state);
    if (ret == 0) ret = m.pi_enable_get(&out.pi_enable);
    if (ret == 0) ret = m.get_slave_count(&out.slave_count);
    if (ret == 0) ret = m.get_active_slave_count(&out.active_slave_count);
    return ret;
}

// Preserve HTTP nullable-axis and empty-result aggregation semantics.
int SrvCommand::status_latest(RpcSession s, int32_t axis, SrvLatestStatus& out) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    slot->cmdIf->updateStatusCache();
    AxisState state = {};
    out.has_axis = slot->cmdIf->getAxisState(axis, state) == 0;
    if (out.has_axis) {
        out.axis = RpcAxisState{state.axis_id, state.axis_state, state.error_id, state.tick,
                                state.act_pos, state.set_pos};
    }
    std::vector<CmdResultInfo> results;
    (void)slot->cmdIf->getCmdResults(results);
    out.cmd_results.clear();
    for (const auto& r : results)
        out.cmd_results.push_back(r);
    return 0;
}

// Capture one mirror snapshot; formatting stays in the HTTP adapter.
int SrvCommand::dashboard(RpcSession s, const std::vector<int32_t>& axes, bool include_flow,
                          bool include_results, SrvDashboard& out) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    slot->cmdIf->updateStatusCache();
    out.axes.clear();
    out.cmd_results.clear();
    for (int32_t axis : axes) {
        SrvAxisSnapshot value = {};
        AxisState2 extended = {};
        if (slot->cmdIf->getAxisState2(axis, extended) == 0) {
            value.state = RpcAxisState2{extended.axis_id, extended.axis_state, extended.error_id,
                                        extended.tick, extended.act_pos, extended.set_pos,
                                        extended.set_vel, extended.set_acc, extended.set_position,
                                        extended.act_position};
            value.is_extended = true;
        } else {
            AxisState basic = {};
            if (slot->cmdIf->getAxisState(axis, basic) != 0) continue;
            value.state = RpcAxisState2{basic.axis_id, basic.axis_state, basic.error_id,
                                        basic.tick, basic.act_pos, basic.set_pos, 0.0, 0.0, 0, 0};
        }
        out.axes.push_back(value);
    }
    if (include_flow) {
        FlowCtrlStatus f = {};
        const int ret = slot->cmdIf->getFlowStatus(f);
        if (ret != 0) return ret;
        RpcFlowCtrlStatus flow = {};
        flow.period = f.period;
        flow.batch_size = f.batchSize;
        flow.verbose_flag = f.verboseFlag;
        // Keep the RPC wire layout; progress reuses the former reserved[1..2] bytes.
        flow.reserved[0] = f.reserved;
        std::memcpy(&flow.reserved[1], &f.processed_seq, sizeof(uint16_t));
        flow.latency = f.latency;
        flow.axis_num = f.axis_num;
        flow.cycle_time = f.cycleTime;
        out.flow_status = flow;
    }
    if (include_results) {
        std::vector<CmdResultInfo> results;
        (void)slot->cmdIf->getCmdResults(results);
        for (const auto& r : results)
            out.cmd_results.push_back(r);
    }
    return 0;
}

}  // namespace srv
}  // namespace zmc
