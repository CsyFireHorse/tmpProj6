// Date: 2026-10-21; drafted by GPT-6.
#ifndef _ZMC_MC_COMM_RESULT_MESSAGE_HPP_
#define _ZMC_MC_COMM_RESULT_MESSAGE_HPP_

#include "common/zmc/mc_errno.h"

static inline const char* host_comm_message(MCCommResult code) {
    switch (code) {
        case ZMC_ERR_RPC_JSON_INVALID:
            return "invalid JSON in RPC/HTTP request";
        case ZMC_ERR_RPC_JSON_MISSING_FIELD:
            return "missing required JSON field in RPC/HTTP request";
        case ZMC_ERR_RPC_CLIENT_INVALID_ARG:
            return "invalid RPC/HTTP client argument";
        case ZMC_ERR_RPC_RECV_BUFF_TOO_SMALL:
            return "RPC/HTTP receive buffer too small";
        case ZMC_ERR_RPC_INVALID_ARG:
            return "invalid RPC/HTTP request argument";
        // HTTP remote file service errors.
        case ZMC_ERR_RPC_FILE_INVALID_NAME:
            return "invalid remote file name";
        case ZMC_ERR_RPC_FILE_NOT_FOUND:
            return "remote file not found";
        case ZMC_ERR_RPC_FILE_ALREADY_EXISTS:
            return "remote file already exists";
        case ZMC_ERR_RPC_FILE_TOO_LARGE:
            return "remote file exceeds the size limit";
        case ZMC_ERR_RPC_FILE_INVALID_REQUEST:
            return "invalid remote file request";
        case ZMC_ERR_RPC_FILE_IO_FAILED:
            return "remote file I/O failed";
        case ZMC_ERR_RPC_CLIENT_CONNECT_FAILED:
            return "RPC client connect failed";
        case ZMC_ERR_RPC_CLIENT_NOT_OPEN:
            return "RPC client session not open";
        case ZMC_ERR_RPC_CLIENT_DISCONNECTED:
            return "RPC client disconnected";
        case ZMC_ERR_RPC_CLIENT_CONNECT_TIMEOUT:
            return "RPC client connect timeout";
        case ZMC_ERR_RPC_CLIENT_CALL_FAILED:
            return "RPC client call failed";
        case ZMC_ERR_ENI_FILE_OPEN_FAILED:
            return "cannot open or read ENI file";
        case ZMC_ERR_ENI_JSON_NOT_FOUND:
            return "ENI embedded JSON marker not found";
        case ZMC_ERR_ENI_JSON_INVALID:
            return "invalid ENI embedded JSON";
        case ZMC_ERR_ENI_SLAVE_INVALID:
            return "invalid or empty ENI slave list";
        case ZMC_ERR_ENI_SLAVE_OVERFLOW:
            return "ENI slave count overflow";
        case ZMC_ERR_ENI_AXIS_OVERFLOW:
            return "ENI servo axis count overflow";
        case ZMC_ERR_ENI_POD_OVERFLOW:
            return "ENI descriptor POD PDO cell overflow";
        case ZMC_ERR_ENI_FILE_TOO_LARGE:
            return "ENI file too large";
        case ZMC_ERR_INI_FILE_OPEN:
            return "cannot open or read INI file";
        case ZMC_ERR_INI_PARSE:
            return "INI file too large or malformed";
        case ZMC_ERR_INI_MISSING_ENI_PATH:
            return "INI [base] section missing eni_path";
        case ZMC_ERR_INI_MISSING_SECTION:
            return "required INI section is missing";
        case ZMC_ERR_INI_MISSING_KEY:
            return "required INI key is missing";
        case ZMC_ERR_INI_KEY:
            return "invalid INI key";
        case ZMC_ERR_INI_VALUE_FORMAT:
            return "invalid INI value format";
        case ZMC_ERR_INI_VALUE_RANGE:
            return "INI value is out of range";
        case ZMC_ERR_INI_DUPLICATE:
            return "duplicate INI configuration entry";
        case ZMC_ERR_INI_AXIS_BINDING_CONFIG:
            return "invalid INI axis binding configuration";
        case ZMC_ERR_INI_IO_BINDING_CONFIG:
            return "invalid INI I/O binding configuration";
        case ZMC_ERR_AX_BIND_ADDR_FAILED:
            return "axis binding address resolution failed";
        case ZMC_ERR_AX_BIND_OVER_RANGE:
            return "axis binding index is out of range";
        case ZMC_ERR_AX_BIND_OVERLAP:
            return "axis binding overlaps an existing binding";
        case ZMC_ERR_DEV_INVALID_HANDLE:
            return "invalid device handle";
        case ZMC_ERR_DEV_RECV_BUFF_TOO_SMALL:
            return "device receive buffer too small";
        case ZMC_ERR_DEV_RECV_TIMEOUT:
            return "device receive timeout";
        case ZMC_ERR_DEV_SEND_TIMEOUT:
            return "device send timeout";
        case ZMC_ERR_DEV_INVALID_ARG:
            return "invalid device argument";
        case ZMC_ERR_DEV_NO_DATA:
            return "no device data available";
        case ZMC_ERR_DEV_NOT_OPEN:
            return "device API handle not open";
        case ZMC_ERR_DEV_MALLOC_FAILED:
            return "device malloc failed";
        case ZMC_ERR_DEV_BOARD_OPEN_FAILED:
            return "device board open failed";
        case ZMC_ERR_DEV_LOCAL_FILE_OPEN_FAILED:
            return "device local file open failed";
        case ZMC_ERR_DEV_FILE_TRANSFER_FAILED:
            return "device file transfer failed";
        case ZMC_ERR_DEV_FILE_STAT_FAILED:
            return "device file stat failed";
        case ZMC_ERR_DEV_SDO_ADAPTER_NULL_ARG:
            return "device SDO adapter null argument";
        case ZMC_ERR_DEV_MASTER_STATE_SWITCH_TIMEOUT:
            return "device master state switch timeout";
        case ZMC_ERR_DEV_IPC_PIPE_FAILED:
            return "device IPC pipe open failed";
        case ZMC_ERR_DEV_NOT_IMPLEMENTED:
            return "device operation not implemented";
        case ZMC_ERR_DEV_INTERNAL:
            return "internal device error";
        case ZMC_ERR_DEV_UNKNOWN:
            return "unknown device error";
        case ZMC_ERR_DEV_QUEUE_FULL:
            return "device queue is full";
        case ZMC_ERR_DEV_NULL_ARG:
            return "null device argument";
        case ZMC_ERR_DEV_OVER_RANGE:
            return "device argument out of range";
        case ZMC_ERR_HOST_INVALID_ARG:
            return "invalid host argument";
        case ZMC_ERR_HOST_NULL_ARG:
            return "null host argument";
        case ZMC_ERR_HOST_OVER_RANGE:
            return "host argument out of range";
        case ZMC_ERR_HOST_MALLOC_FAILED:
            return "host malloc failed";
        case ZMC_ERR_HOST_BOARD_OPEN_FAILED:
            return "host board open failed";
        case ZMC_ERR_HOST_NOT_IMPLEMENTED:
            return "host operation not implemented";
        case ZMC_ERR_HOST_RECV_BUFF_TOO_SMALL:
            return "host receive buffer too small";
        case ZMC_ERR_HOST_INVALID_HANDLE:
            return "invalid host handle";
        case ZMC_ERR_HOST_HANDLE_ALREADY_OPEN:
            return "host handle already open";
        case ZMC_ERR_HOST_NO_DATA:
            return "host no data available";
        // Host-prefixed utility errors require explicit mapping because some values
        // do not have a one-to-one counterpart after removing the host offset.
        case ZMC_ERR_HOST_ENI_FILE_OPEN_FAILED:
            return "cannot open or read ENI file";
        case ZMC_ERR_HOST_ENI_JSON_NOT_FOUND:
            return "ENI embedded JSON marker not found";
        case ZMC_ERR_HOST_ENI_JSON_INVALID:
            return "invalid ENI embedded JSON";
        case ZMC_ERR_HOST_ENI_SLAVE_INVALID:
            return "invalid or empty ENI slave list";
        case ZMC_ERR_HOST_ENI_SLAVE_OVERFLOW:
            return "ENI slave count overflow";
        case ZMC_ERR_HOST_ENI_AXIS_OVERFLOW:
            return "ENI servo axis count overflow";
        case ZMC_ERR_HOST_ENI_POD_OVERFLOW:
            return "ENI descriptor POD PDO cell overflow";
        case ZMC_ERR_HOST_ENI_FILE_TOO_LARGE:
            return "ENI file too large";
        case ZMC_ERR_HOST_INI_FILE_OPEN:
            return "cannot open or read INI file";
        case ZMC_ERR_HOST_INI_PARSE:
            return "INI file too large or malformed";
        case ZMC_ERR_HOST_INI_MISSING_ENI_PATH:
            return "INI [base] section missing eni_path";
        case ZMC_ERR_HOST_INI_AXIS_NUM_OVER:
            return "INI axis count exceeds the supported limit";
        case ZMC_ERR_HOST_RECV_TIMEOUT:
            return "host receive timeout";
        case ZMC_ERR_HOST_SEND_TIMEOUT:
            return "host send timeout";
        case ZMC_ERR_HOST_NOT_OPEN:
            return "host API handle not open";
        case ZMC_ERR_HOST_LOCAL_FILE_OPEN_FAILED:
            return "host local file open failed";
        case ZMC_ERR_HOST_FILE_TRANSFER_FAILED:
            return "host file transfer failed";
        case ZMC_ERR_HOST_IPC_PIPE_FAILED:
            return "host IPC pipe open failed";
        case ZMC_ERR_HOST_INTERNAL:
            return "internal host error";
        case ZMC_ERR_HOST_FILE_STAT_FAILED:
            return "host file stat failed";
        case ZMC_ERR_HOST_MASTER_STATE_SWITCH_TIMEOUT:
            return "host master state switch timeout";
        case ZMC_ERR_HOST_SDO_ADAPTER_NULL_ARG:
            return "host SDO adapter null argument";
        case ZMC_ERR_HOST_UNKNOWN:
            return "unknown host error";
        case ZMC_ERR_HOST_FILE_OPEN_FAILED:
            return "host file open failed";
        // Motion control errors surfaced through the host API.
        case ZMC_ERR_SCM_MODE_SWITCH_TIMEOUT:
            return "controller mode switch timeout";
        case ZMC_ERR_P_WHEN_DRIVE_ERROR:
            return "cannot power on while the drive has an error";
        case ZMC_ERR_P_TIMEOUT:
            return "power operation timeout";
        case ZMC_ERR_R_TIMEOUT:
            return "reset operation timeout";
        case ZMC_ERR_WP_VALUE_INVALID:
            return "invalid write-parameter value";
        case ZMC_ERR_H_AXIS_IN_MOVING:
            return "cannot home while the axis is moving";
        case ZMC_ERR_MSI_SUPERIMPOSED_NOT_SUPPORTED:
            return "superimposed motion is not supported";
        case ZMC_ERR_WRC_DUMP_FAILED:
            return "write cam dump failed";
        case ZMC_ERR_SERVO_ERROR:
            return "servo drive error";
        case ZMC_ERR_UNKNOWN_SW_ERROR:
            return "unknown axis software error";
        case ZMC_ERR_SLAVE_SAME_AXIS_REF:
            return "slave axis must not reference itself";
        case ZMC_ERR_FB_ACTIVATE_AXIS_ERRORSTOP:
            return "cannot activate an axis in error stop";
        case ZMC_ERR_POWEROFF_WHEN_MOVING:
            return "cannot power off while the axis is moving";
        case ZMC_ERR_ERRORSTOP_WHEN_MOVING:
            return "cannot enter error stop while the axis is moving";
        case ZMC_ERR_AXIS_STATE_VIOLATION:
            return "axis state violation";
        case ZMC_ERR_INVALID_SW_ERROR_REACTION:
            return "invalid software error reaction configuration";
        case ZMC_ERR_FB_ACTIVATE_AXIS_STOPPING:
            return "cannot activate an axis while it is stopping";
        case ZMC_ERR_VEL_VEL_ACC_DEC_INVALID:
            return "invalid velocity, acceleration, or deceleration";
        case ZMC_ERR_VEL_VEL_ACC_INVALID:
            return "invalid velocity or acceleration";
        case ZMC_ERR_VEL_BUILD_FAILED:
            return "velocity profile build failed";
        case ZMC_ERR_VEL_POS_JUMP:
            return "velocity command caused a position jump";
        case ZMC_ERR_AXIS_NOT_IN_GEAR:
            return "axis is not geared";
        case ZMC_ERR_AXIS_NOT_IN_CAM:
            return "axis is not cammed";
        case ZMC_ERR_AXIS_NOT_IN_SYNC_MOTION:
            return "axis is not in synchronized motion";
        case ZMC_ERR_POSITION_NOT_CONVERGENCE:
            return "position did not converge";
        case ZMC_ERR_SETPOSITION_WHILE_MOVING:
            return "cannot set position while the axis is moving";
        case ZMC_ERR_REPDBG_TRIG_ERROR:
            return "replay debug trigger error";
        case ZMC_ERR_REPDBG_OVERFLOW_ERROR:
            return "replay debug buffer overflow";
        case ZMC_ERR_INVALIDSTATE_FROMSTOPPING:
            return "invalid state transition from stopping";
        case ZMC_ERR_INVALIDSTATE_FROMERRORSTOP:
            return "invalid state transition from error stop";
        case ZMC_ERR_INVALIDSTATE_FROMDISABLED:
            return "invalid state transition from disabled";
        case ZMC_ERR_INVALID_DIRECTION_POSITIVE:
            return "motion is forbidden in the positive direction";
        case ZMC_ERR_INVALID_DIRECTION_NEGATIVE:
            return "motion is forbidden in the negative direction";
        case ZMC_ERR_POSITION_OVER_POSITIVE_LIMIT:
            return "position exceeds the positive limit";
        case ZMC_ERR_POSITION_OVER_NEGATIVE_LIMIT:
            return "position exceeds the negative limit";
        case ZMC_ERR_CFG_HARDWARE_PARAM_INVALID:
            return "invalid hardware parameter configuration";
        // Axis group errors.
        case ZMC_ERR_NO_AXISGRP_REF:
            return "invalid axis group reference";
        case ZMC_ERR_AXISGRP_EMPTY:
            return "axis group is empty";
        case ZMC_ERR_AXISGRP_FULL:
            return "axis group is full";
        case ZMC_ERR_AXISGRP_INVALID_STATE:
            return "invalid axis group state";
        case ZMC_ERR_AXISGRP_NO_ERROR_TO_RESET:
            return "axis group has no error to reset";
        case ZMC_ERR_AXISGRP_AXIS_NOT_FOUND:
            return "axis was not found in the group";
        case ZMC_ERR_AXISGRP_ERRORSTOP:
            return "axis group is in error stop";
        case ZMC_ERR_MOTCMD_AXIS_STATE_INVALID_AXIS:
            return "invalid axis for status query";
        case ZMC_ERR_MOTCMD_AXIS_STATE2_INVALID_AXIS:
            return "invalid axis for status2 query";
        case ZMC_ERR_MOTCMD_CMD_RESULTS_EMPTY:
            return "no motion status packet available";
        case ZMC_ERR_MOTCMD_WAIT_INVALID_ARG:
            return "invalid motcmd wait argument";
        case ZMC_ERR_MOTCMD_SCRIPT_BUSY_DISCARD:
            return "motcmd script busy discard";
        case ZMC_ERR_MOTCMD_ABORT_PUSH_FAILED:
            return "motcmd abort push failed";
        case ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED:
            return "motcmd packet append failed";
        case ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED:
            return "motcmd queue push failed";
        case ZMC_ERR_MOTCMD_INVALID_ARG:
            return "invalid motcmd argument";
        case ZMC_ERR_MOTCMD_SCRIPT_EMPTY:
            return "motcmd script empty";
        case ZMC_ERR_MOTCMD_SCRIPT_LINE_EMPTY:
            return "motcmd script line empty";
        case ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG:
            return "motcmd script missing argument";
        case ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED:
            return "motcmd script parse failed";
        case ZMC_ERR_MOTCMD_UNKNOWN_COMMAND:
            return "unknown motcmd command";
        case ZMC_ERR_MOTCMD_IO_RESULTS_EMPTY:
            return "no I/O status packet available";
        // AxisFbBridge local API errors.
        case ZMC_ERR_BRIDGE_INVALID_ARG:
            return "invalid bridge argument";
        case ZMC_ERR_BRIDGE_INVALID_HANDLE:
            return "invalid bridge handle";
        case ZMC_ERR_BRIDGE_PDO_UNCONFIGURED:
            return "bridge PDO is not configured";
        case ZMC_ERR_BRIDGE_PDO_EMPTY_LAYOUT:
            return "bridge PDO layout is empty";
        case ZMC_ERR_BRIDGE_SDO_ADAPTER_INVALID:
            return "invalid bridge SDO adapter";
        case ZMC_ERR_BRIDGE_SDO_NOT_STARTED:
            return "bridge SDO service is not started";
        case ZMC_ERR_BRIDGE_SDO_THREAD_START_FAILED:
            return "bridge SDO worker thread failed to start";
        // Bridge IO manager errors.
        case ZMC_ERR_IO_INVALID_ARG:
            return "invalid bridge I/O argument";
        case ZMC_ERR_IO_OBJECT_NOT_FOUND:
            return "bridge I/O object not found";
        case ZMC_ERR_IO_ALIAS_DUPLICATE:
            return "duplicate bridge I/O alias";
        case ZMC_ERR_IO_ALIAS_CAPACITY:
            return "bridge I/O alias capacity exceeded";
        case ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND:
            return "bridge I/O process image is unbound";
        case ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED:
            return "unsupported bridge I/O data size";
        case ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE:
            return "bridge I/O process image access is out of range";
        case ZMC_ERR_IO_MASK_INVALID:
            return "invalid bridge I/O bit mask";
        // Trajectory and spline errors.
        case ZMC_ERR_TRAJ_BUILD_FAILED:
            return "trajectory build failed";
        case ZMC_ERR_TRAJ_NOT_IMPLEMENTED:
            return "trajectory operation is not implemented";
        case ZMC_ERR_TRAJ_INVALID_MODE:
            return "invalid trajectory mode";
        case ZMC_ERR_TRAJ_INVALID_AXIS_NUM:
            return "invalid trajectory axis count";
        case ZMC_ERR_TRAJ_CACHE_INVALID:
            return "trajectory cache is invalid";
        case ZMC_ERR_TRAJ_ALLOC_FAILED:
            return "trajectory allocation failed";
        case ZMC_ERR_TRAJ_INVALID_SPLINE_DATA:
            return "invalid spline data";
        case ZMC_ERR_TRAJ_INVALID_ARG:
            return "invalid trajectory argument";
        case ZMC_NO_ERROR:
            return "ok";
        default:
            return nullptr;
    }
}
#endif  // _ZMC_MC_COMM_RESULT_MESSAGE_HPP_