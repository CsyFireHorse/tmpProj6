#include "EcatMotionFacade.hpp"
#include "EcatMotion.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"

// AI modification: GPT-6 (Codex), 2026-10-28. Keep the runtime private to its slot.
namespace zmc {

struct EcatMotionFacade::Impl {
    EcatMotion motion;
};

EcatMotionFacade::EcatMotionFacade() : impl_(new Impl()) {}
EcatMotionFacade::~EcatMotionFacade() {}

int EcatMotionFacade::openDevice(const std::string& device) {
    (void)device;
    state_ = DeviceState::Open;
    return 0;
}

int EcatMotionFacade::closeAll() {
    const int ret = impl_->motion.reset();
    state_ = DeviceState::Closed;
    return ret;
}

int EcatMotionFacade::openChannel(const std::string& netname) {
    (void)netname;
    return 0;
}

int EcatMotionFacade::closeChannel() { return 0; }

int EcatMotionFacade::startup(const std::string& eni_path, const std::string& ini_path) {
    if (!is_device_open()) return ZMC_ERR_HOST_INVALID_HANDLE;
    return impl_->motion.startup(eni_path, ini_path);
}

int EcatMotionFacade::startupIni(const std::string& ini_path) {
    const int ret = impl_->motion.validate_startup_ini(ini_path);
    if (ret != 0) return ret;
    // Keep the validated path stable if it aliases runtime-owned storage.
    const std::string validated_ini = ini_path;
    return startup(impl_->motion.selected_eni_path(), validated_ini);
}

void EcatMotionFacade::beginClose() {
    if (state_ == DeviceState::Open) state_ = DeviceState::Closing;
}

bool EcatMotionFacade::is_device_open() const { return state_ != DeviceState::Closed; }
bool EcatMotionFacade::is_channel_open() const { return is_device_open(); }

const std::string& EcatMotionFacade::device_name() const {
    static const std::string name(ZMC_DEFAULT_DEVICE_NAME);
    return name;
}

const std::string& EcatMotionFacade::channel_name() const { return impl_->motion.net_name(); }

std::vector<std::string> EcatMotionFacade::available_channel_names() const {
    std::vector<std::string> names;
    if (is_device_open()) names.push_back(ZMC_DEFAULT_NET_NAME);
    return names;
}

// Forward only the operations required by the owning service.
int EcatMotionFacade::prepare_config(const std::vector<std::string>& args) {
    return impl_->motion.prepare_config(args);
}
PdoPowerRunMode EcatMotionFacade::run_mode() const { return impl_->motion.run_mode(); }
int EcatMotionFacade::port() const { return impl_->motion.port(); }
const std::string& EcatMotionFacade::eni_path() const { return impl_->motion.eni_path(); }
const std::string& EcatMotionFacade::ini_path() const { return impl_->motion.ini_path(); }
const std::string& EcatMotionFacade::startup_script() const {
    return impl_->motion.startup_script();
}
int EcatMotionFacade::eni_select(const std::string& eni_path) {
    return impl_->motion.eni_select(eni_path);
}
int EcatMotionFacade::ini_select(const std::string& ini_path) {
    return impl_->motion.ini_select(ini_path);
}
int EcatMotionFacade::stop() { return impl_->motion.stop(); }
int EcatMotionFacade::bind_motion_pdo() { return impl_->motion.bind_motion_pdo(); }
int EcatMotionFacade::axis_count() const { return impl_->motion.axis_count(); }
MotionCmdQueueBase& EcatMotionFacade::command_interface() {
    return impl_->motion.command_interface();
}
const std::string& EcatMotionFacade::version() const { return impl_->motion.version(); }
const std::string& EcatMotionFacade::about_author() const { return impl_->motion.about_author(); }
const std::string& EcatMotionFacade::about_description() const {
    return impl_->motion.about_description();
}
const std::string& EcatMotionFacade::about_code_date() const {
    return impl_->motion.about_code_date();
}
const std::string& EcatMotionFacade::eni_name() const { return impl_->motion.eni_name(); }
const std::string& EcatMotionFacade::ini_name() const { return impl_->motion.ini_name(); }
const std::string& EcatMotionFacade::current_eni_hash() const {
    return impl_->motion.current_eni_hash();
}
const std::string& EcatMotionFacade::current_ini_hash() const {
    return impl_->motion.current_ini_hash();
}
const std::string& EcatMotionFacade::selected_eni_path() const {
    return impl_->motion.selected_eni_path();
}
const std::string& EcatMotionFacade::selected_ini_path() const {
    return impl_->motion.selected_ini_path();
}
const std::string& EcatMotionFacade::selected_eni_hash() const {
    return impl_->motion.selected_eni_hash();
}
const std::string& EcatMotionFacade::selected_ini_hash() const {
    return impl_->motion.selected_ini_hash();
}
bool EcatMotionFacade::is_ecat_running() const { return impl_->motion.is_ecat_running(); }
int EcatMotionFacade::read_eni_json_text(std::string& out) const {
    return impl_->motion.read_eni_json_text(out);
}
int EcatMotionFacade::read_eni_file(std::string& out) const {
    return impl_->motion.read_eni_file(out);
}
int EcatMotionFacade::read_ini_file(std::string& out) const {
    return impl_->motion.read_ini_file(out);
}
int EcatMotionFacade::get_io_metadata(std::vector<IoMetadataEntry>& out,
                                     bool& layout_ready) const {
    return impl_->motion.get_io_metadata(out, layout_ready);
}
int EcatMotionFacade::mailbox_ping(uint32_t pingValue, uint32_t* pongValue) {
    return impl_->motion.mailbox_ping(pingValue, pongValue);
}
int EcatMotionFacade::request_master_state(uint8_t state) {
    return impl_->motion.request_master_state(state);
}
int EcatMotionFacade::bus_is_running(bool* running) {
    return impl_->motion.bus_is_running(running);
}
int EcatMotionFacade::get_master_req_state(uint8_t* state) {
    return impl_->motion.get_master_req_state(state);
}
int EcatMotionFacade::get_master_state(uint8_t* state) {
    return impl_->motion.get_master_state(state);
}
int EcatMotionFacade::get_slave_count(uint16_t* count) {
    return impl_->motion.get_slave_count(count);
}
int EcatMotionFacade::get_active_slave_count(uint16_t* count) {
    return impl_->motion.get_active_slave_count(count);
}
int EcatMotionFacade::get_slave_state(uint16_t slave_id, uint8_t* state) {
    return impl_->motion.get_slave_state(slave_id, state);
}
int EcatMotionFacade::clear_slave_state_error(uint16_t slave_id) {
    return impl_->motion.clear_slave_state_error(slave_id);
}
int EcatMotionFacade::pi_enable_set(bool enable) { return impl_->motion.pi_enable_set(enable); }
int EcatMotionFacade::pi_enable_get(bool* enable) { return impl_->motion.pi_enable_get(enable); }
int EcatMotionFacade::coe_sdo_download(uint16_t slave_id, uint16_t index, uint8_t subindex,
                                      uint8_t complete_access, const void* data, uint32_t data_len,
                                      uint32_t timeout_ms) {
    return impl_->motion.coe_sdo_download(slave_id, index, subindex, complete_access, data,
                                         data_len, timeout_ms);
}
int EcatMotionFacade::coe_sdo_upload(uint16_t slave_id, uint16_t index, uint8_t subindex,
                                    uint8_t complete_access, uint32_t* data_len, void* data,
                                    uint32_t timeout_ms) {
    return impl_->motion.coe_sdo_upload(slave_id, index, subindex, complete_access, data_len, data,
                                       timeout_ms);
}

}  // namespace zmc
