#ifndef _ZML_MOTION_API_EX_H_
#define _ZML_MOTION_API_EX_H_

#include "zml_motion_api.h"
#include "zml_motion_types.h"
#include "zml_io_metadata.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reads one numeric axis parameter in the motion cycle.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] parameter_number Numeric axis parameter number.
 * @param[out] value Receives the requested value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_read_parameter(ZML_HANDLE handle, int32_t axis, int32_t parameter_number,
                                         double* value);

/**
 * @brief Writes one numeric axis parameter and waits for its execution ACK.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] parameter_number Numeric axis parameter number.
 * @param[in] value Value to write.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_write_parameter(ZML_HANDLE handle, int32_t axis,
                                          int32_t parameter_number, double value);

/**
 * @brief Enqueues asynchronous writes to registered virtual digital and analog inputs.
 * @param[in] handle RPC connection handle.
 * @param[in] io_number IO number.
 * @param[in] value Value to write.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_write_virtual_di(ZML_HANDLE handle, uint32_t io_number, bool value);
/**
 * @brief Enqueues an asynchronous write to a registered virtual analog input.
 * @param[in] handle RPC connection handle.
 * @param[in] io_number IO number.
 * @param[in] value Value to write.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_write_virtual_ai(ZML_HANDLE handle, uint32_t io_number, uint32_t value);

/**
 * @brief Sets the flow control configuration.
 * @param[in] handle RPC connection handle.
 * @param[in] cfg Flow control configuration to apply.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_set_flow_config(ZML_HANDLE handle, const ZML_FlowCtrlConfig* cfg);
/**
 * @brief Gets the flow control status.
 * @param[in] handle RPC connection handle.
 * @param[out] out_status Receives the flow control status.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_get_flow_status(ZML_HANDLE handle, ZML_FlowCtrlStatus* out_status);
/**
 * @brief Gets the flow control configuration.
 * @param[in] handle RPC connection handle.
 * @param[out] out_config Receives the flow control configuration.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_get_flow_config(ZML_HANDLE handle, ZML_FlowCtrlConfig* out_config);
/**
 * @brief Gets the axis state.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] out_state Receives the axis state.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_get_state(ZML_HANDLE handle, int32_t axis, ZML_AxisState* out_state);
/**
 * @brief Gets the extended axis state.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] out_state Receives the axis state.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_get_state_ex(ZML_HANDLE handle, int32_t axis,
                                       ZML_AxisStateEx* out_state);
/**
 * @brief Gets command results.
 * @param[in] handle RPC connection handle.
 * @param[out] out Array receiving command results; its capacity is supplied in inout_len.
 * @param[in,out] inout_len On input, output array capacity; on return, required result count.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_get_cmd_results(ZML_HANDLE handle, ZML_CmdResultInfo* out,
                                       uint32_t* inout_len);
/**
 * @brief Reads configured IO alias metadata; out_count supports a null/zero-capacity size query.
 * @param[in] handle RPC connection handle.
 * @param[out] out Array receiving metadata entries; may be null when out_capacity is zero.
 * @param[in] out_capacity Output array capacity in entries.
 * @param[out] out_count Receives the required metadata entry count.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_get_io_metadata(ZML_HANDLE handle, ZML_IoMetadataEntry* out,
                                       uint32_t out_capacity, uint32_t* out_count);

/**
 * @brief Copies a local client file to the named server_files entry, replacing it when present.
 * @param[in] handle RPC connection handle.
 * @param[in] localFileName Local client file path.
 * @param[in] remoteFileName Server file entry name.
 * @param[in] timeout_ms Timeout in milliseconds.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_download_file(ZML_HANDLE handle, const char* localFileName,
                                     const char* remoteFileName, uint32_t timeout_ms);
/**
 * @brief Creates a local client file from the named server_files entry without overwriting it.
 * @param[in] handle RPC connection handle.
 * @param[in] localFileName Local client file path.
 * @param[in] remoteFileName Server file entry name.
 * @param[in] timeout_ms Timeout in milliseconds.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_upload_file(ZML_HANDLE handle, const char* localFileName,
                                   const char* remoteFileName, uint32_t timeout_ms);
/**
 * @brief Downloads to rpc_eni.xml, then selects server_files/rpc_eni.xml using a five-second
 * timeout.
 * @param[in] handle RPC connection handle.
 * @param[in] pathName Local client file path.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_download_eni(ZML_HANDLE handle, const char* pathName);
/**
 * @brief Downloads to rpc_syscfg.ini, then selects server_files/rpc_syscfg.ini using a five-second
 * timeout.
 * @param[in] handle RPC connection handle.
 * @param[in] pathName Local client file path.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_download_syscfg(ZML_HANDLE handle, const char* pathName);
/**
 * @brief Creates a local file from rpc_eni.xml using a five-second timeout.
 * @param[in] handle RPC connection handle.
 * @param[in] pathName Local client file path.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_upload_eni(ZML_HANDLE handle, const char* pathName);
/**
 * @brief Creates a local file from rpc_syscfg.ini using a five-second timeout.
 * @param[in] handle RPC connection handle.
 * @param[in] pathName Local client file path.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_upload_syscfg(ZML_HANDLE handle, const char* pathName);
// ZML_API ZML_RESULT zml_set_base_axis(ZML_HANDLE handle, int32_t axis);
/**
 * @brief Sets the axis fast deceleration.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] fast_decel Fast deceleration value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_fastdecel(ZML_HANDLE handle, int32_t axis, double fast_decel);
/**
 * @brief Sets the axis direction.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] dir Direction value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_direction(ZML_HANDLE handle, int32_t axis, int32_t dir);
/**
 * @brief Sets the axis buffer mode.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] buffer_mode Axis buffer mode.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_buffermode(ZML_HANDLE handle, int32_t axis, int32_t buffer_mode);

/**
 * @brief Sets the axis speed ratio.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] ratio Speed ratio.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_speed_ratio(ZML_HANDLE handle, int32_t axis, double ratio);

/**
 * @brief Sets the axis units conversion ratio.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] num Units conversion ratio numerator.
 * @param[in] den Units conversion ratio denominator.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_set_units(ZML_HANDLE handle, int32_t axis, int32_t num, int32_t den);
/**
 * @brief Reads the units value from the parameter mirror.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[out] units Receives the units value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_get_units(ZML_HANDLE handle, int32_t axis, double* units);

/**
 * @brief Gets the connection state.
 * @param[in] handle RPC connection handle.
 * @return Connection state: 0 initial, 1 connected, 2 disconnected, 3 reset.
 */
ZML_API ZML_RESULT zml_get_connection_state(ZML_HANDLE handle);
/**
 * @brief Waits for the connection to become ready.
 * @param[in] handle RPC connection handle.
 * @param[in] count Maximum number of connection checks, at 50 ms per check.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_wait_until_connection_ready(ZML_HANDLE handle, int32_t count);
/**
 * @brief Deprecated: waits for the specified axis ticks.
 * @param[in] handle RPC connection handle.
 * @param[in] n_tick Number of ticks to wait.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_wait_ticks(ZML_HANDLE handle, uint32_t n_tick, int32_t axis);
/**
 * @brief Waits for the specified number of seconds for an axis.
 * @param[in] handle RPC connection handle.
 * @param[in] seconds Time in seconds.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_wait_seconds(ZML_HANDLE handle, double seconds, int32_t axis);
/**
 * @brief Blocks the caller until the axis reaches standstill; ErrorStop returns an error, any
 * other state until the timeout returns a wait-timeout code.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] timeout_ms Timeout in milliseconds.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_wait_idle(ZML_HANDLE handle, int32_t axis, uint32_t timeout_ms);

/**
 * @brief Client-side sleep: blocks the calling thread for the given milliseconds.
 * @param[in] ms Sleep duration in milliseconds.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_sleep_ms(uint32_t ms);

/**
 * @brief Executes a command string.
 * @param[in] handle RPC connection handle.
 * @param[in] cmd Command string.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_execute_command(ZML_HANDLE handle, const char* cmd);

/**
 * @brief Starts legacy Position-based homing.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] pos Position value.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_home(ZML_HANDLE handle, int32_t axis, double pos);
/**
 * @brief Starts legacy non-SDO homing.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_homing(ZML_HANDLE handle, int32_t axis);
/**
 * @brief Starts legacy datum homing.
 * @param[in] handle RPC connection handle.
 * @param[in] axis Target axis index.
 * @param[in] mode Homing mode.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_ax_datum(ZML_HANDLE handle, int32_t axis, int32_t mode);

/**
 * @brief Commands an absolute move for multiple axes.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes in the axes array.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @param[in] pos Array of target positions, containing ax_num entries.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_moveabs(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                               const double* pos);
/**
 * @brief Commands a relative move for multiple axes.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes in the axes array.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @param[in] distance Array of relative distances, containing ax_num entries.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_moverel(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                               const double* distance);

/**
 * @brief Selects an axis group.
 * @param[in] handle RPC connection handle.
 * @param[in] idx Group index.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_group_select(ZML_HANDLE handle, int32_t idx);
/**
 * @brief Sets the axes in the selected group.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes in the axes array.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_group_setaxes(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes);
/**
 * @brief Enables an axis group.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes in the axes array.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_group_enable(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes);
/**
 * @brief Disables an axis group.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes in the axes array.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_group_disable(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes);
/**
 * @brief Resets an axis group.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes in the axes array.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_group_reset(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes);

/**
 * @brief Commands a circular move using an end point and centre.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes; must be 2.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @param[in] end1 End point coordinate for the first axis.
 * @param[in] end2 End point coordinate for the second axis.
 * @param[in] centre1 Centre coordinate for the first axis.
 * @param[in] centre2 Centre coordinate for the second axis.
 * @param[in] direction Circular motion direction.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_movecirc(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                                double end1, double end2, double centre1, double centre2,
                                int32_t direction);
/**
 * @brief Commands an absolute circular move using an end point and centre.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes; must be 2.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @param[in] end1 End point coordinate for the first axis.
 * @param[in] end2 End point coordinate for the second axis.
 * @param[in] centre1 Centre coordinate for the first axis.
 * @param[in] centre2 Centre coordinate for the second axis.
 * @param[in] direction Circular motion direction.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_movecircabs(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                                   double end1, double end2, double centre1, double centre2,
                                   int32_t direction);
/**
 * @brief Commands a circular move through an intermediate point.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes; must be 2.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @param[in] mid1 Intermediate point coordinate for the first axis.
 * @param[in] mid2 Intermediate point coordinate for the second axis.
 * @param[in] end1 End point coordinate for the first axis.
 * @param[in] end2 End point coordinate for the second axis.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_movecirc2(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                                 double mid1, double mid2, double end1, double end2);
/**
 * @brief Commands an absolute circular move through an intermediate point.
 * @param[in] handle RPC connection handle.
 * @param[in] ax_num Number of axes; must be 2.
 * @param[in] axes Array of axis indices, containing ax_num entries.
 * @param[in] mid1 Intermediate point coordinate for the first axis.
 * @param[in] mid2 Intermediate point coordinate for the second axis.
 * @param[in] end1 End point coordinate for the first axis.
 * @param[in] end2 End point coordinate for the second axis.
 * @return ZML result code.
 */
ZML_API ZML_RESULT zml_movecirc2abs(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                                    double mid1, double mid2, double end1, double end2);
#ifdef __cplusplus
}
#endif

#endif  // _ZML_MOTION_API_EX_H_
