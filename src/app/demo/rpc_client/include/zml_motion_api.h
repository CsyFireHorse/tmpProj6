#ifndef ZML_MOTION_API_H
#define ZML_MOTION_API_H

#include <stdbool.h>
#include <stdint.h>

#include "zml_types_def.h"
#include "zml_errorcode_def.h"

#if defined(_WIN32)
#if defined(ZML_EXPORTS)
#define ZML_API __declspec(dllexport)
#else
#define ZML_API __declspec(dllimport)
#endif
#else
#define ZML_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Opaque context owned by the caller.
typedef struct ZML_Context* ZML_HANDLE;
typedef int32_t ZML_RESULT;

typedef enum ZML_HOME_STATUS {
    ZML_HOME_STATUS_IDLE = 0,
    ZML_HOME_STATUS_BUSY = 3,
    ZML_HOME_STATUS_DONE = 4,
    ZML_HOME_STATUS_ERROR = 5,
    ZML_HOME_STATUS_ABORTED = 6
} ZML_HOME_STATUS;

#define ZML_VERSION_INFO_MAX 80u
#define ZML_ABOUT_INFO_MAX 255u
#define ZML_CONTROLLER_INFO_MAX 80u

// Successful asynchronous command submission means the command was accepted,
// not that execution or motion has completed. Synchronous operations wait for
// their documented result or execution acknowledgement.
// Axis contracts include explicit target axes.

/**
 * @brief Opens an Ethernet RPC connection using an IP address.
 * @param[out] out_handle Receives the opened connection handle.
 * @param[in] ipaddr IP address of the RPC server.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_open_eth(ZML_HANDLE* out_handle, const char* ipaddr);
/**
 * @brief Opens a local PCIe device selected by its device number.
 * @param[out] out_handle Receives the opened connection handle.
 * @param[in] alias PCIe device number.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_open_pci(ZML_HANDLE* out_handle, uint32_t alias);
/**
 * @brief Opens a connection using a device or server address string.
 * @param[out] out_handle Receives the opened connection handle.
 * @param[in] device Device or server address string.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_open(ZML_HANDLE* out_handle, const char* device);
/**
 * @brief Closes the connection and clears the caller's handle.
 * @param[in,out] handle Address of the handle to close; the handle is cleared.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_close(ZML_HANDLE* handle);
/**
 * @brief Sends a synchronous no-op and waits for its motion-cycle confirmation.
 * @param[in] handle RPC connection handle.
 * @param[in] timeout_ms Timeout in milliseconds.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ping(ZML_HANDLE handle, uint32_t timeout_ms);
/**
 * @brief Returns the mailbox ping value unchanged on success.
 * @param[in] handle RPC connection handle.
 * @param[in] pingValue Value sent to the mailbox.
 * @param[out] pongValue Receives the returned mailbox value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_mailbox_ping(ZML_HANDLE handle, uint32_t pingValue, uint32_t* pongValue);

/**
 * @brief Writes the server version to a caller buffer of at least ZML_VERSION_INFO_MAX bytes.
 * @param[in] handle RPC connection handle.
 * @param[out] version Receives the version string; provide at least ZML_VERSION_INFO_MAX bytes.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_get_version_info(ZML_HANDLE handle, char* version);
/**
 * @brief Writes the server description to a caller buffer of at least ZML_ABOUT_INFO_MAX bytes.
 * @param[in] handle RPC connection handle.
 * @param[out] about Receives the description; provide at least ZML_ABOUT_INFO_MAX bytes.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_get_about(ZML_HANDLE handle, char* about);
/**
 * @brief Writes controller information to caller buffers of at least ZML_CONTROLLER_INFO_MAX
 * bytes.
 * @param[in] handle RPC connection handle.
 * @param[out] soft_type Receives the software type; provide at least ZML_CONTROLLER_INFO_MAX
 * bytes.
 * @param[out] soft_version Receives the software version; provide at least ZML_CONTROLLER_INFO_MAX
 * bytes.
 * @param[out] controller_id Receives the controller ID; provide at least ZML_CONTROLLER_INFO_MAX
 * bytes.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_get_controller_info(ZML_HANDLE handle, char* soft_type, char* soft_version,
                                           char* controller_id);

/**
 * @brief Returns one digital input after its motion-cycle read completes.
 * @param[in] handle RPC connection handle.
 * @param[in] io_number IO number.
 * @param[out] value Receives the requested value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_read_di(ZML_HANDLE handle, uint32_t io_number, bool* value);
/**
 * @brief Returns one retained digital-output value after its motion-cycle read completes.
 * @param[in] handle RPC connection handle.
 * @param[in] io_number IO number.
 * @param[out] value Receives the requested value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_read_do(ZML_HANDLE handle, uint32_t io_number, bool* value);
/**
 * @brief Enqueues one digital-output write without waiting for execution.
 * @param[in] handle RPC connection handle.
 * @param[in] io_number IO number.
 * @param[in] value Value to write.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_write_do(ZML_HANDLE handle, uint32_t io_number, bool value);
/**
 * @brief Returns one unscaled analog-input value after its motion-cycle read completes.
 * @param[in] handle RPC connection handle.
 * @param[in] io_number IO number.
 * @param[out] value Receives the requested value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_read_ai(ZML_HANDLE handle, uint32_t io_number, uint32_t* value);
/**
 * @brief Returns one retained analog-output value after its motion-cycle read completes.
 * @param[in] handle RPC connection handle.
 * @param[in] io_number IO number.
 * @param[out] value Receives the requested value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_read_ao(ZML_HANDLE handle, uint32_t io_number, uint32_t* value);
/**
 * @brief Enqueues one unscaled analog-output write without waiting for execution.
 * @param[in] handle RPC connection handle.
 * @param[in] io_number IO number.
 * @param[in] value Value to write.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_write_ao(ZML_HANDLE handle, uint32_t io_number, uint32_t value);

/**
 * @brief Starts the EtherCAT master.
 * @param[in] handle RPC connection handle.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_start(ZML_HANDLE handle);
/**
 * @brief Stops the EtherCAT master.
 * @param[in] handle RPC connection handle.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_stop(ZML_HANDLE handle);

/**
 * @brief Queries whether the EtherCAT bus is running.
 * @param[in] handle RPC connection handle.
 * @param[out] run_flag Receives whether the bus is running.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_bus_is_running(ZML_HANDLE handle, bool* run_flag);
/**
 * @brief Requests an EtherCAT master state.
 * @param[in] handle RPC connection handle.
 * @param[in] state Requested master state.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_request_master_state(ZML_HANDLE handle, uint8_t state);
/**
 * @brief Gets the requested EtherCAT master state.
 * @param[in] handle RPC connection handle.
 * @param[out] state Receives the requested state.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_get_master_req_state(ZML_HANDLE handle, uint8_t* state);
/**
 * @brief Gets the current EtherCAT master state.
 * @param[in] handle RPC connection handle.
 * @param[out] state Receives the requested state.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_get_master_state(ZML_HANDLE handle, uint8_t* state);
/**
 * @brief Downloads an SDO value to a slave.
 * @param[in] handle RPC connection handle.
 * @param[in] slave_id EtherCAT slave ID.
 * @param[in] index SDO object index.
 * @param[in] sub_index SDO object subindex.
 * @param[in] data_len SDO data length in bytes.
 * @param[in] data Buffer containing data_len bytes to download.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_coe_sdo_download(ZML_HANDLE handle, uint16_t slave_id, uint16_t index,
                                             uint8_t sub_index, uint32_t data_len,
                                             const void* data);
/**
 * @brief Uploads an SDO value from a slave.
 * @param[in] handle RPC connection handle.
 * @param[in] slave_id EtherCAT slave ID.
 * @param[in] index SDO object index.
 * @param[in] sub_index SDO object subindex.
 * @param[in] data_len Address of the requested data length in bytes; supported lengths are 1, 2,
 * and 4.
 * @param[out] data Receives the uploaded SDO data; provide data_len bytes.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_coe_sdo_upload(ZML_HANDLE handle, uint16_t slave_id, uint16_t index,
                                           uint8_t sub_index, uint32_t* data_len, void* data);
/**
 * @brief Gets the configured slave count.
 * @param[in] handle RPC connection handle.
 * @param[out] slave_count Receives the number of slaves.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_get_slave_count(ZML_HANDLE handle, uint16_t* slave_count);
/**
 * @brief Gets the active slave count.
 * @param[in] handle RPC connection handle.
 * @param[out] slave_count Receives the number of slaves.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_get_active_slave_count(ZML_HANDLE handle, uint16_t* slave_count);
/**
 * @brief Gets the state of a slave.
 * @param[in] handle RPC connection handle.
 * @param[in] slave_id EtherCAT slave ID.
 * @param[out] state Receives the requested state.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_get_slave_state(ZML_HANDLE handle, uint16_t slave_id, uint8_t* state);
/**
 * @brief Clears a slave state error.
 * @param[in] handle RPC connection handle.
 * @param[in] slave_id EtherCAT slave ID.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_clear_slave_state_error(ZML_HANDLE handle, uint16_t slave_id);

/**
 * @brief PCI only: enables the PDO mirror cache.
 * @param[in] handle RPC connection handle.
 * @param[in] flag PDO mirror cache enable flag.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_pi_enable_set(ZML_HANDLE handle, bool flag);
/**
 * @brief Gets the PDO mirror cache enable flag.
 * @param[in] handle RPC connection handle.
 * @param[out] flag PDO mirror cache enable flag.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ecat_pi_enable_get(ZML_HANDLE handle, bool* flag);

/**
 * @brief Powers on the axis.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_power_on(ZML_HANDLE handle, int32_t axis);
/**
 * @brief Powers off the axis.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_power_off(ZML_HANDLE handle, int32_t axis);
/**
 * @brief Sets the axis enable value.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] enable Enable value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_enable(ZML_HANDLE handle, int32_t axis, int32_t enable);
/**
 * @brief Gets the axis enable value.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] enable Receives the enable value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_get_enable(ZML_HANDLE handle, int32_t axis, int32_t* enable);

/**
 * @brief Reads the axis state word in the motion cycle.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] state Receives the requested state.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_read_state(ZML_HANDLE handle, int32_t axis, int32_t* state);
/**
 * @brief Reads the commanded (set) position in user units.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] position Receives the position in user units.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_read_set_position(ZML_HANDLE handle, int32_t axis, double* position);
/**
 * @brief Reads the actual feedback position in user units.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] position Receives the position in user units.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_read_act_position(ZML_HANDLE handle, int32_t axis, double* position);
/**
 * @brief Reads the negative-direction software position limit in user units.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] limit Receives the software position limit in user units.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_read_sw_limit_negative(ZML_HANDLE handle, int32_t axis, double* limit);
/**
 * @brief Reads whether software position limits are enabled (0: disabled, 1: enabled).
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] enable Receives the enable value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_read_sw_limit_enable(ZML_HANDLE handle, int32_t axis, int32_t* enable);
/**
 * @brief Reads the positive-direction software position limit in user units.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] limit Receives the software position limit in user units.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_read_sw_limit_positive(ZML_HANDLE handle, int32_t axis, double* limit);
/**
 * @brief Reads the technology-units conversion ratio numerator in the motion cycle.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] num Receives the units conversion ratio numerator.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_read_units_num(ZML_HANDLE handle, int32_t axis, int32_t* num);
/**
 * @brief Reads the technology-units conversion ratio denominator in the motion cycle.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] den Receives the units conversion ratio denominator.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_read_units_den(ZML_HANDLE handle, int32_t axis, int32_t* den);

/**
 * @brief Sets the axis velocity.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] speed Configured speed value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_velocity(ZML_HANDLE handle, int32_t axis, double speed);
/**
 * @brief Reads the configured speed from the parameter mirror.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] speed Receives the configured speed value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_get_speed(ZML_HANDLE handle, int32_t axis, double* speed);
/**
 * @brief Sets the axis acceleration.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] accel Configured accel value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_accel(ZML_HANDLE handle, int32_t axis, double accel);
/**
 * @brief Reads the configured acceleration from the parameter mirror.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] accel Receives the configured accel value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_get_accel(ZML_HANDLE handle, int32_t axis, double* accel);
/**
 * @brief Reads the configured deceleration from the parameter mirror.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] decel Receives the configured decel value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_get_decel(ZML_HANDLE handle, int32_t axis, double* decel);
/**
 * @brief Reads the configured jerk from the parameter mirror.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] jerk Receives the configured jerk value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_get_jerk(ZML_HANDLE handle, int32_t axis, double* jerk);
/**
 * @brief Sets the axis deceleration.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] decel Configured decel value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_decel(ZML_HANDLE handle, int32_t axis, double decel);
/**
 * @brief Sets the axis jerk.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] jerk Configured jerk value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_jerk(ZML_HANDLE handle, int32_t axis, double jerk);

/**
 * @brief Sets the axis position.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] pos Position value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_position(ZML_HANDLE handle, int32_t axis, double pos);
/**
 * @brief Stops the axis.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_stop(ZML_HANDLE handle, int32_t axis);
/**
 * @brief Halts the axis.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_halt(ZML_HANDLE handle, int32_t axis);
/**
 * @brief Resets the axis.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_reset(ZML_HANDLE handle, int32_t axis);

/**
 * @brief Commands a relative axis move.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] distance Relative distance value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_move_rel(ZML_HANDLE handle, int32_t axis, double distance);
/**
 * @brief Commands an additive axis move.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] pos Position value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_moveadd(ZML_HANDLE handle, int32_t axis, double pos);
/**
 * @brief Commands an absolute axis move.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] pos Position value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_move_abs(ZML_HANDLE handle, int32_t axis, double pos);

/**
 * @brief Commands an axis velocity move.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] dir Direction value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_move_velocity(ZML_HANDLE handle, int32_t axis, int32_t dir);
/**
 * @brief Commands forward axis motion.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_forward(ZML_HANDLE handle, int32_t axis);
/**
 * @brief Commands reverse axis motion.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_reverse(ZML_HANDLE handle, int32_t axis);

/**
 * @brief Starts drive homing after applying the supplied SDO configuration.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] config Drive homing SDO configuration.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_home_start(ZML_HANDLE handle, int32_t axis,
                                     const ZML_HomeSdoConfig* config);
/**
 * @brief Returns the current or latest terminal drive-home status for one axis.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] status Receives the drive homing status; see ZML_HOME_STATUS.
 * @param[out] error_id Receives the drive homing error ID.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_home_get_status(ZML_HANDLE handle, int32_t axis, int32_t* status,
                                          int32_t* error_id);
/**
 * @brief Stops drive homing through the existing FbtStop command path.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_home_stop(ZML_HANDLE handle, int32_t axis);

/**
 * @brief Sets the master axis.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] master_axis Master axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_set_master_axis(ZML_HANDLE handle, int32_t axis, int32_t master_axis);
/**
 * @brief Engages the axis gear.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_gear_in(ZML_HANDLE handle, int32_t axis);
/**
 * @brief Disengages the axis gear.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_gear_out(ZML_HANDLE handle, int32_t axis);

#ifdef __cplusplus
}
#endif

#endif  // ZML_MOTION_API_H
