#ifndef ZMC_ECAT_MOTION_FACADE_HPP_
#define ZMC_ECAT_MOTION_FACADE_HPP_

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

#include "MotionRunMode.hpp"
#include "io/IoMetadata.hpp"

namespace zmc {

class MotionCmdQueueBase;

// Logical device state is owned by the facade inside its device slot.
// AI modification: GPT-6 (Codex), 2026-10-28.
enum class DeviceState : uint8_t { Closed, Open, Closing };

class EcatMotionFacade {
   public:
    // Construct and release the owned runtime.
    EcatMotionFacade();
    ~EcatMotionFacade();
    // Open the logical device.
    int openDevice(const std::string& device);
    // Reset the runtime and close the logical device.
    int closeAll();
    // Open the compatibility channel.
    int openChannel(const std::string& netname);
    // Close the compatibility channel.
    int closeChannel();
    // Start the runtime after checking the logical device.
    int startup(const std::string& eni_path, const std::string& ini_path);
    // Validate the INI and start with the selected ENI.
    int startupIni(const std::string& ini_path);
    // Return the logical device state.
    DeviceState deviceState() const { return state_; }
    // Begin closing the logical device.
    void beginClose();
    // Report whether the logical device is open.
    bool is_device_open() const;
    // Report the compatibility channel state.
    bool is_channel_open() const;
    // Return the logical device name.
    const std::string& device_name() const;
    // Return the configured channel name.
    const std::string& channel_name() const;
    // Return the available compatibility channel names.
    std::vector<std::string> available_channel_names() const;

    // Prepare startup configuration.
    int prepare_config(const std::vector<std::string>& args);
    // Return the configured transport mode.
    PdoPowerRunMode run_mode() const;
    // Return the configured transport port.
    int port() const;
    // Return eni_path.
    const std::string& eni_path() const;
    // Return ini_path.
    const std::string& ini_path() const;
    // Return startup_script.
    const std::string& startup_script() const;
    // Select the next ENI.
    int eni_select(const std::string& eni_path);
    // Select the next INI.
    int ini_select(const std::string& ini_path);
    // Stop the runtime.
    int stop();
    // Bind the motion PDO handler.
    int bind_motion_pdo();
    // Return the configured axis count.
    int axis_count() const;
    // Return the existing command queue interface.
    MotionCmdQueueBase& command_interface();
    // Return version.
    const std::string& version() const;
    // Return about_author.
    const std::string& about_author() const;
    // Return about_description.
    const std::string& about_description() const;
    // Return about_code_date.
    const std::string& about_code_date() const;
    // Return eni_name.
    const std::string& eni_name() const;
    // Return ini_name.
    const std::string& ini_name() const;
    // Return current_eni_hash.
    const std::string& current_eni_hash() const;
    // Return current_ini_hash.
    const std::string& current_ini_hash() const;
    // Return selected_eni_path.
    const std::string& selected_eni_path() const;
    // Return selected_ini_path.
    const std::string& selected_ini_path() const;
    // Return selected_eni_hash.
    const std::string& selected_eni_hash() const;
    // Return selected_ini_hash.
    const std::string& selected_ini_hash() const;
    // Report whether the EtherCAT runtime is running.
    bool is_ecat_running() const;
    // Read read_eni_json_text.
    int read_eni_json_text(std::string& out) const;
    // Read read_eni_file.
    int read_eni_file(std::string& out) const;
    // Read read_ini_file.
    int read_ini_file(std::string& out) const;
    // Copy the current IO metadata.
    int get_io_metadata(std::vector<IoMetadataEntry>& out, bool& layout_ready) const;
    // Echo a mailbox value.
    int mailbox_ping(uint32_t pingValue, uint32_t* pongValue);
    // Request a master state.
    int request_master_state(uint8_t state);
    // Read the bus running state.
    int bus_is_running(bool* running);
    // Read the requested master state.
    int get_master_req_state(uint8_t* state);
    // Read the current master state.
    int get_master_state(uint8_t* state);
    // Read the configured slave count.
    int get_slave_count(uint16_t* count);
    // Read the active slave count.
    int get_active_slave_count(uint16_t* count);
    // Read a slave state.
    int get_slave_state(uint16_t slave_id, uint8_t* state);
    // Clear a slave state error.
    int clear_slave_state_error(uint16_t slave_id);
    // Set process-image enablement.
    int pi_enable_set(bool enable);
    // Read process-image enablement.
    int pi_enable_get(bool* enable);
    // Download a CoE SDO.
    int coe_sdo_download(uint16_t slave_id, uint16_t index, uint8_t subindex,
                         uint8_t complete_access, const void* data, uint32_t data_len,
                         uint32_t timeout_ms);
    // Upload a CoE SDO.
    int coe_sdo_upload(uint16_t slave_id, uint16_t index, uint8_t subindex,
                       uint8_t complete_access, uint32_t* data_len, void* data,
                       uint32_t timeout_ms);

   private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    DeviceState state_ = DeviceState::Closed;
};

}  // namespace zmc

#endif
