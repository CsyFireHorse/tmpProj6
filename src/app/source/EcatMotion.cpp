/*
 * @ArtifactStatus draft
 * @ArtifactStatusDate 2026-09-14
 * @ArtifactStatusAuthority GPT-5 Codex
 */

#include "EcatMotion.h"

#include <stdio.h>

#include <vector>

#include "AxisFbBridge.h"
#include "AxisFbBridgeWithThread.h"
#include "EcatWrap.h"
#include "FbWrapCmdV2.h"
#include "HttpScriptHelper.hpp"
#include "srv/HttpFileManager.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"
#include "common/zal_version.h"
#include "ecat_mgr.h"
#include "io/IoManager.hpp"
#include "mhelper/ecat_json.h"
#include "mhelper/eni_servo_parser.h"
#include "pou/AxisIniLoader.h"
#include "EcatMotionCli.hpp"
#include "sdo/sdo_rt.hpp"
#include "sdo_backend_ecat.hpp"

namespace zmc {

namespace {

// Read entire file into out. err_open is returned on open/seek/read failure.
int read_file_bytes(const std::string& path, std::string& out, int err_open) {
    out.clear();
    if (path.empty()) {
        return err_open;
    }
    FILE* fd = fopen(path.c_str(), "rb");
    if (!fd) {
        return err_open;
    }
    if (fseek(fd, 0, SEEK_END) != 0) {
        fclose(fd);
        return err_open;
    }
    const long size_l = ftell(fd);
    if (size_l < 0) {
        fclose(fd);
        return err_open;
    }
    if (fseek(fd, 0, SEEK_SET) != 0) {
        fclose(fd);
        return err_open;
    }
    const size_t size = static_cast<size_t>(size_l);
    std::vector<char> buff(size);
    if (size > 0) {
        const size_t nread = fread(buff.data(), 1, size, fd);
        fclose(fd);
        if (nread != size) {
            return err_open;
        }
        out.assign(buff.data(), size);
    } else {
        fclose(fd);
    }
    return 0;
}

// Return the lowercase content hash, or empty when the current file cannot be read.
std::string hash_file_content(const std::string& path, int err_open) {
    std::string bytes;
    if (read_file_bytes(path, bytes, err_open) != 0) {
        return std::string();
    }
    return HttpFileManager::hashBytes(bytes.empty() ? nullptr : bytes.data(), bytes.size());
}

struct IniSelection {
    std::string ini_path;
    std::string ini_hash;
};

int inspect_ini_selection(const std::string& ini_path, IniSelection& selection) {
    if (ini_path.empty()) {
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }

    std::string ini_bytes;
    const int ini_ret = read_file_bytes(ini_path, ini_bytes, ZMC_ERR_INI_FILE_OPEN);
    if (ini_ret != 0) {
        return ini_ret;
    }

    AxisCommonConfig config;
    axisComcfg_default_init(&config);
    const int config_ret = load_axis_ini_base(ini_path.c_str(), &config);
    if (config_ret != 0) {
        return config_ret;
    }
    selection.ini_path = ini_path;
    selection.ini_hash = HttpFileManager::hashBytes(ini_bytes.empty() ? nullptr : ini_bytes.data(),
                                                    ini_bytes.size());
    return 0;
}

int32_t bridge_ecat_sdo_upload(void* ctx, const SdoCKey* key, uint32_t* len, void* data,
                               uint32_t timeout_ms) {
    if (!key) {
        return ZMC_ERR_HOST_SDO_ADAPTER_NULL_ARG;
    }
    return EcatSdoBackend::Upload(ctx, key->slave_id, key->index, key->subindex, len, data,
                                  timeout_ms);
}

int32_t bridge_ecat_sdo_download(void* ctx, const SdoCKey* key, uint32_t len, const void* data,
                                 uint32_t timeout_ms) {
    if (!key) {
        return ZMC_ERR_HOST_SDO_ADAPTER_NULL_ARG;
    }
    return EcatSdoBackend::Download(ctx, key->slave_id, key->index, key->subindex, len, data,
                                    timeout_ms);
}

int32_t bridge_ecat_get_slave_state(void* ctx, uint16_t slave_id, uint8_t* slave_state) {
    return EcatGetSlaveState(static_cast<ECAT_HANDLE>(ctx), slave_id, slave_state);
}

int32_t bridge_ecat_get_slave_count(void* ctx, uint16_t* slave_count) {
    return EcatGetActiveSlaveCount(static_cast<ECAT_HANDLE>(ctx), slave_count);
}

int32_t bridge_ecat_get_master_state(void* ctx, uint8_t* master_state) {
    return EcatGetMasterState(static_cast<ECAT_HANDLE>(ctx), master_state);
}

}  // namespace

EcatMotion::EcatMotion() { bridge_.reset(new AxisFbBridgeWithThread()); }

EcatMotion::~EcatMotion() { (void)reset(); }

// The transport-neutral callback lets HTTP and RPC share one motion PDO binding.
int EcatMotion::motion_pdo_callback(void* context, const void* input, void* output) {
    if (!context || !input || !output) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    EcatMotion* motion = static_cast<EcatMotion*>(context);
    const proc_img_i_addr* txpdo = static_cast<const proc_img_i_addr*>(input);
    proc_img_o_addr* rxpdo = static_cast<proc_img_o_addr*>(output);
    return motion->run_pdo_cycle_parsed(txpdo, rxpdo);
}

int EcatMotion::bind_motion_pdo() {
    if (!ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->bindPdoHandler(&EcatMotion::motion_pdo_callback, this);
}

AxisFbBridge& EcatMotion::bridge() { return bridge_->bridge(); }

const AxisFbBridge& EcatMotion::bridge() const { return bridge_->bridge(); }

int EcatMotion::run_pdo_cycle_parsed(const proc_img_i_addr* inp, proc_img_o_addr* oup) {
    return bridge().run_cycle_parsed(inp, oup);
}

MotionCmdQueueBase& EcatMotion::command_interface() { return bridge().command_interface(); }

bool EcatMotion::is_started() const { return bus_started_; }

bool EcatMotion::is_ecat_running() const { return is_started(); }

int EcatMotion::get_slave_count(uint16_t* count) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->get_slave_count(count);
}

int EcatMotion::get_active_slave_count(uint16_t* count) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->get_active_slave_count(count);
}

int EcatMotion::get_slave_state(uint16_t slave_id, uint8_t* state) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->get_slave_state(slave_id, state);
}

int EcatMotion::clear_slave_state_error(uint16_t slave_id) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->clear_slave_state_error(slave_id);
}

const std::string& EcatMotion::version() const {
    static const std::string kVersion(ZAL_VERSION_STR);
    return kVersion;
}

const std::string& EcatMotion::about_author() const {
    static const std::string kAuthor(ZAL_AUTHOR);
    return kAuthor;
}

const std::string& EcatMotion::about_description() const {
    static const std::string kDescription(ZAL_DESCRIPTION);
    return kDescription;
}

const std::string& EcatMotion::about_code_date() const {
    static const std::string kCodeDate(ZAL_CODE_DATE);
    return kCodeDate;
}

int EcatMotion::axis_count() const { return bridge().axis_count(); }

int EcatMotion::get_axis_address(int axis_idx) const { return bridge().get_address(axis_idx); }

int EcatMotion::get_axis_actual_position(int axis_idx) const {
    return bridge().get_actual_position(axis_idx);
}

int EcatMotion::parse_args(const std::vector<std::string>& args) {
    const int ret = parse_ecat_motion_args(args, &ini_args_);
    if (ret == 0 && !ini_args_.netName.empty()) {
        net_name_ = ini_args_.netName;
    }
    return ret;
}

int EcatMotion::prepare_config(const std::vector<std::string>& args) {
    // Parse each invocation from the defaults for a fresh runtime configuration.
    ini_args_ = PdoPowerIniArgs();
    net_name_ = ZMC_DEFAULT_NET_NAME;
    startup_script_.clear();
    svc_eni_path_.clear();
    svc_ini_path_.clear();
    svc_eni_hash_.clear();
    svc_ini_hash_.clear();

    int ret = parse_args(args);
    if (ret != 0) {
        return ret;
    }

    std::string script_path;
    if (parse_script_path_from_args(args, 0, script_path)) {
        std::string script_content;
        if (!read_text_file_all(script_path, script_content)) {
            printf("Error: failed to open script file: %s\n", script_path.c_str());
            return ZMC_ERR_HOST_LOCAL_FILE_OPEN_FAILED;
        }
        if (script_content.empty()) {
            printf("Error: script file is empty: %s\n", script_path.c_str());
            return ZMC_ERR_MOTCMD_SCRIPT_EMPTY;
        }
        printf("[startup-script] startup script: %s\n", script_path.c_str());
        set_startup_script(script_content);
    }

    return load_axis_config();
}

PdoPowerRunMode EcatMotion::run_mode() const { return ini_args_.runMode; }

int EcatMotion::port() const { return ini_args_.port; }

bool EcatMotion::script_exit() const { return ini_args_.scriptExit; }

const std::string& EcatMotion::eni_name() const {
    return current_eni_path_;
}

const std::string& EcatMotion::ini_name() const {
    return current_ini_path_;
}

int EcatMotion::read_eni_json_text(std::string& out) const {
    out.clear();
    const std::string& path = eni_name();
    if (path.empty()) {
        return ZMC_ERR_ENI_FILE_OPEN_FAILED;
    }
    return extract_eni_json_text(path.c_str(), out);
}

int EcatMotion::read_eni_file(std::string& out) const {
    return read_file_bytes(eni_name(), out, ZMC_ERR_ENI_FILE_OPEN_FAILED);
}

int EcatMotion::read_ini_file(std::string& out) const {
    return read_file_bytes(ini_name(), out, ZMC_ERR_INI_FILE_OPEN);
}

int EcatMotion::get_io_metadata(std::vector<IoMetadataEntry>& out, bool& layout_ready) const {
    layout_ready = bridge().pdo_layout_ready();
    return bridge().get_io_metadata(out);
}

void EcatMotion::set_startup_script(const std::string& text) { startup_script_ = text; }

void EcatMotion::set_motion_options(unsigned int axis_num, int init_opmode) {
    ini_args_.axis_num = axis_num;
    ini_args_.init_opmode = init_opmode;
}

void EcatMotion::set_port(int port) { ini_args_.port = port; }
void EcatMotion::set_log_level(int level) { log_level_ = level; }

int EcatMotion::load_axis_config() {
    if (bus_started_) {
        return ZMC_ERR_ECAT_ALREADY_RUNNING;
    }
    if (ini_args_.iniName.empty()) {
        if (!ini_args_.netName.empty()) {
            net_name_ = ini_args_.netName;
        }
        return 0;
    }
    const char* ini_name = ini_args_.iniName.c_str();
    AxisCommonConfig cfg;
    axisComcfg_default_init(&cfg);
    int errCode = load_axis_ini_base(ini_name, &cfg);
    if (errCode) {
        printf("load %s failed %d\n", ini_name, errCode);
        return errCode;
    }
    if (!ini_args_.netName.empty()) {
        net_name_ = ini_args_.netName;
    } else if (cfg.net_name) {
        net_name_ = cfg.net_name;
    }
    return 0;
}

int EcatMotion::_initEcat(const std::string& netname) {
    if (session_open_) {
        return 0;
        // ZMC_ERR_HOST_HANDLE_ALREADY_OPEN;
    }
    if (netname.empty()) {
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }

    net_name_ = netname;
    const int ret = EcatWrapMgrSg::init_ecat(net_name_);
    if (ret != 0) {
        return ret;
    }
    master_fd_ = EcatWrapMgrSg::find_netname(net_name_);
    if (master_fd_ < 0) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    ecat_ = EcatWrapMgrSg::get_ecat(static_cast<unsigned int>(master_fd_));
    if (!ecat_) {
        master_fd_ = -1;
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }

    session_open_ = true;
    bus_started_ = false;
    return 0;
}

int EcatMotion::reset() {
    int close_ret = 0;
    if (bus_started_) {
        close_ret = stop();
    }
    if (bridge_) {
        (void)bridge_->reset();
    }

    ecat_ = nullptr;
    master_fd_ = -1;
    current_eni_path_.clear();
    current_ini_path_.clear();
    current_eni_hash_.clear();
    current_ini_hash_.clear();
    svc_eni_path_.clear();
    svc_ini_path_.clear();
    svc_eni_hash_.clear();
    svc_ini_hash_.clear();
    session_open_ = false;
    bus_started_ = false;
    return close_ret;
}

int EcatMotion::eni_select(const std::string& eni_path) {
    if (eni_path.empty()) {
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }
    std::string eni_bytes;
    const int ret = read_file_bytes(eni_path, eni_bytes, ZMC_ERR_ENI_FILE_OPEN_FAILED);
    if (ret != 0) {
        return ret;
    }
    const std::string hash = HttpFileManager::hashBytes(
        eni_bytes.empty() ? nullptr : eni_bytes.data(), eni_bytes.size());
    svc_eni_path_ = eni_path;
    svc_eni_hash_ = hash;
    return 0;
}

int EcatMotion::ini_select(const std::string& ini_path) {
    IniSelection selection;
    const int ret = inspect_ini_selection(ini_path, selection);
    if (ret != 0) {
        return ret;
    }
    svc_ini_path_ = selection.ini_path;
    svc_ini_hash_ = selection.ini_hash;
    return 0;
}

int EcatMotion::load_ecat_eni(const std::string& eni_path) {
    if (!ecat_ || eni_path.empty()) {
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }
    int ret = ecat_->set_eni(eni_path.c_str());
    if (ret != 0) {
        printf("set ECAT ENI failed: ret=%d\n", ret);
        return ret;
    }
    ret = ecat_->load_eni_json(eni_path.c_str());
    if (ret != 0) {
        printf("load ECAT ENI failed: ret=%d, path=%s\n", ret, eni_path.c_str());
        return ret;
    }
    if (log_level_ >= 1) printf("load ecat eni %s\n", eni_path.c_str());
    if (log_level_ >= 2) print_eni_servo_mapper(*ecat_->pdoAddr.pServo);
    return 0;
}

int EcatMotion::start_ecat() {
    if (!ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    ECAT_RESULT ret = ecat_->start();
    if (ret) {
        printf("Ecat Start failed: ret=%d\n", ret);
        return ret;
    }
    if (log_level_ >= 1) printf("start ecat %s\n", net_name_.c_str());
    bus_started_ = true;
    return 0;
}

int EcatMotion::setup_axis_graph(const std::string& ini_path) {
    const bool has_ini = !ini_path.empty();
    const char* ini_cstr = has_ini ? ini_path.c_str() : nullptr;
    const unsigned int axis_num =
        has_ini ? ini_args_.axis_num : static_cast<unsigned int>(bridge().servo_count());
    const int ret = bridge().configure(ini_cstr, nullptr, axis_num);
    if (ret != 0) {
        printf("setup axis graph failed: configure bridge failed, ret=%d\n", ret);
        return ret;
    }
    return 0;
}

int EcatMotion::bind_ecat_state_adapter() {
    EcatStateAdapter adapter = {};
    adapter.ctx = ecat_->ctx.Master;
    adapter.get_slave_count = bridge_ecat_get_slave_count;
    adapter.get_slave_state = bridge_ecat_get_slave_state;
    adapter.get_master_state = bridge_ecat_get_master_state;
    return bridge().bind_ecat_state_adapter(&adapter);
}

int EcatMotion::start_sdo_watch() {
    SdoCAdapter adapter = {};
    adapter.ctx = ecat_->ctx.Master;
    adapter.upload = bridge_ecat_sdo_upload;
    adapter.download = bridge_ecat_sdo_download;
    adapter.get_slave_state = bridge_ecat_get_slave_state;
    // Preserve the adapter error at the runtime boundary. GPT-6, 2026-10-21.
    int ret = bridge().bind_sdo_adapter(&adapter);
    if (ret) {
        return ret;
    }
    ret = bridge_->start_sdo();
    if (ret) {
        printf("start sdo failed: ret=%d\n", ret);
        return ret;
    }
    bridge().native_sdo()->resume();
    auto init_opmode = ini_args_.init_opmode;
    ret = bridge().register_opmode_watches(init_opmode);
    if (ret) {
        printf("Failed to register opmode watches: ret=%d\n", ret);
        return ret;
    }

    if (init_opmode) {
        // if (ecat_->config_servo_op_mode(static_cast<uint32_t>(init_opmode))) {
        //     printf("Failed to configure op mode %d for all servos\n", init_opmode);
        //     return -1;
        // }
        // printf("configure op mode %d\n", init_opmode);
    }
    return 0;
}

int EcatMotion::startup(const std::string& eni_path, const std::string& ini_path) {
    if (bus_started_) {
        return ZMC_ERR_HOST_HANDLE_ALREADY_OPEN;
    }
    if (net_name_.empty()) {
        net_name_ = ZMC_DEFAULT_NET_NAME;
    }

    const std::string& effective_eni = !eni_path.empty() ? eni_path : svc_eni_path_;
    const std::string& effective_ini = !ini_path.empty() ? ini_path : svc_ini_path_;
    if (effective_eni.empty()) {
        printf("startup failed: ENI path required\n");
        return ZMC_ERR_MOTCMD_INVALID_ARG;
    }

    int ret = _initEcat(net_name_);

    if (ret == 0) {
        ret = load_ecat_eni(effective_eni);
    }
    if (ret == 0) {
        ret = bridge().load_eni(effective_eni.c_str());
    }
    if (ret == 0) {
        ret = setup_axis_graph(effective_ini);
    }
    if (ret == 0) {
        ret = start_ecat();
    }
    if (ret == 0) {
        ret = bind_ecat_state_adapter();
    }
    if (ret == 0) {
        ret = start_sdo_watch();
    }
    if (ret != 0) {
        rollback_runtime();
        ecat_ = nullptr;
        master_fd_ = -1;
        return ret;
    }
    // Publish the current configuration only after the runtime startup completes.
    current_eni_path_ = effective_eni;
    current_ini_path_ = effective_ini;
    current_eni_hash_ = hash_file_content(effective_eni, ZMC_ERR_ENI_FILE_OPEN_FAILED);
    current_ini_hash_ = effective_ini.empty()
                            ? std::string()
                            : hash_file_content(effective_ini, ZMC_ERR_INI_FILE_OPEN);
    return 0;
}

int EcatMotion::validate_startup_ini(const std::string& ini_path) const {
    IniSelection selection;
    return inspect_ini_selection(ini_path, selection);
}

int EcatMotion::startupIni(const std::string& ini_path) {
    const int ret = validate_startup_ini(ini_path);
    if (ret != 0) {
        return ret;
    }
    // Preserve the validated path when the caller passed a runtime-owned string.
    const std::string validated_ini = ini_path;
    return startup(svc_eni_path_, validated_ini);
}

int EcatMotion::stop() {
    if (!bus_started_) {
        return 0;
    }
    if (!ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }

    ecat_->unbind();
    if (bridge_ && bridge_->sdo_started()) {
        (void)bridge_->stop_sdo();
    }
    const int ret = ecat_->stop();
    if (bridge_) {
        (void)bridge_->reset();
    }
    bus_started_ = false;
    return ret;
}

// Echo the complete unsigned value to caller storage. GPT-6 Codex.
int EcatMotion::mailbox_ping(uint32_t pingValue, uint32_t* pongValue) {
    if (!pongValue) return ZMC_ERR_MOTCMD_INVALID_ARG;
    *pongValue = pingValue;
    return 0;
}

int EcatMotion::request_master_state(uint8_t state) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->request_master_state(state);
}

int EcatMotion::bus_is_running(bool* running) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->bus_is_running(running);
}

int EcatMotion::get_master_req_state(uint8_t* state) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->get_master_req_state(state);
}

int EcatMotion::get_master_state(uint8_t* state) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->get_master_state(state);
}

int EcatMotion::pi_enable_set(bool enable) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->pi_enable_set(enable);
}

int EcatMotion::pi_enable_get(bool* enable) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->pi_enable_get(enable);
}

int EcatMotion::coe_sdo_download(uint16_t slave_id, uint16_t index, uint8_t subindex,
                                 uint8_t complete_access, const void* data, uint32_t data_len,
                                 uint32_t timeout_ms) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->coe_sdo_download(slave_id, index, subindex, complete_access, data, data_len,
                                   timeout_ms);
}

int EcatMotion::coe_sdo_upload(uint16_t slave_id, uint16_t index, uint8_t subindex,
                               uint8_t complete_access, uint32_t* data_len, void* data,
                               uint32_t timeout_ms) {
    if (!bus_started_ || !ecat_) {
        return ZMC_ERR_HOST_INVALID_HANDLE;
    }
    return ecat_->coe_sdo_upload(slave_id, index, subindex, complete_access, data_len, data,
                                 timeout_ms);
}

void EcatMotion::rollback_runtime() {
    if (ecat_ && bus_started_) {
        ecat_->unbind();
    }
    if (bridge_ && bridge_->sdo_started()) {
        (void)bridge_->stop_sdo();
    }
    if (ecat_ && bus_started_) {
        (void)ecat_->stop();
        bus_started_ = false;
    }
    if (bridge_) {
        (void)bridge_->reset();
    }
}

#if 0  // Disabled: external runtime/factory dispatch bypasses the singleton. GPT-6.
int EcatMotion::dispatch_run_mode() {
    if (ini_args_.runMode == MODE_CMD)
        return runCmdLoopV2(bridge().command_interface(), startup_script_, ini_args_.scriptExit);
#ifndef ZMC_ENABLE_RPC
    if (ini_args_.runMode == MODE_RPC) {
        printf("RPC mode requires build with -DENABLE_RPC=ON\n");
        return 0;
    }
#endif
    if (ini_args_.runMode != MODE_HTTP && ini_args_.runMode != MODE_RPC) return 0;
    int ret = 0;
    if (ini_args_.runMode == MODE_HTTP) ret = runEcatHttpServer(ini_args_.port);
#ifdef ZMC_ENABLE_RPC
    else ret = runRpcServer("0.0.0.0", ini_args_.port);
#endif
    return ret;
}
#endif
int EcatMotion::dispatch_run_mode() {
    // Legacy CMD is retained; service modes are owned exclusively by SrvCommandSingleton.
    if (ini_args_.runMode == MODE_CMD)
        return runCmdLoopV2(bridge().command_interface(), startup_script_, ini_args_.scriptExit);
    return ZMC_ERR_HOST_NOT_IMPLEMENTED;
}

int EcatMotion::wait_for_exit() {
    // HTTP/RPC already blocked in dispatch_run_mode until the service exits.
    if (ini_args_.runMode != MODE_CMD) {
        return 0;
    }
    if (ini_args_.scriptExit) {
        return 0;
    }
    printf("Press e key to exit\n");
    while (getchar() != 'e') {
    }
    return 0;
}

}  // namespace zmc
