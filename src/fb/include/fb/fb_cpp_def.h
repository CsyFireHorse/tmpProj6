/**
 * History:
 * ================================================================
 * 2025-04-14 ChenSy <ChenSy@zlg.cn> created.
 *
 */
	
#ifndef _FB_CPP_DEF_H_
#define _FB_CPP_DEF_H_

#include "fbt_common.h"
#include "mc_ref_def.h"

namespace zmc{
    
#pragma pack(push, AW_PLC_NATIVE_ALIGN)
typedef struct FbtPowerStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_BOOL bRegulatorOn;
    ZMC_BOOL bDriveStart;
    // output
    ZMC_BOOL Status;
    ZMC_BOOL bRegulatorRealState;
    ZMC_BOOL bDriveStartRealState;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtPowerStruct;

typedef struct FbtResetStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtResetStruct;

typedef struct FbtSetPositionStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Position;
    ZMC_BOOL Mode; // TRUE = RELATIVE, FALSE = ABSOLUTE
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtSetPositionStruct;

typedef struct FbtReadStatusStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_BOOL Disabled;
    ZMC_BOOL ErrorStop;
    ZMC_BOOL Stopping;
    ZMC_BOOL Standstill;
    ZMC_BOOL DiscreteMotion;
    ZMC_BOOL ContinuousMotion;
    ZMC_BOOL SynchronizedMotion;
    ZMC_BOOL Homing;
    ZMC_BOOL ConstantVelocity;
    ZMC_BOOL Accelerating;
    ZMC_BOOL Decelerating;
    ZMC_BOOL FBErrorOccurred;
}FbtReadStatusStruct;

typedef struct FbtReadActualPositionStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Position;
}FbtReadActualPositionStruct;

typedef struct FbtReadActualTorqueStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Torque;
}FbtReadActualTorqueStruct;

typedef struct FbtReadActualVelocityStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Velocity;
}FbtReadActualVelocityStruct;

typedef struct FbtReadAxisErrorStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_BOOL AxisError;
    ZMC_DWORD AxisErrorID;
    ZMC_BOOL SWEndSwitchActive;
}FbtReadAxisErrorStruct;

typedef struct FbtReadSetPositionStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Position;
}FbtReadSetPositionStruct;

typedef struct FbtReadBoolParameterStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_DINT ParameterNumber;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_BOOL Value;
}FbtReadBoolParameterStruct;

typedef struct FbtReadParameterStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_DINT ParameterNumber;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Value;
}FbtReadParameterStruct;

typedef struct FbtReadCoeParameterStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    // Parameter number encoded by zmc_parameter_number_coe (CoE objects only)
    ZMC_DINT ParameterNumber;
    // Watchdog budget in cycles; 0 selects the built-in default
    ZMC_DINT TimeoutCycles;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Value;
}FbtReadCoeParameterStruct;

typedef struct FbtWriteBoolParameterStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_DINT ParameterNumber;
    ZMC_BOOL Value;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtWriteBoolParameterStruct;

typedef struct FbtWriteParameterStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_DINT ParameterNumber;
    ZMC_LREAL Value;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtWriteParameterStruct;

typedef struct FbtWriteCoeParameterStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    // Parameter number encoded by zmc_parameter_number_coe (CoE objects only)
    ZMC_DINT ParameterNumber;
    ZMC_LREAL Value;
    // Watchdog budget in cycles; 0 selects the built-in default
    ZMC_DINT TimeoutCycles;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtWriteCoeParameterStruct;

typedef struct FbtStopStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtStopStruct;

typedef struct FbtHaltStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtHaltStruct;

typedef struct FbtMoveAbsoluteStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_DIRECTION Direction;
    MC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtMoveAbsoluteStruct;

typedef struct FbtMoveAdditiveStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Distance;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtMoveAdditiveStruct;

typedef struct FbtMoveRelativeStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Distance;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtMoveRelativeStruct;

typedef struct FbtMoveVelocityStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_DIRECTION Direction;
    MC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL InVelocity;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtMoveVelocityStruct;

typedef struct FbtMoveContinuousAbsoluteStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL EndVelocity;
    MC_DIRECTION EndVelocityDirection;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_DIRECTION Direction;
    ZMC_BOOL AdaptEndVelToAvoidOvershoot;
    // output
    ZMC_BOOL InEndVelocity;
    ZMC_BOOL PositionReached;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtMoveContinuousAbsoluteStruct;

typedef struct FbtMoveContinuousRelativeStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Distance;
    ZMC_LREAL Velocity;
    ZMC_LREAL EndVelocity;
    MC_DIRECTION EndVelocityDirection;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    ZMC_BOOL AdaptEndVelToAvoidOvershoot;
    // output
    ZMC_BOOL InEndVelocity;
    ZMC_BOOL DistanceTravelled;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtMoveContinuousRelativeStruct;

typedef struct FbtMoveSuperImposedStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_BOOL Abort;
    ZMC_LREAL Distance;
    ZMC_LREAL VelocityDiff;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL DistanceTravelled;
    ZMC_LREAL SuperImposedVelocity;
    ZMC_LREAL SuperImposedAcceleration;
}FbtMoveSuperImposedStruct;

typedef struct FbtHaltSuperImposedStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtHaltSuperImposedStruct;

typedef struct FbtPositionProfileStruct{
    void* pImpl;

    AXIS_REF Axis;
    MC_TP_REF TimePosition;
    // input
    ZMC_BOOL Execute;
    ZMC_INT ArraySize;
    ZMC_LREAL PositionScale;
    ZMC_LREAL Offset;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtPositionProfileStruct;

typedef struct FbtAccelerationProfileStruct{
    void* pImpl;

    AXIS_REF Axis;
    MC_TA_REF TimeAcceleration;
    // input
    ZMC_BOOL Execute;
    ZMC_INT ArraySize;
    ZMC_LREAL AccelerationScale;
    ZMC_LREAL Offset;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtAccelerationProfileStruct;

typedef struct FbtVelocityProfileStruct{
    void* pImpl;

    AXIS_REF Axis;
    MC_TV_REF TimeVelocity;
    // input
    ZMC_BOOL Execute;
    ZMC_INT ArraySize;
    ZMC_LREAL VelocityScale;
    ZMC_LREAL Offset;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtVelocityProfileStruct;

typedef struct FbtHomeStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Position;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtHomeStruct;

typedef struct FbtHomingStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL fHomePosition;
    ZMC_LREAL fVelocitySlow;
    ZMC_LREAL fVelocityFast;
    ZMC_LREAL fAcceleration;
    ZMC_LREAL fDeceleration;
    ZMC_LREAL fJerk;
    MC_DIRECTION nDirection;
    ZMC_BOOL bReferenceSwitch;
    ZMC_LREAL fSignalDelay;
    MC_HOMING_MODE nHomingMode;
    ZMC_BOOL bReturnToZero;
    ZMC_BOOL bIndexOccured;
    ZMC_LREAL fIndexPosition;
    ZMC_BOOL bIgnoreHWLimit;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
    ZMC_BOOL bStartLatchingIndex;
}FbtHomingStruct;

typedef struct FbtJogStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL JogForward;
    ZMC_BOOL JogBackward;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtJogStruct;

typedef struct FbtInchStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL InchForward;
    ZMC_BOOL InchBackward;
    ZMC_LREAL Distance;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtInchStruct;

typedef struct FbtGearInStruct{
    void* pImpl;

    AXIS_REF Master;
    AXIS_REF Slave;
    // input
    ZMC_BOOL Execute;
    ZMC_DINT RatioNumerator;
    ZMC_UDINT RatioDenominator;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL InGear;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtGearInStruct;

typedef struct FbtGearInPosStruct{
    void* pImpl;

    AXIS_REF Master;
    AXIS_REF Slave;
    // input
    ZMC_BOOL Execute;
    ZMC_DINT RatioNumerator;
    ZMC_DINT RatioDenominator;
    ZMC_LREAL MasterSyncPosition;
    ZMC_LREAL SlaveSyncPosition;
    ZMC_LREAL MasterStartDistance;
    MC_BUFFER_MODE BufferMode;
    ZMC_BOOL AvoidReversal;
    // output
    ZMC_BOOL StartSync;
    ZMC_BOOL InSync;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtGearInPosStruct;

typedef struct FbtGearOutStruct{
    void* pImpl;

    AXIS_REF Slave;
    // input
    ZMC_BOOL Execute;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtGearOutStruct;

typedef struct FbtPhasingStruct{
    void* pImpl;

    AXIS_REF Master;
    AXIS_REF Slave;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL PhaseShift;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtPhasingStruct;

typedef struct FbtSetTorqueStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bEnable;
    ZMC_LREAL fTorque;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
}FbtSetTorqueStruct;

typedef struct FbtFollowPositionStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL fSetPosition;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    MC_ERROR iErrorID;
}FbtFollowPositionStruct;

typedef struct FbtFollowPositionVelocityStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL fSetPosition;
    ZMC_LREAL fSetVelocity;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    MC_ERROR iErrorID;
}FbtFollowPositionVelocityStruct;

typedef struct FbtFollowVelocityStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL fSetVelocity;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    MC_ERROR iErrorID;
}FbtFollowVelocityStruct;

typedef struct FbtFollowSetValuesStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_BOOL bAbort;
    ZMC_DWORD dwValueMask;
    ZMC_LREAL fSetPosition;
    ZMC_LREAL fSetVelocity;
    ZMC_LREAL fSetAcceleration;
    ZMC_LREAL fSetJerk;
    ZMC_LREAL fSetTorque;
    ZMC_LREAL fSetCurrent;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    MC_ERROR iErrorID;
}FbtFollowSetValuesStruct;

typedef struct FbtTrackAxisStruct{
    void* pImpl;

    AXIS_REF Master;
    AXIS_REF Slave;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    MC_ERROR iErrorID;
    ZMC_BOOL bInSync;
}FbtTrackAxisStruct;

typedef struct FbtTrackSetValuesStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL fSetPosition;
    ZMC_LREAL fSetVelocity;
    ZMC_LREAL fSetAcceleration;
    ZMC_LREAL fSetJerk;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    ZMC_LREAL SignalModuloPeriod;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    MC_ERROR iErrorID;
    ZMC_BOOL bInSync;
}FbtTrackSetValuesStruct;

typedef struct FbtCamInStruct{
    void* pImpl;

    AXIS_REF Master;
    AXIS_REF Slave;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL MasterOffset;
    ZMC_LREAL SlaveOffset;
    ZMC_LREAL MasterScaling;
    ZMC_LREAL SlaveScaling;
    MC_STARTMODE StartMode;
    MC_CAM_ID CamTableID;
    ZMC_LREAL VelocityDiff;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    ZMC_LREAL TappetHysteresis;
    // output
    ZMC_BOOL InSync;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_BOOL EndOfProfile;
    SMC_TappetData Tappets;
}FbtCamInStruct;



typedef struct FbtCamCalcStruct{
    void* pImpl;
    MC_CAM_REF CamTable;
    // input
    ZMC_BOOL Enable;
    ZMC_LREAL MasterPosition;
    ZMC_LREAL MasterVelocity;
    ZMC_LREAL MasterAcceleration;

    ZMC_LREAL MasterOffset;
    ZMC_LREAL SlaveOffset;
    ZMC_LREAL MasterScaling;
    ZMC_LREAL SlaveScaling;
    ZMC_LREAL MasterEngagePos;
    ZMC_LREAL SlaveEngagePos;
    ZMC_BOOL MasterAbsolute;
    ZMC_BOOL SlaveAbsolute;
    // output
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL SlavePosition;
    ZMC_LREAL SlaveVelocity;
    ZMC_LREAL SlaveAcceleration;
}FbtCamCalcStruct;


typedef struct FbtCamOutStruct{
    void* pImpl;

    AXIS_REF Slave;
    // input
    ZMC_BOOL Execute;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtCamOutStruct;

typedef struct FbtCamTableSelectStruct{
    void* pImpl;

    AXIS_REF Master;
    AXIS_REF Slave;
    MC_CAM_REF CamTable;
    // input
    ZMC_BOOL Execute;
    ZMC_BOOL Periodic;
    ZMC_BOOL MasterAbsolute;
    ZMC_BOOL SlaveAbsolute;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    MC_CAM_ID CamTableID;
}FbtCamTableSelectStruct;

typedef struct FbtGetTappetValueStruct{
    void* pImpl;

    SMC_TappetData_REF Tappets;
    // input
    ZMC_INT iID;
    ZMC_BOOL bInitValue;
    ZMC_BOOL bSetInitValueAtReset;
    // output
    ZMC_BOOL bTappet;
}FbtGetTappetValueStruct;

typedef struct FbtCamRegisterStruct{
    void* pImpl;

    AXIS_REF Master;
    MC_CAM_REF CamTable;
    TAPPET_BITS_REF bTappet;
    // input
    ZMC_BOOL Enable;
    ZMC_LREAL MasterOffset;
    ZMC_LREAL MasterScaling;
    ZMC_LREAL TappetHysteresis;
    ZMC_LREAL DeadTimeCompensation;
    // output
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_BOOL EndOfProfile;
}FbtCamRegisterStruct;

typedef struct FbtCAMBoundsStruct{
    void* pImpl;

    MC_CAM_REF CAM;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL dMasterVelMax;
    ZMC_LREAL dMasterAccMax;
    ZMC_LREAL dMasterScaling;
    ZMC_LREAL dSlaveScaling;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
    ZMC_LREAL dMaxPos;
    ZMC_LREAL dMinPos;
    ZMC_LREAL dMaxVel;
    ZMC_LREAL dMinVel;
    ZMC_LREAL dMaxAccDec;
    ZMC_LREAL dMinAccDec;
}FbtCAMBoundsStruct;

typedef struct FbtCAMBounds_PosStruct{
    void* pImpl;

    MC_CAM_REF CAM;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL dMasterVelMax;
    ZMC_LREAL dMasterAccMax;
    ZMC_LREAL dMasterScaling;
    ZMC_LREAL dSlaveScaling;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
    ZMC_LREAL dMaxPos;
    ZMC_LREAL dMinPos;
}FbtCAMBounds_PosStruct;

typedef struct FbtGetCamSlaveSetPositionStruct{
    void* pImpl;

    AXIS_REF Master;
    AXIS_REF Slave;
    // input
    ZMC_BOOL Enable;
    ZMC_LREAL MasterOffset;
    ZMC_LREAL SlaveOffset;
    ZMC_LREAL MasterScaling;
    ZMC_LREAL SlaveScaling;
    MC_CAM_ID CamTableID;
    // output
    ZMC_LREAL fStartPosition;
    ZMC_LREAL fStartVelocity;
    ZMC_LREAL fStartAcceleration;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtGetCamSlaveSetPositionStruct;

typedef struct FbtAbortTriggerStruct{
    void* pImpl;

    AXIS_REF Axis;
    TRIGGER_REF TriggerInput;
    // input
    ZMC_BOOL Execute;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtAbortTriggerStruct;

typedef struct FbtTouchProbeStruct{
    void* pImpl;

    AXIS_REF Axis;
    TRIGGER_REF TriggerInput;
    // input
    ZMC_BOOL Execute;
    ZMC_BOOL WindowOnly;
    ZMC_LREAL FirstPosition;
    ZMC_LREAL LastPosition;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL RecordedPosition;
    ZMC_BOOL CommandAborted;
}FbtTouchProbeStruct;

typedef struct FbtDigitalCamSwitchStruct{
    void* pImpl;

    AXIS_REF Axis;
    MC_CAMSWITCH_REF Switches;
    MC_OUTPUT_REF Outputs;
    MC_TRACK_REF TrackOptions;
    // input
    ZMC_BOOL Enable;
    ZMC_DWORD EnableMask;
    MC_TAPPETMODE TappetMode;
    // output
    ZMC_BOOL InOperation;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_INT SwitchCorrupted;
}FbtDigitalCamSwitchStruct;

typedef struct FbtReadCAMStruct{
    void* pImpl;

    // input
    ZMC_BOOL bExecute;
    ZMC_STRING_80 sFileName;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR ErrorID;
    MC_CAM_REF CAM;
}FbtReadCAMStruct;

typedef struct FbtWriteCAMStruct{
    void* pImpl;

    MC_CAM_REF CAM;
    // input
    ZMC_BOOL bExecute;
    ZMC_STRING_80 sFileName;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR ErrorID;
}FbtWriteCAMStruct;

typedef struct FbtReadSetValuesStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Jerk;
}FbtReadSetValuesStruct;

typedef struct FbtReadActualValuesStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Jerk;
}FbtReadActualValuesStruct;

typedef struct FbtReadSetDriveValuesStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    int Position;
    int Velocity;
    int Acceleration;
    int Jerk;
}FbtReadSetDriveValuesStruct;

typedef struct FbtReadActualDriveValuesStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    int Position;
    int Velocity;
    int Acceleration;
    int Jerk;
}FbtReadActualDriveValuesStruct;

typedef struct FbtReadValuesStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
	MC_ValueSelect Source;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Jerk;
}FbtReadValuesStruct;


typedef struct FbtReadPowerParamStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    uint16_t ControlWord;
    uint16_t StatusWord;
    int SetPosition;
    int ActPosition;
}FbtReadPowerParamStruct;

// GetTrackingError
typedef struct FbtGetTrackingErrorStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable; 
    ZMC_DWORD TimeStamp;
    ZMC_BYTE DeadTimeCycles;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL ActTrackingError;
    ZMC_LREAL MaxTrackingError;
    ZMC_DWORD TimeAtMax;
} FbtGetTrackingErrorStruct;

typedef struct FbtGetTravelledDistanceStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output 
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL Distance;
} FbtGetTravelledDistanceStruct;


typedef struct FbtGetMaxSetAccDecStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_DWORD TimeStamp;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL MaxAcceleration;
    ZMC_DWORD TimeAtMax;
} FbtGetMaxSetAccDecStruct;

// GetMaxSetVelocity 
typedef struct FbtGetMaxSetVelocityStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_DWORD TimeStamp;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL MaxVelocity;
    ZMC_DWORD TimeAtMax;
} FbtGetMaxSetVelocityStruct;

typedef struct FbtGetOverrideStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    ZMC_LREAL VelFactor;
    ZMC_LREAL AccFactor;
    ZMC_LREAL JerkFactor;
} FbtGetOverrideStruct;

typedef struct FbtSetOverrideStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_LREAL VelFactor;
    ZMC_LREAL AccFactor;
    ZMC_LREAL JerkFactor;
    // output
    ZMC_BOOL Enabled;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
} FbtSetOverrideStruct;

typedef struct FbtCheckLimitsStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bEnable;
    ZMC_BOOL bCheckVel;
    ZMC_BOOL bCheckAccDec;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR iErrorID;
    ZMC_BOOL bLimitsExceeded;
} FbtCheckLimitsStruct;

typedef struct FbtInPositionStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bEnable;
    ZMC_LREAL fPosWindow;
    ZMC_LREAL fPosTime;
    ZMC_LREAL fTimeOut;
    // output
    ZMC_BOOL bInPosition;
    ZMC_BOOL bBusy;
    ZMC_BOOL bTimeOut;
    ZMC_BOOL bError;
    MC_ERROR iErrorID;
} FbtInPositionStruct;

typedef struct  FbtMoveDiscreteStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL PosOrDist;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_DIRECTION Direction;
    MC_BUFFER_MODE BufferMode;
    // o for absolute, 1 for relative
    int MotionType;
    // output
    ZMC_BOOL Active;
    ZMC_BOOL Busy;
    ZMC_BOOL Done;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
} FbtMoveDiscreteStruct;

typedef struct  FbtMoveDiscreteStruct  FbtMoveDisOrVelStruct;

typedef struct  FbtMoveDisOrContStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL PosOrDist;
    ZMC_LREAL Velocity;
    ZMC_LREAL EndVelocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_DIRECTION Direction;
    MC_DIRECTION EndVelocityDirection;
    MC_BUFFER_MODE BufferMode;
    int MotionType;
    // output
    ZMC_BOOL Active;
    ZMC_BOOL Busy;
    ZMC_BOOL Done;
    ZMC_BOOL InVelocity;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
} FbtMoveDisOrContStruct;


// SetMovementType function block structure
typedef struct FbtSetMovementTypeStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_INT iMovementType;
    ZMC_LREAL fPositionPeriod;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bError;
    MC_ERROR eErrorID;
} FbtSetMovementTypeStruct;

// SetRampType function block structure
typedef struct FbtSetRampTypeStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    MC_RAMPTYPE eRampType;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bError;
    MC_ERROR eErrorID;
} FbtSetRampTypeStruct;

// ChangeDynamicLimits function block structure
typedef struct FbtChangeDynamicLimitsStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL fMaxVelocity;
    ZMC_LREAL fMaxAcceleration;
    ZMC_LREAL fMaxDeceleration;
    ZMC_LREAL fMaxJerk;
    ZMC_LREAL fMaxTorque;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR ErrorID;
} FbtChangeDynamicLimitsStruct;

// ChangeGearingRatio function block structure
typedef struct FbtChangeGearingRatioStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_DWORD dwRatioTechUnitsDenom;
    ZMC_DINT iRatioTechUnitsNum;
    ZMC_LREAL fPositionPeriod;
    ZMC_INT iMovementType;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
} FbtChangeGearingRatioStruct;


typedef struct FbtChangeRatioStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_DWORD dwRatioTechUnitsDenom;
    ZMC_DINT iRatioTechUnitsNum;
    ZMC_LREAL fPositionPeriod;
    ZMC_INT iMovementType;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
} FbtChangeRatioStruct;

// SetSoftwareLimits function block structure
typedef struct FbtSetSoftwareLimitsStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_BOOL SWL_Activated;
    ZMC_LREAL SWL_Positive;
    ZMC_LREAL SWL_Negative;
    ZMC_BOOL SWL_Error_Decelerate;
    ZMC_LREAL SWL_Error_Deceleration;
    ZMC_LREAL SWL_Error_MaxDistance;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bError;
    MC_ERROR ErrorID;
} FbtSetSoftwareLimitsStruct;

// SetControllerMode function block structure
typedef struct FbtSetControllerModeStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    MC_CONTROLLER_MODE nControllerMode;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
} FbtSetControllerModeStruct;



// ReinitDrive function block structure
typedef struct FbtReinitDriveStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_BOOL bVirtual;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
} FbtReinitDriveStruct;

// SetCustomRampType function block structure
typedef struct FbtSetCustomRampTypeStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    MC_TG_IRAMPTYPE iRampType1;
    MC_TG_IRAMPTYPE iRampType2;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bError;
    MC_ERROR eErrorID;
} FbtSetCustomRampTypeStruct;

// SetAdditionalConversionFactors function block structure
typedef struct FbtSetAdditionalConversionFactorsStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL TorqueFactor;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
} FbtSetAdditionalConversionFactorsStruct;

// MeasureDistance function block structure
typedef struct FbtMeasureDistanceStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    // output
    ZMC_LREAL fDistance;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
} FbtMeasureDistanceStruct;

// AxisDiagnosticLog function block structure
typedef struct FbtAxisDiagnosticLogStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_BOOL bCloseFile;
    ZMC_STRING_80 sFileName; // STRING(255)
    ZMC_BOOL bSetPosition;
    ZMC_BOOL bActPosition;
    ZMC_BOOL bSetVelocity;
    ZMC_BOOL bActVelocity;
    ZMC_BOOL bSetAcceleration;
    ZMC_BOOL bActAcceleration;
    ZMC_BYTE bySeparatorChar;
    ZMC_STRING_80 sRecordSeparatorString; // STRING(3)
    MC_LOGGERMODE eMode;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR ErrorID;
    ZMC_BOOL bRecording;
} FbtAxisDiagnosticLogStruct;

// CheckAxisCommunication function block structure
typedef struct FbtCheckAxisCommunicationStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bEnable;
    // output
    ZMC_BOOL bValid;
    ZMC_BOOL bError;
    MC_ERROR eErrorID;
    ZMC_BOOL bOperational;
    MC_CommunicationState eComState;
    ZMC_WORD wComState;
} FbtCheckAxisCommunicationStruct;

// BacklashCompensation function block structure
typedef struct FbtBacklashCompensationStruct {
    void* pImpl;

    AXIS_REF Master;
    AXIS_REF Slave;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL fBacklash;
    ZMC_LREAL fCompensationVel;
    ZMC_LREAL fCompensationAcc;
    ZMC_LREAL fCompensationDec;
    ZMC_LREAL fCompensationJerk;
    MC_BACKLASH_MODE eBacklashMode;
    MC_BACKLASH_STARTSTATE eBacklashStartState;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    MC_ERROR iErrorID;
    ZMC_BOOL bCompensating;
} FbtBacklashCompensationStruct;

// BrakeStatus function block structure
typedef struct FbtBrakeStatusStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bEnable;
    // output
    ZMC_BOOL bValid;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
    ZMC_BOOL bBrakeClosed;
} FbtBrakeStatusStruct;

// BrakeControl function block structure
typedef struct FbtBrakeControlStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    MC_BrakeSetState eSetBrake;
    ZMC_LREAL fTimeOut;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    MC_ERROR nErrorID;
} FbtBrakeControlStruct;

// VirtualAxis function block structure
typedef struct FbtVirtualAxisStruct {
    void* pImpl;

    // input
    ZMC_WORD wDriveID;
    ZMC_INT iMovementType;
    ZMC_LREAL fPositionPeriod;
    MC_RAMPTYPE eRampType;
    ZMC_LREAL fSWMaxVelocity;
    ZMC_LREAL fSWMaxAcceleration;
    ZMC_LREAL fSWMaxDeceleration;
    ZMC_LREAL fSWMaxJerk;
    ZMC_LREAL fSWMaxTorque;
    ZMC_LREAL fSWLimitPositive;
    ZMC_LREAL fSWLimitNegative;
    ZMC_LREAL fSWLimitDeceleration;
    ZMC_BOOL bSWLimitEnable;
    ZMC_LREAL fSWErrorMaxDistance;
    // output
    AXIS_REF Axis;
    ZMC_BOOL bError;
} FbtVirtualAxisStruct;

// LogicalAxis function block structure
typedef struct FbtLogicalAxisStruct {
    void* pImpl;

    // input
    ZMC_BOOL Execute;
    ZMC_WORD wDriveID;
    ZMC_INT iMovementType;
    ZMC_LREAL fPositionPeriod;
    MC_ValueSelect eValueReference;
    ZMC_LREAL usiFilterDepthPosition;
    ZMC_LREAL usiFilterDepthVelocity;
    ZMC_LREAL fTimeShiftCycles;
    AXIS_REF RealAxis;
    // output
    ZMC_BOOL Initialized;
    ZMC_BOOL InSteadyState;
    ZMC_BOOL Error;
    AXIS_REF Axis;
} FbtLogicalAxisStruct;

// EstimateDeadTime function block structure
typedef struct FbtEstimateDeadTimeStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy; // Busy,Error, ErrorID add by csy@zlg
    ZMC_BOOL Error; 
    ZMC_ERROR ErrorID;
    ZMC_LREAL DeadTimeCycles;
} FbtEstimateDeadTimeStruct;

// ChangeAxisScalingLinear function block structure
typedef struct FbtChangeAxisScalingLinearStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_UDINT Increments;
    ZMC_UDINT TechnicalUnits;
    ZMC_BOOL InvertDirection;
    ZMC_LREAL ModuloPeriod;
    ZMC_INT AxisType;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
} FbtChangeAxisScalingLinearStruct;

// ChangeAxisScalingRotary function block structure
typedef struct FbtChangeAxisScalingRotaryStruct {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_UDINT Increments;
    ZMC_UDINT MotorTurns1;
    ZMC_UDINT MotorTurns2;
    ZMC_UDINT GearOutput1;
    ZMC_UDINT GearOutput2;
    ZMC_UDINT TechnicalUnits;
    ZMC_BOOL InvertDirection;
    ZMC_LREAL ModuloPeriod;
    ZMC_INT AxisType;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
} FbtChangeAxisScalingRotaryStruct;

#pragma pack(pop)

class FbtPower:public FbtPowerStruct{
public:
    FbtPower();
    ~FbtPower();
    void runCycle();
};

class FbtReset:public FbtResetStruct{
public:
    FbtReset();
    ~FbtReset();
    void runCycle();
};

class FbtSetPosition:public FbtSetPositionStruct{
public:
    FbtSetPosition();
    ~FbtSetPosition();
    void runCycle();
};

class FbtReadStatus:public FbtReadStatusStruct{
public:
    FbtReadStatus();
    ~FbtReadStatus();
    void runCycle();
};

class FbtReadActualPosition:public FbtReadActualPositionStruct{
public:
    FbtReadActualPosition();
    ~FbtReadActualPosition();
    void runCycle();
};

class FbtReadActualTorque:public FbtReadActualTorqueStruct{
public:
    FbtReadActualTorque();
    ~FbtReadActualTorque();
    void runCycle();
};

class FbtReadActualVelocity:public FbtReadActualVelocityStruct{
public:
    FbtReadActualVelocity();
    ~FbtReadActualVelocity();
    void runCycle();
};

class FbtReadAxisError:public FbtReadAxisErrorStruct{
public:
    FbtReadAxisError();
    ~FbtReadAxisError();
    void runCycle();
};

class FbtReadSetPosition:public FbtReadSetPositionStruct{
public:
    FbtReadSetPosition();
    ~FbtReadSetPosition();
    void runCycle();
};
class FbtReadCommandPosition:public FbtReadSetPosition{};

class FbtReadBoolParameter:public FbtReadBoolParameterStruct{
public:
    FbtReadBoolParameter();
    ~FbtReadBoolParameter();
    void runCycle();
};

class FbtReadParameter:public FbtReadParameterStruct{
public:
    FbtReadParameter();
    ~FbtReadParameter();
    void runCycle();
};

class FbtReadCoeParameter:public FbtReadCoeParameterStruct{
public:
    FbtReadCoeParameter();
    ~FbtReadCoeParameter();
    void runCycle();
};

class FbtWriteBoolParameter:public FbtWriteBoolParameterStruct{
public:
    FbtWriteBoolParameter();
    ~FbtWriteBoolParameter();
    void runCycle();
};

class FbtWriteParameter:public FbtWriteParameterStruct{
public:
    FbtWriteParameter();
    ~FbtWriteParameter();
    void runCycle();
};

class FbtWriteCoeParameter:public FbtWriteCoeParameterStruct{
public:
    FbtWriteCoeParameter();
    ~FbtWriteCoeParameter();
    void runCycle();
};

class FbtStop:public FbtStopStruct{
public:
    FbtStop();
    ~FbtStop();
    void runCycle();
};

class FbtHalt:public FbtHaltStruct{
public:
    FbtHalt();
    ~FbtHalt();
    void runCycle();
};

class FbtMoveAbsolute:public FbtMoveAbsoluteStruct{
public:
    FbtMoveAbsolute();
    ~FbtMoveAbsolute();
    void runCycle();
};

class FbtMoveAdditive:public FbtMoveAdditiveStruct{
public:
    FbtMoveAdditive();
    ~FbtMoveAdditive();
    void runCycle();
};

class FbtMoveRelative:public FbtMoveRelativeStruct{
public:
    FbtMoveRelative();
    ~FbtMoveRelative();
    void runCycle();
};

class FbtMoveVelocity:public FbtMoveVelocityStruct{
public:
    FbtMoveVelocity();
    ~FbtMoveVelocity();
    void runCycle();
};

class FbtMoveContinuousAbsolute:public FbtMoveContinuousAbsoluteStruct{
public:
    FbtMoveContinuousAbsolute();
    ~FbtMoveContinuousAbsolute();
    void runCycle();
};

class FbtMoveContinuousRelative:public FbtMoveContinuousRelativeStruct{
public:
    FbtMoveContinuousRelative();
    ~FbtMoveContinuousRelative();
    void runCycle();
};

class FbtMoveSuperImposed:public FbtMoveSuperImposedStruct{
public:
    FbtMoveSuperImposed();
    ~FbtMoveSuperImposed();
    void runCycle();
};

class FbtHaltSuperImposed:public FbtHaltSuperImposedStruct{
public:
    FbtHaltSuperImposed();
    ~FbtHaltSuperImposed();
    void runCycle();
};

class FbtPositionProfile:public FbtPositionProfileStruct{
public:
    FbtPositionProfile();
    ~FbtPositionProfile();
    void runCycle();
};

class FbtAccelerationProfile:public FbtAccelerationProfileStruct{
public:
    FbtAccelerationProfile();
    ~FbtAccelerationProfile();
    void runCycle();
};

class FbtVelocityProfile:public FbtVelocityProfileStruct{
public:
    FbtVelocityProfile();
    ~FbtVelocityProfile();
    void runCycle();
};

class FbtHome:public FbtHomeStruct{
public:
    FbtHome();
    ~FbtHome();
    void configureSdo(ZMC_DINT mode, ZMC_LREAL offset, ZMC_LREAL velocityFast,
                      ZMC_LREAL velocitySlow, ZMC_LREAL acceleration);
    void disableSdo();
    void setSkipPosition(bool skipPosition);
    void setTimeoutMs(ZMC_DINT timeoutMs);
    void runCycle();
};

class FbtHoming:public FbtHomingStruct{
public:
    FbtHoming();
    ~FbtHoming();
    void runCycle();
};

class FbtJog:public FbtJogStruct{
public:
    FbtJog();
    ~FbtJog();
    void runCycle();
};

class FbtInch:public FbtInchStruct{
public:
    FbtInch();
    ~FbtInch();
    void runCycle();
};

class FbtGearIn:public FbtGearInStruct{
public:
    FbtGearIn();
    ~FbtGearIn();
    void runCycle();
};

class FbtGearInPos:public FbtGearInPosStruct{
public:
    FbtGearInPos();
    ~FbtGearInPos();
    void runCycle();
};

class FbtGearOut:public FbtGearOutStruct{
public:
    FbtGearOut();
    ~FbtGearOut();
    void runCycle();
};

class FbtPhasing:public FbtPhasingStruct{
public:
    FbtPhasing();
    ~FbtPhasing();
    void runCycle();
};

class FbtSetTorque:public FbtSetTorqueStruct{
public:
    FbtSetTorque();
    ~FbtSetTorque();
    void runCycle();
};

class FbtFollowPosition:public FbtFollowPositionStruct{
public:
    FbtFollowPosition();
    ~FbtFollowPosition();
    void runCycle();
};

class FbtFollowPositionVelocity:public FbtFollowPositionVelocityStruct{
public:
    FbtFollowPositionVelocity();
    ~FbtFollowPositionVelocity();
    void runCycle();
};

class FbtFollowVelocity:public FbtFollowVelocityStruct{
public:
    FbtFollowVelocity();
    ~FbtFollowVelocity();
    void runCycle();
};

class FbtFollowSetValues:public FbtFollowSetValuesStruct{
public:
    FbtFollowSetValues();
    ~FbtFollowSetValues();
    void runCycle();
};

class FbtTrackAxis:public FbtTrackAxisStruct{
public:
    FbtTrackAxis();
    ~FbtTrackAxis();
    void runCycle();
};

class FbtTrackSetValues:public FbtTrackSetValuesStruct{
public:
    FbtTrackSetValues();
    ~FbtTrackSetValues();
    void runCycle();
};

class FbtCamIn:public FbtCamInStruct{
public:
    FbtCamIn();
    ~FbtCamIn();
    void runCycle();
};

class FbtCamOut:public FbtCamOutStruct{
public:
    FbtCamOut();
    ~FbtCamOut();
    void runCycle();
};

class FbtCamTableSelect:public FbtCamTableSelectStruct{
public:
    FbtCamTableSelect();
    ~FbtCamTableSelect();
    void runCycle();
};

class FbtGetTappetValue:public FbtGetTappetValueStruct{
public:
    FbtGetTappetValue();
    ~FbtGetTappetValue();
    void runCycle();
};

class FbtCamRegister:public FbtCamRegisterStruct{
public:
    FbtCamRegister();
    ~FbtCamRegister();
    void runCycle();
};

class FbtCAMBounds:public FbtCAMBoundsStruct{
public:
    FbtCAMBounds();
    ~FbtCAMBounds();
    void runCycle();
};

class FbtCAMBounds_Pos:public FbtCAMBounds_PosStruct{
public:
    FbtCAMBounds_Pos();
    ~FbtCAMBounds_Pos();
    void runCycle();
};

class FbtGetCamSlaveSetPosition:public FbtGetCamSlaveSetPositionStruct{
public:
    FbtGetCamSlaveSetPosition();
    ~FbtGetCamSlaveSetPosition();
    void runCycle();
};

class FbtCamCalc:public FbtCamCalcStruct{
public:
    FbtCamCalc();
    ~FbtCamCalc();
    void runCycle();
};

class FbtAbortTrigger:public FbtAbortTriggerStruct{
public:
    FbtAbortTrigger();
    ~FbtAbortTrigger();
    void runCycle();
};

class FbtTouchProbe:public FbtTouchProbeStruct{
public:
    FbtTouchProbe();
    ~FbtTouchProbe();
    void runCycle();
};

class FbtDigitalCamSwitch:public FbtDigitalCamSwitchStruct{
public:
    FbtDigitalCamSwitch();
    ~FbtDigitalCamSwitch();
    void runCycle();
};

class FbtReadCAM:public FbtReadCAMStruct{
public:
    FbtReadCAM();
    ~FbtReadCAM();
    void runCycle();
};

class FbtWriteCAM:public FbtWriteCAMStruct{
public:
    FbtWriteCAM();
    ~FbtWriteCAM();
    void runCycle();
};

class FbtReadSetValues:public FbtReadSetValuesStruct{
public:
    FbtReadSetValues();
    ~FbtReadSetValues();
    void runCycle();
};

class FbtReadActualValues:public FbtReadActualValuesStruct{
public:
    FbtReadActualValues();
    ~FbtReadActualValues();
    void runCycle();
};

class FbtReadSetDriveValues:public FbtReadSetDriveValuesStruct{
public:
    FbtReadSetDriveValues();
    ~FbtReadSetDriveValues();
    void runCycle();
};

class FbtReadActualDriveValues:public FbtReadActualDriveValuesStruct{
public:
    FbtReadActualDriveValues();
    ~FbtReadActualDriveValues();
    void runCycle();
};

class FbtReadValues:public FbtReadValuesStruct{
public:
    FbtReadValues();
    ~FbtReadValues();
    void runCycle();
};

class FbtReadPowerParam:public FbtReadPowerParamStruct{
public:
    FbtReadPowerParam();
    ~FbtReadPowerParam();
    void runCycle();
};

class FbtGetTrackingError : public FbtGetTrackingErrorStruct {
public:
    FbtGetTrackingError();
    ~FbtGetTrackingError();
    void runCycle();
};

class FbtGetTravelledDistance : public FbtGetTravelledDistanceStruct {
public:
    FbtGetTravelledDistance();
    ~FbtGetTravelledDistance();
    void runCycle();
};

class FbtGetMaxSetAccDec : public FbtGetMaxSetAccDecStruct {
public:
    FbtGetMaxSetAccDec();
    ~FbtGetMaxSetAccDec();
    void runCycle();
};

class FbtGetMaxSetVelocity : public FbtGetMaxSetVelocityStruct {
public:
    FbtGetMaxSetVelocity();
    ~FbtGetMaxSetVelocity();
    void runCycle();
};

class FbtGetOverride : public FbtGetOverrideStruct {
public:
    FbtGetOverride();
    ~FbtGetOverride();
    void runCycle();
};

class FbtSetOverride : public FbtSetOverrideStruct {
public:
    FbtSetOverride();
    ~FbtSetOverride();
    void runCycle();
};

class FbtCheckLimits : public FbtCheckLimitsStruct {
public:
    FbtCheckLimits();
    ~FbtCheckLimits();
    void runCycle();
};

class FbtInPosition : public FbtInPositionStruct {
public:
    FbtInPosition();
    ~FbtInPosition();
    void runCycle();
};

class FbtMoveDiscrete : public FbtMoveDiscreteStruct {
public:
    FbtMoveDiscrete();
    ~FbtMoveDiscrete();
    void runCycle();
};

class FbtMoveDisOrVel : public FbtMoveDisOrVelStruct {
public:
    FbtMoveDisOrVel();
    ~FbtMoveDisOrVel();
    void runCycle();
};

class FbtMoveDisOrCont : public FbtMoveDisOrContStruct {
public:
    FbtMoveDisOrCont();
    ~FbtMoveDisOrCont();
    void runCycle();
};

class FbtSetMovementType : public FbtSetMovementTypeStruct {
public:
    FbtSetMovementType();
    ~FbtSetMovementType();
    void runCycle();
};

class FbtSetRampType : public FbtSetRampTypeStruct {
public:
    FbtSetRampType();
    ~FbtSetRampType();
    void runCycle();
};

class FbtChangeDynamicLimits : public FbtChangeDynamicLimitsStruct {
public:
    FbtChangeDynamicLimits();
    ~FbtChangeDynamicLimits();
    void runCycle();
};

class FbtChangeGearingRatio : public FbtChangeGearingRatioStruct {
public:
    FbtChangeGearingRatio();
    ~FbtChangeGearingRatio();
    void runCycle();
};

class FbtChangeRatio : public FbtChangeRatioStruct {
public:
    FbtChangeRatio();
    ~FbtChangeRatio();
    void runCycle();
};

class FbtSetSoftwareLimits : public FbtSetSoftwareLimitsStruct {
public:
    FbtSetSoftwareLimits();
    ~FbtSetSoftwareLimits();
    void runCycle();
};

class FbtSetControllerMode : public FbtSetControllerModeStruct {
public:
    FbtSetControllerMode();
    ~FbtSetControllerMode();
    void runCycle();
};


class FbtReinitDrive : public FbtReinitDriveStruct {
public:
    FbtReinitDrive();
    ~FbtReinitDrive();
    void runCycle();
};

class FbtSetCustomRampType : public FbtSetCustomRampTypeStruct {
public:
    FbtSetCustomRampType();
    ~FbtSetCustomRampType();
    void runCycle();
};

class FbtSetAdditionalConversionFactors : public FbtSetAdditionalConversionFactorsStruct {
public:
    FbtSetAdditionalConversionFactors();
    ~FbtSetAdditionalConversionFactors();
    void runCycle();
};

class FbtMeasureDistance : public FbtMeasureDistanceStruct {
public:
    FbtMeasureDistance();
    ~FbtMeasureDistance();
    void runCycle();
};

class FbtAxisDiagnosticLog : public FbtAxisDiagnosticLogStruct {
public:
    FbtAxisDiagnosticLog();
    ~FbtAxisDiagnosticLog();
    void runCycle();
};

class FbtCheckAxisCommunication : public FbtCheckAxisCommunicationStruct {
public:
    FbtCheckAxisCommunication();
    ~FbtCheckAxisCommunication();
    void runCycle();
};

class FbtBacklashCompensation : public FbtBacklashCompensationStruct {
public:
    FbtBacklashCompensation();
    ~FbtBacklashCompensation();
    void runCycle();
};

class FbtBrakeStatus : public FbtBrakeStatusStruct {
public:
    FbtBrakeStatus();
    ~FbtBrakeStatus();
    void runCycle();
};

class FbtBrakeControl : public FbtBrakeControlStruct {
public:
    FbtBrakeControl();
    ~FbtBrakeControl();
    void runCycle();
};

class FbtVirtualAxis : public FbtVirtualAxisStruct {
public:
    FbtVirtualAxis();
    ~FbtVirtualAxis();
    void runCycle();
};

class FbtLogicalAxis : public FbtLogicalAxisStruct {
public:
    FbtLogicalAxis();
    ~FbtLogicalAxis();
    void runCycle();
};

class FbtEstimateDeadTime : public FbtEstimateDeadTimeStruct {
public:
    FbtEstimateDeadTime();
    ~FbtEstimateDeadTime();
    void runCycle();
};

class FbtChangeAxisScalingLinear : public FbtChangeAxisScalingLinearStruct {
public:
    FbtChangeAxisScalingLinear();
    ~FbtChangeAxisScalingLinear();
    void runCycle();
};

class FbtChangeAxisScalingRotary : public FbtChangeAxisScalingRotaryStruct {
public:
    FbtChangeAxisScalingRotary();
    ~FbtChangeAxisScalingRotary();
    void runCycle();
};

template<typename T>
inline void set_fb_vel_config(T& fb, double vel, double acc, double dec, double jerk){
    fb.Velocity = vel;
    fb.Acceleration = acc;
    fb.Deceleration = dec;
    fb.Jerk = jerk;
}

template<typename T>
inline void set_fb_acc_config(T& fb, double acc, double dec, double jerk){
    fb.Acceleration = acc;
    fb.Deceleration = dec;
    fb.Jerk = jerk;
}

void setFbImplContinousUpdate(void* pImpl, bool b);
void setFbImplFlagSpec(void* pImpl, bool b);
template <typename T>
inline void setFbContinousUpdate(T& obj, bool b) {
    setFbImplContinousUpdate(obj.pImpl, b);
}
void setFbImplVelEndLimit(void* pImpl, double val);
template <typename T>
inline void setFbVelEndLimit(T& obj, double val) {
    setFbImplVelEndLimit(obj.pImpl, val);
}

inline double getFbDistance(void* pFb){
    return reinterpret_cast<FbtMoveRelativeStruct*>(pFb)->Distance;
}
inline void setFbExecute(void* pFb, bool val){
    // only use for this struct { void* pImpl; AXIS_REF Axis; bool Execute;};
    // cant use for fb with Master and Slave
    reinterpret_cast<FbtMoveRelativeStruct*>(pFb)->Execute = val;
}

inline void setFbDistance(void* pFb, double val){
    reinterpret_cast<FbtMoveRelativeStruct*>(pFb)->Distance = val;
}
void shiftFbImplVelDistance(void* pImpl, double distance);

int getFbImplType(void* pFb);
int getFbImplStat(void* pFb);
int getFbImplErrorCode(void* pFb);

}
#endif
