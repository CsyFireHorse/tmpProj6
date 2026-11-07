#ifndef ZML_MOTION_API_H
#define ZML_MOTION_API_H
/* AI modification: GPT-6 Codex, 2026-10-12. Parameter mirror getters. */

#include <stdbool.h>
#include <stdint.h>

#include "zml_types_def.h"

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

/* Opaque context owned by the caller. */
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

/* Model signature: GPT-6. Axis contracts include explicit target axes. */
/* Opens an Ethernet RPC connection using ip address*/
ZML_API ZML_RESULT zml_open_eth(ZML_HANDLE* out_handle, const char* ipaddr);
/* Opens a local PCIe device selected by its device number. */
ZML_API ZML_RESULT zml_open_pci(ZML_HANDLE* out_handle, uint32_t alias);
/* Supports device, ip, ip:port, and ip:port/device strings. */
ZML_API ZML_RESULT zml_open(ZML_HANDLE* out_handle, const char* device);
/* Closes the connection and clears the caller's handle. */
ZML_API ZML_RESULT zml_close(ZML_HANDLE* handle);
/* Sends a synchronous no-op and waits for its motion-cycle confirmation. */
ZML_API ZML_RESULT zml_ping(ZML_HANDLE handle, uint32_t timeout_ms);
/* Returns the mailbox ping value unchanged on success. GPT-6 Codex. */
ZML_API ZML_RESULT zml_mailbox_ping(ZML_HANDLE handle, uint32_t pingValue, uint32_t* pongValue);

/* Writes the server version to a caller buffer of at least ZML_VERSION_INFO_MAX bytes. */
ZML_API ZML_RESULT zml_get_version_info(ZML_HANDLE handle, char* version);
/* Writes the server description to a caller buffer of at least ZML_ABOUT_INFO_MAX bytes. */
ZML_API ZML_RESULT zml_get_about(ZML_HANDLE handle, char* about);
/* Writes controller information to caller buffers of at least ZML_CONTROLLER_INFO_MAX bytes. */
ZML_API ZML_RESULT zml_get_controller_info(ZML_HANDLE handle, char* soft_type, char* soft_version,
                                           char* controller_id);

/* Returns one digital input after its motion-cycle read completes. */
ZML_API ZML_RESULT zml_read_di(ZML_HANDLE handle, uint32_t io_number, bool* value);
/* Returns one retained digital-output value after its motion-cycle read completes. */
ZML_API ZML_RESULT zml_read_do(ZML_HANDLE handle, uint32_t io_number, bool* value);
/* Enqueues one digital-output write without waiting for execution. */
ZML_API ZML_RESULT zml_write_do(ZML_HANDLE handle, uint32_t io_number, bool value);
/* Returns one unscaled analog-input value after its motion-cycle read completes. */
ZML_API ZML_RESULT zml_read_ai(ZML_HANDLE handle, uint32_t io_number, uint32_t* value);
/* Returns one retained analog-output value after its motion-cycle read completes. */
ZML_API ZML_RESULT zml_read_ao(ZML_HANDLE handle, uint32_t io_number, uint32_t* value);
/* Enqueues one unscaled analog-output write without waiting for execution. */
ZML_API ZML_RESULT zml_write_ao(ZML_HANDLE handle, uint32_t io_number, uint32_t value);

/* EtherCAT master status and control APIs. */
ZML_API ZML_RESULT zml_ecat_start(ZML_HANDLE handle);
ZML_API ZML_RESULT zml_ecat_stop(ZML_HANDLE handle);

ZML_API ZML_RESULT zml_ecat_bus_is_running(ZML_HANDLE handle, bool* run_flag);
ZML_API ZML_RESULT zml_ecat_request_master_state(ZML_HANDLE handle, uint8_t state);
ZML_API ZML_RESULT zml_ecat_get_master_req_state(ZML_HANDLE handle, uint8_t* state);
ZML_API ZML_RESULT zml_ecat_get_master_state(ZML_HANDLE handle, uint8_t* state);
ZML_API ZML_RESULT zml_ecat_coe_sdo_download(ZML_HANDLE handle, uint16_t slave_id, uint16_t index,
                                             uint8_t sub_index, uint32_t data_len,
                                             const void* data);
ZML_API ZML_RESULT zml_ecat_coe_sdo_upload(ZML_HANDLE handle, uint16_t slave_id, uint16_t index,
                                           uint8_t sub_index, uint32_t* data_len, void* data);
ZML_API ZML_RESULT zml_ecat_get_slave_count(ZML_HANDLE handle, uint16_t* slave_count);
ZML_API ZML_RESULT zml_ecat_get_active_slave_count(ZML_HANDLE handle, uint16_t* slave_count);
ZML_API ZML_RESULT zml_ecat_get_slave_state(ZML_HANDLE handle, uint16_t slave_id, uint8_t* state);
ZML_API ZML_RESULT zml_ecat_clear_slave_state_error(ZML_HANDLE handle, uint16_t slave_id);

// PCI only: enable PDO mirror cache
ZML_API ZML_RESULT zml_ecat_pi_enable_set(ZML_HANDLE handle, bool flag);
ZML_API ZML_RESULT zml_ecat_pi_enable_get(ZML_HANDLE handle, bool* flag);

ZML_API ZML_RESULT zml_ax_power_on(ZML_HANDLE handle, int32_t axis);
ZML_API ZML_RESULT zml_ax_power_off(ZML_HANDLE handle, int32_t axis);
ZML_API ZML_RESULT zml_ax_set_enable(ZML_HANDLE handle, int32_t axis, int32_t enable);
ZML_API ZML_RESULT zml_ax_get_enable(ZML_HANDLE handle, int32_t axis, int32_t* enable);

/* Reads the axis state word in the motion cycle. */
ZML_API ZML_RESULT zml_ax_read_state(ZML_HANDLE handle, int32_t axis, int32_t* state);
/* Reads the commanded (set) position in user units. */
ZML_API ZML_RESULT zml_ax_read_set_position(ZML_HANDLE handle, int32_t axis, double* position);
/* Reads the actual feedback position in user units. */
ZML_API ZML_RESULT zml_ax_read_act_position(ZML_HANDLE handle, int32_t axis, double* position);
/* Reads the negative-direction software position limit in user units. */
ZML_API ZML_RESULT zml_ax_read_sw_limit_negative(ZML_HANDLE handle, int32_t axis, double* limit);
/* Reads whether software position limits are enabled (0: disabled, 1: enabled). */
ZML_API ZML_RESULT zml_ax_read_sw_limit_enable(ZML_HANDLE handle, int32_t axis, int32_t* enable);
/* Reads the positive-direction software position limit in user units. */
ZML_API ZML_RESULT zml_ax_read_sw_limit_positive(ZML_HANDLE handle, int32_t axis, double* limit);
/* Reads the technology-units conversion ratio numerator in the motion cycle. */
ZML_API ZML_RESULT zml_ax_read_units_num(ZML_HANDLE handle, int32_t axis, int32_t* num);
/* Reads the technology-units conversion ratio denominator in the motion cycle. */
ZML_API ZML_RESULT zml_ax_read_units_den(ZML_HANDLE handle, int32_t axis, int32_t* den);

ZML_API ZML_RESULT zml_ax_set_velocity(ZML_HANDLE handle, int32_t axis, double speed);
/* Read the configured speed from the parameter mirror. */
ZML_API ZML_RESULT zml_ax_get_speed(ZML_HANDLE handle, int32_t axis, double* speed);
ZML_API ZML_RESULT zml_ax_set_accel(ZML_HANDLE handle, int32_t axis, double accel);
/* Read the configured acceleration from the parameter mirror. */
ZML_API ZML_RESULT zml_ax_get_accel(ZML_HANDLE handle, int32_t axis, double* accel);
/* Read the configured deceleration from the parameter mirror. */
ZML_API ZML_RESULT zml_ax_get_decel(ZML_HANDLE handle, int32_t axis, double* decel);
/* Read the configured jerk from the parameter mirror. */
ZML_API ZML_RESULT zml_ax_get_jerk(ZML_HANDLE handle, int32_t axis, double* jerk);
ZML_API ZML_RESULT zml_ax_set_decel(ZML_HANDLE handle, int32_t axis, double decel);
ZML_API ZML_RESULT zml_ax_set_jerk(ZML_HANDLE handle, int32_t axis, double jerk);

ZML_API ZML_RESULT zml_ax_set_position(ZML_HANDLE handle, int32_t axis, double pos);
ZML_API ZML_RESULT zml_ax_stop(ZML_HANDLE handle, int32_t axis);
ZML_API ZML_RESULT zml_ax_halt(ZML_HANDLE handle, int32_t axis);
ZML_API ZML_RESULT zml_ax_reset(ZML_HANDLE handle, int32_t axis);

ZML_API ZML_RESULT zml_ax_move_rel(ZML_HANDLE handle, int32_t axis, double distance);
ZML_API ZML_RESULT zml_ax_moveadd(ZML_HANDLE handle, int32_t axis, double pos);
ZML_API ZML_RESULT zml_ax_move_abs(ZML_HANDLE handle, int32_t axis, double pos);

ZML_API ZML_RESULT zml_ax_move_velocity(ZML_HANDLE handle, int32_t axis, int32_t dir);
ZML_API ZML_RESULT zml_ax_forward(ZML_HANDLE handle, int32_t axis);
ZML_API ZML_RESULT zml_ax_reverse(ZML_HANDLE handle, int32_t axis);

/* Starts drive homing after applying the supplied SDO configuration. */
ZML_API ZML_RESULT zml_ax_home_start(ZML_HANDLE handle, int32_t axis,
                                  const ZML_HomeSdoConfig* config);
/* Returns the current or latest terminal drive-home status for one axis. */
ZML_API ZML_RESULT zml_ax_home_get_status(ZML_HANDLE handle, int32_t axis, int32_t* status,
                                       int32_t* error_id);
/* Stops drive homing through the existing FbtStop command path. */
ZML_API ZML_RESULT zml_ax_home_stop(ZML_HANDLE handle, int32_t axis);

// ZML_API ZML_RESULT zml_set_base_axis(ZML_HANDLE handle, int32_t axis);
ZML_API ZML_RESULT zml_set_master_axis(ZML_HANDLE handle, int32_t axis, int32_t master_axis);
ZML_API ZML_RESULT zml_gear_in(ZML_HANDLE handle, int32_t axis);
ZML_API ZML_RESULT zml_gear_out(ZML_HANDLE handle, int32_t axis);

#ifdef __cplusplus
}
#endif

#endif /* ZML_MOTION_API_H */
