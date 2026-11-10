#ifndef ZML_ERRORCODE_DEF_H
#define ZML_ERRORCODE_DEF_H

// Public ZML error codes.
typedef enum ZML_ERROR_CODE {
    ZML_ERR_NO_ERROR = 0,  // The operation succeeded.

    // Axis and motion function block errors.
    ZML_ERR_AXIS_IS_NO_AXIS_REF = 31,  // The axis reference is invalid.
    ZML_ERR_FB_ACTIVE_AXIS_DISABLED = 33,  // The axis is disabled or in ErrorStop.
    ZML_ERR_AXIS_NOT_READY_FOR_MOTION = 34,  // The axis is not ready for motion.
    ZML_ERR_SCM_MODE_SWITCH_TIMEOUT = 73,  // The controller mode switch timed out.
    ZML_ERR_DI_GENERAL_COMMUNICATION_ERROR = 101,  // The fieldbus communication failed.
    ZML_ERR_AXIS_IS_NOT_OPERATIONAL = 103,  // The axis is not operational.
    ZML_ERR_P_WHEN_DRIVE_ERROR = 111,  // The drive reported an error.
    ZML_ERR_P_TIMEOUT = 112,  // The power operation timed out.
    ZML_ERR_DI_POSITIONLAGERROR = 116,  // The position following error exceeded its limit.
    ZML_ERR_DI_HOMING_ERROR = 117,  // The drive reported a homing error.
    ZML_ERR_R_NO_ERROR_TO_RESET = 120,  // The axis has no error to reset.
    // An active communication error cannot be reset.
    ZML_ERR_R_CANNOT_RESET_COMMUNICATION_ERROR = 124,
    ZML_ERR_R_TIMEOUT = 125,  // The axis reset timed out.
    ZML_ERR_WP_PARAM_INVALID = 140,  // The parameter is invalid or not writable.
    ZML_ERR_WP_VALUE_INVALID = 144,  // The parameter value is invalid.
    ZML_ERR_WP_SDO_ABORTED = 151,  // The CoE request failed.
    ZML_ERR_WP_SDO_TIMEOUT = 152,  // The axis CoE request timed out.
    ZML_ERR_WP_SDO_QUEUE_FULL = 153,  // The axis CoE request queue is full.
    // The axis was not in the required standstill state.
    ZML_ERR_H_AXIS_WASNT_STANDSTILL = 170,
    ZML_ERR_H_TIMEOUT = 172,  // Homing timed out.
    ZML_ERR_H_AXIS_IN_ERRORSTOP = 174,  // The axis is in ErrorStop.
    ZML_ERR_H_SET_OP_MODE_HOMING_FAILED = 175,  // The drive did not switch to homing mode.
    ZML_ERR_H_AXIS_IN_MOVING = 178,  // The axis is moving or already homing.
    // A motion velocity, acceleration, deceleration, or jerk is invalid.
    ZML_ERR_INVALID_VELACCJERK_VALUES = 501,
    // The requested motion buffer mode is not supported.
    ZML_ERR_BUFFER_MODE_NOT_SUPPORTED = 1955,
    ZML_ERR_UNKNOWN_SW_ERROR = 16386,  // An unspecified axis software error occurred.
    // The axis state prevents function block activation.
    ZML_ERR_FB_ACTIVATE_AXIS_ERRORSTOP = 16388,
    ZML_ERR_POWEROFF_WHEN_MOVING = 16389,  // The axis was powered off during motion.
    ZML_ERR_ERRORSTOP_WHEN_MOVING = 16390,  // The axis entered ErrorStop during motion.
    ZML_ERR_FB_ACTIVATE_AXIS_STOPPING = 16393,  // The axis is stopping.
    ZML_ERR_VEL_BUILD_FAILED = 16422,  // The velocity trajectory could not be built.
    ZML_ERR_AXIS_NOT_IN_GEAR = 16430,  // The axis is not in gear.
    ZML_ERR_AXIS_NOT_IN_SYNC_MOTION = 16432,  // The axis is not in synchronized motion.
    ZML_ERR_SETPOSITION_WHILE_MOVING = 16441,  // The axis is moving while setting its position.
    ZML_ERR_AXISGRP_ERRORSTOP = 17006,  // The axis group entered ErrorStop.

    // Command submission, status, and script errors.
    // The axis state is not available in the cache.
    ZML_ERR_MOTCMD_AXIS_STATE_INVALID_AXIS = 17100,
    // The extended axis state is not available in the cache.
    ZML_ERR_MOTCMD_AXIS_STATE2_INVALID_AXIS = 17101,
    ZML_ERR_MOTCMD_CMD_RESULTS_EMPTY = 17102,  // The command result cache is empty.
    ZML_ERR_MOTCMD_WAIT_INVALID_ARG = 17103,  // A wait argument is invalid.
    ZML_ERR_MOTCMD_PACKET_APPEND_FAILED = 17106,  // The command could not be added to a packet.
    ZML_ERR_MOTCMD_QUEUE_PUSH_FAILED = 17107,  // The command queue could not accept the packet.
    ZML_ERR_MOTCMD_INVALID_ARG = 17108,  // A command argument is invalid.
    ZML_ERR_MOTCMD_SCRIPT_EMPTY = 17109,  // The command text is empty.
    ZML_ERR_MOTCMD_SCRIPT_LINE_EMPTY = 17110,  // A command line contains no command.
    ZML_ERR_MOTCMD_SCRIPT_MISSING_ARG = 17111,  // A command argument is missing.
    ZML_ERR_MOTCMD_SCRIPT_PARSE_FAILED = 17112,  // The command text could not be parsed.
    ZML_ERR_MOTCMD_UNKNOWN_COMMAND = 17113,  // The command is not recognized.
    // The axis information is not available in the cache.
    ZML_ERR_MOTCMD_AXIS_INFO_UNAVAILABLE = 17115,
    ZML_ERR_MOTCMD_WAIT_TIMEOUT = 17116,  // The axis wait timed out.
    ZML_ERR_MOTCMD_WAIT_AXIS_ERROR = 17117,  // The axis entered ErrorStop while waiting.
    ZML_ERR_MOTCMD_FLOW_PAUSED = 17118,  // Command flow is paused pending acknowledgement.

    // Configuration and process image errors.
    ZML_ERR_ENI_FILE_OPEN_FAILED = 17301,  // The ENI file could not be opened or read.
    ZML_ERR_ENI_JSON_NOT_FOUND = 17302,  // The ENI file contains no embedded JSON.
    ZML_ERR_ENI_JSON_INVALID = 17303,  // The embedded ENI JSON is invalid.
    ZML_ERR_ENI_SLAVE_INVALID = 17304,  // The ENI contains no valid slave list.
    ZML_ERR_ENI_AXIS_OVERFLOW = 17306,  // The ENI axis count exceeds the supported capacity.
    ZML_ERR_ENI_FILE_TOO_LARGE = 17308,  // The ENI file exceeds the parser size limit.
    ZML_ERR_INI_FILE_OPEN = 17351,  // The INI file could not be opened or read.
    ZML_ERR_INI_PARSE = 17352,  // The INI file could not be parsed.
    ZML_ERR_INI_MISSING_SECTION = 17354,  // A required INI section is missing.
    ZML_ERR_INI_MISSING_KEY = 17355,  // A required INI key is missing.
    ZML_ERR_INI_KEY = 17356,  // An INI key is invalid.
    ZML_ERR_INI_VALUE_FORMAT = 17357,  // An INI value has an invalid format.
    ZML_ERR_INI_VALUE_RANGE = 17358,  // An INI value is out of range.
    ZML_ERR_INI_DUPLICATE = 17359,  // An INI configuration entry is duplicated.
    ZML_ERR_INI_AXIS_BINDING_CONFIG = 17360,  // The axis binding configuration is invalid.
    ZML_ERR_INI_IO_BINDING_CONFIG = 17361,  // The IO binding configuration is invalid.
    ZML_ERR_IO_OBJECT_NOT_FOUND = 17721,  // The requested IO object or alias was not found.
    ZML_ERR_IO_ALIAS_DUPLICATE = 17722,  // IO aliases overlap or are duplicated.
    ZML_ERR_IO_ALIAS_CAPACITY = 17723,  // The IO alias capacity was exceeded.
    ZML_ERR_IO_PROCESS_IMAGE_UNBOUND = 17724,  // The process image is not bound.
    ZML_ERR_IO_DATA_SIZE_UNSUPPORTED = 17725,  // The IO data size is not supported.
    ZML_ERR_IO_PROCESS_IMAGE_OVER_RANGE = 17726,  // The IO access exceeds the process image.
    ZML_ERR_IO_MASK_INVALID = 17727,  // The IO mask is invalid for the selected object.
    ZML_ERR_SDO_QUEUE_FULL = 17742,  // The SDO request queue is full.
    // The SDO worker thread could not be started.
    ZML_ERR_BRIDGE_SDO_THREAD_START_FAILED = 17749,

    // RPC client and remote file errors.
    ZML_ERR_RPC_CLIENT_CONNECT_FAILED = 18200,  // The RPC client could not connect.
    ZML_ERR_RPC_CLIENT_NOT_OPEN = 18201,  // The RPC client handle is not open.
    ZML_ERR_RPC_CLIENT_DISCONNECTED = 18202,  // The RPC client is disconnected.
    ZML_ERR_RPC_CLIENT_CONNECT_TIMEOUT = 18203,  // The RPC connection or client wait timed out.
    ZML_ERR_RPC_CLIENT_CALL_FAILED = 18204,  // The RPC call failed.
    ZML_ERR_RPC_CLIENT_INVALID_ARG = 18222,  // A client argument is invalid.
    ZML_ERR_RPC_FILE_INVALID_NAME = 18240,  // The remote file name is invalid.
    // The remote file was not found or is not a regular file.
    ZML_ERR_RPC_FILE_NOT_FOUND = 18241,
    ZML_ERR_RPC_FILE_ALREADY_EXISTS = 18242,  // The destination file already exists.
    ZML_ERR_RPC_FILE_TOO_LARGE = 18243,  // The file exceeds the transfer size limit.
    ZML_ERR_RPC_FILE_IO_FAILED = 18245,  // A local or remote file operation failed.

    // Service session and synchronous request errors.
    ZML_ERR_HOST_INVALID_HANDLE = 18401,  // The service session or device handle is invalid.
    ZML_ERR_HOST_RECV_TIMEOUT = 18403,  // The service timed out waiting for a result.
    ZML_ERR_HOST_INVALID_ARG = 18405,  // A service argument is invalid.
    ZML_ERR_HOST_NOT_OPEN = 18407,  // The service or session is not open.
    ZML_ERR_HOST_MALLOC_FAILED = 18408,  // The service could not allocate memory.
    ZML_ERR_HOST_INTERNAL = 18414,  // The service reported an internal error.
    // A service configuration value exceeds its supported range.
    ZML_ERR_HOST_OVER_RANGE = 18417,
    ZML_ERR_HOST_HANDLE_ALREADY_OPEN = 18446,  // The service device is already running.
    ZML_ERR_HOST_SESSION_CAPACITY = 18448,  // All service session slots are in use.
    // No successful master state request is recorded.
    ZML_ERR_ECAT_MASTER_REQ_STATE_UNAVAILABLE = 19017
} ZML_ERROR_CODE;

#endif // ZML_ERRORCODE_DEF_H
