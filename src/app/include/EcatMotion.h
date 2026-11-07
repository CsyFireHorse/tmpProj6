/*
 * @ArtifactStatus draft
 * @ArtifactStatusDate 2026-09-14
 * @ArtifactStatusAuthority GPT-5 Codex
 */

#ifndef _ECAT_MOTION_H_
#define _ECAT_MOTION_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "MotionRunMode.hpp"
#include "io/IoMetadata.hpp"
#include "mhelper/pdo_servo_addr.h"

namespace zmc {

class EcatServo;
class AxisFbBridge;
class AxisFbBridgeWithThread;
class MotionCmdQueueBase;

struct PdoPowerIniArgs {
    std::string eniName;
    std::string iniName;
    std::string netName;
    PdoPowerRunMode runMode = MODE_HTTP;
    int init_opmode = 0;
    int port = ZMC_MOTAPP_HTTP_DEFAULT_PORT;
    unsigned int axis_num = 0;  // Optional lower bound; let the INI select zero axes by default.
    bool scriptExit = false;
};

// Owns EtherCAT startup and motion runtime resources.
class EcatMotion {
   public:
    EcatMotion();
    ~EcatMotion();

    // Parse arguments, then load the startup script and axis INI configuration.
    int prepare_config(const std::vector<std::string>& args);
    int parse_args(const std::vector<std::string>& args);
    int load_axis_config();

    // Initialize the EtherCAT master for the configured network.
    int _initEcat(const std::string& netname);

    // Service INI selection snapshots only the INI; ENI selection stays independent.
    int eni_select(const std::string& eni_path);
    int ini_select(const std::string& ini_path);

    // Bus lifecycle: load eni, configure motion, start ecat/sdo.
    int startup(const std::string& eni_path, const std::string& ini_path);
    // Start with the selected ENI candidate and this INI path for one run.
    int startupIni(const std::string& ini_path);
    // Validate an INI without changing the selected configuration.
    int validate_startup_ini(const std::string& ini_path) const;
    int stop();

    // Stop and reset the EtherCAT and motion runtime resources.
    int reset();

    // Return the mailbox ping value unchanged. GPT-6 Codex.
    int mailbox_ping(uint32_t pingValue, uint32_t* pongValue);

    int request_master_state(uint8_t state);
    int bus_is_running(bool* running);
    int get_master_req_state(uint8_t* state);
    int get_master_state(uint8_t* state);
    int get_slave_count(uint16_t* count);
    int get_active_slave_count(uint16_t* count);
    int get_slave_state(uint16_t slave_id, uint8_t* state);
    int clear_slave_state_error(uint16_t slave_id);
    int pi_enable_set(bool enable);
    int pi_enable_get(bool* enable);
    int coe_sdo_download(uint16_t slave_id, uint16_t index, uint8_t subindex,
                         uint8_t complete_access, const void* data, uint32_t data_len,
                         uint32_t timeout_ms);
    int coe_sdo_upload(uint16_t slave_id, uint16_t index, uint8_t subindex,
                       uint8_t complete_access, uint32_t* data_len, void* data,
                       uint32_t timeout_ms);

    bool is_started() const;
    bool is_ecat_running() const;
    // Expose the existing configured axis count for preheat. GPT-6 Codex, 2026-10-12.
    int axis_count() const;
    MotionCmdQueueBase& command_interface();

    int dispatch_run_mode();
    int wait_for_exit();

    PdoPowerRunMode run_mode() const;
    int port() const;
    bool script_exit() const;
    const std::string& ini_path() const { return ini_args_.iniName; }
    const std::string& eni_path() const { return ini_args_.eniName; }
    // Channel name (same as EtherCAT netname).
    const std::string& net_name() const { return net_name_; }

    // Current runtime configuration names; empty until startup succeeds.
    const std::string& eni_name() const;
    const std::string& ini_name() const;
    const std::string& current_eni_hash() const { return current_eni_hash_; }
    const std::string& current_ini_hash() const { return current_ini_hash_; }
    // Return the next-startup candidate paths and their selection-time hashes.
    const std::string& selected_eni_path() const { return svc_eni_path_; }
    const std::string& selected_ini_path() const { return svc_ini_path_; }
    const std::string& selected_eni_hash() const { return svc_eni_hash_; }
    const std::string& selected_ini_hash() const { return svc_ini_hash_; }

    // Read current ENI embedded JSON text / raw ENI file / raw INI file.
    int read_eni_json_text(std::string& out) const;
    int read_eni_file(std::string& out) const;
    int read_ini_file(std::string& out) const;

    // Copy current IO metadata and report whether the PDO layout is ready.
    int get_io_metadata(std::vector<IoMetadataEntry>& out, bool& layout_ready) const;

    // Build / runtime metadata.
    const std::string& version() const;
    const std::string& about_author() const;
    const std::string& about_description() const;
    const std::string& about_code_date() const;

    void set_startup_script(const std::string& text);
    // Return the already loaded startup script as a business value. GPT-6.
    const std::string& startup_script() const { return startup_script_; }

    // Inject runtime options used by motion configure / SDO (CLI or HTTP demo).
    void set_motion_options(unsigned int axis_num, int init_opmode);
    void set_port(int port);
    // Set startup logging: 0 quiet, 1 events, 2 ENI details. Configure before startup.
    void set_log_level(int level);

    EcatServo* ecat() const { return ecat_; }
    int bind_motion_pdo();
    int run_pdo_cycle_parsed(const proc_img_i_addr* inp, proc_img_o_addr* oup);

   protected:
    const PdoPowerIniArgs& ini_args() const { return ini_args_; }

    int get_axis_address(int axis_idx) const;
    int get_axis_actual_position(int axis_idx) const;

   private:
    AxisFbBridge& bridge();
    const AxisFbBridge& bridge() const;

    int load_ecat_eni(const std::string& eni_path);
    int setup_axis_graph(const std::string& ini_path);
    int start_ecat();
    int bind_ecat_state_adapter();
    int start_sdo_watch();
    void rollback_runtime();
    static int motion_pdo_callback(void* context, const void* input, void* output);


    EcatServo* ecat_ = nullptr;
    int master_fd_ = -1;
    PdoPowerIniArgs ini_args_;
    std::string net_name_ = ZMC_DEFAULT_NET_NAME;
    std::string startup_script_;
    // Last configuration loaded successfully; service selections below remain startup candidates.
    std::string current_eni_path_;
    std::string current_ini_path_;
    std::string current_eni_hash_;
    std::string current_ini_hash_;
    std::string svc_eni_path_;
    std::string svc_ini_path_;
    std::string svc_eni_hash_;
    std::string svc_ini_hash_;
    std::unique_ptr<AxisFbBridgeWithThread> bridge_;
    bool session_open_ = false;
    bool bus_started_ = false;
    int log_level_ = 0;
};

}  // namespace zmc

#endif
