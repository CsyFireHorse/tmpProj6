/**
 * @file zmc_predef.h
 * @brief ZMC Predefined Constants and Macros
 * @author ChenShengyong
 * @date 2026-02-11
 */

#ifndef _ZMC_PREDEF_H_
#define _ZMC_PREDEF_H_

// ============================================================================
// Axis & Servo Limits
// ============================================================================

// all slaves include IO slave & servo slave
// axis num =  SERVO_NUM + AUX_AXIS_NUM

// Maximum total number of logical axes, including real and virtual axes
#ifndef PCIE_HOST_AXIS_NUM
#define PCIE_HOST_AXIS_NUM (64)
#endif

#define MC_MOTION_MAX_AXES PCIE_HOST_AXIS_NUM

// Maximum number of real servo axes per EtherCAT master
#ifndef MAX_NUM_SERVOS
#define MAX_NUM_SERVOS (32)
#endif

// Maximum number of EtherCAT slaves per master
#ifndef MAX_NUM_SLAVES
#define MAX_NUM_SLAVES (128)
#endif

// Maximum number of servo groups
#ifndef MAX_SERVO_GROUPS
#define MAX_SERVO_GROUPS (8)
#endif

// Maximum number of auxiliary or virtual axes when no real axis is configured
#ifndef MAX_AXIS_AUX
#define MAX_AXIS_AUX MC_MOTION_MAX_AXES
#endif

#ifndef SLAVE_HAS_SERVO_MAX_NUM
#define SLAVE_HAS_SERVO_MAX_NUM (4)
#endif

#ifndef MC_ENI_POD_MAX_PDO_CELLS
#define MC_ENI_POD_MAX_PDO_CELLS 256u
#endif

// Maximum logical port count for each IO kind (DI, DO, AI, and AO)
#ifndef MC_IO_MAX_COUNT
#define MC_IO_MAX_COUNT (1024)
#endif
// ============================================================================
// EtherCAT Communication Parameters
// ============================================================================

// Default EtherCAT cycle time in microseconds (1ms = 1000us)
#ifndef ECAT_DEFAULT_CYCLE_TIME_US
#define ECAT_DEFAULT_CYCLE_TIME_US (1000)
#endif

// SDO timeout in milliseconds
#ifndef ECAT_SDO_TIMEOUT_MS
#define ECAT_SDO_TIMEOUT_MS (100)
#endif

// Maximum retry attempts for SDO operations
#ifndef ECAT_SDO_MAX_RETRY
#define ECAT_SDO_MAX_RETRY (10)
#endif

// Legacy alias for MAX_NUM_SLAVES
#ifndef ECAT_MAX_SLAVES
#define ECAT_MAX_SLAVES MAX_NUM_SLAVES
#endif

// PDO mapping offset for multi-servo slaves
#ifndef PDO_MULTI_SERVO_OFFSET
#define PDO_MULTI_SERVO_OFFSET (0x800)
#endif

// ============================================================================
// Motion Control Parameters
// ============================================================================

// Default position tolerance (in encoder counts)
#ifndef DEFAULT_POS_TOLERANCE
#define DEFAULT_POS_TOLERANCE (10)
#endif

// Default velocity tolerance (in counts/second)
#ifndef DEFAULT_VEL_TOLERANCE
#define DEFAULT_VEL_TOLERANCE (100)
#endif

// Maximum following error (in encoder counts)
#ifndef MAX_FOLLOWING_ERROR
#define MAX_FOLLOWING_ERROR (10000)
#endif

// Per-axis move FB object pool depth (relative/absolute/additive/velocity)
#ifndef ZMC_FB_MOVE_POOL_SIZE
#define ZMC_FB_MOVE_POOL_SIZE (32)
#endif

// Default operation mode for initialization
#ifndef DEFAULT_OP_MODE
#define DEFAULT_OP_MODE (8)  // CSP mode
#endif

// ============================================================================
// Timing and Delays
// ============================================================================

// Delay after mode change in microseconds (100ms)
#ifndef MODE_CHANGE_DELAY_US
#define MODE_CHANGE_DELAY_US (100000)
#endif

// Delay after power on in microseconds (500ms)
#ifndef POWER_ON_DELAY_US
#define POWER_ON_DELAY_US (500000)
#endif

// Watchdog timeout in milliseconds
#ifndef WATCHDOG_TIMEOUT_MS
#define WATCHDOG_TIMEOUT_MS (1000)
#endif

// ============================================================================
// Buffer and String Sizes
// ============================================================================

// Maximum length of network interface name
#ifndef MAX_NET_NAME_LEN
#define MAX_NET_NAME_LEN (32)
#endif

// Maximum length of ENI file path
#ifndef MAX_ENI_PATH_LEN
#define MAX_ENI_PATH_LEN (256)
#endif

// Maximum complete INI content carried through the host/firmware setup path
#ifndef MC_INI_MAX_CONTENT_LENGTH
#define MC_INI_MAX_CONTENT_LENGTH (40U * 1024U)
#endif

// Maximum log message buffer size
#ifndef MAX_LOG_MSG_SIZE
#define MAX_LOG_MSG_SIZE (512)
#endif

// Maximum axis name length
#ifndef MAX_AXIS_NAME_LEN
#define MAX_AXIS_NAME_LEN (64)
#endif

// ============================================================================
// Feature Flags
// ============================================================================

// Enable debug mode (0: disabled, 1: enabled)
#ifndef ZMC_DEBUG_MODE
#define ZMC_DEBUG_MODE (0)
#endif

// Enable verbose logging (0: disabled, 1: enabled)
#ifndef ZMC_VERBOSE_LOG
#define ZMC_VERBOSE_LOG (0)
#endif

// Enable servo simulation mode (0: real hardware, 1: simulation)
#ifndef ZMC_SERVO_SIMULATION
#define ZMC_SERVO_SIMULATION (0)
#endif

// ZMC_SDO_USE_PTHREAD: use pthread_create for SDO worker (embedded targets).
// Default (undefined): std::thread in AxisFbBridgeWithThread.

// ============================================================================
// Motion App Network
// ============================================================================

// Default device name reported by motapp / HTTP device_name API
#ifndef ZMC_DEFAULT_DEVICE_NAME
#define ZMC_DEFAULT_DEVICE_NAME "ZMC600e"
#endif

// Default EtherCAT NIC / channel name (netname)
#ifndef ZMC_DEFAULT_NET_NAME
#define ZMC_DEFAULT_NET_NAME "zecm0"
#endif

// Default HTTP/RPC server port for motapp (avoid common 8080 conflicts)
#ifndef ZMC_MOTAPP_HTTP_DEFAULT_PORT
#define ZMC_MOTAPP_HTTP_DEFAULT_PORT 8768
#endif

#ifndef ZMC_STR_HELPER
#define ZMC_STR_HELPER(x) #x
#define ZMC_STR(x) ZMC_STR_HELPER(x)
#endif

#ifndef ZMC_MOTAPP_HTTP_DEFAULT_PORT_STR
#define ZMC_MOTAPP_HTTP_DEFAULT_PORT_STR ZMC_STR(ZMC_MOTAPP_HTTP_DEFAULT_PORT)
#endif

// #define ZMC_SDO_USE_PTHREAD

#endif  // _ZMC_PREDEF_H_
