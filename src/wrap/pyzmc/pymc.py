"""
PyZMC - Python Motion Control Wrapper
=====================================

This module provides Python bindings for the ZMC motion control library.
It wraps C++ motion control function blocks (FBs) for easy use in Python.

Main Features:
- Axis control and configuration
- Motion control (absolute, relative, velocity)
- Cam table operations (read, write, select, sync)
- Tappet control and status monitoring
- Power management and error handling

Usage:
    from pymc import FbAxisWrap, FbPower, FbMoveAbsolute
    
    # Create and configure axis
    axis = FbAxisWrap()
    power = FbPower()
    power.Axis = axis.ref
    
    # Enable axis and execute motion
    power.Enable = True
    power.runCycle()

Author: ZMC Development Team
Version: 1.0
License: Proprietary
"""

from ctypes import *
import os
import sys
sys.path.append(os.path.dirname(os.path.abspath(__file__)))


# 定义基础类型
ZMC_BOOL = c_bool
ZMC_ERROR = c_short
ZMC_LREAL = c_double
ZMC_DIRECTION = c_short
ZMC_BUFFER_MODE = c_ushort
ZMC_STARTMODE = c_ushort
ZMC_START_MODE = ZMC_STARTMODE  # Alias for compatibility
ZMC_INT = c_short

# 定义枚举值
class Direction:
    mcNegativeDirection = -1
    mcShortestWay = 0
    mcPositiveDirection = 1
    mcCurrentDirection = 2
    mcFastestWay = 3
    mcFiniteDirection = 4

class BufferMode:
    mcAborting = 0
    mcBuffered = 1
    mcBlendingLow = 2
    mcBlendingPrevious = 3
    mcBlendingNext = 4
    mcBlendingHigh = 5

# 定义 PdoEnableFlag 结构体
class PdoEnableFlag(Structure):
    _fields_ = [
        # ("enable_value", c_uint32),
        ("enable_status_word", c_uint8, 1),
        ("enable_actual_postion", c_uint8, 1),
        ("enable_actual_velocity", c_uint8, 1),
        ("enable_actual_torque", c_uint8, 1),
        ("enable_digital_inputs", c_uint8, 1),
        ("enable_following_error", c_uint8, 1),
        ("enable_modes_of_operation_display", c_uint8, 1),
        ("enable_touch_probe_1_falling_edge", c_uint8, 1),
        ("enable_touch_probe_1_rising_edge", c_uint8, 1),
        ("enable_touch_probe_2_falling_edge", c_uint8, 1),
        ("enable_touch_probe_2_rising_edge", c_uint8, 1),
        ("enable_touch_probe_status", c_uint8, 1),
        ("enable_control_word", c_uint8, 1),
        ("enable_set_position", c_uint8, 1),
        ("enable_set_velocity", c_uint8, 1),
        ("enable_set_torque", c_uint8, 1),
        ("enable_add_torque_value", c_uint8, 1),
        ("enable_add_velocity_value", c_uint8, 1),
        ("enable_digital_output", c_uint8, 1),
        ("enable_modes_of_operation", c_uint8, 1),
        ("enable_touch_probe_function", c_uint8, 1),
        # ("enable_arr", c_uint8 * 4)
    ]

# 定义 AxisDynamicLimits 结构体
class AxisDynamicLimits(Structure):
    _fields_ = [
        ("velocity", c_double),
        ("acceleration", c_double),
        ("deceleration", c_double),
        ("jerk", c_double)
    ]

# 定义 AxisSoftwareErrorReaction 结构体
class AxisSoftwareErrorReaction(Structure):
    _fields_ = [
        ("deceleration", c_double),
        ("max_distance", c_double)
    ]

# 定义 AxisLinearParam 结构体
class AxisLinearParam(Structure):
    _fields_ = [
        ("increments_units_in_application_num", c_uint32),
        ("increments_units_in_application_den", c_uint32)
    ]

# 定义 UnSwHwLimitFlag 联合体
class UnSwHwLimitFlag(Union):
    _fields_ = [
        ("value", c_uint8),
        ("bSWLimitDisable", c_uint8, 1),
        ("bHWLimitEnable", c_uint8, 1),
        ("bDynamicLimitDisable", c_uint8, 1),
        ("bDiscontinuityCheckEnable", c_uint8, 1),
        ("bPosDirectionDisable", c_uint8, 1),
        ("bNegDirectionDisable", c_uint8, 1)
    ]

# 定义 DebugParam 结构体
class DebugParam(Structure):
    _fields_ = [
        ("enable_servoAda_param", c_bool),
        ("eAxisMtype", c_uint8),
        ("limit_flag", UnSwHwLimitFlag),
        ("mode", c_uint8),
        ("level", c_uint8),
        ("axis_dtype", c_uint8),
        ("arg_value_flag", c_uint8),
        ("batch_size", c_uint16),
        ("modulo", c_uint32),
        ("dbg_input_delay", c_uint16),
        ("dbg_output_delay", c_uint16),
        ("exit_when_count", c_uint32),
        ("servo_fak_statw_sm", c_uint8),
        ("servo_fak_pos", c_int32),
        ("servo_fak_rcfilter_scale", c_double),
        ("arg_value", c_int32 * 4),
        ("arg_value_f", c_double * 4)
    ]

# 定义 AxisCiaConfig 结构体
class AxisCiaConfig(Structure):
    _fields_ = [
        ("is_axis_virtual", c_bool),
        ("axis_index", c_uint32),
        ("slave_node_id", c_uint32),
        ("id", c_uint32),
        ("cycle_time", c_uint32),
        ("axis_type", c_uint8),
        ("modulo_value", c_double),
        ("software_limits_is_activate", c_bool),
        ("software_limits_negative", c_double),
        ("software_limits_positive", c_double),
        ("dynamic_limits", AxisDynamicLimits),
        ("software_error_reaction", AxisSoftwareErrorReaction),
        ("velocity_ramp_option", c_uint8),
        ("position_lag_option", c_uint8),
        ("position_lag_supervision_is_changeable", c_bool),
        ("position_lag_supervision_value", c_double),
        ("motor_type", c_uint8),
        ("is_invert_direction", c_bool),
        ("linear", AxisLinearParam),
        ("rotary_gear_application_num", c_uint32),
        ("rotary_gear_application_den", c_uint32),
        ("rotary_increments_motor_turns_num", c_uint32),
        ("rotary_increments_motor_turns_den", c_uint32),
        ("rotary_motor_gear_num", c_uint32),
        ("rotary_motor_gear_den", c_uint32),
        ("task_name", c_char_p),
        ("is_automatic_mapping", c_bool),
        ("channels_stat_len", c_uint32),
        ("channels_ctrl_len", c_uint32),
        ("channels_stat_name", c_char_p),
        ("channels_ctrl_name", c_char_p),
        ("mapping_id", c_uint32),
        ("bit_width", c_uint8),
        ("pdo_flag", PdoEnableFlag),
        ("debug", DebugParam)
    ]

# Define CamXyva structure for CAM table operations
class CamXyva(Structure):
    _fields_ = [
        ("dX", c_double),  # master position
        ("dY", c_double),  # slave position  
        ("dV", c_double),  # first derivative dY/dX; slave velocity
        ("dA", c_double)   # slave acceleration
    ]

# Define CamAffineCoefficient structure for affine transformations
class CamAffineCoefficient(Structure):
    _fields_ = [
        ("masterScale", c_double),      # master scaling factor
        ("masterIntercept", c_double),  # master intercept
        ("slaveScale", c_double),       # slave scaling factor
        ("slaveIntercept", c_double)    # slave intercept
    ]

# Define CamAffineFullParam structure for full affine parameters
class CamAffineFullParam(Structure):
    _fields_ = [
        ("MasterScaling", c_double),    # master scaling
        ("SlaveScaling", c_double),     # slave scaling
        ("MasterOffset", c_double),     # master offset
        ("SlaveOffset", c_double),      # slave offset
        ("masterAbsolute", c_bool),     # true: absolute position, false: relative position
        ("slaveAbsolute", c_bool),      # slave absolute flag
        ("masterEngagePos", c_double),  # position of master axis when cam table starts to cam in
        ("slaveEngagePos", c_double)    # position of slave axis when cam table starts to cam in
    ]

# Define PosVelAcc structure for position, velocity, acceleration
class PosVelAcc(Structure):
    _fields_ = [
        ("pos", c_double),  # position
        ("vel", c_double),  # velocity
        ("acc", c_double)   # acceleration
    ]

# class AxisConfig(Structure):
#     _fields_ = [
#         # 根据实际需求添加字段
#         ("max_velocity", c_double),
#         ("acceleration", c_double),
#         ("deceleration", c_double),
#         ("jerk", c_double),
#         # 添加更多配置参数...
#     ]

class McAxis(Structure):
    _fields_ = [
        ("storage", c_char * 568),  # 固定大小的存储空间
    ]

# AXIS_REF = c_void_p
AXIS_REF = POINTER(McAxis)
AxisCiaConfig_REF = POINTER(AxisCiaConfig)
# MC_CAM_REF is a pointer to MC_CAM_TABLE (defined as void* for simplicity)
MC_CAM_REF = c_void_p

# Axis FB bridge public packet and SDO adapter types.
AXFB_BRIDGE_PACKET_PAYLOAD_SIZE = 508
axfb_bridge_handle_t = c_void_p


class AxFbBridgePacket(Structure):
    _fields_ = [
        ("seq", c_uint16),
        ("payload_len", c_uint16),
        ("payload", c_uint8 * AXFB_BRIDGE_PACKET_PAYLOAD_SIZE)
    ]


class SdoCKey(Structure):
    _fields_ = [
        ("slave_id", c_uint16),
        ("index", c_uint16),
        ("subindex", c_uint8)
    ]


SdoCUploadFn = CFUNCTYPE(c_int32, c_void_p, POINTER(SdoCKey), POINTER(c_uint32), c_void_p, c_uint32)
SdoCDownloadFn = CFUNCTYPE(c_int32, c_void_p, POINTER(SdoCKey), c_uint32, c_void_p, c_uint32)
EcatCGetSlaveStateFn = CFUNCTYPE(c_int32, c_void_p, c_uint16, POINTER(c_uint8))


class SdoCAdapter(Structure):
    _fields_ = [
        ("ctx", c_void_p),
        ("upload", SdoCUploadFn),
        ("download", SdoCDownloadFn),
        ("get_slave_state", EcatCGetSlaveStateFn)
    ]

# SMC_TappetData_REF will be defined after SMC_TappetData class
# 定义 McPower 结构体
class McPower(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        # 输入部分
        ("Enable", ZMC_BOOL),
        ("bRegulatorOn", ZMC_BOOL),
        ("bDriveStart", ZMC_BOOL),
        # 输出部分
        ("Status", ZMC_BOOL),
        ("bRegulatorRealState", ZMC_BOOL),
        ("bDriveStartRealState", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McMoveAbsolute 结构体
class McMoveAbsolute(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        # 输入部分
        ("Execute", ZMC_BOOL),
        ("Position", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("Direction", ZMC_DIRECTION),
        ("BufferMode", ZMC_BUFFER_MODE),
        # 输出部分
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Active", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McMoveRelative 结构体
class McMoveRelative(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        # 输入部分
        ("Execute", ZMC_BOOL),
        ("Distance", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("BufferMode", ZMC_BUFFER_MODE),
        # 输出部分
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Active", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McMoveAdditive 结构体
class McMoveAdditive(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        # 输入部分
        ("Execute", ZMC_BOOL),
        ("Distance", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        # 输出部分
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McStop 结构体
class McStop(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        # 输入部分
        ("Execute", ZMC_BOOL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        # 输出部分
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McHalt 结构体
class McHalt(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        # 输入部分
        ("Execute", ZMC_BOOL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        # 输出部分
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McMoveVelocity 结构体
class McMoveVelocity(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        # 输入部分
        ("Execute", ZMC_BOOL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("Direction", ZMC_DIRECTION),
        ("BufferMode", ZMC_BUFFER_MODE),
        # 输出部分
        ("InVelocity", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Active", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# ========== CamTable/Read/Write/CamIn 功能块结构体 ==========
class MC_CAM_ID(Structure):
    _fields_ = [
        ("pCT", POINTER(c_ubyte)),   # ZMC_BYTE* pCT
        ("Periodic", ZMC_BOOL),
        ("MasterAbsolute", ZMC_BOOL),
        ("SlaveAbsolute", ZMC_BOOL),
        ("StartMaster", ZMC_LREAL),
        ("EndMaster", ZMC_LREAL),
        ("StartSlave", ZMC_LREAL),
        ("EndSlave", ZMC_LREAL),
        ("byCompatibilityMode", c_ubyte)
    ]

class McCamTableSelect(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Master", AXIS_REF),
        ("Slave", AXIS_REF),
        ("CamTable", MC_CAM_REF),
        ("Execute", ZMC_BOOL),
        ("Periodic", ZMC_BOOL),
        ("MasterAbsolute", ZMC_BOOL),
        ("SlaveAbsolute", ZMC_BOOL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("CamTableID", MC_CAM_ID)
    ]

class McReadCAM(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("bExecute", ZMC_BOOL),
        ("sFileName", c_char * 81),
        ("bDone", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("CAM", MC_CAM_REF)
    ]

class FbWriteCAM(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("bExecute", ZMC_BOOL),
        ("sFileName", c_char * 81),
        ("CAM", c_void_p),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]
class SMC_TappetData(Structure):
    _fields_ = [
        ("pTaps", c_void_p * 3),   # SMC_CAMTappet* pTaps[3]
        ("dwCycleTime", c_uint32),
        ("byChannels", c_ubyte),
        ("bRestart", ZMC_BOOL)
    ]

# Define SMC_TappetData_REF after SMC_TappetData class
SMC_TappetData_REF = POINTER(SMC_TappetData)
    
class McCamIn(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Master", AXIS_REF),
        ("Slave", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("MasterOffset", ZMC_LREAL),
        ("SlaveOffset", ZMC_LREAL),
        ("MasterScaling", ZMC_LREAL),
        ("SlaveScaling", ZMC_LREAL),
        ("StartMode", ZMC_START_MODE),
        ("CamTableID", MC_CAM_ID),
        ("VelocityDiff", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("TappetHysteresis", ZMC_LREAL),
        ("InSync", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("EndOfProfile", ZMC_BOOL),
        ("Tappets", SMC_TappetData)
    ]

# ========== 新增功能块结构体定义 ==========

# McReset - 轴复位功能块
class McReset(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# McSetPosition - 设置位置功能块
class McSetPosition(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("Position", ZMC_LREAL),
        ("Mode", ZMC_BOOL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# McReadStatus - 读取轴状态功能块
class McReadStatus(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Disabled", ZMC_BOOL),
        ("ErrorStop", ZMC_BOOL),
        ("Stopping", ZMC_BOOL),
        ("Standstill", ZMC_BOOL),
        ("DiscreteMotion", ZMC_BOOL),
        ("ContinuousMotion", ZMC_BOOL),
        ("SynchronizedMotion", ZMC_BOOL),
        ("Homing", ZMC_BOOL),
        ("ConstantVelocity", ZMC_BOOL),
        ("Accelerating", ZMC_BOOL),
        ("Decelerating", ZMC_BOOL),
        ("FBErrorOccurred", ZMC_BOOL)
    ]

# McReadActualPosition - 读取实际位置功能块
class McReadActualPosition(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Position", ZMC_LREAL)
    ]

# McReadActualVelocity - 读取实际速度功能块
class McReadActualVelocity(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Velocity", ZMC_LREAL)
    ]

# McReadAxisError - 读取轴错误功能块
class McReadAxisError(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("AxisError", ZMC_BOOL),
        ("AxisErrorID", c_uint32),
        ("SWEndSwitchActive", ZMC_BOOL)
    ]

# McReadParameter - 读取参数功能块
class McReadParameter(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("ParameterNumber", c_int32),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Value", ZMC_LREAL)
    ]

# McWriteParameter - 写入参数功能块
class McWriteParameter(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("ParameterNumber", c_int32),
        ("Value", ZMC_LREAL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# McHome - 简单回零功能块
class McHome(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("Position", ZMC_LREAL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# McHoming - 高级回零功能块
class McHoming(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("fPosition", ZMC_LREAL),
        ("fHomePosition", ZMC_LREAL),
        ("fVelocitySlow", ZMC_LREAL),
        ("fVelocityFast", ZMC_LREAL),
        ("fAcceleration", ZMC_LREAL),
        ("fDeceleration", ZMC_LREAL),
        ("fJerk", ZMC_LREAL),
        ("nDirection", ZMC_DIRECTION),
        ("bReferenceSwitch", ZMC_BOOL),
        ("fSignalDelay", ZMC_LREAL),
        ("nHomingMode", c_uint8),
        ("bReturnToZero", ZMC_BOOL),
        ("bIndexOccured", ZMC_BOOL),
        ("fIndexPosition", ZMC_LREAL),
        ("bIgnoreHWLimit", ZMC_BOOL),
        ("bDone", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bCommandAborted", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("nErrorID", ZMC_ERROR),
        ("bStartLatchingIndex", ZMC_BOOL)
    ]

# McJog - 点动功能块
class McJog(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("JogForward", ZMC_BOOL),
        ("JogBackward", ZMC_BOOL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# McGearIn - 齿轮同步进入功能块
class McGearIn(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Master", AXIS_REF),
        ("Slave", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("RatioNumerator", c_int32),
        ("RatioDenominator", c_uint32),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("BufferMode", ZMC_BUFFER_MODE),
        ("InGear", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Active", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# McGearOut - 齿轮同步退出功能块
class McGearOut(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Slave", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# McPhasing - 相位同步功能块
class McPhasing(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Master", AXIS_REF),
        ("Slave", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("PhaseShift", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# McSetTorque - 设置转矩功能块
class McSetTorque(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bEnable", ZMC_BOOL),
        ("fTorque", ZMC_LREAL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("nErrorID", ZMC_ERROR)
    ]

# McFollowPosition - 位置跟随功能块
class McFollowPosition(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("fSetPosition", ZMC_LREAL),
        ("bBusy", ZMC_BOOL),
        ("bCommandAborted", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("iErrorID", ZMC_ERROR)
    ]

# McFollowVelocity - 速度跟随功能块
class McFollowVelocity(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("fSetVelocity", ZMC_LREAL),
        ("bBusy", ZMC_BOOL),
        ("bCommandAborted", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("iErrorID", ZMC_ERROR)
    ]

# McTrackAxis - 轴跟踪功能块
class McTrackAxis(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Master", AXIS_REF),
        ("Slave", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("bBusy", ZMC_BOOL),
        ("bCommandAborted", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("iErrorID", ZMC_ERROR),
        ("bInSync", ZMC_BOOL)
    ]

# McReadSetValues - 读取设定值功能块
class McReadSetValues(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Position", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL)
    ]

# McReadActualValues - 读取实际值功能块
class McReadActualValues(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Position", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL)
    ]

# McInPosition - 位置到达检测功能块
class McInPosition(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bEnable", ZMC_BOOL),
        ("fPosWindow", ZMC_LREAL),
        ("fPosTime", ZMC_LREAL),
        ("fTimeOut", ZMC_LREAL),
        ("bInPosition", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bTimeOut", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("iErrorID", ZMC_ERROR)
    ]

# McCheckLimits - 限位检查功能块
class McCheckLimits(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bEnable", ZMC_BOOL),
        ("bCheckVel", ZMC_BOOL),
        ("bCheckAccDec", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("iErrorID", ZMC_ERROR),
        ("bLimitsExceeded", ZMC_BOOL)
    ]

# 定义 McReadActualTorque 结构体
class McReadActualTorque(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Torque", ZMC_LREAL)
    ]

# 定义 McReadCommandPosition 结构体
class McReadCommandPosition(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Position", ZMC_LREAL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McReadBoolParameter 结构体
class McReadBoolParameter(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("ParameterNumber", c_int),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Value", ZMC_BOOL)
    ]

# 定义 McWriteBoolParameter 结构体
class McWriteBoolParameter(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("ParameterNumber", c_int),
        ("Value", ZMC_BOOL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McMoveContinuousAbsolute 结构体
class McMoveContinuousAbsolute(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("Position", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("EndVelocity", ZMC_LREAL),
        ("EndVelocityDirection", ZMC_DIRECTION),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("Direction", ZMC_DIRECTION),
        ("AdaptEndVelToAvoidOvershoot", ZMC_BOOL),
        ("InEndVelocity", ZMC_BOOL),
        ("PositionReached", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McMoveContinuousRelative 结构体
class McMoveContinuousRelative(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("Distance", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("EndVelocity", ZMC_LREAL),
        ("EndVelocityDirection", ZMC_DIRECTION),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("AdaptEndVelToAvoidOvershoot", ZMC_BOOL),
        ("InEndVelocity", ZMC_BOOL),
        ("DistanceTravelled", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McMoveSuperImposed 结构体
class McMoveSuperImposed(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("Abort", ZMC_BOOL),
        ("Distance", ZMC_LREAL),
        ("VelocityDiff", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("DistanceTravelled", ZMC_LREAL),
        ("SuperImposedVelocity", ZMC_LREAL),
        ("SuperImposedAcceleration", ZMC_LREAL)
    ]

# 定义 McPositionProfile 结构体
class McPositionProfile(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("TimePosition", c_void_p),
        ("Execute", ZMC_BOOL),
        ("ArraySize", ZMC_INT),
        ("PositionScale", ZMC_LREAL),
        ("Offset", ZMC_LREAL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McAccelerationProfile 结构体
class McAccelerationProfile(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("TimeAcceleration", c_void_p),
        ("Execute", ZMC_BOOL),
        ("ArraySize", ZMC_INT),
        ("AccelerationScale", ZMC_LREAL),
        ("Offset", ZMC_LREAL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McVelocityProfile 结构体
class McVelocityProfile(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("TimeVelocity", c_void_p),
        ("Execute", ZMC_BOOL),
        ("ArraySize", ZMC_INT),
        ("VelocityScale", ZMC_LREAL),
        ("Offset", ZMC_LREAL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McGearInPos 结构体
class McGearInPos(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Slave", AXIS_REF),
        ("Slave", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("RatioNumerator", c_int),
        ("RatioDenominator", c_int),
        ("MasterSyncPosition", ZMC_LREAL),
        ("SlaveSyncPosition", ZMC_LREAL),
        ("MasterStartDistance", ZMC_LREAL),
        ("BufferMode", ZMC_BUFFER_MODE),
        ("AvoidReversal", ZMC_BOOL),
        ("StartSync", ZMC_BOOL),
        ("InSync", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Active", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McFollowPositionVelocity 结构体
class McFollowPositionVelocity(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("fSetPosition", ZMC_LREAL),
        ("fSetVelocity", ZMC_LREAL),
        ("bBusy", ZMC_BOOL),
        ("bCommandAborted", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("iErrorID", ZMC_ERROR)
    ]

# 定义 McFollowSetValues 结构体
class McFollowSetValues(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("bAbort", ZMC_BOOL),
        ("dwValueMask", c_uint32),
        ("fSetPosition", ZMC_LREAL),
        ("fSetVelocity", ZMC_LREAL),
        ("fSetAcceleration", ZMC_LREAL),
        ("fSetJerk", ZMC_LREAL),
        ("fSetTorque", ZMC_LREAL),
        ("fSetCurrent", ZMC_LREAL),
        ("bBusy", ZMC_BOOL),
        ("bCommandAborted", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("iErrorID", ZMC_ERROR)
    ]

# 定义 McTrackSetValues 结构体
class McTrackSetValues(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("fSetPosition", ZMC_LREAL),
        ("fSetVelocity", ZMC_LREAL),
        ("fSetAcceleration", ZMC_LREAL),
        ("fSetJerk", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("SignalModuloPeriod", ZMC_LREAL),
        ("bBusy", ZMC_BOOL),
        ("bCommandAborted", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("iErrorID", ZMC_ERROR),
        ("bInSync", ZMC_BOOL)
    ]

# 定义 McCamOut 结构体
class McCamOut(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Slave", AXIS_REF),
        ("Execute", ZMC_BOOL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McCamRegister 结构体
class McCamRegister(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Slave", AXIS_REF),
        ("CamTable", c_void_p),
        ("bTappet", c_void_p),
        ("Enable", ZMC_BOOL),
        ("MasterOffset", ZMC_LREAL),
        ("MasterScaling", ZMC_LREAL),
        ("TappetHysteresis", ZMC_LREAL),
        ("DeadTimeCompensation", ZMC_LREAL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("EndOfProfile", ZMC_BOOL)
    ]

class McCamCalc(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("CamTable", c_void_p),
        # input
        ("Enable", ZMC_BOOL),
        ("MasterPosition", ZMC_LREAL),
        ("MasterVelocity", ZMC_LREAL),
        ("MasterAcceleration", ZMC_LREAL),
        ("MasterOffset", ZMC_LREAL),
        ("SlaveOffset", ZMC_LREAL),
        ("MasterScaling", ZMC_LREAL),
        ("SlaveScaling", ZMC_LREAL),
        ("MasterEngagePos", ZMC_LREAL),
        ("SlaveEngagePos", ZMC_LREAL),
        ("MasterAbsolute", ZMC_BOOL),
        ("SlaveAbsolute", ZMC_BOOL),
        # output
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("SlavePosition", ZMC_LREAL),
        ("SlaveVelocity", ZMC_LREAL),
        ("SlaveAcceleration", ZMC_LREAL)
    ]

# 定义 McCAMBounds 结构体
class McCAMBounds(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("CAM", c_void_p),
        ("bExecute", ZMC_BOOL),
        ("dMasterVelMax", ZMC_LREAL),
        ("dMasterAccMax", ZMC_LREAL),
        ("dMasterScaling", ZMC_LREAL),
        ("dSlaveScaling", ZMC_LREAL),
        ("bDone", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("nErrorID", ZMC_ERROR),
        ("dMaxPos", ZMC_LREAL),
        ("dMinPos", ZMC_LREAL),
        ("dMaxVel", ZMC_LREAL),
        ("dMinVel", ZMC_LREAL),
        ("dMaxAccDec", ZMC_LREAL),
        ("dMinAccDec", ZMC_LREAL)
    ]

# 定义 McCAMBounds_Pos 结构体
class McCAMBounds_Pos(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("CAM", c_void_p),
        ("bExecute", ZMC_BOOL),
        ("dMasterVelMax", ZMC_LREAL),
        ("dMasterAccMax", ZMC_LREAL),
        ("dMasterScaling", ZMC_LREAL),
        ("dSlaveScaling", ZMC_LREAL),
        ("bDone", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("nErrorID", ZMC_ERROR),
        ("dMaxPos", ZMC_LREAL),
        ("dMinPos", ZMC_LREAL)
    ]

# 定义 McGetCamSlaveSetPosition 结构体
class McGetCamSlaveSetPosition(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Master", AXIS_REF),
        ("Slave", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("MasterOffset", ZMC_LREAL),
        ("SlaveOffset", ZMC_LREAL),
        ("MasterScaling", ZMC_LREAL),
        ("SlaveScaling", ZMC_LREAL),
        ("CamTableID", MC_CAM_ID),
        ("fStartPosition", ZMC_LREAL),
        ("fStartVelocity", ZMC_LREAL),
        ("fStartAcceleration", ZMC_LREAL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McAbortTrigger 结构体
class McAbortTrigger(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("TriggerInput", c_void_p),
        ("Execute", ZMC_BOOL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McTouchProbe 结构体
class McTouchProbe(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("TriggerInput", c_void_p),
        ("Execute", ZMC_BOOL),
        ("WindowOnly", ZMC_BOOL),
        ("FirstPosition", ZMC_LREAL),
        ("LastPosition", ZMC_LREAL),
        ("Done", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("RecordedPosition", ZMC_LREAL),
        ("CommandAborted", ZMC_BOOL)
    ]

# 定义 McDigitalCamSwitch 结构体
class McDigitalCamSwitch(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Switches", c_void_p),
        ("Outputs", c_void_p),
        ("TrackOptions", c_void_p),
        ("Enable", ZMC_BOOL),
        ("EnableMask", c_uint32),
        ("TappetMode", c_short),
        ("InOperation", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("SwitchCorrupted", ZMC_INT)
    ]

# 定义 McReadCAM 结构体
class McReadCAM(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("bExecute", ZMC_BOOL),
        ("sFileName", c_char * 81),
        ("bDone", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("CAM", c_void_p)
    ]

# 定义 McWriteCAM 结构体
class McWriteCAM(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("CAM", c_void_p),
        ("bExecute", ZMC_BOOL),
        ("sFileName", c_char * 81),
        ("bDone", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McReadSetDriveValues 结构体
class McReadSetDriveValues(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Position", c_int),
        ("Velocity", c_int),
        ("Acceleration", c_int),
        ("Jerk", c_int)
    ]

# 定义 McReadActualDriveValues 结构体
class McReadActualDriveValues(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Position", c_int),
        ("Velocity", c_int),
        ("Acceleration", c_int),
        ("Jerk", c_int)
    ]

# 定义 McGetTrackingError 结构体
class McGetTrackingError(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("TimeStamp", c_uint32),
        ("DeadTimeCycles", c_uint8),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("ActTrackingError", ZMC_LREAL),
        ("MaxTrackingError", ZMC_LREAL),
        ("TimeAtMax", c_uint32)
    ]

# 定义 McGetTravelledDistance 结构体
class McGetTravelledDistance(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("Distance", ZMC_LREAL)
    ]

# 定义 McGetMaxSetAccDec 结构体
class McGetMaxSetAccDec(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("TimeStamp", c_uint32),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("MaxAcceleration", ZMC_LREAL),
        ("TimeAtMax", c_uint32)
    ]

# 定义 McGetMaxSetVelocity 结构体
class McGetMaxSetVelocity(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("TimeStamp", c_uint32),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("MaxVelocity", ZMC_LREAL),
        ("TimeAtMax", c_uint32)
    ]

# 定义 McGetOverride 结构体
class McGetOverride(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("Valid", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR),
        ("VelFactor", ZMC_LREAL),
        ("AccFactor", ZMC_LREAL),
        ("JerkFactor", ZMC_LREAL)
    ]

# 定义 McSetOverride 结构体
class McSetOverride(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("Enable", ZMC_BOOL),
        ("VelFactor", ZMC_LREAL),
        ("AccFactor", ZMC_LREAL),
        ("JerkFactor", ZMC_LREAL),
        ("Enabled", ZMC_BOOL),
        ("Busy", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McSetMovementType 结构体
class McSetMovementType(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("iMovementType", ZMC_INT),
        ("fPositionPeriod", ZMC_LREAL),
        ("bDone", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("eErrorID", ZMC_ERROR)
    ]

# 定义 McSetRampType 结构体
class McSetRampType(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("eRampType", c_short),
        ("bDone", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("eErrorID", ZMC_ERROR)
    ]

# 定义 McChangeDynamicLimits 结构体
class McChangeDynamicLimits(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("fMaxVelocity", ZMC_LREAL),
        ("fMaxAcceleration", ZMC_LREAL),
        ("fMaxDeceleration", ZMC_LREAL),
        ("fMaxJerk", ZMC_LREAL),
        ("fMaxTorque", ZMC_LREAL),
        ("bDone", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McChangeGearingRatio 结构体
class McChangeGearingRatio(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("dwRatioTechUnitsDenom", c_uint32),
        ("iRatioTechUnitsNum", c_int),
        ("fPositionPeriod", ZMC_LREAL),
        ("iMovementType", ZMC_INT),
        ("bDone", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("nErrorID", ZMC_ERROR)
    ]

# 定义 McSetSoftwareLimits 结构体
class McSetSoftwareLimits(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("SWL_Activated", ZMC_BOOL),
        ("SWL_Positive", ZMC_LREAL),
        ("SWL_Negative", ZMC_LREAL),
        ("SWL_Error_Decelerate", ZMC_BOOL),
        ("SWL_Error_Deceleration", ZMC_LREAL),
        ("SWL_Error_MaxDistance", ZMC_LREAL),
        ("bDone", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# 定义 McSetControllerMode 结构体
class McSetControllerMode(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("bExecute", ZMC_BOOL),
        ("nControllerMode", c_short),
        ("bDone", ZMC_BOOL),
        ("bBusy", ZMC_BOOL),
        ("bError", ZMC_BOOL),
        ("nErrorID", ZMC_ERROR)
    ]

# 定义 McInch 结构体
class McInch(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Axis", AXIS_REF),
        ("InchForward", ZMC_BOOL),
        ("InchBackward", ZMC_BOOL),
        ("Distance", ZMC_LREAL),
        ("Velocity", ZMC_LREAL),
        ("Acceleration", ZMC_LREAL),
        ("Deceleration", ZMC_LREAL),
        ("Jerk", ZMC_LREAL),
        ("Busy", ZMC_BOOL),
        ("CommandAborted", ZMC_BOOL),
        ("Error", ZMC_BOOL),
        ("ErrorID", ZMC_ERROR)
    ]

# ========== TappetData & FbGetTappet 功能块结构体 ========== 


class McGetTappetValue(Structure):
    _fields_ = [
        ("pImpl", c_void_p),
        ("Tappets",  SMC_TappetData_REF),
        ("iID", ZMC_INT),
        ("bInitValue", ZMC_BOOL),
        ("bSetInitValueAtReset", ZMC_BOOL),
        ("bTappet", ZMC_BOOL)
    ]


mcapi = os.path.join(os.path.dirname(os.path.abspath(__file__)), "mcapi.dll")

try:
    # print(os.path.exists(mcapi))
    zmc_dll = cdll.LoadLibrary(mcapi)  # 或指定完整路径如 r"C:\path\to\zmc.dll"
    # zmc_dll = windll.LoadLibrary(mcapi)
    # print("DLL loaded successfully")
except Exception as e:
    print(f"Failed to load DLL: {e}")
    zmc_dll = None


if zmc_dll:    
    # helper functions for double arrays (mc_traj_api)
    mc_new_doubleArray = zmc_dll.mc_new_doubleArray
    mc_new_doubleArray.argtypes = [c_uint32]
    mc_new_doubleArray.restype = POINTER(c_double)

    mc_delete_doubleArray = zmc_dll.mc_delete_doubleArray
    mc_delete_doubleArray.argtypes = [POINTER(c_double)]
    mc_delete_doubleArray.restype = None

    mc_doubleArray_getitem = zmc_dll.mc_doubleArray_getitem
    mc_doubleArray_getitem.argtypes = [POINTER(c_double), c_uint32]
    mc_doubleArray_getitem.restype = c_double

    mc_doubleArray_setitem = zmc_dll.mc_doubleArray_setitem
    mc_doubleArray_setitem.argtypes = [POINTER(c_double), c_uint32, c_double]
    mc_doubleArray_setitem.restype = None

    # 虚轴API函数定义
    axis_virtual_new = zmc_dll.axis_virtual_new
    axis_virtual_new.argtypes = []
    axis_virtual_new.restype = AXIS_REF

    axis_virtual_runcycle = zmc_dll.axis_virtual_runcycle
    axis_virtual_runcycle.argtypes = [AXIS_REF]
    axis_virtual_runcycle.restype = None

    axis_virtual_set_config = zmc_dll.axis_virtual_set_config
    axis_virtual_set_config.argtypes = [AXIS_REF, AxisCiaConfig_REF]
    axis_virtual_set_config.restype = None

    axis_virtual_print = zmc_dll.axis_virtual_print
    axis_virtual_print.argtypes = [AXIS_REF, c_int]
    axis_virtual_print.restype = None

    axis_virtual_delete = zmc_dll.axis_virtual_delete
    axis_virtual_delete.argtypes = [AXIS_REF]
    axis_virtual_delete.restype = None

    # AxisCiaConfig API函数定义
    axis_config_new = zmc_dll.axis_config_new
    axis_config_new.argtypes = []
    axis_config_new.restype = AxisCiaConfig_REF

    axis_config_default_init = zmc_dll.axis_config_default_init
    axis_config_default_init.argtypes = [AxisCiaConfig_REF]
    axis_config_default_init.restype = None

    axis_config_check = zmc_dll.axis_config_check
    axis_config_check.argtypes = [AxisCiaConfig_REF]
    axis_config_check.restype = c_int

    axis_config_delete = zmc_dll.axis_configl_delete
    axis_config_delete.argtypes = [AxisCiaConfig_REF]
    axis_config_delete.restype = None

    # 定义函数原型
    MC_Power_create = zmc_dll.MC_Power_create
    MC_Power_create.argtypes = [POINTER(McPower)]
    MC_Power_create.restype = c_int

    MC_Power_exec = zmc_dll.MC_Power_exec
    MC_Power_exec.argtypes = [POINTER(McPower)]
    MC_Power_exec.restype = c_int

    MC_Power_destroy = zmc_dll.MC_Power_destroy
    MC_Power_destroy.argtypes = [POINTER(McPower)]
    MC_Power_destroy.restype = c_int

    # McMoveAbsolute 函数原型
    MC_MoveAbsolute_create = zmc_dll.MC_MoveAbsolute_create
    MC_MoveAbsolute_create.argtypes = [POINTER(McMoveAbsolute)]
    MC_MoveAbsolute_create.restype = c_int

    MC_MoveAbsolute_exec = zmc_dll.MC_MoveAbsolute_exec
    MC_MoveAbsolute_exec.argtypes = [POINTER(McMoveAbsolute)]
    MC_MoveAbsolute_exec.restype = c_int

    MC_MoveAbsolute_destroy = zmc_dll.MC_MoveAbsolute_destroy
    MC_MoveAbsolute_destroy.argtypes = [POINTER(McMoveAbsolute)]
    MC_MoveAbsolute_destroy.restype = c_int

    # McMoveRelative 函数原型
    MC_MoveRelative_create = zmc_dll.MC_MoveRelative_create
    MC_MoveRelative_create.argtypes = [POINTER(McMoveRelative)]
    MC_MoveRelative_create.restype = c_int

    MC_MoveRelative_exec = zmc_dll.MC_MoveRelative_exec
    MC_MoveRelative_exec.argtypes = [POINTER(McMoveRelative)]
    MC_MoveRelative_exec.restype = c_int

    MC_MoveRelative_destroy = zmc_dll.MC_MoveRelative_destroy
    MC_MoveRelative_destroy.argtypes = [POINTER(McMoveRelative)]
    MC_MoveRelative_destroy.restype = c_int

    # McMoveAdditive 函数原型
    MC_MoveAdditive_create = zmc_dll.MC_MoveAdditive_create
    MC_MoveAdditive_create.argtypes = [POINTER(McMoveAdditive)]
    MC_MoveAdditive_create.restype = c_int

    MC_MoveAdditive_exec = zmc_dll.MC_MoveAdditive_exec
    MC_MoveAdditive_exec.argtypes = [POINTER(McMoveAdditive)]
    MC_MoveAdditive_exec.restype = c_int

    MC_MoveAdditive_destroy = zmc_dll.MC_MoveAdditive_destroy
    MC_MoveAdditive_destroy.argtypes = [POINTER(McMoveAdditive)]
    MC_MoveAdditive_destroy.restype = c_int

    # McStop 函数原型
    MC_Stop_create = zmc_dll.MC_Stop_create
    MC_Stop_create.argtypes = [POINTER(McStop)]
    MC_Stop_create.restype = c_int

    MC_Stop_exec = zmc_dll.MC_Stop_exec
    MC_Stop_exec.argtypes = [POINTER(McStop)]
    MC_Stop_exec.restype = c_int

    MC_Stop_destroy = zmc_dll.MC_Stop_destroy
    MC_Stop_destroy.argtypes = [POINTER(McStop)]
    MC_Stop_destroy.restype = c_int

    # McHalt 函数原型
    MC_Halt_create = zmc_dll.MC_Halt_create
    MC_Halt_create.argtypes = [POINTER(McHalt)]
    MC_Halt_create.restype = c_int

    MC_Halt_exec = zmc_dll.MC_Halt_exec
    MC_Halt_exec.argtypes = [POINTER(McHalt)]
    MC_Halt_exec.restype = c_int

    MC_Halt_destroy = zmc_dll.MC_Halt_destroy
    MC_Halt_destroy.argtypes = [POINTER(McHalt)]
    MC_Halt_destroy.restype = c_int

    # McMoveVelocity 函数原型
    MC_MoveVelocity_create = zmc_dll.MC_MoveVelocity_create
    MC_MoveVelocity_create.argtypes = [POINTER(McMoveVelocity)]
    MC_MoveVelocity_create.restype = c_int

    MC_MoveVelocity_exec = zmc_dll.MC_MoveVelocity_exec
    MC_MoveVelocity_exec.argtypes = [POINTER(McMoveVelocity)]
    MC_MoveVelocity_exec.restype = c_int

    MC_MoveVelocity_destroy = zmc_dll.MC_MoveVelocity_destroy
    MC_MoveVelocity_destroy.argtypes = [POINTER(McMoveVelocity)]
    MC_MoveVelocity_destroy.restype = c_int

    # CamTableSelect
    MC_CamTableSelect_create = zmc_dll.MC_CamTableSelect_create
    MC_CamTableSelect_create.argtypes = [POINTER(McCamTableSelect)]
    MC_CamTableSelect_create.restype = c_int
    MC_CamTableSelect_exec = zmc_dll.MC_CamTableSelect_exec
    MC_CamTableSelect_exec.argtypes = [POINTER(McCamTableSelect)]
    MC_CamTableSelect_exec.restype = c_int
    MC_CamTableSelect_destroy = zmc_dll.MC_CamTableSelect_destroy
    MC_CamTableSelect_destroy.argtypes = [POINTER(McCamTableSelect)]
    MC_CamTableSelect_destroy.restype = c_int
    # ReadCAM
    MC_ReadCAM_create = zmc_dll.MC_ReadCAM_create
    MC_ReadCAM_create.argtypes = [POINTER(McReadCAM)]
    MC_ReadCAM_create.restype = c_int
    MC_ReadCAM_exec = zmc_dll.MC_ReadCAM_exec
    MC_ReadCAM_exec.argtypes = [POINTER(McReadCAM)]
    MC_ReadCAM_exec.restype = c_int
    MC_ReadCAM_destroy = zmc_dll.MC_ReadCAM_destroy
    MC_ReadCAM_destroy.argtypes = [POINTER(McReadCAM)]
    MC_ReadCAM_destroy.restype = c_int
    # WriteCAM
    MC_WriteCAM_create = zmc_dll.MC_WriteCAM_create
    MC_WriteCAM_create.argtypes = [POINTER(McWriteCAM)]
    MC_WriteCAM_create.restype = c_int
    MC_WriteCAM_exec = zmc_dll.MC_WriteCAM_exec
    MC_WriteCAM_exec.argtypes = [POINTER(McWriteCAM)]
    MC_WriteCAM_exec.restype = c_int
    MC_WriteCAM_destroy = zmc_dll.MC_WriteCAM_destroy
    MC_WriteCAM_destroy.argtypes = [POINTER(McWriteCAM)]
    MC_WriteCAM_destroy.restype = c_int
    # CamIn
    MC_CamIn_create = zmc_dll.MC_CamIn_create
    MC_CamIn_create.argtypes = [POINTER(McCamIn)]
    MC_CamIn_create.restype = c_int
    MC_CamIn_exec = zmc_dll.MC_CamIn_exec
    MC_CamIn_exec.argtypes = [POINTER(McCamIn)]
    MC_CamIn_exec.restype = c_int
    MC_CamIn_destroy = zmc_dll.MC_CamIn_destroy
    MC_CamIn_destroy.argtypes = [POINTER(McCamIn)]
    MC_CamIn_destroy.restype = c_int

    # FbGetTappet
    MC_GetTappetValue_create = zmc_dll.MC_GetTappetValue_create
    MC_GetTappetValue_create.argtypes = [POINTER(McGetTappetValue)]
    MC_GetTappetValue_create.restype = c_int
    MC_GetTappetValue_exec = zmc_dll.MC_GetTappetValue_exec
    MC_GetTappetValue_exec.argtypes = [POINTER(McGetTappetValue)]
    MC_GetTappetValue_exec.restype = c_int
    MC_GetTappetValue_destroy = zmc_dll.MC_GetTappetValue_destroy
    MC_GetTappetValue_destroy.argtypes = [POINTER(McGetTappetValue)]
    MC_GetTappetValue_destroy.restype = c_int

    # setTappetFromCamIn function
    setTappetFromCamIn = zmc_dll.setTappetFromCamIn
    setTappetFromCamIn.argtypes = [POINTER(McGetTappetValue), POINTER(McCamIn)]
    setTappetFromCamIn.restype = c_int

    # Axis FB bridge API
    axfb_bridge_create = zmc_dll.axfb_bridge_create
    axfb_bridge_create.argtypes = [POINTER(axfb_bridge_handle_t)]
    axfb_bridge_create.restype = c_int

    axfb_bridge_configure = zmc_dll.axfb_bridge_configure
    axfb_bridge_configure.argtypes = [axfb_bridge_handle_t, c_char_p, c_char_p, c_uint]
    axfb_bridge_configure.restype = c_int

    axfb_bridge_destroy = zmc_dll.axfb_bridge_destroy
    axfb_bridge_destroy.argtypes = [axfb_bridge_handle_t]
    axfb_bridge_destroy.restype = None

    axfb_bridge_bind_sdo_adapter = zmc_dll.axfb_bridge_bind_sdo_adapter
    axfb_bridge_bind_sdo_adapter.argtypes = [axfb_bridge_handle_t, POINTER(SdoCAdapter)]
    axfb_bridge_bind_sdo_adapter.restype = c_int

    axfb_bridge_native_sdo = zmc_dll.axfb_bridge_native_sdo
    axfb_bridge_native_sdo.argtypes = [axfb_bridge_handle_t]
    axfb_bridge_native_sdo.restype = c_void_p

    axfb_bridge_start_sdo = zmc_dll.axfb_bridge_start_sdo
    axfb_bridge_start_sdo.argtypes = [axfb_bridge_handle_t]
    axfb_bridge_start_sdo.restype = c_int

    axfb_bridge_stop_sdo = zmc_dll.axfb_bridge_stop_sdo
    axfb_bridge_stop_sdo.argtypes = [axfb_bridge_handle_t]
    axfb_bridge_stop_sdo.restype = c_int

    axfb_bridge_register_opmode_watches = zmc_dll.axfb_bridge_register_opmode_watches
    axfb_bridge_register_opmode_watches.argtypes = [axfb_bridge_handle_t, c_uint8]
    axfb_bridge_register_opmode_watches.restype = c_int

    axfb_bridge_apply_pio_layout = zmc_dll.axfb_bridge_apply_pio_layout
    axfb_bridge_apply_pio_layout.argtypes = [axfb_bridge_handle_t, c_void_p]
    axfb_bridge_apply_pio_layout.restype = c_int

    axfb_bridge_servo_count = zmc_dll.axfb_bridge_servo_count
    axfb_bridge_servo_count.argtypes = [axfb_bridge_handle_t, POINTER(c_int)]
    axfb_bridge_servo_count.restype = c_int

    axfb_bridge_pdo_layout_ready = zmc_dll.axfb_bridge_pdo_layout_ready
    axfb_bridge_pdo_layout_ready.argtypes = [axfb_bridge_handle_t, POINTER(c_int)]
    axfb_bridge_pdo_layout_ready.restype = c_int

    axfb_bridge_auto_bind = zmc_dll.axfb_bridge_auto_bind
    axfb_bridge_auto_bind.argtypes = [axfb_bridge_handle_t, c_int, c_int]
    axfb_bridge_auto_bind.restype = c_int

    axfb_bridge_manual_bind = zmc_dll.axfb_bridge_manual_bind
    axfb_bridge_manual_bind.argtypes = [axfb_bridge_handle_t, c_int, POINTER(c_int), c_int]
    axfb_bridge_manual_bind.restype = c_int

    axfb_bridge_run_cycle = zmc_dll.axfb_bridge_run_cycle
    axfb_bridge_run_cycle.argtypes = [axfb_bridge_handle_t, c_void_p, c_void_p]
    axfb_bridge_run_cycle.restype = c_int

    axfb_bridge_run_cycle_parsed = zmc_dll.axfb_bridge_run_cycle_parsed
    axfb_bridge_run_cycle_parsed.argtypes = [axfb_bridge_handle_t, c_void_p, c_void_p]
    axfb_bridge_run_cycle_parsed.restype = c_int

    axfb_bridge_axis_count = zmc_dll.axfb_bridge_axis_count
    axfb_bridge_axis_count.argtypes = [axfb_bridge_handle_t, POINTER(c_int)]
    axfb_bridge_axis_count.restype = c_int

    axfb_bridge_get_address = zmc_dll.axfb_bridge_get_address
    axfb_bridge_get_address.argtypes = [axfb_bridge_handle_t, c_int, POINTER(c_int)]
    axfb_bridge_get_address.restype = c_int

    axfb_bridge_get_actual_position = zmc_dll.axfb_bridge_get_actual_position
    axfb_bridge_get_actual_position.argtypes = [axfb_bridge_handle_t, c_int, POINTER(c_int)]
    axfb_bridge_get_actual_position.restype = c_int

    axfb_bridge_push_cmd_packet = zmc_dll.axfb_bridge_push_cmd_packet
    axfb_bridge_push_cmd_packet.argtypes = [axfb_bridge_handle_t, POINTER(AxFbBridgePacket)]
    axfb_bridge_push_cmd_packet.restype = c_int

    axfb_bridge_pop_status_packet = zmc_dll.axfb_bridge_pop_status_packet
    axfb_bridge_pop_status_packet.argtypes = [axfb_bridge_handle_t, POINTER(AxFbBridgePacket)]
    axfb_bridge_pop_status_packet.restype = c_int

    axfb_bridge_consume_exit_flag = zmc_dll.axfb_bridge_consume_exit_flag
    axfb_bridge_consume_exit_flag.argtypes = [axfb_bridge_handle_t]
    axfb_bridge_consume_exit_flag.restype = c_int

    # ========== 新增功能块函数声明 ==========
    
    # McReset 函数原型
    MC_Reset_create = zmc_dll.MC_Reset_create
    MC_Reset_create.argtypes = [POINTER(McReset)]
    MC_Reset_create.restype = c_int
    
    MC_Reset_exec = zmc_dll.MC_Reset_exec
    MC_Reset_exec.argtypes = [POINTER(McReset)]
    MC_Reset_exec.restype = c_int
    
    MC_Reset_destroy = zmc_dll.MC_Reset_destroy
    MC_Reset_destroy.argtypes = [POINTER(McReset)]
    MC_Reset_destroy.restype = c_int
    
    # McSetPosition 函数原型
    MC_SetPosition_create = zmc_dll.MC_SetPosition_create
    MC_SetPosition_create.argtypes = [POINTER(McSetPosition)]
    MC_SetPosition_create.restype = c_int
    
    MC_SetPosition_exec = zmc_dll.MC_SetPosition_exec
    MC_SetPosition_exec.argtypes = [POINTER(McSetPosition)]
    MC_SetPosition_exec.restype = c_int
    
    MC_SetPosition_destroy = zmc_dll.MC_SetPosition_destroy
    MC_SetPosition_destroy.argtypes = [POINTER(McSetPosition)]
    MC_SetPosition_destroy.restype = c_int
    
    # McReadStatus 函数原型
    MC_ReadStatus_create = zmc_dll.MC_ReadStatus_create
    MC_ReadStatus_create.argtypes = [POINTER(McReadStatus)]
    MC_ReadStatus_create.restype = c_int
    
    MC_ReadStatus_exec = zmc_dll.MC_ReadStatus_exec
    MC_ReadStatus_exec.argtypes = [POINTER(McReadStatus)]
    MC_ReadStatus_exec.restype = c_int
    
    MC_ReadStatus_destroy = zmc_dll.MC_ReadStatus_destroy
    MC_ReadStatus_destroy.argtypes = [POINTER(McReadStatus)]
    MC_ReadStatus_destroy.restype = c_int
    
    # McReadActualPosition 函数原型
    MC_ReadActualPosition_create = zmc_dll.MC_ReadActualPosition_create
    MC_ReadActualPosition_create.argtypes = [POINTER(McReadActualPosition)]
    MC_ReadActualPosition_create.restype = c_int
    
    MC_ReadActualPosition_exec = zmc_dll.MC_ReadActualPosition_exec
    MC_ReadActualPosition_exec.argtypes = [POINTER(McReadActualPosition)]
    MC_ReadActualPosition_exec.restype = c_int
    
    MC_ReadActualPosition_destroy = zmc_dll.MC_ReadActualPosition_destroy
    MC_ReadActualPosition_destroy.argtypes = [POINTER(McReadActualPosition)]
    MC_ReadActualPosition_destroy.restype = c_int
    
    # McReadActualVelocity 函数原型
    MC_ReadActualVelocity_create = zmc_dll.MC_ReadActualVelocity_create
    MC_ReadActualVelocity_create.argtypes = [POINTER(McReadActualVelocity)]
    MC_ReadActualVelocity_create.restype = c_int
    
    MC_ReadActualVelocity_exec = zmc_dll.MC_ReadActualVelocity_exec
    MC_ReadActualVelocity_exec.argtypes = [POINTER(McReadActualVelocity)]
    MC_ReadActualVelocity_exec.restype = c_int
    
    MC_ReadActualVelocity_destroy = zmc_dll.MC_ReadActualVelocity_destroy
    MC_ReadActualVelocity_destroy.argtypes = [POINTER(McReadActualVelocity)]
    MC_ReadActualVelocity_destroy.restype = c_int
    
    # McReadAxisError 函数原型
    MC_ReadAxisError_create = zmc_dll.MC_ReadAxisError_create
    MC_ReadAxisError_create.argtypes = [POINTER(McReadAxisError)]
    MC_ReadAxisError_create.restype = c_int
    
    MC_ReadAxisError_exec = zmc_dll.MC_ReadAxisError_exec
    MC_ReadAxisError_exec.argtypes = [POINTER(McReadAxisError)]
    MC_ReadAxisError_exec.restype = c_int
    
    MC_ReadAxisError_destroy = zmc_dll.MC_ReadAxisError_destroy
    MC_ReadAxisError_destroy.argtypes = [POINTER(McReadAxisError)]
    MC_ReadAxisError_destroy.restype = c_int
    
    # McReadParameter 函数原型
    MC_ReadParameter_create = zmc_dll.MC_ReadParameter_create
    MC_ReadParameter_create.argtypes = [POINTER(McReadParameter)]
    MC_ReadParameter_create.restype = c_int
    
    MC_ReadParameter_exec = zmc_dll.MC_ReadParameter_exec
    MC_ReadParameter_exec.argtypes = [POINTER(McReadParameter)]
    MC_ReadParameter_exec.restype = c_int
    
    MC_ReadParameter_destroy = zmc_dll.MC_ReadParameter_destroy
    MC_ReadParameter_destroy.argtypes = [POINTER(McReadParameter)]
    MC_ReadParameter_destroy.restype = c_int
    
    # McWriteParameter 函数原型
    MC_WriteParameter_create = zmc_dll.MC_WriteParameter_create
    MC_WriteParameter_create.argtypes = [POINTER(McWriteParameter)]
    MC_WriteParameter_create.restype = c_int
    
    MC_WriteParameter_exec = zmc_dll.MC_WriteParameter_exec
    MC_WriteParameter_exec.argtypes = [POINTER(McWriteParameter)]
    MC_WriteParameter_exec.restype = c_int
    
    MC_WriteParameter_destroy = zmc_dll.MC_WriteParameter_destroy
    MC_WriteParameter_destroy.argtypes = [POINTER(McWriteParameter)]
    MC_WriteParameter_destroy.restype = c_int
    
    # McHome 函数原型
    MC_Home_create = zmc_dll.MC_Home_create
    MC_Home_create.argtypes = [POINTER(McHome)]
    MC_Home_create.restype = c_int
    
    MC_Home_exec = zmc_dll.MC_Home_exec
    MC_Home_exec.argtypes = [POINTER(McHome)]
    MC_Home_exec.restype = c_int
    
    MC_Home_destroy = zmc_dll.MC_Home_destroy
    MC_Home_destroy.argtypes = [POINTER(McHome)]
    MC_Home_destroy.restype = c_int
    
    # McHoming 函数原型
    MC_Homing_create = zmc_dll.MC_Homing_create
    MC_Homing_create.argtypes = [POINTER(McHoming)]
    MC_Homing_create.restype = c_int
    
    MC_Homing_exec = zmc_dll.MC_Homing_exec
    MC_Homing_exec.argtypes = [POINTER(McHoming)]
    MC_Homing_exec.restype = c_int
    
    MC_Homing_destroy = zmc_dll.MC_Homing_destroy
    MC_Homing_destroy.argtypes = [POINTER(McHoming)]
    MC_Homing_destroy.restype = c_int
    
    # McJog 函数原型
    MC_Jog_create = zmc_dll.MC_Jog_create
    MC_Jog_create.argtypes = [POINTER(McJog)]
    MC_Jog_create.restype = c_int
    
    MC_Jog_exec = zmc_dll.MC_Jog_exec
    MC_Jog_exec.argtypes = [POINTER(McJog)]
    MC_Jog_exec.restype = c_int
    
    MC_Jog_destroy = zmc_dll.MC_Jog_destroy
    MC_Jog_destroy.argtypes = [POINTER(McJog)]
    MC_Jog_destroy.restype = c_int
    
    # McGearIn 函数原型
    MC_GearIn_create = zmc_dll.MC_GearIn_create
    MC_GearIn_create.argtypes = [POINTER(McGearIn)]
    MC_GearIn_create.restype = c_int
    
    MC_GearIn_exec = zmc_dll.MC_GearIn_exec
    MC_GearIn_exec.argtypes = [POINTER(McGearIn)]
    MC_GearIn_exec.restype = c_int
    
    MC_GearIn_destroy = zmc_dll.MC_GearIn_destroy
    MC_GearIn_destroy.argtypes = [POINTER(McGearIn)]
    MC_GearIn_destroy.restype = c_int
    
    # McGearOut 函数原型
    MC_GearOut_create = zmc_dll.MC_GearOut_create
    MC_GearOut_create.argtypes = [POINTER(McGearOut)]
    MC_GearOut_create.restype = c_int
    
    MC_GearOut_exec = zmc_dll.MC_GearOut_exec
    MC_GearOut_exec.argtypes = [POINTER(McGearOut)]
    MC_GearOut_exec.restype = c_int
    
    MC_GearOut_destroy = zmc_dll.MC_GearOut_destroy
    MC_GearOut_destroy.argtypes = [POINTER(McGearOut)]
    MC_GearOut_destroy.restype = c_int
    
    # McPhasing 函数原型
    MC_Phasing_create = zmc_dll.MC_Phasing_create
    MC_Phasing_create.argtypes = [POINTER(McPhasing)]
    MC_Phasing_create.restype = c_int
    
    MC_Phasing_exec = zmc_dll.MC_Phasing_exec
    MC_Phasing_exec.argtypes = [POINTER(McPhasing)]
    MC_Phasing_exec.restype = c_int
    
    MC_Phasing_destroy = zmc_dll.MC_Phasing_destroy
    MC_Phasing_destroy.argtypes = [POINTER(McPhasing)]
    MC_Phasing_destroy.restype = c_int
    
    # McSetTorque 函数原型
    MC_SetTorque_create = zmc_dll.MC_SetTorque_create
    MC_SetTorque_create.argtypes = [POINTER(McSetTorque)]
    MC_SetTorque_create.restype = c_int
    
    MC_SetTorque_exec = zmc_dll.MC_SetTorque_exec
    MC_SetTorque_exec.argtypes = [POINTER(McSetTorque)]
    MC_SetTorque_exec.restype = c_int
    
    MC_SetTorque_destroy = zmc_dll.MC_SetTorque_destroy
    MC_SetTorque_destroy.argtypes = [POINTER(McSetTorque)]
    MC_SetTorque_destroy.restype = c_int
    
    # McFollowPosition 函数原型
    MC_FollowPosition_create = zmc_dll.MC_FollowPosition_create
    MC_FollowPosition_create.argtypes = [POINTER(McFollowPosition)]
    MC_FollowPosition_create.restype = c_int
    
    MC_FollowPosition_exec = zmc_dll.MC_FollowPosition_exec
    MC_FollowPosition_exec.argtypes = [POINTER(McFollowPosition)]
    MC_FollowPosition_exec.restype = c_int
    
    MC_FollowPosition_destroy = zmc_dll.MC_FollowPosition_destroy
    MC_FollowPosition_destroy.argtypes = [POINTER(McFollowPosition)]
    MC_FollowPosition_destroy.restype = c_int
    
    # McFollowVelocity 函数原型
    MC_FollowVelocity_create = zmc_dll.MC_FollowVelocity_create
    MC_FollowVelocity_create.argtypes = [POINTER(McFollowVelocity)]
    MC_FollowVelocity_create.restype = c_int
    
    MC_FollowVelocity_exec = zmc_dll.MC_FollowVelocity_exec
    MC_FollowVelocity_exec.argtypes = [POINTER(McFollowVelocity)]
    MC_FollowVelocity_exec.restype = c_int
    
    MC_FollowVelocity_destroy = zmc_dll.MC_FollowVelocity_destroy
    MC_FollowVelocity_destroy.argtypes = [POINTER(McFollowVelocity)]
    MC_FollowVelocity_destroy.restype = c_int
    
    # McTrackAxis 函数原型
    MC_TrackAxis_create = zmc_dll.MC_TrackAxis_create
    MC_TrackAxis_create.argtypes = [POINTER(McTrackAxis)]
    MC_TrackAxis_create.restype = c_int
    
    MC_TrackAxis_exec = zmc_dll.MC_TrackAxis_exec
    MC_TrackAxis_exec.argtypes = [POINTER(McTrackAxis)]
    MC_TrackAxis_exec.restype = c_int
    
    MC_TrackAxis_destroy = zmc_dll.MC_TrackAxis_destroy
    MC_TrackAxis_destroy.argtypes = [POINTER(McTrackAxis)]
    MC_TrackAxis_destroy.restype = c_int
    
    # McReadSetValues 函数原型
    MC_ReadSetValues_create = zmc_dll.MC_ReadSetValues_create
    MC_ReadSetValues_create.argtypes = [POINTER(McReadSetValues)]
    MC_ReadSetValues_create.restype = c_int
    
    MC_ReadSetValues_exec = zmc_dll.MC_ReadSetValues_exec
    MC_ReadSetValues_exec.argtypes = [POINTER(McReadSetValues)]
    MC_ReadSetValues_exec.restype = c_int
    
    MC_ReadSetValues_destroy = zmc_dll.MC_ReadSetValues_destroy
    MC_ReadSetValues_destroy.argtypes = [POINTER(McReadSetValues)]
    MC_ReadSetValues_destroy.restype = c_int
    
    # McReadActualValues 函数原型
    MC_ReadActualValues_create = zmc_dll.MC_ReadActualValues_create
    MC_ReadActualValues_create.argtypes = [POINTER(McReadActualValues)]
    MC_ReadActualValues_create.restype = c_int
    
    MC_ReadActualValues_exec = zmc_dll.MC_ReadActualValues_exec
    MC_ReadActualValues_exec.argtypes = [POINTER(McReadActualValues)]
    MC_ReadActualValues_exec.restype = c_int
    
    MC_ReadActualValues_destroy = zmc_dll.MC_ReadActualValues_destroy
    MC_ReadActualValues_destroy.argtypes = [POINTER(McReadActualValues)]
    MC_ReadActualValues_destroy.restype = c_int
    
    # McInPosition 函数原型
    MC_InPosition_create = zmc_dll.MC_InPosition_create
    MC_InPosition_create.argtypes = [POINTER(McInPosition)]
    MC_InPosition_create.restype = c_int
    
    MC_InPosition_exec = zmc_dll.MC_InPosition_exec
    MC_InPosition_exec.argtypes = [POINTER(McInPosition)]
    MC_InPosition_exec.restype = c_int
    
    MC_InPosition_destroy = zmc_dll.MC_InPosition_destroy
    MC_InPosition_destroy.argtypes = [POINTER(McInPosition)]
    MC_InPosition_destroy.restype = c_int
    
    # McCheckLimits 函数原型
    MC_CheckLimits_create = zmc_dll.MC_CheckLimits_create
    MC_CheckLimits_create.argtypes = [POINTER(McCheckLimits)]
    MC_CheckLimits_create.restype = c_int
    
    MC_CheckLimits_exec = zmc_dll.MC_CheckLimits_exec
    MC_CheckLimits_exec.argtypes = [POINTER(McCheckLimits)]
    MC_CheckLimits_exec.restype = c_int
    
    MC_CheckLimits_destroy = zmc_dll.MC_CheckLimits_destroy
    MC_CheckLimits_destroy.argtypes = [POINTER(McCheckLimits)]
    MC_CheckLimits_destroy.restype = c_int

    # McReadActualTorque 函数原型
    MC_ReadActualTorque_create = zmc_dll.MC_ReadActualTorque_create
    MC_ReadActualTorque_create.argtypes = [POINTER(McReadActualTorque)]
    MC_ReadActualTorque_create.restype = c_int
    
    MC_ReadActualTorque_exec = zmc_dll.MC_ReadActualTorque_exec
    MC_ReadActualTorque_exec.argtypes = [POINTER(McReadActualTorque)]
    MC_ReadActualTorque_exec.restype = c_int
    
    MC_ReadActualTorque_destroy = zmc_dll.MC_ReadActualTorque_destroy
    MC_ReadActualTorque_destroy.argtypes = [POINTER(McReadActualTorque)]
    MC_ReadActualTorque_destroy.restype = c_int

    # McReadCommandPosition 函数原型
    MC_ReadCommandPosition_create = zmc_dll.MC_ReadCommandPosition_create
    MC_ReadCommandPosition_create.argtypes = [POINTER(McReadCommandPosition)]
    MC_ReadCommandPosition_create.restype = c_int
    
    MC_ReadCommandPosition_exec = zmc_dll.MC_ReadCommandPosition_exec
    MC_ReadCommandPosition_exec.argtypes = [POINTER(McReadCommandPosition)]
    MC_ReadCommandPosition_exec.restype = c_int
    
    MC_ReadCommandPosition_destroy = zmc_dll.MC_ReadCommandPosition_destroy
    MC_ReadCommandPosition_destroy.argtypes = [POINTER(McReadCommandPosition)]
    MC_ReadCommandPosition_destroy.restype = c_int

    # McReadBoolParameter 函数原型
    MC_ReadBoolParameter_create = zmc_dll.MC_ReadBoolParameter_create
    MC_ReadBoolParameter_create.argtypes = [POINTER(McReadBoolParameter)]
    MC_ReadBoolParameter_create.restype = c_int
    
    MC_ReadBoolParameter_exec = zmc_dll.MC_ReadBoolParameter_exec
    MC_ReadBoolParameter_exec.argtypes = [POINTER(McReadBoolParameter)]
    MC_ReadBoolParameter_exec.restype = c_int
    
    MC_ReadBoolParameter_destroy = zmc_dll.MC_ReadBoolParameter_destroy
    MC_ReadBoolParameter_destroy.argtypes = [POINTER(McReadBoolParameter)]
    MC_ReadBoolParameter_destroy.restype = c_int

    # McWriteBoolParameter 函数原型
    MC_WriteBoolParameter_create = zmc_dll.MC_WriteBoolParameter_create
    MC_WriteBoolParameter_create.argtypes = [POINTER(McWriteBoolParameter)]
    MC_WriteBoolParameter_create.restype = c_int
    
    MC_WriteBoolParameter_exec = zmc_dll.MC_WriteBoolParameter_exec
    MC_WriteBoolParameter_exec.argtypes = [POINTER(McWriteBoolParameter)]
    MC_WriteBoolParameter_exec.restype = c_int
    
    MC_WriteBoolParameter_destroy = zmc_dll.MC_WriteBoolParameter_destroy
    MC_WriteBoolParameter_destroy.argtypes = [POINTER(McWriteBoolParameter)]
    MC_WriteBoolParameter_destroy.restype = c_int

    # McMoveContinuousAbsolute 函数原型
    MC_MoveContinuousAbsolute_create = zmc_dll.MC_MoveContinuousAbsolute_create
    MC_MoveContinuousAbsolute_create.argtypes = [POINTER(McMoveContinuousAbsolute)]
    MC_MoveContinuousAbsolute_create.restype = c_int
    
    MC_MoveContinuousAbsolute_exec = zmc_dll.MC_MoveContinuousAbsolute_exec
    MC_MoveContinuousAbsolute_exec.argtypes = [POINTER(McMoveContinuousAbsolute)]
    MC_MoveContinuousAbsolute_exec.restype = c_int
    
    MC_MoveContinuousAbsolute_destroy = zmc_dll.MC_MoveContinuousAbsolute_destroy
    MC_MoveContinuousAbsolute_destroy.argtypes = [POINTER(McMoveContinuousAbsolute)]
    MC_MoveContinuousAbsolute_destroy.restype = c_int

    # McMoveContinuousRelative 函数原型
    MC_MoveContinuousRelative_create = zmc_dll.MC_MoveContinuousRelative_create
    MC_MoveContinuousRelative_create.argtypes = [POINTER(McMoveContinuousRelative)]
    MC_MoveContinuousRelative_create.restype = c_int
    
    MC_MoveContinuousRelative_exec = zmc_dll.MC_MoveContinuousRelative_exec
    MC_MoveContinuousRelative_exec.argtypes = [POINTER(McMoveContinuousRelative)]
    MC_MoveContinuousRelative_exec.restype = c_int
    
    MC_MoveContinuousRelative_destroy = zmc_dll.MC_MoveContinuousRelative_destroy
    MC_MoveContinuousRelative_destroy.argtypes = [POINTER(McMoveContinuousRelative)]
    MC_MoveContinuousRelative_destroy.restype = c_int

    # McMoveSuperImposed 函数原型
    MC_MoveSuperImposed_create = zmc_dll.MC_MoveSuperImposed_create
    MC_MoveSuperImposed_create.argtypes = [POINTER(McMoveSuperImposed)]
    MC_MoveSuperImposed_create.restype = c_int
    
    MC_MoveSuperImposed_exec = zmc_dll.MC_MoveSuperImposed_exec
    MC_MoveSuperImposed_exec.argtypes = [POINTER(McMoveSuperImposed)]
    MC_MoveSuperImposed_exec.restype = c_int
    
    MC_MoveSuperImposed_destroy = zmc_dll.MC_MoveSuperImposed_destroy
    MC_MoveSuperImposed_destroy.argtypes = [POINTER(McMoveSuperImposed)]
    MC_MoveSuperImposed_destroy.restype = c_int

    # McPositionProfile 函数原型
    MC_PositionProfile_create = zmc_dll.MC_PositionProfile_create
    MC_PositionProfile_create.argtypes = [POINTER(McPositionProfile)]
    MC_PositionProfile_create.restype = c_int
    
    MC_PositionProfile_exec = zmc_dll.MC_PositionProfile_exec
    MC_PositionProfile_exec.argtypes = [POINTER(McPositionProfile)]
    MC_PositionProfile_exec.restype = c_int
    
    MC_PositionProfile_destroy = zmc_dll.MC_PositionProfile_destroy
    MC_PositionProfile_destroy.argtypes = [POINTER(McPositionProfile)]
    MC_PositionProfile_destroy.restype = c_int

    # McAccelerationProfile 函数原型
    MC_AccelerationProfile_create = zmc_dll.MC_AccelerationProfile_create
    MC_AccelerationProfile_create.argtypes = [POINTER(McAccelerationProfile)]
    MC_AccelerationProfile_create.restype = c_int
    
    MC_AccelerationProfile_exec = zmc_dll.MC_AccelerationProfile_exec
    MC_AccelerationProfile_exec.argtypes = [POINTER(McAccelerationProfile)]
    MC_AccelerationProfile_exec.restype = c_int
    
    MC_AccelerationProfile_destroy = zmc_dll.MC_AccelerationProfile_destroy
    MC_AccelerationProfile_destroy.argtypes = [POINTER(McAccelerationProfile)]
    MC_AccelerationProfile_destroy.restype = c_int

    # McVelocityProfile 函数原型
    MC_VelocityProfile_create = zmc_dll.MC_VelocityProfile_create
    MC_VelocityProfile_create.argtypes = [POINTER(McVelocityProfile)]
    MC_VelocityProfile_create.restype = c_int
    
    MC_VelocityProfile_exec = zmc_dll.MC_VelocityProfile_exec
    MC_VelocityProfile_exec.argtypes = [POINTER(McVelocityProfile)]
    MC_VelocityProfile_exec.restype = c_int
    
    MC_VelocityProfile_destroy = zmc_dll.MC_VelocityProfile_destroy
    MC_VelocityProfile_destroy.argtypes = [POINTER(McVelocityProfile)]
    MC_VelocityProfile_destroy.restype = c_int

    # McGearInPos 函数原型
    MC_GearInPos_create = zmc_dll.MC_GearInPos_create
    MC_GearInPos_create.argtypes = [POINTER(McGearInPos)]
    MC_GearInPos_create.restype = c_int
    
    MC_GearInPos_exec = zmc_dll.MC_GearInPos_exec
    MC_GearInPos_exec.argtypes = [POINTER(McGearInPos)]
    MC_GearInPos_exec.restype = c_int
    
    MC_GearInPos_destroy = zmc_dll.MC_GearInPos_destroy
    MC_GearInPos_destroy.argtypes = [POINTER(McGearInPos)]
    MC_GearInPos_destroy.restype = c_int

    # McFollowPositionVelocity 函数原型
    MC_FollowPositionVelocity_create = zmc_dll.MC_FollowPositionVelocity_create
    MC_FollowPositionVelocity_create.argtypes = [POINTER(McFollowPositionVelocity)]
    MC_FollowPositionVelocity_create.restype = c_int
    
    MC_FollowPositionVelocity_exec = zmc_dll.MC_FollowPositionVelocity_exec
    MC_FollowPositionVelocity_exec.argtypes = [POINTER(McFollowPositionVelocity)]
    MC_FollowPositionVelocity_exec.restype = c_int
    
    MC_FollowPositionVelocity_destroy = zmc_dll.MC_FollowPositionVelocity_destroy
    MC_FollowPositionVelocity_destroy.argtypes = [POINTER(McFollowPositionVelocity)]
    MC_FollowPositionVelocity_destroy.restype = c_int

    # McFollowSetValues 函数原型
    MC_FollowSetValues_create = zmc_dll.MC_FollowSetValues_create
    MC_FollowSetValues_create.argtypes = [POINTER(McFollowSetValues)]
    MC_FollowSetValues_create.restype = c_int
    
    MC_FollowSetValues_exec = zmc_dll.MC_FollowSetValues_exec
    MC_FollowSetValues_exec.argtypes = [POINTER(McFollowSetValues)]
    MC_FollowSetValues_exec.restype = c_int
    
    MC_FollowSetValues_destroy = zmc_dll.MC_FollowSetValues_destroy
    MC_FollowSetValues_destroy.argtypes = [POINTER(McFollowSetValues)]
    MC_FollowSetValues_destroy.restype = c_int

    # McTrackSetValues 函数原型
    MC_TrackSetValues_create = zmc_dll.MC_TrackSetValues_create
    MC_TrackSetValues_create.argtypes = [POINTER(McTrackSetValues)]
    MC_TrackSetValues_create.restype = c_int
    
    MC_TrackSetValues_exec = zmc_dll.MC_TrackSetValues_exec
    MC_TrackSetValues_exec.argtypes = [POINTER(McTrackSetValues)]
    MC_TrackSetValues_exec.restype = c_int
    
    MC_TrackSetValues_destroy = zmc_dll.MC_TrackSetValues_destroy
    MC_TrackSetValues_destroy.argtypes = [POINTER(McTrackSetValues)]
    MC_TrackSetValues_destroy.restype = c_int

    # McCamOut 函数原型
    MC_CamOut_create = zmc_dll.MC_CamOut_create
    MC_CamOut_create.argtypes = [POINTER(McCamOut)]
    MC_CamOut_create.restype = c_int
    
    MC_CamOut_exec = zmc_dll.MC_CamOut_exec
    MC_CamOut_exec.argtypes = [POINTER(McCamOut)]
    MC_CamOut_exec.restype = c_int
    
    MC_CamOut_destroy = zmc_dll.MC_CamOut_destroy
    MC_CamOut_destroy.argtypes = [POINTER(McCamOut)]
    MC_CamOut_destroy.restype = c_int

    # McCamRegister 函数原型
    MC_CamRegister_create = zmc_dll.MC_CamRegister_create
    MC_CamRegister_create.argtypes = [POINTER(McCamRegister)]
    MC_CamRegister_create.restype = c_int
    
    MC_CamRegister_exec = zmc_dll.MC_CamRegister_exec
    MC_CamRegister_exec.argtypes = [POINTER(McCamRegister)]
    MC_CamRegister_exec.restype = c_int
    
    MC_CamRegister_destroy = zmc_dll.MC_CamRegister_destroy
    MC_CamRegister_destroy.argtypes = [POINTER(McCamRegister)]
    MC_CamRegister_destroy.restype = c_int

    # MC_CamCalc 函数原型
    MC_CamCalc_create = zmc_dll.MC_CamCalc_create
    MC_CamCalc_create.argtypes = [POINTER(McCamCalc)]
    MC_CamCalc_create.restype = c_int
    
    MC_CamCalc_exec = zmc_dll.MC_CamCalc_exec
    MC_CamCalc_exec.argtypes = [POINTER(McCamCalc)]
    MC_CamCalc_exec.restype = c_int
    
    MC_CamCalc_destroy = zmc_dll.MC_CamCalc_destroy
    MC_CamCalc_destroy.argtypes = [POINTER(McCamCalc)]
    MC_CamCalc_destroy.restype = c_int

    # McCAMBounds 函数原型
    MC_CAMBounds_create = zmc_dll.MC_CAMBounds_create
    MC_CAMBounds_create.argtypes = [POINTER(McCAMBounds)]
    MC_CAMBounds_create.restype = c_int
    
    MC_CAMBounds_exec = zmc_dll.MC_CAMBounds_exec
    MC_CAMBounds_exec.argtypes = [POINTER(McCAMBounds)]
    MC_CAMBounds_exec.restype = c_int
    
    MC_CAMBounds_destroy = zmc_dll.MC_CAMBounds_destroy
    MC_CAMBounds_destroy.argtypes = [POINTER(McCAMBounds)]
    MC_CAMBounds_destroy.restype = c_int

    # McCAMBounds_Pos 函数原型
    MC_CAMBounds_Pos_create = zmc_dll.MC_CAMBounds_Pos_create
    MC_CAMBounds_Pos_create.argtypes = [POINTER(McCAMBounds_Pos)]
    MC_CAMBounds_Pos_create.restype = c_int
    
    MC_CAMBounds_Pos_exec = zmc_dll.MC_CAMBounds_Pos_exec
    MC_CAMBounds_Pos_exec.argtypes = [POINTER(McCAMBounds_Pos)]
    MC_CAMBounds_Pos_exec.restype = c_int
    
    MC_CAMBounds_Pos_destroy = zmc_dll.MC_CAMBounds_Pos_destroy
    MC_CAMBounds_Pos_destroy.argtypes = [POINTER(McCAMBounds_Pos)]
    MC_CAMBounds_Pos_destroy.restype = c_int

    # McGetCamSlaveSetPosition 函数原型
    MC_GetCamSlaveSetPosition_create = zmc_dll.MC_GetCamSlaveSetPosition_create
    MC_GetCamSlaveSetPosition_create.argtypes = [POINTER(McGetCamSlaveSetPosition)]
    MC_GetCamSlaveSetPosition_create.restype = c_int
    
    MC_GetCamSlaveSetPosition_exec = zmc_dll.MC_GetCamSlaveSetPosition_exec
    MC_GetCamSlaveSetPosition_exec.argtypes = [POINTER(McGetCamSlaveSetPosition)]
    MC_GetCamSlaveSetPosition_exec.restype = c_int
    
    MC_GetCamSlaveSetPosition_destroy = zmc_dll.MC_GetCamSlaveSetPosition_destroy
    MC_GetCamSlaveSetPosition_destroy.argtypes = [POINTER(McGetCamSlaveSetPosition)]
    MC_GetCamSlaveSetPosition_destroy.restype = c_int

    # McAbortTrigger 函数原型
    MC_AbortTrigger_create = zmc_dll.MC_AbortTrigger_create
    MC_AbortTrigger_create.argtypes = [POINTER(McAbortTrigger)]
    MC_AbortTrigger_create.restype = c_int
    
    MC_AbortTrigger_exec = zmc_dll.MC_AbortTrigger_exec
    MC_AbortTrigger_exec.argtypes = [POINTER(McAbortTrigger)]
    MC_AbortTrigger_exec.restype = c_int
    
    MC_AbortTrigger_destroy = zmc_dll.MC_AbortTrigger_destroy
    MC_AbortTrigger_destroy.argtypes = [POINTER(McAbortTrigger)]
    MC_AbortTrigger_destroy.restype = c_int

    # McTouchProbe 函数原型
    MC_TouchProbe_create = zmc_dll.MC_TouchProbe_create
    MC_TouchProbe_create.argtypes = [POINTER(McTouchProbe)]
    MC_TouchProbe_create.restype = c_int
    
    MC_TouchProbe_exec = zmc_dll.MC_TouchProbe_exec
    MC_TouchProbe_exec.argtypes = [POINTER(McTouchProbe)]
    MC_TouchProbe_exec.restype = c_int
    
    MC_TouchProbe_destroy = zmc_dll.MC_TouchProbe_destroy
    MC_TouchProbe_destroy.argtypes = [POINTER(McTouchProbe)]
    MC_TouchProbe_destroy.restype = c_int

    # McDigitalCamSwitch 函数原型
    MC_DigitalCamSwitch_create = zmc_dll.MC_DigitalCamSwitch_create
    MC_DigitalCamSwitch_create.argtypes = [POINTER(McDigitalCamSwitch)]
    MC_DigitalCamSwitch_create.restype = c_int
    
    MC_DigitalCamSwitch_exec = zmc_dll.MC_DigitalCamSwitch_exec
    MC_DigitalCamSwitch_exec.argtypes = [POINTER(McDigitalCamSwitch)]
    MC_DigitalCamSwitch_exec.restype = c_int
    
    MC_DigitalCamSwitch_destroy = zmc_dll.MC_DigitalCamSwitch_destroy
    MC_DigitalCamSwitch_destroy.argtypes = [POINTER(McDigitalCamSwitch)]
    MC_DigitalCamSwitch_destroy.restype = c_int

    # McReadSetDriveValues 函数原型
    MC_ReadSetDriveValues_create = zmc_dll.MC_ReadSetDriveValues_create
    MC_ReadSetDriveValues_create.argtypes = [POINTER(McReadSetDriveValues)]
    MC_ReadSetDriveValues_create.restype = c_int
    
    MC_ReadSetDriveValues_exec = zmc_dll.MC_ReadSetDriveValues_exec
    MC_ReadSetDriveValues_exec.argtypes = [POINTER(McReadSetDriveValues)]
    MC_ReadSetDriveValues_exec.restype = c_int
    
    MC_ReadSetDriveValues_destroy = zmc_dll.MC_ReadSetDriveValues_destroy
    MC_ReadSetDriveValues_destroy.argtypes = [POINTER(McReadSetDriveValues)]
    MC_ReadSetDriveValues_destroy.restype = c_int

    # McReadActualDriveValues 函数原型
    MC_ReadActualDriveValues_create = zmc_dll.MC_ReadActualDriveValues_create
    MC_ReadActualDriveValues_create.argtypes = [POINTER(McReadActualDriveValues)]
    MC_ReadActualDriveValues_create.restype = c_int
    
    MC_ReadActualDriveValues_exec = zmc_dll.MC_ReadActualDriveValues_exec
    MC_ReadActualDriveValues_exec.argtypes = [POINTER(McReadActualDriveValues)]
    MC_ReadActualDriveValues_exec.restype = c_int
    
    MC_ReadActualDriveValues_destroy = zmc_dll.MC_ReadActualDriveValues_destroy
    MC_ReadActualDriveValues_destroy.argtypes = [POINTER(McReadActualDriveValues)]
    MC_ReadActualDriveValues_destroy.restype = c_int

    # McGetTrackingError 函数原型
    MC_GetTrackingError_create = zmc_dll.MC_GetTrackingError_create
    MC_GetTrackingError_create.argtypes = [POINTER(McGetTrackingError)]
    MC_GetTrackingError_create.restype = c_int
    
    MC_GetTrackingError_exec = zmc_dll.MC_GetTrackingError_exec
    MC_GetTrackingError_exec.argtypes = [POINTER(McGetTrackingError)]
    MC_GetTrackingError_exec.restype = c_int
    
    MC_GetTrackingError_destroy = zmc_dll.MC_GetTrackingError_destroy
    MC_GetTrackingError_destroy.argtypes = [POINTER(McGetTrackingError)]
    MC_GetTrackingError_destroy.restype = c_int

    # McGetTravelledDistance 函数原型
    MC_GetTravelledDistance_create = zmc_dll.MC_GetTravelledDistance_create
    MC_GetTravelledDistance_create.argtypes = [POINTER(McGetTravelledDistance)]
    MC_GetTravelledDistance_create.restype = c_int
    
    MC_GetTravelledDistance_exec = zmc_dll.MC_GetTravelledDistance_exec
    MC_GetTravelledDistance_exec.argtypes = [POINTER(McGetTravelledDistance)]
    MC_GetTravelledDistance_exec.restype = c_int
    
    MC_GetTravelledDistance_destroy = zmc_dll.MC_GetTravelledDistance_destroy
    MC_GetTravelledDistance_destroy.argtypes = [POINTER(McGetTravelledDistance)]
    MC_GetTravelledDistance_destroy.restype = c_int

    # McGetMaxSetAccDec 函数原型
    MC_GetMaxSetAccDec_create = zmc_dll.MC_GetMaxSetAccDec_create
    MC_GetMaxSetAccDec_create.argtypes = [POINTER(McGetMaxSetAccDec)]
    MC_GetMaxSetAccDec_create.restype = c_int
    
    MC_GetMaxSetAccDec_exec = zmc_dll.MC_GetMaxSetAccDec_exec
    MC_GetMaxSetAccDec_exec.argtypes = [POINTER(McGetMaxSetAccDec)]
    MC_GetMaxSetAccDec_exec.restype = c_int
    
    MC_GetMaxSetAccDec_destroy = zmc_dll.MC_GetMaxSetAccDec_destroy
    MC_GetMaxSetAccDec_destroy.argtypes = [POINTER(McGetMaxSetAccDec)]
    MC_GetMaxSetAccDec_destroy.restype = c_int

    # McGetMaxSetVelocity 函数原型
    MC_GetMaxSetVelocity_create = zmc_dll.MC_GetMaxSetVelocity_create
    MC_GetMaxSetVelocity_create.argtypes = [POINTER(McGetMaxSetVelocity)]
    MC_GetMaxSetVelocity_create.restype = c_int
    
    MC_GetMaxSetVelocity_exec = zmc_dll.MC_GetMaxSetVelocity_exec
    MC_GetMaxSetVelocity_exec.argtypes = [POINTER(McGetMaxSetVelocity)]
    MC_GetMaxSetVelocity_exec.restype = c_int
    
    MC_GetMaxSetVelocity_destroy = zmc_dll.MC_GetMaxSetVelocity_destroy
    MC_GetMaxSetVelocity_destroy.argtypes = [POINTER(McGetMaxSetVelocity)]
    MC_GetMaxSetVelocity_destroy.restype = c_int

    # McGetOverride 函数原型
    MC_GetOverride_create = zmc_dll.MC_GetOverride_create
    MC_GetOverride_create.argtypes = [POINTER(McGetOverride)]
    MC_GetOverride_create.restype = c_int
    
    MC_GetOverride_exec = zmc_dll.MC_GetOverride_exec
    MC_GetOverride_exec.argtypes = [POINTER(McGetOverride)]
    MC_GetOverride_exec.restype = c_int
    
    MC_GetOverride_destroy = zmc_dll.MC_GetOverride_destroy
    MC_GetOverride_destroy.argtypes = [POINTER(McGetOverride)]
    MC_GetOverride_destroy.restype = c_int

    # McSetOverride 函数原型
    MC_SetOverride_create = zmc_dll.MC_SetOverride_create
    MC_SetOverride_create.argtypes = [POINTER(McSetOverride)]
    MC_SetOverride_create.restype = c_int
    
    MC_SetOverride_exec = zmc_dll.MC_SetOverride_exec
    MC_SetOverride_exec.argtypes = [POINTER(McSetOverride)]
    MC_SetOverride_exec.restype = c_int
    
    MC_SetOverride_destroy = zmc_dll.MC_SetOverride_destroy
    MC_SetOverride_destroy.argtypes = [POINTER(McSetOverride)]
    MC_SetOverride_destroy.restype = c_int

    # McSetMovementType 函数原型
    MC_SetMovementType_create = zmc_dll.MC_SetMovementType_create
    MC_SetMovementType_create.argtypes = [POINTER(McSetMovementType)]
    MC_SetMovementType_create.restype = c_int
    
    MC_SetMovementType_exec = zmc_dll.MC_SetMovementType_exec
    MC_SetMovementType_exec.argtypes = [POINTER(McSetMovementType)]
    MC_SetMovementType_exec.restype = c_int
    
    MC_SetMovementType_destroy = zmc_dll.MC_SetMovementType_destroy
    MC_SetMovementType_destroy.argtypes = [POINTER(McSetMovementType)]
    MC_SetMovementType_destroy.restype = c_int

    # McSetRampType 函数原型
    MC_SetRampType_create = zmc_dll.MC_SetRampType_create
    MC_SetRampType_create.argtypes = [POINTER(McSetRampType)]
    MC_SetRampType_create.restype = c_int
    
    MC_SetRampType_exec = zmc_dll.MC_SetRampType_exec
    MC_SetRampType_exec.argtypes = [POINTER(McSetRampType)]
    MC_SetRampType_exec.restype = c_int
    
    MC_SetRampType_destroy = zmc_dll.MC_SetRampType_destroy
    MC_SetRampType_destroy.argtypes = [POINTER(McSetRampType)]
    MC_SetRampType_destroy.restype = c_int

    # McChangeDynamicLimits 函数原型
    MC_ChangeDynamicLimits_create = zmc_dll.MC_ChangeDynamicLimits_create
    MC_ChangeDynamicLimits_create.argtypes = [POINTER(McChangeDynamicLimits)]
    MC_ChangeDynamicLimits_create.restype = c_int
    
    MC_ChangeDynamicLimits_exec = zmc_dll.MC_ChangeDynamicLimits_exec
    MC_ChangeDynamicLimits_exec.argtypes = [POINTER(McChangeDynamicLimits)]
    MC_ChangeDynamicLimits_exec.restype = c_int
    
    MC_ChangeDynamicLimits_destroy = zmc_dll.MC_ChangeDynamicLimits_destroy
    MC_ChangeDynamicLimits_destroy.argtypes = [POINTER(McChangeDynamicLimits)]
    MC_ChangeDynamicLimits_destroy.restype = c_int

    # McChangeGearingRatio 函数原型
    MC_ChangeGearingRatio_create = zmc_dll.MC_ChangeGearingRatio_create
    MC_ChangeGearingRatio_create.argtypes = [POINTER(McChangeGearingRatio)]
    MC_ChangeGearingRatio_create.restype = c_int
    
    MC_ChangeGearingRatio_exec = zmc_dll.MC_ChangeGearingRatio_exec
    MC_ChangeGearingRatio_exec.argtypes = [POINTER(McChangeGearingRatio)]
    MC_ChangeGearingRatio_exec.restype = c_int
    
    MC_ChangeGearingRatio_destroy = zmc_dll.MC_ChangeGearingRatio_destroy
    MC_ChangeGearingRatio_destroy.argtypes = [POINTER(McChangeGearingRatio)]
    MC_ChangeGearingRatio_destroy.restype = c_int

    # McSetSoftwareLimits 函数原型
    MC_SetSoftwareLimits_create = zmc_dll.MC_SetSoftwareLimits_create
    MC_SetSoftwareLimits_create.argtypes = [POINTER(McSetSoftwareLimits)]
    MC_SetSoftwareLimits_create.restype = c_int
    
    MC_SetSoftwareLimits_exec = zmc_dll.MC_SetSoftwareLimits_exec
    MC_SetSoftwareLimits_exec.argtypes = [POINTER(McSetSoftwareLimits)]
    MC_SetSoftwareLimits_exec.restype = c_int
    
    MC_SetSoftwareLimits_destroy = zmc_dll.MC_SetSoftwareLimits_destroy
    MC_SetSoftwareLimits_destroy.argtypes = [POINTER(McSetSoftwareLimits)]
    MC_SetSoftwareLimits_destroy.restype = c_int

    # McSetControllerMode 函数原型
    MC_SetControllerMode_create = zmc_dll.MC_SetControllerMode_create
    MC_SetControllerMode_create.argtypes = [POINTER(McSetControllerMode)]
    MC_SetControllerMode_create.restype = c_int
    
    MC_SetControllerMode_exec = zmc_dll.MC_SetControllerMode_exec
    MC_SetControllerMode_exec.argtypes = [POINTER(McSetControllerMode)]
    MC_SetControllerMode_exec.restype = c_int
    
    MC_SetControllerMode_destroy = zmc_dll.MC_SetControllerMode_destroy
    MC_SetControllerMode_destroy.argtypes = [POINTER(McSetControllerMode)]
    MC_SetControllerMode_destroy.restype = c_int

    # McInch 函数原型
    MC_Inch_create = zmc_dll.MC_Inch_create
    MC_Inch_create.argtypes = [POINTER(McInch)]
    MC_Inch_create.restype = c_int
    
    MC_Inch_exec = zmc_dll.MC_Inch_exec
    MC_Inch_exec.argtypes = [POINTER(McInch)]
    MC_Inch_exec.restype = c_int
    
    MC_Inch_destroy = zmc_dll.MC_Inch_destroy
    MC_Inch_destroy.argtypes = [POINTER(McInch)]
    MC_Inch_destroy.restype = c_int

    # cam_ref_get_pva 函数原型
    cam_ref_get_pva = zmc_dll.cam_ref_get_pva
    cam_ref_get_pva.argtypes = [MC_CAM_REF, c_double, POINTER(CamXyva)]
    cam_ref_get_pva.restype = c_int

    # cam_ref_get_pva_affine 函数原型
    cam_ref_get_pva_affine = zmc_dll.cam_ref_get_pva_affine
    cam_ref_get_pva_affine.argtypes = [MC_CAM_REF, POINTER(PosVelAcc), POINTER(PosVelAcc), POINTER(CamAffineCoefficient)]
    cam_ref_get_pva_affine.restype = c_int

    # cam_affine_from_cam_rel 函数原型
    cam_affine_from_cam_rel = zmc_dll.cam_affine_from_cam_rel
    cam_affine_from_cam_rel.argtypes = [POINTER(CamAffineCoefficient), POINTER(CamAffineFullParam)]
    cam_affine_from_cam_rel.restype = None

    # mc_vel_api
    mc_vel_new = zmc_dll.mc_vel_new
    mc_vel_new.argtypes = []
    mc_vel_new.restype = c_void_p
    mc_vel_delete = zmc_dll.mc_vel_delete
    mc_vel_delete.argtypes = [c_void_p]
    mc_vel_delete.restype = None
    mc_vel_set_config = zmc_dll.mc_vel_set_config
    mc_vel_set_config.argtypes = [c_void_p, c_double, c_double, c_double, c_double]
    mc_vel_set_config.restype = None
    mc_vel_set_begin_pva = zmc_dll.mc_vel_set_begin_pva
    mc_vel_set_begin_pva.argtypes = [c_void_p, c_double, c_double, c_double]
    mc_vel_set_begin_pva.restype = None
    mc_vel_set_end_pva = zmc_dll.mc_vel_set_end_pva
    mc_vel_set_end_pva.argtypes = [c_void_p, c_double, c_double, c_double]
    mc_vel_set_end_pva.restype = None
    mc_vel_build = zmc_dll.mc_vel_build
    mc_vel_build.argtypes = [c_void_p]
    mc_vel_build.restype = c_int
    mc_vel_get_end_time = zmc_dll.mc_vel_get_end_time
    mc_vel_get_end_time.argtypes = [c_void_p]
    mc_vel_get_end_time.restype = c_double
    mc_vel_get_slice = zmc_dll.mc_vel_get_slice
    mc_vel_get_slice.argtypes = [c_void_p, c_double, POINTER(c_double), POINTER(c_double), POINTER(c_double)]
    mc_vel_get_slice.restype = c_int

    # mc_traj_api
    mc_traj_new = zmc_dll.mc_traj_new
    mc_traj_new.argtypes = []
    mc_traj_new.restype = c_void_p
    mc_traj_delete = zmc_dll.mc_traj_delete
    mc_traj_delete.argtypes = [c_void_p]
    mc_traj_delete.restype = None
    mc_traj_get_axis_num = zmc_dll.mc_traj_get_axis_num
    mc_traj_get_axis_num.argtypes = [c_void_p]
    mc_traj_get_axis_num.restype = c_uint8
    mc_traj_get_traj_type = zmc_dll.mc_traj_get_traj_type
    mc_traj_get_traj_type.argtypes = [c_void_p]
    mc_traj_get_traj_type.restype = c_uint8
    mc_traj_init_axis_num = zmc_dll.mc_traj_init_axis_num
    mc_traj_init_axis_num.argtypes = [c_void_p, c_uint8]
    mc_traj_init_axis_num.restype = None
    mc_traj_set_begin_pos = zmc_dll.mc_traj_set_begin_pos
    mc_traj_set_begin_pos.argtypes = [c_void_p, POINTER(c_double)]
    mc_traj_set_begin_pos.restype = None
    mc_traj_set_sg_param = zmc_dll.mc_traj_set_sg_param
    mc_traj_set_sg_param.argtypes = [c_void_p, c_double, c_bool, c_bool]
    mc_traj_set_sg_param.restype = None
    mc_traj_set_line_param = zmc_dll.mc_traj_set_line_param
    mc_traj_set_line_param.argtypes = [c_void_p, POINTER(c_double), c_bool]
    mc_traj_set_line_param.restype = None
    mc_traj_set_circ_param = zmc_dll.mc_traj_set_circ_param
    mc_traj_set_circ_param.argtypes = [c_void_p, POINTER(c_double), POINTER(c_double), c_bool, c_bool, c_int]
    mc_traj_set_circ_param.restype = None
    mc_traj_build = zmc_dll.mc_traj_build
    mc_traj_build.argtypes = [c_void_p]
    mc_traj_build.restype = c_int
    mc_traj_get_begin_pos = zmc_dll.mc_traj_get_begin_pos
    mc_traj_get_begin_pos.argtypes = [c_void_p, POINTER(c_double)]
    mc_traj_get_begin_pos.restype = None
    mc_traj_get_end_pos = zmc_dll.mc_traj_get_end_pos
    mc_traj_get_end_pos.argtypes = [c_void_p, POINTER(c_double)]
    mc_traj_get_end_pos.restype = None
    mc_traj_get_plan = zmc_dll.mc_traj_get_plan
    mc_traj_get_plan.argtypes = [c_void_p]
    mc_traj_get_plan.restype = c_double
    mc_traj_get_slice = zmc_dll.mc_traj_get_slice
    mc_traj_get_slice.argtypes = [c_void_p, c_double, POINTER(c_double), POINTER(c_double)]
    mc_traj_get_slice.restype = None
    mc_traj_get_slice2 = zmc_dll.mc_traj_get_slice2
    mc_traj_get_slice2.argtypes = [c_void_p, c_double, c_double, POINTER(c_double), POINTER(c_double)]
    mc_traj_get_slice2.restype = None
    mc_traj_get_slice3 = zmc_dll.mc_traj_get_slice3
    mc_traj_get_slice3.argtypes = [c_void_p, c_double, c_double, c_double,
                                   POINTER(c_double), POINTER(c_double), POINTER(c_double)]
    mc_traj_get_slice3.restype = None


class AxFbBridge:
    """Python wrapper for the Axis FB bridge C handle API."""

    def __init__(self):
        if not zmc_dll:
            raise RuntimeError("DLL not loaded. Cannot create AxFbBridge.")

        handle = axfb_bridge_handle_t()
        ret = axfb_bridge_create(byref(handle))
        if ret != 0 or not handle.value:
            raise RuntimeError(f"axfb_bridge_create failed with error: {ret}")
        self._handle = handle

    @staticmethod
    def make_packet(seq, payload):
        """Build an AxFbBridgePacket from a bytes-like payload."""
        payload_bytes = bytes(payload)
        if len(payload_bytes) > AXFB_BRIDGE_PACKET_PAYLOAD_SIZE:
            raise ValueError(
                f"payload length {len(payload_bytes)} exceeds "
                f"{AXFB_BRIDGE_PACKET_PAYLOAD_SIZE}"
            )

        packet = AxFbBridgePacket()
        packet.seq = int(seq)
        packet.payload_len = len(payload_bytes)
        for idx, value in enumerate(payload_bytes):
            packet.payload[idx] = value
        return packet

    @staticmethod
    def _as_c_string(value):
        """Convert Python strings to UTF-8 bytes for char* API arguments."""
        if value is None:
            return None
        if isinstance(value, bytes):
            return value
        return str(value).encode("utf-8")

    @staticmethod
    def _as_void_p(value):
        """Accept ctypes pointers, arrays, and c_void_p for void* API arguments."""
        if value is None:
            return None
        if isinstance(value, c_void_p):
            return value
        if isinstance(value, int):
            return c_void_p(value)
        return cast(value, c_void_p)

    @staticmethod
    def _as_adapter_pointer(adapter):
        """Keep adapter callback ownership with the caller."""
        if adapter is None:
            return None
        if isinstance(adapter, SdoCAdapter):
            return byref(adapter)
        return adapter

    @property
    def handle(self):
        return self._handle

    def close(self):
        if getattr(self, "_handle", None):
            axfb_bridge_destroy(self._handle)
            self._handle = None

    def configure(self, ini_path="", ini_content="", axis_num=0):
        return axfb_bridge_configure(
            self._handle,
            self._as_c_string(ini_path),
            self._as_c_string(ini_content),
            c_uint(axis_num)
        )

    def bind_sdo_adapter(self, adapter):
        return axfb_bridge_bind_sdo_adapter(self._handle, self._as_adapter_pointer(adapter))

    def native_sdo(self):
        return axfb_bridge_native_sdo(self._handle)

    def start_sdo(self):
        return axfb_bridge_start_sdo(self._handle)

    def stop_sdo(self):
        return axfb_bridge_stop_sdo(self._handle)

    def register_opmode_watches(self, opmode):
        return axfb_bridge_register_opmode_watches(self._handle, c_uint8(opmode))

    def apply_pio_layout(self, layout):
        return axfb_bridge_apply_pio_layout(self._handle, self._as_void_p(layout))

    def servo_count(self):
        out = c_int()
        ret = axfb_bridge_servo_count(self._handle, byref(out))
        return ret, out.value

    def pdo_layout_ready(self):
        out = c_int()
        ret = axfb_bridge_pdo_layout_ready(self._handle, byref(out))
        return ret, bool(out.value)

    def auto_bind(self, bind_begin, servo_num):
        return axfb_bridge_auto_bind(self._handle, int(bind_begin), int(servo_num))

    def manual_bind(self, axis_count, bind_axis_arr, servo_num):
        arr = None
        ptr = None
        if axis_count > 0:
            if isinstance(bind_axis_arr, (list, tuple)):
                if len(bind_axis_arr) < axis_count:
                    raise ValueError("bind_axis_arr length is shorter than axis_count")
                arr = (c_int * axis_count)(*bind_axis_arr)
                ptr = arr
            else:
                ptr = bind_axis_arr
        return axfb_bridge_manual_bind(self._handle, int(axis_count), ptr, int(servo_num))

    def run_cycle(self, inp, oup):
        return axfb_bridge_run_cycle(
            self._handle,
            self._as_void_p(inp),
            self._as_void_p(oup)
        )

    def run_cycle_parsed(self, proc_img_inp, proc_img_oup):
        return axfb_bridge_run_cycle_parsed(
            self._handle,
            self._as_void_p(proc_img_inp),
            self._as_void_p(proc_img_oup)
        )

    def axis_count(self):
        out = c_int()
        ret = axfb_bridge_axis_count(self._handle, byref(out))
        return ret, out.value

    def get_address(self, axis_idx):
        out = c_int()
        ret = axfb_bridge_get_address(self._handle, int(axis_idx), byref(out))
        return ret, out.value

    def get_actual_position(self, axis_idx):
        out = c_int()
        ret = axfb_bridge_get_actual_position(self._handle, int(axis_idx), byref(out))
        return ret, out.value

    def push_cmd_packet(self, packet_or_payload, seq=0):
        if isinstance(packet_or_payload, AxFbBridgePacket):
            packet = packet_or_payload
        else:
            packet = self.make_packet(seq, packet_or_payload)
        return axfb_bridge_push_cmd_packet(self._handle, byref(packet))

    def pop_status_packet(self):
        packet = AxFbBridgePacket()
        ret = axfb_bridge_pop_status_packet(self._handle, byref(packet))
        return ret, packet

    def consume_exit_flag(self):
        return axfb_bridge_consume_exit_flag(self._handle)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()
        return False

    def __del__(self):
        self.close()


# 封装 Axis 类
class McVelWrap:
    def __init__(self):
        self.handle = mc_vel_new()
        if not self.handle:
            raise RuntimeError("Failed to create mc_vel instance")

    def set_config(self, vmax, acc, dec, jerk):
        mc_vel_set_config(self.handle, vmax, acc, dec, jerk)

    def set_begin_pva(self, s0, v0, a0=0.0):
        mc_vel_set_begin_pva(self.handle, s0, v0, a0)

    def set_end_pva(self, sd, vd, ad=0.0):
        mc_vel_set_end_pva(self.handle, sd, vd, ad)

    def build(self):
        return mc_vel_build(self.handle)

    def get_end_time(self):
        return mc_vel_get_end_time(self.handle)

    def get_slice(self, t):
        pos = c_double()
        vel = c_double()
        acc = c_double()
        ret = mc_vel_get_slice(self.handle, t, byref(pos), byref(vel), byref(acc))
        return ret, pos.value, vel.value, acc.value

    def __del__(self):
        if hasattr(self, "handle") and self.handle:
            mc_vel_delete(self.handle)
            self.handle = None


class McTrajWrap:
    def __init__(self):
        self.handle = mc_traj_new()
        if not self.handle:
            raise RuntimeError("Failed to create mc_traj instance")

    @staticmethod
    def _to_double_array0(values):
        """Convert a Python sequence of numbers to a C double array using mc_* helpers."""
        n = len(values)
        arr = mc_new_doubleArray(n)
        for i, v in enumerate(values):
            mc_doubleArray_setitem(arr, i, float(v))
        return arr

    @staticmethod
    def _to_double_array(values):
        """Convert a Python sequence of numbers to a C double array using mc_* helpers."""
        n = len(values)
        arr = (c_double *n)()
        for i, v in enumerate(values):
            arr[i] = float(v)
        return arr
    
    def get_axis_num(self):
        return mc_traj_get_axis_num(self.handle)

    def get_traj_type(self):
        return mc_traj_get_traj_type(self.handle)

    def init_axis_num(self, axis_num: int):
        mc_traj_init_axis_num(self.handle, c_uint8(axis_num))

    def set_begin_pos(self, pos):
        arr = self._to_double_array(pos)
        mc_traj_set_begin_pos(self.handle, arr)

    def set_sg_param(self, pos_or_dist, is_relative, no_end=False):
        mc_traj_set_sg_param(self.handle, pos_or_dist, is_relative, no_end)

    def set_line_param(self, pos_or_diff_arr, is_relative):
        arr = self._to_double_array(pos_or_diff_arr)
        mc_traj_set_line_param(self.handle, arr, is_relative)

    def set_circ_param(self, aux_point, end_point, path_choice, is_relative, mode):
        aux = self._to_double_array(aux_point)
        end = self._to_double_array(end_point)
        mc_traj_set_circ_param(self.handle, aux, end, path_choice, is_relative, mode)

    def build(self):
        return mc_traj_build(self.handle)

    def get_plan(self):
        return mc_traj_get_plan(self.handle)

    def get_begin_pos(self, axis_num):
        out = (c_double * axis_num)()
        mc_traj_get_begin_pos(self.handle, out)
        return [out[i] for i in range(axis_num)]

    def get_end_pos(self, axis_num):
        out = (c_double * axis_num)()
        mc_traj_get_end_pos(self.handle, out)
        return [out[i] for i in range(axis_num)]

    def get_slice(self, d, axis_num):
        abs_pos = (c_double * axis_num)()
        dif_pos = (c_double * axis_num)()
        mc_traj_get_slice(self.handle, d, abs_pos, dif_pos)
        return [abs_pos[i] for i in range(axis_num)], [dif_pos[i] for i in range(axis_num)]

    def get_slice2(self, d, v, axis_num):
        abs_pos = (c_double * axis_num)()
        dif_pos = (c_double * axis_num)()
        mc_traj_get_slice2(self.handle, d, v, abs_pos, dif_pos)
        return [abs_pos[i] for i in range(axis_num)], [dif_pos[i] for i in range(axis_num)]

    def get_slice3(self, d, v, a, axis_num):
        abs_pos = (c_double * axis_num)()
        dif_pos = (c_double * axis_num)()
        dif2_pos = (c_double * axis_num)()
        mc_traj_get_slice3(self.handle, d, v, a, abs_pos, dif_pos, dif2_pos)
        return (
            [abs_pos[i] for i in range(axis_num)],
            [dif_pos[i] for i in range(axis_num)],
            [dif2_pos[i] for i in range(axis_num)],
        )

    def __del__(self):
        if hasattr(self, "handle") and self.handle:
            mc_traj_delete(self.handle)
            self.handle = None


class FbAxisWrap:
    def __init__(self):
        self.axis_ref = axis_virtual_new()
        if not self.axis_ref:
            raise RuntimeError("Failed to create Axis instance")
        # Axis_init(self.axis_ref)
    
    def set_config(self, config):
        """设置轴配置"""
        axis_virtual_set_config(self.axis_ref, config)
    
    # def set_config2(self, config):
    #     """设置轴配置"""
    #     axis_virtual_set_config(self.axis_ref, config)
    def run_cycle(self):
        """运行轴周期"""
        axis_virtual_runcycle(self.axis_ref)
    def print(self, level):
        axis_virtual_print(self.axis_ref, level)

    def __del__(self):
        axis_virtual_delete(self.axis_ref)
        # print("axis_virtual_delete")
    
    @property
    def ref(self):
        """获取内部引用指针"""
        return self.axis_ref

class FbPower(McPower):
    def __init__(self):
        super().__init__()
        ret = MC_Power_create(byref(self))
        if ret != 0:
            print(f"MC_Power_create failed with error: {ret}")

    def runCycle(self):
        return MC_Power_exec(byref(self))
    
    def __del__(self):
        ret = MC_Power_destroy(byref(self))
        if ret != 0:
            print(f"MC_Power_destroy failed with error: {ret}")

class FbMoveAbsolute(McMoveAbsolute):
    """MoveAbsolute功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_MoveAbsolute_create(byref(self))
        if ret != 0:
            print(f"MC_MoveAbsolute_create failed with error: {ret}")

    def runCycle(self):
        return MC_MoveAbsolute_exec(byref(self))
    
    def set_move_params(self, position, velocity, acceleration, deceleration, jerk, 
                       direction=Direction.mcShortestWay, buffer_mode=BufferMode.mcAborting):
        """设置运动参数"""
        self.Position = position
        self.Velocity = velocity
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
        self.Direction = direction
        self.BufferMode = buffer_mode
    
    def __del__(self):
        ret = MC_MoveAbsolute_destroy(byref(self))
        if ret != 0:
            print(f"MC_MoveAbsolute_destroy failed with error: {ret}")

class FbMoveRelative(McMoveRelative):
    """MoveRelative功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_MoveRelative_create(byref(self))
        if ret != 0:
            print(f"MC_MoveRelative_create failed with error: {ret}")

    def runCycle(self):
        return MC_MoveRelative_exec(byref(self))
    
    def set_move_params(self, distance, velocity, acceleration, deceleration, jerk, 
                       buffer_mode=BufferMode.mcAborting):
        """设置运动参数"""
        self.Distance = distance
        self.Velocity = velocity
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
        self.BufferMode = buffer_mode
    
    def __del__(self):
        ret = MC_MoveRelative_destroy(byref(self))
        if ret != 0:
            print(f"MC_MoveRelative_destroy failed with error: {ret}")

class FbMoveAdditive(McMoveAdditive):
    """MoveAdditive功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_MoveAdditive_create(byref(self))
        if ret != 0:
            print(f"MC_MoveAdditive_create failed with error: {ret}")

    def runCycle(self):
        return MC_MoveAdditive_exec(byref(self))
    
    def set_move_params(self, distance, velocity, acceleration, deceleration, jerk):
        """设置运动参数"""
        self.Distance = distance
        self.Velocity = velocity
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
    
    def __del__(self):
        ret = MC_MoveAdditive_destroy(byref(self))
        if ret != 0:
            print(f"MC_MoveAdditive_destroy failed with error: {ret}")

class FbStop(McStop):
    """Stop功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_Stop_create(byref(self))
        if ret != 0:
            print(f"MC_Stop_create failed with error: {ret}")

    def runCycle(self):
        return MC_Stop_exec(byref(self))
    
    def set_stop_params(self, deceleration, jerk):
        """设置停止参数"""
        self.Deceleration = deceleration
        self.Jerk = jerk
    
    def __del__(self):
        ret = MC_Stop_destroy(byref(self))
        if ret != 0:
            print(f"MC_Stop_destroy failed with error: {ret}")

class FbHalt(McHalt):
    """Halt功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_Halt_create(byref(self))
        if ret != 0:
            print(f"MC_Halt_create failed with error: {ret}")

    def runCycle(self):
        return MC_Halt_exec(byref(self))
    
    def set_halt_params(self, deceleration, jerk):
        """设置暂停参数"""
        self.Deceleration = deceleration
        self.Jerk = jerk
    
    def __del__(self):
        ret = MC_Halt_destroy(byref(self))
        if ret != 0:
            print(f"MC_Halt_destroy failed with error: {ret}")

class FbMoveVelocity(McMoveVelocity):
    """MoveVelocity功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_MoveVelocity_create(byref(self))
        if ret != 0:
            print(f"MC_MoveVelocity_create failed with error: {ret}")

    def runCycle(self):
        return MC_MoveVelocity_exec(byref(self))
    
    def set_move_params(self, velocity, acceleration, deceleration, jerk, 
                       direction=Direction.mcPositiveDirection, buffer_mode=BufferMode.mcAborting):
        """设置运动参数"""
        self.Velocity = velocity
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
        self.Direction = direction
        self.BufferMode = buffer_mode
    
    def __del__(self):
        ret = MC_MoveVelocity_destroy(byref(self))
        if ret != 0:
            print(f"MC_MoveVelocity_destroy failed with error: {ret}")

class AxisCiaConfigWrapper:
    """AxisCiaConfig封装类"""
    
    def __init__(self):
        self.config_ptr = axis_config_new()
        if not self.config_ptr:
            raise RuntimeError("Failed to create AxisCiaConfig instance")
        # 初始化默认配置
        axis_config_default_init(self.config_ptr)
    
    def check_config(self):
        """检查配置是否有效"""
        return axis_config_check(self.config_ptr)
    
    def set_dynamic_limits(self, velocity, acceleration, deceleration, jerk):
        """设置动态限制"""
        self.config_ptr.contents.dynamic_limits.velocity = velocity
        self.config_ptr.contents.dynamic_limits.acceleration = acceleration
        self.config_ptr.contents.dynamic_limits.deceleration = deceleration
        self.config_ptr.contents.dynamic_limits.jerk = jerk
    
    def set_id(self, axis_id:int):
        """设置轴ID"""
        self.config_ptr.contents.id = axis_id
    def set_software_limits(self, is_activate, negative, positive):
        """设置软件限位"""
        self.config_ptr.contents.software_limits_is_activate = is_activate
        self.config_ptr.contents.software_limits_negative = negative
        self.config_ptr.contents.software_limits_positive = positive
    
    def set_axis_type(self, axis_type, modulo_value=1e11):
        """设置轴类型"""
        self.config_ptr.contents.axis_type = axis_type
        self.config_ptr.contents.modulo_value = modulo_value
    
    def set_motor_type(self, motor_type, is_invert_direction=False):
        """设置电机类型"""
        self.config_ptr.contents.motor_type = motor_type
        self.config_ptr.contents.is_invert_direction = is_invert_direction
    
    def set_linear_params(self, num, den):
        """设置线性参数"""
        self.config_ptr.contents.linear.increments_units_in_application_num = num
        self.config_ptr.contents.linear.increments_units_in_application_den = den
    
    def set_rotary_params(self, gear_num, gear_den, turns_num, turns_den, motor_gear_num, motor_gear_den):
        """设置旋转参数"""
        self.config_ptr.contents.rotary_gear_application_num = gear_num
        self.config_ptr.contents.rotary_gear_application_den = gear_den
        self.config_ptr.contents.rotary_increments_motor_turns_num = turns_num
        self.config_ptr.contents.rotary_increments_motor_turns_den = turns_den
        self.config_ptr.contents.rotary_motor_gear_num = motor_gear_num
        self.config_ptr.contents.rotary_motor_gear_den = motor_gear_den
    
    def set_cycle_time(self, cycle_time_us):
        """设置周期时间（微秒）"""
        self.config_ptr.contents.cycle_time = cycle_time_us
    
    def set_virtual_mode(self, is_virtual):
        """设置虚拟模式"""
        self.config_ptr.contents.is_axis_virtual = is_virtual
    
    def get_config_ptr(self):
        """获取配置指针"""
        return self.config_ptr
    
    def __del__(self):
        axis_config_delete(self.config_ptr)

# ========== Python 封装类 ==========
class FbCamTableSelect(McCamTableSelect):
    def __init__(self):
        super().__init__()
        ret = MC_CamTableSelect_create(byref(self))
        if ret != 0:
            print(f"MC_CamTableSelect_create failed: {ret}")
    def runCycle(self):
        return MC_CamTableSelect_exec(byref(self))
    def __del__(self):
        MC_CamTableSelect_destroy(byref(self))

class FbReadCAM(McReadCAM):
    def __init__(self):
        super().__init__()
        ret = MC_ReadCAM_create(byref(self))
        if ret != 0:
            print(f"MC_ReadCAM_create failed: {ret}")
    def runCycle(self):
        return MC_ReadCAM_exec(byref(self))
    def __del__(self):
        MC_ReadCAM_destroy(byref(self))

class FbWriteCAM(McWriteCAM):
    def __init__(self):
        super().__init__()
        ret = MC_WriteCAM_create(byref(self))
        if ret != 0:
            print(f"MC_WriteCAM_create failed: {ret}")
    def runCycle(self):
        return MC_WriteCAM_exec(byref(self))
    def __del__(self):
        MC_WriteCAM_destroy(byref(self))

class FbCamIn(McCamIn):
    def __init__(self):
        super().__init__()
        ret = MC_CamIn_create(byref(self))
        if ret != 0:
            print(f"MC_CamIn_create failed: {ret}")
    def runCycle(self):
        return MC_CamIn_exec(byref(self))
    def __del__(self):
        MC_CamIn_destroy(byref(self))

class FbGetTappetValue(McGetTappetValue):
    def __init__(self):
        super().__init__()
        ret = MC_GetTappetValue_create(byref(self))
        if ret != 0:
            print(f"MC_GetTappetValue_create failed: {ret}")
        
    def set_tappet(self, tappet:SMC_TappetData):
        # tappet is struct, Tappets is pointer, convert to c pointer
        # return
        """
        FIXME 
        File "pymc.py", line 931, in runCycle
        return MC_GetTappetValue_exec(byref(self))
        OSError: exception: access violation reading 0x000000000000010D
        """
        self.Tappets = pointer(tappet) # cast(pointer(tappet), c_void_p)
        
        
    def set_tappet_from_cam_in(self, cam_in):
        if hasattr(cam_in, 'Tappets'):
            ret = setTappetFromCamIn(byref(self), byref(cam_in))
            if ret == 0:
                pass # print("Successfully set tappet from CamIn")
            else:
                print(f"Failed to set tappet from CamIn, error code: {ret}")
        else:
            print("Error: cam_in object does not have Tappets attribute")
    
    def runCycle(self):
        return MC_GetTappetValue_exec(byref(self))

    def __del__(self):
        MC_GetTappetValue_destroy(byref(self))

# ========== 新增功能块Python封装类 ==========

class FbReset(McReset):
    """Reset功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_Reset_create(byref(self))
        if ret != 0:
            print(f"MC_Reset_create failed with error: {ret}")

    def runCycle(self):
        return MC_Reset_exec(byref(self))
    
    def __del__(self):
        ret = MC_Reset_destroy(byref(self))
        if ret != 0:
            print(f"MC_Reset_destroy failed with error: {ret}")

class FbSetPosition(McSetPosition):
    """SetPosition功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_SetPosition_create(byref(self))
        if ret != 0:
            print(f"MC_SetPosition_create failed with error: {ret}")

    def runCycle(self):
        return MC_SetPosition_exec(byref(self))
    
    def set_position_params(self, position, mode=False):
        """设置位置参数"""
        self.Position = position
        self.Mode = mode
    
    def __del__(self):
        ret = MC_SetPosition_destroy(byref(self))
        if ret != 0:
            print(f"MC_SetPosition_destroy failed with error: {ret}")

class FbReadStatus(McReadStatus):
    """ReadStatus功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadStatus_create(byref(self))
        if ret != 0:
            print(f"MC_ReadStatus_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadStatus_exec(byref(self))
    
    def get_status_summary(self):
        """获取状态摘要"""
        return {
            'Valid': self.Valid,
            'Busy': self.Busy,
            'Error': self.Error,
            'Disabled': self.Disabled,
            'Standstill': self.Standstill,
            'DiscreteMotion': self.DiscreteMotion,
            'ContinuousMotion': self.ContinuousMotion,
            'SynchronizedMotion': self.SynchronizedMotion,
            'Homing': self.Homing,
            'ConstantVelocity': self.ConstantVelocity,
            'Accelerating': self.Accelerating,
            'Decelerating': self.Decelerating
        }
    
    def __del__(self):
        ret = MC_ReadStatus_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadStatus_destroy failed with error: {ret}")

class FbReadActualPosition(McReadActualPosition):
    """ReadActualPosition功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadActualPosition_create(byref(self))
        if ret != 0:
            print(f"MC_ReadActualPosition_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadActualPosition_exec(byref(self))
    
    def __del__(self):
        ret = MC_ReadActualPosition_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadActualPosition_destroy failed with error: {ret}")

class FbReadActualVelocity(McReadActualVelocity):
    """ReadActualVelocity功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadActualVelocity_create(byref(self))
        if ret != 0:
            print(f"MC_ReadActualVelocity_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadActualVelocity_exec(byref(self))
    
    def __del__(self):
        ret = MC_ReadActualVelocity_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadActualVelocity_destroy failed with error: {ret}")

class FbReadAxisError(McReadAxisError):
    """ReadAxisError功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadAxisError_create(byref(self))
        if ret != 0:
            print(f"MC_ReadAxisError_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadAxisError_exec(byref(self))
    
    def __del__(self):
        ret = MC_ReadAxisError_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadAxisError_destroy failed with error: {ret}")

class FbReadParameter(McReadParameter):
    """ReadParameter功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadParameter_create(byref(self))
        if ret != 0:
            print(f"MC_ReadParameter_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadParameter_exec(byref(self))
    
    def set_parameter_number(self, param_number):
        """设置参数号"""
        self.ParameterNumber = param_number
    
    def __del__(self):
        ret = MC_ReadParameter_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadParameter_destroy failed with error: {ret}")

class FbWriteParameter(McWriteParameter):
    """WriteParameter功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_WriteParameter_create(byref(self))
        if ret != 0:
            print(f"MC_WriteParameter_create failed with error: {ret}")

    def runCycle(self):
        return MC_WriteParameter_exec(byref(self))
    
    def set_write_params(self, param_number, value):
        """设置写入参数"""
        self.ParameterNumber = param_number
        self.Value = value
    
    def __del__(self):
        ret = MC_WriteParameter_destroy(byref(self))
        if ret != 0:
            print(f"MC_WriteParameter_destroy failed with error: {ret}")

class FbHome(McHome):
    """Home功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_Home_create(byref(self))
        if ret != 0:
            print(f"MC_Home_create failed with error: {ret}")

    def runCycle(self):
        return MC_Home_exec(byref(self))
    
    def set_home_params(self, position):
        """设置回零参数"""
        self.Position = position
    
    def __del__(self):
        ret = MC_Home_destroy(byref(self))
        if ret != 0:
            print(f"MC_Home_destroy failed with error: {ret}")

class FbHoming(McHoming):
    """Homing功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_Homing_create(byref(self))
        if ret != 0:
            print(f"MC_Homing_create failed with error: {ret}")

    def runCycle(self):
        return MC_Homing_exec(byref(self))
    
    def set_homing_params(self, position, home_position, velocity_slow, velocity_fast, 
                         acceleration, deceleration, jerk, direction):
        """设置回零参数"""
        self.fPosition = position
        self.fHomePosition = home_position
        self.fVelocitySlow = velocity_slow
        self.fVelocityFast = velocity_fast
        self.fAcceleration = acceleration
        self.fDeceleration = deceleration
        self.fJerk = jerk
        self.nDirection = direction
    
    def __del__(self):
        ret = MC_Homing_destroy(byref(self))
        if ret != 0:
            print(f"MC_Homing_destroy failed with error: {ret}")

class FbJog(McJog):
    """Jog功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_Jog_create(byref(self))
        if ret != 0:
            print(f"MC_Jog_create failed with error: {ret}")

    def runCycle(self):
        return MC_Jog_exec(byref(self))
    
    def set_jog_params(self, velocity, acceleration, deceleration, jerk):
        """设置点动参数"""
        self.Velocity = velocity
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
    
    def jog_forward(self):
        """正向点动"""
        self.JogForward = True
        self.JogBackward = False
    
    def jog_backward(self):
        """负向点动"""
        self.JogForward = False
        self.JogBackward = True
    
    def stop_jog(self):
        """停止点动"""
        self.JogForward = False
        self.JogBackward = False
    
    def __del__(self):
        ret = MC_Jog_destroy(byref(self))
        if ret != 0:
            print(f"MC_Jog_destroy failed with error: {ret}")

class FbGearIn(McGearIn):
    """GearIn功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_GearIn_create(byref(self))
        if ret != 0:
            print(f"MC_GearIn_create failed with error: {ret}")

    def runCycle(self):
        return MC_GearIn_exec(byref(self))
    
    def set_gear_params(self, ratio_numerator, ratio_denominator, acceleration, 
                       deceleration, jerk, buffer_mode=BufferMode.mcAborting):
        """设置齿轮同步参数"""
        self.RatioNumerator = ratio_numerator
        self.RatioDenominator = ratio_denominator
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
        self.BufferMode = buffer_mode
    
    def __del__(self):
        ret = MC_GearIn_destroy(byref(self))
        if ret != 0:
            print(f"MC_GearIn_destroy failed with error: {ret}")

class FbGearOut(McGearOut):
    """GearOut功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_GearOut_create(byref(self))
        if ret != 0:
            print(f"MC_GearOut_create failed with error: {ret}")

    def runCycle(self):
        return MC_GearOut_exec(byref(self))
    
    def __del__(self):
        ret = MC_GearOut_destroy(byref(self))
        if ret != 0:
            print(f"MC_GearOut_destroy failed with error: {ret}")

class FbPhasing(McPhasing):
    """Phasing功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_Phasing_create(byref(self))
        if ret != 0:
            print(f"MC_Phasing_create failed with error: {ret}")

    def runCycle(self):
        return MC_Phasing_exec(byref(self))
    
    def set_phasing_params(self, phase_shift, velocity, acceleration, deceleration, jerk):
        """设置相位同步参数"""
        self.PhaseShift = phase_shift
        self.Velocity = velocity
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
    
    def __del__(self):
        ret = MC_Phasing_destroy(byref(self))
        if ret != 0:
            print(f"MC_Phasing_destroy failed with error: {ret}")

class FbSetTorque(McSetTorque):
    """SetTorque功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_SetTorque_create(byref(self))
        if ret != 0:
            print(f"MC_SetTorque_create failed with error: {ret}")

    def runCycle(self):
        return MC_SetTorque_exec(byref(self))
    
    def set_torque_params(self, enable, torque):
        """设置转矩参数"""
        self.bEnable = enable
        self.fTorque = torque
    
    def __del__(self):
        ret = MC_SetTorque_destroy(byref(self))
        if ret != 0:
            print(f"MC_SetTorque_destroy failed with error: {ret}")

class FbFollowPosition(McFollowPosition):
    """FollowPosition功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_FollowPosition_create(byref(self))
        if ret != 0:
            print(f"MC_FollowPosition_create failed with error: {ret}")

    def runCycle(self):
        return MC_FollowPosition_exec(byref(self))
    
    def set_follow_params(self, position):
        """设置跟随参数"""
        self.fSetPosition = position
    
    def __del__(self):
        ret = MC_FollowPosition_destroy(byref(self))
        if ret != 0:
            print(f"MC_FollowPosition_destroy failed with error: {ret}")

class FbFollowVelocity(McFollowVelocity):
    """FollowVelocity功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_FollowVelocity_create(byref(self))
        if ret != 0:
            print(f"MC_FollowVelocity_create failed with error: {ret}")

    def runCycle(self):
        return MC_FollowVelocity_exec(byref(self))
    
    def set_follow_params(self, velocity):
        """设置跟随参数"""
        self.fSetVelocity = velocity
    
    def __del__(self):
        ret = MC_FollowVelocity_destroy(byref(self))
        if ret != 0:
            print(f"MC_FollowVelocity_destroy failed with error: {ret}")

class FbTrackAxis(McTrackAxis):
    """TrackAxis功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_TrackAxis_create(byref(self))
        if ret != 0:
            print(f"MC_TrackAxis_create failed with error: {ret}")

    def runCycle(self):
        return MC_TrackAxis_exec(byref(self))
    
    def set_track_params(self, velocity, acceleration, deceleration, jerk):
        """设置跟踪参数"""
        self.Velocity = velocity
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
    
    def __del__(self):
        ret = MC_TrackAxis_destroy(byref(self))
        if ret != 0:
            print(f"MC_TrackAxis_destroy failed with error: {ret}")

class FbReadSetValues(McReadSetValues):
    """ReadSetValues功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadSetValues_create(byref(self))
        if ret != 0:
            print(f"MC_ReadSetValues_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadSetValues_exec(byref(self))
    
    def get_set_values(self):
        """获取设定值"""
        return {
            'Position': self.Position,
            'Velocity': self.Velocity,
            'Acceleration': self.Acceleration,
            'Jerk': self.Jerk
        }
    
    def __del__(self):
        ret = MC_ReadSetValues_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadSetValues_destroy failed with error: {ret}")

class FbReadActualValues(McReadActualValues):
    """ReadActualValues功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadActualValues_create(byref(self))
        if ret != 0:
            print(f"MC_ReadActualValues_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadActualValues_exec(byref(self))
    
    def get_actual_values(self):
        """获取实际值"""
        return {
            'Position': self.Position,
            'Velocity': self.Velocity,
            'Acceleration': self.Acceleration,
            'Jerk': self.Jerk
        }
    
    def __del__(self):
        ret = MC_ReadActualValues_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadActualValues_destroy failed with error: {ret}")

class FbInPosition(McInPosition):
    """InPosition功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_InPosition_create(byref(self))
        if ret != 0:
            print(f"MC_InPosition_create failed with error: {ret}")

    def runCycle(self):
        return MC_InPosition_exec(byref(self))
    
    def set_inposition_params(self, pos_window, pos_time, timeout):
        """设置位置到达检测参数"""
        self.fPosWindow = pos_window
        self.fPosTime = pos_time
        self.fTimeOut = timeout
    
    def __del__(self):
        ret = MC_InPosition_destroy(byref(self))
        if ret != 0:
            print(f"MC_InPosition_destroy failed with error: {ret}")

class FbCheckLimits(McCheckLimits):
    """CheckLimits功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_CheckLimits_create(byref(self))
        if ret != 0:
            print(f"MC_CheckLimits_create failed with error: {ret}")

    def runCycle(self):
        return MC_CheckLimits_exec(byref(self))
    
    def set_check_params(self, check_vel=True, check_acc_dec=True):
        """设置检查参数"""
        self.bCheckVel = check_vel
        self.bCheckAccDec = check_acc_dec
    
    def __del__(self):
        ret = MC_CheckLimits_destroy(byref(self))
        if ret != 0:
            print(f"MC_CheckLimits_destroy failed with error: {ret}")

# ========== 新增功能块封装类 ==========

class FbReadActualTorque(McReadActualTorque):
    """ReadActualTorque功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadActualTorque_create(byref(self))
        if ret != 0:
            print(f"MC_ReadActualTorque_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadActualTorque_exec(byref(self))
    
    def __del__(self):
        ret = MC_ReadActualTorque_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadActualTorque_destroy failed with error: {ret}")

class FbReadCommandPosition(McReadCommandPosition):
    """ReadCommandPosition功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadCommandPosition_create(byref(self))
        if ret != 0:
            print(f"MC_ReadCommandPosition_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadCommandPosition_exec(byref(self))
    
    def __del__(self):
        ret = MC_ReadCommandPosition_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadCommandPosition_destroy failed with error: {ret}")

class FbReadBoolParameter(McReadBoolParameter):
    """ReadBoolParameter功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadBoolParameter_create(byref(self))
        if ret != 0:
            print(f"MC_ReadBoolParameter_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadBoolParameter_exec(byref(self))
    
    def set_parameter_number(self, param_number):
        """设置参数号"""
        self.ParameterNumber = param_number
    
    def __del__(self):
        ret = MC_ReadBoolParameter_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadBoolParameter_destroy failed with error: {ret}")

class FbWriteBoolParameter(McWriteBoolParameter):
    """WriteBoolParameter功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_WriteBoolParameter_create(byref(self))
        if ret != 0:
            print(f"MC_WriteBoolParameter_create failed with error: {ret}")

    def runCycle(self):
        return MC_WriteBoolParameter_exec(byref(self))
    
    def set_write_params(self, param_number, value):
        """设置写入参数"""
        self.ParameterNumber = param_number
        self.Value = value
    
    def __del__(self):
        ret = MC_WriteBoolParameter_destroy(byref(self))
        if ret != 0:
            print(f"MC_WriteBoolParameter_destroy failed with error: {ret}")

class FbMoveContinuousAbsolute(McMoveContinuousAbsolute):
    """MoveContinuousAbsolute功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_MoveContinuousAbsolute_create(byref(self))
        if ret != 0:
            print(f"MC_MoveContinuousAbsolute_create failed with error: {ret}")

    def runCycle(self):
        return MC_MoveContinuousAbsolute_exec(byref(self))
    
    def set_move_params(self, position, velocity, end_velocity, end_velocity_direction,
                       acceleration, deceleration, jerk, direction, adapt_end_vel=False):
        """设置连续绝对运动参数"""
        self.Position = position
        self.Velocity = velocity
        self.EndVelocity = end_velocity
        self.EndVelocityDirection = end_velocity_direction
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
        self.Direction = direction
        self.AdaptEndVelToAvoidOvershoot = adapt_end_vel
    
    def __del__(self):
        ret = MC_MoveContinuousAbsolute_destroy(byref(self))
        if ret != 0:
            print(f"MC_MoveContinuousAbsolute_destroy failed with error: {ret}")

class FbMoveContinuousRelative(McMoveContinuousRelative):
    """MoveContinuousRelative功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_MoveContinuousRelative_create(byref(self))
        if ret != 0:
            print(f"MC_MoveContinuousRelative_create failed with error: {ret}")

    def runCycle(self):
        return MC_MoveContinuousRelative_exec(byref(self))
    
    def set_move_params(self, distance, velocity, end_velocity, end_velocity_direction,
                       acceleration, deceleration, jerk, adapt_end_vel=False):
        """设置连续相对运动参数"""
        self.Distance = distance
        self.Velocity = velocity
        self.EndVelocity = end_velocity
        self.EndVelocityDirection = end_velocity_direction
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
        self.AdaptEndVelToAvoidOvershoot = adapt_end_vel
    
    def __del__(self):
        ret = MC_MoveContinuousRelative_destroy(byref(self))
        if ret != 0:
            print(f"MC_MoveContinuousRelative_destroy failed with error: {ret}")

class FbMoveSuperImposed(McMoveSuperImposed):
    """MoveSuperImposed功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_MoveSuperImposed_create(byref(self))
        if ret != 0:
            print(f"MC_MoveSuperImposed_create failed with error: {ret}")

    def runCycle(self):
        return MC_MoveSuperImposed_exec(byref(self))
    
    def set_move_params(self, distance, velocity_diff, acceleration, deceleration, jerk):
        """设置叠加运动参数"""
        self.Distance = distance
        self.VelocityDiff = velocity_diff
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
    
    def __del__(self):
        ret = MC_MoveSuperImposed_destroy(byref(self))
        if ret != 0:
            print(f"MC_MoveSuperImposed_destroy failed with error: {ret}")

class FbPositionProfile(McPositionProfile):
    """PositionProfile功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_PositionProfile_create(byref(self))
        if ret != 0:
            print(f"MC_PositionProfile_create failed with error: {ret}")

    def runCycle(self):
        return MC_PositionProfile_exec(byref(self))
    
    def set_profile_params(self, array_size, position_scale, offset):
        """设置位置轮廓参数"""
        self.ArraySize = array_size
        self.PositionScale = position_scale
        self.Offset = offset
    
    def __del__(self):
        ret = MC_PositionProfile_destroy(byref(self))
        if ret != 0:
            print(f"MC_PositionProfile_destroy failed with error: {ret}")

class FbAccelerationProfile(McAccelerationProfile):
    """AccelerationProfile功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_AccelerationProfile_create(byref(self))
        if ret != 0:
            print(f"MC_AccelerationProfile_create failed with error: {ret}")

    def runCycle(self):
        return MC_AccelerationProfile_exec(byref(self))
    
    def set_profile_params(self, array_size, acceleration_scale, offset):
        """设置加速度轮廓参数"""
        self.ArraySize = array_size
        self.AccelerationScale = acceleration_scale
        self.Offset = offset
    
    def __del__(self):
        ret = MC_AccelerationProfile_destroy(byref(self))
        if ret != 0:
            print(f"MC_AccelerationProfile_destroy failed with error: {ret}")

class FbVelocityProfile(McVelocityProfile):
    """VelocityProfile功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_VelocityProfile_create(byref(self))
        if ret != 0:
            print(f"MC_VelocityProfile_create failed with error: {ret}")

    def runCycle(self):
        return MC_VelocityProfile_exec(byref(self))
    
    def set_profile_params(self, array_size, velocity_scale, offset):
        """设置速度轮廓参数"""
        self.ArraySize = array_size
        self.VelocityScale = velocity_scale
        self.Offset = offset
    
    def __del__(self):
        ret = MC_VelocityProfile_destroy(byref(self))
        if ret != 0:
            print(f"MC_VelocityProfile_destroy failed with error: {ret}")

class FbGearInPos(McGearInPos):
    """GearInPos功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_GearInPos_create(byref(self))
        if ret != 0:
            print(f"MC_GearInPos_create failed with error: {ret}")

    def runCycle(self):
        return MC_GearInPos_exec(byref(self))
    
    def set_gear_params(self, ratio_numerator, ratio_denominator, master_sync_pos,
                       slave_sync_pos, master_start_distance, buffer_mode=BufferMode.mcAborting,
                       avoid_reversal=False):
        """设置位置齿轮同步参数"""
        self.RatioNumerator = ratio_numerator
        self.RatioDenominator = ratio_denominator
        self.MasterSyncPosition = master_sync_pos
        self.SlaveSyncPosition = slave_sync_pos
        self.MasterStartDistance = master_start_distance
        self.BufferMode = buffer_mode
        self.AvoidReversal = avoid_reversal
    
    def __del__(self):
        ret = MC_GearInPos_destroy(byref(self))
        if ret != 0:
            print(f"MC_GearInPos_destroy failed with error: {ret}")

class FbFollowPositionVelocity(McFollowPositionVelocity):
    """FollowPositionVelocity功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_FollowPositionVelocity_create(byref(self))
        if ret != 0:
            print(f"MC_FollowPositionVelocity_create failed with error: {ret}")

    def runCycle(self):
        return MC_FollowPositionVelocity_exec(byref(self))
    
    def set_follow_params(self, position, velocity):
        """设置跟随参数"""
        self.fSetPosition = position
        self.fSetVelocity = velocity
    
    def __del__(self):
        ret = MC_FollowPositionVelocity_destroy(byref(self))
        if ret != 0:
            print(f"MC_FollowPositionVelocity_destroy failed with error: {ret}")

class FbFollowSetValues(McFollowSetValues):
    """FollowSetValues功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_FollowSetValues_create(byref(self))
        if ret != 0:
            print(f"MC_FollowSetValues_create failed with error: {ret}")

    def runCycle(self):
        return MC_FollowSetValues_exec(byref(self))
    
    def set_follow_params(self, position, velocity, acceleration, jerk, torque, current):
        """设置跟随参数"""
        self.fSetPosition = position
        self.fSetVelocity = velocity
        self.fSetAcceleration = acceleration
        self.fSetJerk = jerk
        self.fSetTorque = torque
        self.fSetCurrent = current
    
    def __del__(self):
        ret = MC_FollowSetValues_destroy(byref(self))
        if ret != 0:
            print(f"MC_FollowSetValues_destroy failed with error: {ret}")

class FbTrackSetValues(McTrackSetValues):
    """TrackSetValues功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_TrackSetValues_create(byref(self))
        if ret != 0:
            print(f"MC_TrackSetValues_create failed with error: {ret}")

    def runCycle(self):
        return MC_TrackSetValues_exec(byref(self))
    
    def set_track_params(self, set_position, set_velocity, set_acceleration, set_jerk,
                        velocity, acceleration, deceleration, jerk, signal_modulo_period):
        """设置跟踪参数"""
        self.fSetPosition = set_position
        self.fSetVelocity = set_velocity
        self.fSetAcceleration = set_acceleration
        self.fSetJerk = set_jerk
        self.Velocity = velocity
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
        self.SignalModuloPeriod = signal_modulo_period
    
    def __del__(self):
        ret = MC_TrackSetValues_destroy(byref(self))
        if ret != 0:
            print(f"MC_TrackSetValues_destroy failed with error: {ret}")

class FbCamOut(McCamOut):
    """CamOut功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_CamOut_create(byref(self))
        if ret != 0:
            print(f"MC_CamOut_create failed with error: {ret}")

    def runCycle(self):
        return MC_CamOut_exec(byref(self))
    
    def __del__(self):
        ret = MC_CamOut_destroy(byref(self))
        if ret != 0:
            print(f"MC_CamOut_destroy failed with error: {ret}")

class FbCamRegister(McCamRegister):
    """CamRegister功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_CamRegister_create(byref(self))
        if ret != 0:
            print(f"MC_CamRegister_create failed with error: {ret}")

    def runCycle(self):
        return MC_CamRegister_exec(byref(self))
    
    def set_register_params(self, master_offset, master_scaling, tappet_hysteresis, dead_time_compensation):
        """设置凸轮注册参数"""
        self.MasterOffset = master_offset
        self.MasterScaling = master_scaling
        self.TappetHysteresis = tappet_hysteresis
        self.DeadTimeCompensation = dead_time_compensation
    
    def __del__(self):
        ret = MC_CamRegister_destroy(byref(self))
        if ret != 0:
            print(f"MC_CamRegister_destroy failed with error: {ret}")

class FbCamCalc(McCamCalc):
    """CamCalc功能块封装 - 凸轮计算功能块"""
    
    def __init__(self):
        super().__init__()
        ret = MC_CamCalc_create(byref(self))
        if ret != 0:
            print(f"MC_CamCalc_create failed with error: {ret}")

    def runCycle(self):
        return MC_CamCalc_exec(byref(self))
    
    def set_cam_table(self, cam_table):
        """设置凸轮表"""
        self.CamTable = cam_table
    
    def set_master_pva(self, position, velocity, acceleration):
        """设置主轴PVA"""
        self.MasterPosition = position
        self.MasterVelocity = velocity
        self.MasterAcceleration = acceleration
    
    def set_affine_params(self, master_offset=0.0, slave_offset=0.0, 
                         master_scaling=1.0, slave_scaling=1.0,
                         master_engage_pos=0.0, slave_engage_pos=0.0,
                         master_absolute=True, slave_absolute=True):
        """设置仿射变换参数"""
        self.MasterOffset = master_offset
        self.SlaveOffset = slave_offset
        self.MasterScaling = master_scaling
        self.SlaveScaling = slave_scaling
        self.MasterEngagePos = master_engage_pos
        self.SlaveEngagePos = slave_engage_pos
        self.MasterAbsolute = master_absolute
        self.SlaveAbsolute = slave_absolute
    
    def get_slave_pva(self):
        """获取从轴PVA"""
        return {
            'position': self.SlavePosition,
            'velocity': self.SlaveVelocity,
            'acceleration': self.SlaveAcceleration
        }
    
    def is_busy(self):
        """检查是否忙碌"""
        return bool(self.Busy)
    
    def is_error(self):
        """检查是否有错误"""
        return bool(self.Error)
    
    def get_error_id(self):
        """获取错误ID"""
        return self.ErrorID
    
    def __del__(self):
        ret = MC_CamCalc_destroy(byref(self))
        if ret != 0:
            print(f"MC_CamCalc_destroy failed with error: {ret}")

class FbCAMBounds(McCAMBounds):
    """CAMBounds功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_CAMBounds_create(byref(self))
        if ret != 0:
            print(f"MC_CAMBounds_create failed with error: {ret}")

    def runCycle(self):
        return MC_CAMBounds_exec(byref(self))
    
    def set_bounds_params(self, master_vel_max, master_acc_max, master_scaling, slave_scaling):
        """设置CAM边界参数"""
        self.dMasterVelMax = master_vel_max
        self.dMasterAccMax = master_acc_max
        self.dMasterScaling = master_scaling
        self.dSlaveScaling = slave_scaling
    
    def __del__(self):
        ret = MC_CAMBounds_destroy(byref(self))
        if ret != 0:
            print(f"MC_CAMBounds_destroy failed with error: {ret}")

class FbCAMBounds_Pos(McCAMBounds_Pos):
    """CAMBounds_Pos功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_CAMBounds_Pos_create(byref(self))
        if ret != 0:
            print(f"MC_CAMBounds_Pos_create failed with error: {ret}")

    def runCycle(self):
        return MC_CAMBounds_Pos_exec(byref(self))
    
    def set_bounds_params(self, master_vel_max, master_acc_max, master_scaling, slave_scaling):
        """设置CAM位置边界参数"""
        self.dMasterVelMax = master_vel_max
        self.dMasterAccMax = master_acc_max
        self.dMasterScaling = master_scaling
        self.dSlaveScaling = slave_scaling
    
    def __del__(self):
        ret = MC_CAMBounds_Pos_destroy(byref(self))
        if ret != 0:
            print(f"MC_CAMBounds_Pos_destroy failed with error: {ret}")

class FbGetCamSlaveSetPosition(McGetCamSlaveSetPosition):
    """GetCamSlaveSetPosition功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_GetCamSlaveSetPosition_create(byref(self))
        if ret != 0:
            print(f"MC_GetCamSlaveSetPosition_create failed with error: {ret}")

    def runCycle(self):
        return MC_GetCamSlaveSetPosition_exec(byref(self))
    
    def set_cam_params(self, master_offset, slave_offset, master_scaling, slave_scaling):
        """设置CAM参数"""
        self.MasterOffset = master_offset
        self.SlaveOffset = slave_offset
        self.MasterScaling = master_scaling
        self.SlaveScaling = slave_scaling
    
    def __del__(self):
        ret = MC_GetCamSlaveSetPosition_destroy(byref(self))
        if ret != 0:
            print(f"MC_GetCamSlaveSetPosition_destroy failed with error: {ret}")

class FbAbortTrigger(McAbortTrigger):
    """AbortTrigger功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_AbortTrigger_create(byref(self))
        if ret != 0:
            print(f"MC_AbortTrigger_create failed with error: {ret}")

    def runCycle(self):
        return MC_AbortTrigger_exec(byref(self))
    
    def __del__(self):
        ret = MC_AbortTrigger_destroy(byref(self))
        if ret != 0:
            print(f"MC_AbortTrigger_destroy failed with error: {ret}")

class FbTouchProbe(McTouchProbe):
    """TouchProbe功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_TouchProbe_create(byref(self))
        if ret != 0:
            print(f"MC_TouchProbe_create failed with error: {ret}")

    def runCycle(self):
        return MC_TouchProbe_exec(byref(self))
    
    def set_probe_params(self, window_only, first_position, last_position):
        """设置探针参数"""
        self.WindowOnly = window_only
        self.FirstPosition = first_position
        self.LastPosition = last_position
    
    def __del__(self):
        ret = MC_TouchProbe_destroy(byref(self))
        if ret != 0:
            print(f"MC_TouchProbe_destroy failed with error: {ret}")

class FbDigitalCamSwitch(McDigitalCamSwitch):
    """DigitalCamSwitch功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_DigitalCamSwitch_create(byref(self))
        if ret != 0:
            print(f"MC_DigitalCamSwitch_create failed with error: {ret}")

    def runCycle(self):
        return MC_DigitalCamSwitch_exec(byref(self))
    
    def set_switch_params(self, enable_mask, tappet_mode):
        """设置数字凸轮开关参数"""
        self.EnableMask = enable_mask
        self.TappetMode = tappet_mode
    
    def __del__(self):
        ret = MC_DigitalCamSwitch_destroy(byref(self))
        if ret != 0:
            print(f"MC_DigitalCamSwitch_destroy failed with error: {ret}")

class FbReadSetDriveValues(McReadSetDriveValues):
    """ReadSetDriveValues功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadSetDriveValues_create(byref(self))
        if ret != 0:
            print(f"MC_ReadSetDriveValues_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadSetDriveValues_exec(byref(self))
    
    def get_drive_values(self):
        """获取驱动器设定值"""
        return {
            'Position': self.Position,
            'Velocity': self.Velocity,
            'Acceleration': self.Acceleration,
            'Jerk': self.Jerk
        }
    
    def __del__(self):
        ret = MC_ReadSetDriveValues_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadSetDriveValues_destroy failed with error: {ret}")

class FbReadActualDriveValues(McReadActualDriveValues):
    """ReadActualDriveValues功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ReadActualDriveValues_create(byref(self))
        if ret != 0:
            print(f"MC_ReadActualDriveValues_create failed with error: {ret}")

    def runCycle(self):
        return MC_ReadActualDriveValues_exec(byref(self))
    
    def get_drive_values(self):
        """获取驱动器实际值"""
        return {
            'Position': self.Position,
            'Velocity': self.Velocity,
            'Acceleration': self.Acceleration,
            'Jerk': self.Jerk
        }
    
    def __del__(self):
        ret = MC_ReadActualDriveValues_destroy(byref(self))
        if ret != 0:
            print(f"MC_ReadActualDriveValues_destroy failed with error: {ret}")

class FbGetTrackingError(McGetTrackingError):
    """GetTrackingError功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_GetTrackingError_create(byref(self))
        if ret != 0:
            print(f"MC_GetTrackingError_create failed with error: {ret}")

    def runCycle(self):
        return MC_GetTrackingError_exec(byref(self))
    
    def set_tracking_params(self, timestamp, dead_time_cycles):
        """设置跟踪误差参数"""
        self.TimeStamp = timestamp
        self.DeadTimeCycles = dead_time_cycles
    
    def __del__(self):
        ret = MC_GetTrackingError_destroy(byref(self))
        if ret != 0:
            print(f"MC_GetTrackingError_destroy failed with error: {ret}")

class FbGetTravelledDistance(McGetTravelledDistance):
    """GetTravelledDistance功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_GetTravelledDistance_create(byref(self))
        if ret != 0:
            print(f"MC_GetTravelledDistance_create failed with error: {ret}")

    def runCycle(self):
        return MC_GetTravelledDistance_exec(byref(self))
    
    def __del__(self):
        ret = MC_GetTravelledDistance_destroy(byref(self))
        if ret != 0:
            print(f"MC_GetTravelledDistance_destroy failed with error: {ret}")

class FbGetMaxSetAccDec(McGetMaxSetAccDec):
    """GetMaxSetAccDec功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_GetMaxSetAccDec_create(byref(self))
        if ret != 0:
            print(f"MC_GetMaxSetAccDec_create failed with error: {ret}")

    def runCycle(self):
        return MC_GetMaxSetAccDec_exec(byref(self))
    
    def set_timestamp(self, timestamp):
        """设置时间戳"""
        self.TimeStamp = timestamp
    
    def __del__(self):
        ret = MC_GetMaxSetAccDec_destroy(byref(self))
        if ret != 0:
            print(f"MC_GetMaxSetAccDec_destroy failed with error: {ret}")

class FbGetMaxSetVelocity(McGetMaxSetVelocity):
    """GetMaxSetVelocity功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_GetMaxSetVelocity_create(byref(self))
        if ret != 0:
            print(f"MC_GetMaxSetVelocity_create failed with error: {ret}")

    def runCycle(self):
        return MC_GetMaxSetVelocity_exec(byref(self))
    
    def set_timestamp(self, timestamp):
        """设置时间戳"""
        self.TimeStamp = timestamp
    
    def __del__(self):
        ret = MC_GetMaxSetVelocity_destroy(byref(self))
        if ret != 0:
            print(f"MC_GetMaxSetVelocity_destroy failed with error: {ret}")

class FbGetOverride(McGetOverride):
    """GetOverride功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_GetOverride_create(byref(self))
        if ret != 0:
            print(f"MC_GetOverride_create failed with error: {ret}")

    def runCycle(self):
        return MC_GetOverride_exec(byref(self))
    
    def __del__(self):
        ret = MC_GetOverride_destroy(byref(self))
        if ret != 0:
            print(f"MC_GetOverride_destroy failed with error: {ret}")

class FbSetOverride(McSetOverride):
    """SetOverride功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_SetOverride_create(byref(self))
        if ret != 0:
            print(f"MC_SetOverride_create failed with error: {ret}")

    def runCycle(self):
        return MC_SetOverride_exec(byref(self))
    
    def set_override_params(self, vel_factor, acc_factor, jerk_factor):
        """设置覆盖因子参数"""
        self.VelFactor = vel_factor
        self.AccFactor = acc_factor
        self.JerkFactor = jerk_factor
    
    def __del__(self):
        ret = MC_SetOverride_destroy(byref(self))
        if ret != 0:
            print(f"MC_SetOverride_destroy failed with error: {ret}")

class FbSetMovementType(McSetMovementType):
    """SetMovementType功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_SetMovementType_create(byref(self))
        if ret != 0:
            print(f"MC_SetMovementType_create failed with error: {ret}")

    def runCycle(self):
        return MC_SetMovementType_exec(byref(self))
    
    def set_movement_params(self, movement_type, position_period):
        """设置运动类型参数"""
        self.iMovementType = movement_type
        self.fPositionPeriod = position_period
    
    def __del__(self):
        ret = MC_SetMovementType_destroy(byref(self))
        if ret != 0:
            print(f"MC_SetMovementType_destroy failed with error: {ret}")

class FbSetRampType(McSetRampType):
    """SetRampType功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_SetRampType_create(byref(self))
        if ret != 0:
            print(f"MC_SetRampType_create failed with error: {ret}")

    def runCycle(self):
        return MC_SetRampType_exec(byref(self))
    
    def set_ramp_params(self, ramp_type):
        """设置斜坡类型参数"""
        self.eRampType = ramp_type
    
    def __del__(self):
        ret = MC_SetRampType_destroy(byref(self))
        if ret != 0:
            print(f"MC_SetRampType_destroy failed with error: {ret}")

class FbChangeDynamicLimits(McChangeDynamicLimits):
    """ChangeDynamicLimits功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ChangeDynamicLimits_create(byref(self))
        if ret != 0:
            print(f"MC_ChangeDynamicLimits_create failed with error: {ret}")

    def runCycle(self):
        return MC_ChangeDynamicLimits_exec(byref(self))
    
    def set_dynamic_limits(self, max_velocity, max_acceleration, max_deceleration, max_jerk, max_torque):
        """设置动态限制参数"""
        self.fMaxVelocity = max_velocity
        self.fMaxAcceleration = max_acceleration
        self.fMaxDeceleration = max_deceleration
        self.fMaxJerk = max_jerk
        self.fMaxTorque = max_torque
    
    def __del__(self):
        ret = MC_ChangeDynamicLimits_destroy(byref(self))
        if ret != 0:
            print(f"MC_ChangeDynamicLimits_destroy failed with error: {ret}")

class FbChangeGearingRatio(McChangeGearingRatio):
    """ChangeGearingRatio功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_ChangeGearingRatio_create(byref(self))
        if ret != 0:
            print(f"MC_ChangeGearingRatio_create failed with error: {ret}")

    def runCycle(self):
        return MC_ChangeGearingRatio_exec(byref(self))
    
    def set_gearing_params(self, ratio_denom, ratio_num, position_period, movement_type):
        """设置齿轮比参数"""
        self.dwRatioTechUnitsDenom = ratio_denom
        self.iRatioTechUnitsNum = ratio_num
        self.fPositionPeriod = position_period
        self.iMovementType = movement_type
    
    def __del__(self):
        ret = MC_ChangeGearingRatio_destroy(byref(self))
        if ret != 0:
            print(f"MC_ChangeGearingRatio_destroy failed with error: {ret}")

class FbSetSoftwareLimits(McSetSoftwareLimits):
    """SetSoftwareLimits功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_SetSoftwareLimits_create(byref(self))
        if ret != 0:
            print(f"MC_SetSoftwareLimits_create failed with error: {ret}")

    def runCycle(self):
        return MC_SetSoftwareLimits_exec(byref(self))
    
    def set_software_limits(self, is_activated, positive_limit, negative_limit, 
                           error_decelerate=False, error_deceleration=0, error_max_distance=0):
        """设置软件限位参数"""
        self.SWL_Activated = is_activated
        self.SWL_Positive = positive_limit
        self.SWL_Negative = negative_limit
        self.SWL_Error_Decelerate = error_decelerate
        self.SWL_Error_Deceleration = error_deceleration
        self.SWL_Error_MaxDistance = error_max_distance
    
    def __del__(self):
        ret = MC_SetSoftwareLimits_destroy(byref(self))
        if ret != 0:
            print(f"MC_SetSoftwareLimits_destroy failed with error: {ret}")

class FbSetControllerMode(McSetControllerMode):
    """SetControllerMode功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_SetControllerMode_create(byref(self))
        if ret != 0:
            print(f"MC_SetControllerMode_create failed with error: {ret}")

    def runCycle(self):
        return MC_SetControllerMode_exec(byref(self))
    
    def set_controller_mode(self, controller_mode):
        """设置控制器模式"""
        self.nControllerMode = controller_mode
    
    def __del__(self):
        ret = MC_SetControllerMode_destroy(byref(self))
        if ret != 0:
            print(f"MC_SetControllerMode_destroy failed with error: {ret}")

class FbInch(McInch):
    """Inch功能块封装"""
    
    def __init__(self):
        super().__init__()
        ret = MC_Inch_create(byref(self))
        if ret != 0:
            print(f"MC_Inch_create failed with error: {ret}")

    def runCycle(self):
        return MC_Inch_exec(byref(self))
    
    def set_inch_params(self, distance, velocity, acceleration, deceleration, jerk):
        """设置寸动参数"""
        self.Distance = distance
        self.Velocity = velocity
        self.Acceleration = acceleration
        self.Deceleration = deceleration
        self.Jerk = jerk
    
    def inch_forward(self):
        """正向寸动"""
        self.InchForward = True
        self.InchBackward = False
    
    def inch_backward(self):
        """负向寸动"""
        self.InchForward = False
        self.InchBackward = True
    
    def stop_inch(self):
        """停止寸动"""
        self.InchForward = False
        self.InchBackward = False
    
    def __del__(self):
        ret = MC_Inch_destroy(byref(self))
        if ret != 0:
            print(f"MC_Inch_destroy failed with error: {ret}")



def powerDemo2():
    power = FbPower()
    axis = FbAxisWrap()
    
    power.Axis = axis.ref   
    power.Enable = True        
    power.bRegulatorOn = True  
    power.bDriveStart = True  
    
    for i in range(10):
        power.runCycle()
        print(f"Status: {power.Status}, Busy: {power.Busy}")
        axis.run_cycle()

def readDemo2():
    power = FbPower()
    axis = FbAxisWrap()
    rdPara = FbReadParameter()
    power.Axis = axis.ref   
    power.Enable = True        
    power.bRegulatorOn = True  
    power.bDriveStart = True  
    
    rdPara.ParameterNumber = 1015
    rdPara.Axis = axis.ref 
    rdPara.Enable = True
    
    for i in range(10):
        power.runCycle()
        rdPara.runCycle()
        print(f"Status: {power.Status}, Busy: {power.Busy}, {rdPara.Value}")
        axis.run_cycle()

def moveAbsoluteDemo():
    """MoveAbsolute功能块演示"""
    power = FbPower()
    axis = FbAxisWrap()
    move_abs = FbMoveAbsolute()
    power.Axis = axis.ref
    power.Enable = True
    power.bRegulatorOn = True
    power.bDriveStart = True
    # 设置轴引用
    move_abs.Axis = axis.ref
    
    # 设置运动参数
    move_abs.set_move_params(
        position=100.0,    
        velocity=50.0,     
        acceleration=100.0,
        deceleration=100.0,
        jerk=0,     
        direction=Direction.mcPositiveDirection,  # 正方向
        buffer_mode=BufferMode.mcAborting  # 中断模式
    )
    for i in range(50):
        if i==10:
            move_abs.Execute = True
        power.runCycle()
        move_abs.runCycle()
        axis.run_cycle()
        axis.print(1)

def moveJogDemo():
    """Jog功能块演示"""
    axis = FbAxisWrap()
    cfg = AxisCiaConfigWrapper()
    cfg.set_cycle_time(10000)
    axis.set_config(cfg.get_config_ptr())

    power = FbPower()
    jog = FbJog()
    power.Axis = axis.ref
    power.Enable = True
    power.bRegulatorOn = True
    power.bDriveStart = True
    # Set axis reference for jog
    jog.Axis = axis.ref

    # Set jog parameters
    jog.Velocity = 50.0
    jog.Acceleration = 500.0
    jog.Deceleration = 500.0
    jog.Jerk = 0

    for i in range(80):
        # Simulate jog forward for first 20 cycles, then stop
        if i == 10:
            jog.JogForward = True
            jog.JogBackward = False
        if i == 30:
            jog.JogForward = False
            jog.JogBackward = True
        if i == 50:
            jog.JogForward = False
            jog.JogBackward = False    
        power.runCycle()
        jog.runCycle()
        axis.run_cycle()
        axis.print(1)

def moveVelDemo():
    axis = FbAxisWrap()
    cfg2 = AxisCiaConfigWrapper()
    cfg2.set_cycle_time(10000)
    axis.set_config(cfg2.get_config_ptr())
    
    # cfg = AxisCiaConfig()
    # axis_config_default_init(byref(cfg))
    # cfg.cycle_time = 10000
    # axis.set_config(byref(cfg))
    
    power = FbPower()
    move = FbMoveVelocity()
    power.Axis = axis.ref
    power.Enable = True
    power.bRegulatorOn = True
    power.bDriveStart = True
    # 设置轴引用
    move.Axis = axis.ref
    
    # 设置运动参数
    move.set_move_params(   
        velocity=50.0,     
        acceleration=500.0,
        deceleration=500.0,
        jerk=0,     
        direction=Direction.mcPositiveDirection,  # 正方向
        buffer_mode=BufferMode.mcAborting  # 中断模式
    )
    for i in range(50):
        if i==10:
            move.Execute = True
        power.runCycle()
        move.runCycle()
        axis.run_cycle()
        axis.print(1)

def axisConfigDemo():
    """AxisCiaConfig演示"""
    print("=== AxisCiaConfig演示 ===")
    
    # 创建配置
    config = AxisCiaConfigWrapper()
    
    # 设置基本参数
    config.set_virtual_mode(True)
    config.set_cycle_time(1000)  # 1ms周期
    config.set_axis_type(0)  # 有限轴
    
    # 设置动态限制
    config.set_dynamic_limits(
        velocity=1000.0,
        acceleration=5000.0,
        deceleration=5000.0,
        jerk=0.0
    )
    
    # 设置软件限位
    config.set_software_limits(
        is_activate=True,
        negative=-1000.0,
        positive=1000.0
    )
    
    # 设置电机类型（线性）
    config.set_motor_type(0, False)  # 线性电机，不反转
    config.set_linear_params(1, 1)  # 1:1比例
    
    # 检查配置
    result = config.check_config()
    if result == 0:
        print("✓ 配置检查通过")
    else:
        print(f"✗ 配置检查失败，错误码: {result}")
    
    print("AxisCiaConfig演示完成")

def py_cam_in_demo():
    """Python CamIn demo, fully ctypes, 参考camIn_demo.cpp"""
    fname = b'abc2.cam'

    # 创建主从轴
    axis_master = FbAxisWrap()
    axis_slave = FbAxisWrap()

    # 配置主轴
    cfgM = AxisCiaConfigWrapper()
    cfgM.set_virtual_mode(True)
    cfgM.set_cycle_time(10000)
    axis_master.set_config(cfgM.get_config_ptr())

    # 配置从轴
    cfgS = AxisCiaConfigWrapper()
    cfgS.set_virtual_mode(True)
    cfgS.set_cycle_time(10000)
    cfgS.set_id(1)
    axis_slave.set_config(cfgS.get_config_ptr())

    # 功能块
    powerM = FbPower()
    powerS = FbPower()
    moveVel = FbMoveVelocity()
    readCam = FbReadCAM()
    camTbl = FbCamTableSelect()
    camIn = FbCamIn()
    tappet = FbGetTappetValue()
    
    # 轴绑定
    powerM.Axis = axis_master.ref
    powerS.Axis = axis_slave.ref
    moveVel.Axis = axis_master.ref
    camTbl.Master = axis_master.ref
    camTbl.Slave = axis_slave.ref
    camIn.Master = axis_master.ref
    camIn.Slave = axis_slave.ref

    # 参数设置
    moveVel.set_move_params(velocity=50, acceleration=500, deceleration=500, jerk=0)
    camTbl.Periodic = False
    camTbl.MasterAbsolute = False
    camTbl.SlaveAbsolute = False
    camIn.MasterOffset = 0
    camIn.SlaveOffset = 0
    camIn.MasterScaling = 1
    camIn.SlaveScaling = 1
    camIn.Acceleration = 1000
    camIn.Deceleration = 1000
    camIn.Jerk = 0

    tappet.iID = 1
    tappet.bInitValue = False

    for idx in range(30):
        if idx == 1:
            powerM.Enable = True
            powerM.bRegulatorOn = True
            powerM.bDriveStart = True
            powerS.Enable = True
            powerS.bRegulatorOn = True
            powerS.bDriveStart = True
        if idx == 5:
            readCam.bExecute = True
            readCam.sFileName = fname
            print(readCam.sFileName)
        if idx == 8:
            camTbl.Execute = True
            camTbl.CamTable = readCam.CAM
        if idx == 10:
            moveVel.Execute = True
        if idx == 15:
            camIn.Execute = True
            camIn.CamTableID = camTbl.CamTableID

        powerM.runCycle()
        powerS.runCycle()
        readCam.runCycle()
        camTbl.runCycle()
        moveVel.runCycle()
        camIn.runCycle()
        # tappet.set_tappet(camIn.Tappets)
        tappet.set_tappet_from_cam_in(camIn)
        tappet.runCycle()
        axis_master.run_cycle()
        axis_slave.run_cycle()
        axis_master.print(1)
        axis_slave.print(1)
        print(tappet.bTappet)

def cam_print_demo(cam_filename="abc3.cam", start_angle=0.0, end_angle=360.0, step=10.0):
    """
    凸轮打印函数 - 使用 FbReadCAM 读取凸轮文件，然后使用 cam_ref_get_pva 打印指定区间的点
    
    Args:
        cam_filename: str - 凸轮文件名
        start_angle: float - 起始角度 (默认 0.0)
        end_angle: float - 结束角度 (默认 360.0) 
        step: float - 角度步长 (默认 10.0)
    """
    print("=== 凸轮打印演示 ===")
    print(f"凸轮文件: {cam_filename}")
    print(f"角度范围: [{start_angle}, {end_angle}]")
    print(f"步长: {step}")
    print("-" * 50)
    
    # 创建 FbReadCAM 实例
    read_cam = FbReadCAM()
    
    # 设置文件名
    read_cam.sFileName = cam_filename.encode('utf-8')
    
    # 执行读取
    read_cam.bExecute = True
    result = read_cam.runCycle()
    
    if result != 0:
        print(f"读取凸轮文件失败，错误码: {result}")
        return
    
    # 检查是否完成
    if not read_cam.bDone:
        print("凸轮文件读取未完成")
        return
    
    if read_cam.bError:
        print(f"读取凸轮文件时发生错误，错误ID: {read_cam.ErrorID}")
        return
    
    print("✓ 凸轮文件读取成功")
    
    # 获取 CAM 引用
    cam_ref = read_cam.CAM
    if not cam_ref:
        print("获取 CAM 引用失败")
        return
    
    print("✓ 获取 CAM 引用成功")
    print()
    
    # 打印表头
    print(f"{'角度(度)':<10} {'位置':<12} {'速度':<12} {'加速度':<12}")
    print("-" * 50)
    
    # 遍历指定角度范围
    angle = start_angle
    while angle <= end_angle:
        # 创建 CamXyva 结构体用于接收结果
        pva = CamXyva()
        
        # 调用 cam_ref_get_pva 函数
        result = cam_ref_get_pva(cam_ref, angle, byref(pva))
        
        if result == 0:  # 成功
            print(f"{angle:<10.1f} {pva.dY:<12.6f} {pva.dV:<12.6f} {pva.dA:<12.6f}")
        else:
            print(f"{angle:<10.1f} {'ERROR':<12} {'ERROR':<12} {'ERROR':<12}")
        
        angle += step
    
    print("-" * 50)
    print("凸轮打印完成")
    
    # 清理资源
    del read_cam


def traj_vel_demo():
    print("=== Vel + Traj demo ===")
    if not zmc_dll:
        print("DLL not loaded")
        return

    vel = McVelWrap()
    vel.set_config(50.0, 200.0, 200.0, 0.0)
    vel.set_begin_pva(0.0, 0.0, 0.0)
    vel.set_end_pva(100.0, 0.0, 0.0)
    vel_ret = vel.build()
    print(f"[vel] build ret={vel_ret}, end_time={vel.get_end_time():.6f}")
    if vel_ret == 0:
        for t in [0.0, 0.1, 0.2, 0.5, 1.0]:
            ret, p, v, a = vel.get_slice(t)
            print(f"[vel] t={t:>5.2f} ret={ret} pos={p:>9.4f} vel={v:>9.4f} acc={a:>9.4f}")

    traj = McTrajWrap()
    traj.set_begin_pos([0.0])
    traj.set_sg_param(100.0, False, False)
    traj_ret = traj.build()
    plan = traj.get_plan()
    print(f"[traj] build ret={traj_ret}, plan={plan:.6f}, axis_num={traj.get_axis_num()}")
    if traj_ret == 0:
        for d in [0.0, 25.0, 50.0, 75.0, 100.0]:
            abs_pos, dif_pos = traj.get_slice(d, 1)
            print(f"[traj] d={d:>6.2f} abs={abs_pos[0]:>9.4f} dif={dif_pos[0]:>9.4f}")

if __name__ == "__main__":
    # powerDemo2()
    # moveAbsoluteDemo()
    # moveVelDemo()
    # axisConfigDemo()
    # moveJogDemo()
    # readDemo2()
    # py_cam_in_demo()
    # tappetDemo()
    traj_vel_demo()
    # cam_print_demo()
