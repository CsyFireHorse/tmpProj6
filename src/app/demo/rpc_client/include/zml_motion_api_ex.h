#ifndef _ZML_MOTION_API_EX_H_
#define _ZML_MOTION_API_EX_H_

#include "zml_motion_api.h"
#include "zml_motion_types.h"
#include "zml_io_metadata.h"

/* AI modification signature: GPT-5 Codex */

#ifdef __cplusplus
extern "C" {
#endif

/* Read one numeric axis parameter in the motion cycle. Model signature: GPT-6 Codex. */
ZML_API ZML_RESULT zml_ax_read_parameter(ZML_HANDLE handle, int32_t axis, int32_t parameter_number,
                                      double* value);

/* Write one numeric axis parameter and wait for its execution ACK. GPT-6 Codex. */
ZML_API ZML_RESULT zml_ax_write_parameter(ZML_HANDLE handle, int32_t axis, int32_t parameter_number,
                                       double value);

/* Enqueue asynchronous writes to registered virtual digital and analog inputs. */
ZML_API ZML_RESULT zml_write_virtual_di(ZML_HANDLE handle, uint32_t io_number, bool value);
ZML_API ZML_RESULT zml_write_virtual_ai(ZML_HANDLE handle, uint32_t io_number, uint32_t value);

ZML_API ZML_RESULT zml_set_flow_config(ZML_HANDLE handle, const ZML_FlowCtrlConfig* cfg);
ZML_API ZML_RESULT zml_get_flow_status(ZML_HANDLE handle, ZML_FlowCtrlStatus* out_status);
ZML_API ZML_RESULT zml_get_flow_config(ZML_HANDLE handle, ZML_FlowCtrlConfig* out_config);
ZML_API ZML_RESULT zml_ax_get_state(ZML_HANDLE handle, int32_t axis, ZML_AxisState* out_state);
ZML_API ZML_RESULT zml_ax_get_state_ex(ZML_HANDLE handle, int32_t axis,
                                         ZML_AxisStateEx* out_state);
ZML_API ZML_RESULT zml_get_cmd_results(ZML_HANDLE handle, ZML_CmdResultInfo* out,
                                       uint32_t* inout_len);
/* Read configured IO alias metadata; out_count supports a null/zero-capacity size query. */
ZML_API ZML_RESULT zml_get_io_metadata(ZML_HANDLE handle, ZML_IoMetadataEntry* out,
                                       uint32_t out_capacity, uint32_t* out_count);

/* Copies a local client file to the named server_files entry, replacing it when present. */
ZML_API ZML_RESULT zml_download_file(ZML_HANDLE handle, const char* localFileName,
                                     const char* remoteFileName, uint32_t timeout_ms);
/* Creates a local client file from the named server_files entry without overwriting it. */
ZML_API ZML_RESULT zml_upload_file(ZML_HANDLE handle, const char* localFileName,
                                   const char* remoteFileName, uint32_t timeout_ms);
/* Downloads to rpc_eni.xml, then selects server_files/rpc_eni.xml using a five-second timeout. */
ZML_API ZML_RESULT zml_download_eni(ZML_HANDLE handle, const char* pathName);
/* Downloads to rpc_syscfg.ini, then selects server_files/rpc_syscfg.ini at five seconds. */
ZML_API ZML_RESULT zml_download_syscfg(ZML_HANDLE handle, const char* pathName);
/* Creates a local file from rpc_eni.xml using a five-second timeout. */
ZML_API ZML_RESULT zml_upload_eni(ZML_HANDLE handle, const char* pathName);
/* Creates a local file from rpc_syscfg.ini using a five-second timeout. */
ZML_API ZML_RESULT zml_upload_syscfg(ZML_HANDLE handle, const char* pathName);

ZML_API ZML_RESULT zml_ax_set_fastdecel(ZML_HANDLE handle, int32_t axis, double fast_decel);
ZML_API ZML_RESULT zml_ax_set_direction(ZML_HANDLE handle, int32_t axis, int32_t dir);
ZML_API ZML_RESULT zml_ax_set_buffermode(ZML_HANDLE handle, int32_t axis, int32_t buffer_mode);

ZML_API ZML_RESULT zml_ax_set_speed_ratio(ZML_HANDLE handle, int32_t axis, double ratio);

ZML_API ZML_RESULT zml_ax_set_units(ZML_HANDLE handle, int32_t axis, int32_t num, int32_t den);
/* Read the units value from the parameter mirror. */
ZML_API ZML_RESULT zml_ax_get_units(ZML_HANDLE handle, int32_t axis, double* units);

// enum{initial = 0, connected= 1, disconnected= 2, reset= 3}
ZML_API ZML_RESULT zml_get_connection_state(ZML_HANDLE handle);
ZML_API ZML_RESULT zml_wait_until_connection_ready(ZML_HANDLE handle, int32_t count);
// depreciated
ZML_API ZML_RESULT zml_ax_wait_ticks(ZML_HANDLE handle, uint32_t n_tick, int32_t axis);
ZML_API ZML_RESULT zml_ax_wait_seconds(ZML_HANDLE handle, double seconds, int32_t axis);
/* Blocks the caller until the axis reaches standstill; ErrorStop returns an error,
   any other state until the timeout returns a wait-timeout code. */
ZML_API ZML_RESULT zml_ax_wait_idle(ZML_HANDLE handle, int32_t axis, uint32_t timeout_ms);

/* Client-side sleep: blocks the calling thread for the given milliseconds. */
ZML_API ZML_RESULT zml_sleep_ms(uint32_t ms);

ZML_API ZML_RESULT zml_execute_command(ZML_HANDLE handle, const char* cmd);

/* Legacy Position-based and non-SDO homing APIs. */
ZML_API ZML_RESULT zml_ax_home(ZML_HANDLE handle, int32_t axis, double pos);
ZML_API ZML_RESULT zml_ax_homing(ZML_HANDLE handle, int32_t axis);
ZML_API ZML_RESULT zml_ax_datum(ZML_HANDLE handle, int32_t axis, int32_t mode);

ZML_API ZML_RESULT zml_moveabs(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                               const double* pos);
ZML_API ZML_RESULT zml_moverel(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                               const double* distance);

ZML_API ZML_RESULT zml_group_select(ZML_HANDLE handle, int32_t idx);
ZML_API ZML_RESULT zml_group_setaxes(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes);
ZML_API ZML_RESULT zml_group_enable(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes);
ZML_API ZML_RESULT zml_group_disable(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes);
ZML_API ZML_RESULT zml_group_reset(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes);

/* Circular RPC calls require ax_num == 2 and axes to contain both axis indices. */
ZML_API ZML_RESULT zml_movecirc(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                                double end1, double end2, double centre1, double centre2,
                                int32_t direction);
ZML_API ZML_RESULT zml_movecircabs(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                                   double end1, double end2, double centre1, double centre2,
                                   int32_t direction);
ZML_API ZML_RESULT zml_movecirc2(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                                 double mid1, double mid2, double end1, double end2);
ZML_API ZML_RESULT zml_movecirc2abs(ZML_HANDLE handle, int32_t ax_num, const int32_t* axes,
                                    double mid1, double mid2, double end1, double end2);
#ifdef __cplusplus
}
#endif

#endif /* _ZML_MOTION_API_EX_H_ */
