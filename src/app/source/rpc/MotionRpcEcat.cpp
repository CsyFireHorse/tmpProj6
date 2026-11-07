#include "rpc/MotRpc.hpp"
#include "srv/MotSrv.hpp"
#include "rpc/MotionRpcWireTypes.h"
#include "rpc/server.h"
#include <cstring>
#include <tuple>
namespace zmc {
// EtherCAT wire conversion only. Model signature: GPT-6.
MotRpcEcat::MotRpcEcat(const std::string& host, int port) : MotRpcQcmd(host, port) {}
// Join any partially started workers before derived route handlers are destroyed.
MotRpcEcat::~MotRpcEcat() { stop(); }

int MotRpcEcat::bind(srv::MotSrv& service) {
    // Create the listener only after the application's core is initialized.
    server_.reset(new rpc::server(host_, static_cast<uint16_t>(port_)));
    service_ = &service;
    bindBase();
    bindQcmd();
    bindFiles();
    bindEcat();
    return 0;
}

void MotRpcEcat::bindEcat() {
    // Preserve the service result and unsigned echo value. GPT-6 Codex.
    server_->bind("mailbox_ping", [this](RpcSession s, uint32_t pingValue) {
        uint32_t pongValue = 0;
        const int ret = service_->mailboxPing(s, pingValue, pongValue);
        return std::make_tuple(static_cast<int32_t>(ret), pongValue);
    });
    server_->bind("eni_select", [this](RpcSession s, const std::string& path) {
        return static_cast<int32_t>(service_->eni_select(s, path));
    });
    server_->bind("ini_select", [this](RpcSession s, const std::string& path) {
        return static_cast<int32_t>(service_->ini_select(s, path));
    });
    server_->bind("ecat_start",
                  [this](RpcSession s) { return static_cast<int32_t>(service_->ecat_start(s)); });
    server_->bind("ecat_stop",
                  [this](RpcSession s) { return static_cast<int32_t>(service_->ecat_stop(s)); });
    server_->bind("ecat_request_master_state", [this](RpcSession s, uint8_t state) {
        return static_cast<int32_t>(service_->requestMasterState(s, state));
    });
    server_->bind("ecat_pi_enable_set", [this](RpcSession s, bool flag) {
        return static_cast<int32_t>(service_->piEnableSet(s, flag));
    });
    server_->bind("ecat_clear_slave_state_error", [this](RpcSession s, uint16_t slave_id) {
        return static_cast<int32_t>(service_->clearSlaveStateError(s, slave_id));
    });
    server_->bind("ecat_coe_sdo_download", [this](RpcSession s, uint16_t slave_id, uint16_t index,
                                                  uint8_t subindex, uint32_t size, uint32_t data) {
        return static_cast<int32_t>(
            service_->ecat_coe_sdo_download(s, slave_id, index, subindex, size, data));
    });
    server_->bind("ecat_bus_is_running", [this](RpcSession s) {
        bool out = 0;
        const int ret = service_->busIsRunning(s, out);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("ecat_get_master_req_state", [this](RpcSession s) {
        uint8_t out = 0;
        const int ret = service_->getMasterReqState(s, out);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("ecat_get_master_state", [this](RpcSession s) {
        uint8_t out = 0;
        const int ret = service_->getMasterState(s, out);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("ecat_pi_enable_get", [this](RpcSession s) {
        bool out = 0;
        const int ret = service_->piEnableGet(s, out);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("ecat_get_slave_count", [this](RpcSession s) {
        uint16_t out = 0;
        const int ret = service_->getSlaveCount(s, out);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("ecat_get_active_slave_count", [this](RpcSession s) {
        uint16_t out = 0;
        const int ret = service_->getActiveSlaveCount(s, out);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("ecat_get_slave_state", [this](RpcSession s, uint16_t slave_id) {
        uint8_t out = 0;
        const int ret = service_->getSlaveState(s, slave_id, out);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("ecat_coe_sdo_upload", [this](RpcSession s, uint16_t slave_id, uint16_t index,
                                                uint8_t subindex, uint32_t size) {
        uint32_t out = 0;
        const int ret = service_->ecat_coe_sdo_upload(s, slave_id, index, subindex, size, out);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("get_version_info", [this](RpcSession s) {
        RpcVersionInfoWire out = {};
        srv::SrvIdentity value = {};
        const int ret = service_->getIdentity(s, value);
        if (ret == 0) std::strncpy(out.version, value.version.c_str(), sizeof(out.version) - 1);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("get_about", [this](RpcSession s) {
        RpcAboutInfoWire out = {};
        srv::SrvIdentity value = {};
        const int ret = service_->getIdentity(s, value);
        if (ret == 0) std::strncpy(out.about, value.description.c_str(), sizeof(out.about) - 1);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("get_controller_info", [this](RpcSession s) {
        RpcControllerInfoWire out = {};
        srv::SrvIdentity value = {};
        const int ret = service_->getIdentity(s, value);
        if (ret == 0)
            std::strncpy(out.soft_type, value.device_name.c_str(), sizeof(out.soft_type) - 1);
        if (ret == 0)
            std::strncpy(out.soft_version, value.version.c_str(), sizeof(out.soft_version) - 1);
        if (ret == 0)
            std::strncpy(out.controller_id, value.controller_id.c_str(),
                         sizeof(out.controller_id) - 1);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
}
}  // namespace zmc
