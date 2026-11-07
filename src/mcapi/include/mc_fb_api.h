/**
 * History:
 * ================================================================
 * 2025-04-14 ChenSy <ChenSy@zlg.cn> created.
 *
 */
#ifndef _MC_FB_API_H_
#define _MC_FB_API_H_

#include "zmc_types_def.h"
#include "zmc_enum_cdef.h"
#include "mc_ref_def.h"
#include "AxisRef.h"
#include "zmc_common.h"

#ifndef AW_PLC_NATIVE_ALIGN
#define AW_PLC_NATIVE_ALIGN 8
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, AW_PLC_NATIVE_ALIGN)

typedef struct McPower{
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
    ZMC_ERROR ErrorID;
}McPower;

typedef struct McReset{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McReset;

typedef struct McSetPosition{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Position;
    ZMC_BOOL Mode;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McSetPosition;

typedef struct McReadStatus{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
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
}McReadStatus;

typedef struct McReadActualPosition{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL Position;
}McReadActualPosition;

typedef struct McReadActualTorque{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL Torque;
}McReadActualTorque;

typedef struct McReadActualVelocity{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL Velocity;
}McReadActualVelocity;

typedef struct McReadAxisError{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_BOOL AxisError;
    ZMC_DWORD AxisErrorID;
    ZMC_BOOL SWEndSwitchActive;
}McReadAxisError;

typedef struct McReadCommandPosition{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL Position;
}McReadCommandPosition;

typedef struct McReadCommandPosition McReadSetPosition;

typedef struct McReadBoolParameter{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_DINT ParameterNumber;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_BOOL Value;
}McReadBoolParameter;

typedef struct McReadParameter{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_DINT ParameterNumber;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL Value;
}McReadParameter;

typedef struct McWriteBoolParameter{
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
    ZMC_ERROR ErrorID;
}McWriteBoolParameter;

typedef struct McWriteParameter{
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
    ZMC_ERROR ErrorID;
}McWriteParameter;

typedef struct McStop{
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
    ZMC_ERROR ErrorID;
}McStop;

typedef struct McHalt{
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
    ZMC_ERROR ErrorID;
}McHalt;

typedef struct McMoveAbsolute{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    ZMC_DIRECTION Direction;
    ZMC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McMoveAbsolute;

typedef struct McMoveAdditive{
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
    ZMC_ERROR ErrorID;
}McMoveAdditive;

typedef struct McMoveRelative{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Distance;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    ZMC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McMoveRelative;

typedef struct McMoveVelocity{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    ZMC_DIRECTION Direction;
    ZMC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL InVelocity;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McMoveVelocity;

typedef struct McMoveContinuousAbsolute{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL EndVelocity;
    ZMC_DIRECTION EndVelocityDirection;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    ZMC_DIRECTION Direction;
    ZMC_BOOL AdaptEndVelToAvoidOvershoot;
    // output
    ZMC_BOOL InEndVelocity;
    ZMC_BOOL PositionReached;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McMoveContinuousAbsolute;

typedef struct McMoveContinuousRelative{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Distance;
    ZMC_LREAL Velocity;
    ZMC_LREAL EndVelocity;
    ZMC_DIRECTION EndVelocityDirection;
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
    ZMC_ERROR ErrorID;
}McMoveContinuousRelative;

typedef struct McMoveSuperImposed{
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
    ZMC_ERROR ErrorID;
    ZMC_LREAL DistanceTravelled;
    ZMC_LREAL SuperImposedVelocity;
    ZMC_LREAL SuperImposedAcceleration;
}McMoveSuperImposed;

typedef struct McHaltSuperImposed{
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
    ZMC_ERROR ErrorID;
}McHaltSuperImposed;

typedef struct McPositionProfile{
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
    ZMC_ERROR ErrorID;
}McPositionProfile;

typedef struct McAccelerationProfile{
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
    ZMC_ERROR ErrorID;
}McAccelerationProfile;

typedef struct McVelocityProfile{
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
    ZMC_ERROR ErrorID;
}McVelocityProfile;

typedef struct McHome{
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
    ZMC_ERROR ErrorID;
}McHome;

typedef struct McHoming{
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
    ZMC_DIRECTION nDirection;
    ZMC_BOOL bReferenceSwitch;
    ZMC_LREAL fSignalDelay;
    ZMC_HOMING_MODE nHomingMode;
    ZMC_BOOL bReturnToZero;
    ZMC_BOOL bIndexOccured;
    ZMC_LREAL fIndexPosition;
    ZMC_BOOL bIgnoreHWLimit;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    ZMC_ERROR nErrorID;
    ZMC_BOOL bStartLatchingIndex;
}McHoming;

typedef struct McJog{
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
    ZMC_ERROR ErrorID;
}McJog;

typedef struct McInch{
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
    ZMC_ERROR ErrorID;
}McInch;

typedef struct McGearIn{
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
    ZMC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL InGear;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McGearIn;

typedef struct McGearInPos{
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
    ZMC_BUFFER_MODE BufferMode;
    ZMC_BOOL AvoidReversal;
    // output
    ZMC_BOOL StartSync;
    ZMC_BOOL InSync;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McGearInPos;

typedef struct McGearOut{
    void* pImpl;

    AXIS_REF Slave;
    // input
    ZMC_BOOL Execute;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McGearOut;

typedef struct McPhasing{
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
    ZMC_ERROR ErrorID;
}McPhasing;

typedef struct McSetTorque{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bEnable;
    ZMC_LREAL fTorque;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    ZMC_ERROR nErrorID;
}McSetTorque;

typedef struct McFollowPosition{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL fSetPosition;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    ZMC_ERROR iErrorID;
}McFollowPosition;

typedef struct McFollowPositionVelocity{
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
    ZMC_ERROR iErrorID;
}McFollowPositionVelocity;

typedef struct McFollowVelocity{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_LREAL fSetVelocity;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    ZMC_ERROR iErrorID;
}McFollowVelocity;

typedef struct McFollowSetValues{
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
    ZMC_ERROR iErrorID;
}McFollowSetValues;

typedef struct McTrackAxis{
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
    ZMC_ERROR iErrorID;
    ZMC_BOOL bInSync;
}McTrackAxis;

typedef struct McTrackSetValues{
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
    ZMC_ERROR iErrorID;
    ZMC_BOOL bInSync;
}McTrackSetValues;

typedef struct McCamIn{
    void* pImpl;

    AXIS_REF Master;
    AXIS_REF Slave;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL MasterOffset;
    ZMC_LREAL SlaveOffset;
    ZMC_LREAL MasterScaling;
    ZMC_LREAL SlaveScaling;
    ZMC_STARTMODE StartMode;
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
    ZMC_ERROR ErrorID;
    ZMC_BOOL EndOfProfile;
    SMC_TappetData Tappets;
}McCamIn;

typedef struct McCamOut{
    void* pImpl;

    AXIS_REF Slave;
    // input
    ZMC_BOOL Execute;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McCamOut;

typedef struct McCamTableSelect{
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
    ZMC_ERROR ErrorID;
    MC_CAM_ID CamTableID;
}McCamTableSelect;

typedef struct McGetTappetValue{
    void* pImpl;

    SMC_TappetData_REF Tappets;
    // input
    ZMC_INT iID;
    ZMC_BOOL bInitValue;
    ZMC_BOOL bSetInitValueAtReset;
    // output
    ZMC_BOOL bTappet;
}McGetTappetValue;

typedef struct McCamRegister{
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
    ZMC_ERROR ErrorID;
    ZMC_BOOL EndOfProfile;
}McCamRegister;

typedef struct McCamCalc{
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
    ZMC_ERROR ErrorID;
    ZMC_LREAL SlavePosition;
    ZMC_LREAL SlaveVelocity;
    ZMC_LREAL SlaveAcceleration;
}McCamCalc;

typedef struct McCAMBounds{
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
    ZMC_ERROR nErrorID;
    ZMC_LREAL dMaxPos;
    ZMC_LREAL dMinPos;
    ZMC_LREAL dMaxVel;
    ZMC_LREAL dMinVel;
    ZMC_LREAL dMaxAccDec;
    ZMC_LREAL dMinAccDec;
}McCAMBounds;

typedef struct McCAMBounds_Pos{
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
    ZMC_ERROR nErrorID;
    ZMC_LREAL dMaxPos;
    ZMC_LREAL dMinPos;
}McCAMBounds_Pos;

typedef struct McGetCamSlaveSetPosition{
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
    ZMC_ERROR ErrorID;
}McGetCamSlaveSetPosition;

typedef struct McAbortTrigger{
    void* pImpl;

    AXIS_REF Axis;
    TRIGGER_REF TriggerInput;
    // input
    ZMC_BOOL Execute;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
}McAbortTrigger;

typedef struct McTouchProbe{
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
    ZMC_ERROR ErrorID;
    ZMC_LREAL RecordedPosition;
    ZMC_BOOL CommandAborted;
}McTouchProbe;

typedef struct McDigitalCamSwitch{
    void* pImpl;

    AXIS_REF Axis;
    MC_CAMSWITCH_REF Switches;
    MC_OUTPUT_REF Outputs;
    MC_TRACK_REF TrackOptions;
    // input
    ZMC_BOOL Enable;
    ZMC_DWORD EnableMask;
    ZMC_TAPPETMODE TappetMode;
    // output
    ZMC_BOOL InOperation;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_INT SwitchCorrupted;
}McDigitalCamSwitch;

typedef struct McReadCAM{
    void* pImpl;

    // input
    ZMC_BOOL bExecute;
    ZMC_STRING_80 sFileName;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    ZMC_ERROR ErrorID;
    MC_CAM_REF CAM;
}McReadCAM;

typedef struct McWriteCAM{
    void* pImpl;

    MC_CAM_REF CAM;
    // input
    ZMC_BOOL bExecute;
    ZMC_STRING_80 sFileName;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    ZMC_ERROR ErrorID;
}McWriteCAM;

typedef struct McReadSetValues{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Jerk;
}McReadSetValues;

typedef struct McReadActualValues{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Jerk;
}McReadActualValues;

typedef struct McReadValues{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_VALUESELECT Source;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Jerk;
}McReadValues;

typedef struct McReadPowerParam{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    uint16_t ControlWord;
    uint16_t StatusWord;
    int SetPosition;
    int ActPosition;
}McReadPowerParam;

typedef struct McReadSetDriveValues{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    int Position;
    int Velocity;
    int Acceleration;
    int Jerk;
}McReadSetDriveValues;

typedef struct McReadActualDriveValues{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    int Position;
    int Velocity;
    int Acceleration;
    int Jerk;
}McReadActualDriveValues;

typedef struct McGetTrackingError {
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
    ZMC_ERROR ErrorID;
    ZMC_LREAL ActTrackingError;
    ZMC_LREAL MaxTrackingError;
    ZMC_DWORD TimeAtMax;
} McGetTrackingError;

typedef struct McGetTravelledDistance {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output 
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL Distance;
} McGetTravelledDistance;

typedef struct McGetMaxSetAccDec {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_DWORD TimeStamp;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL MaxAcceleration;
    ZMC_DWORD TimeAtMax;
} McGetMaxSetAccDec;

typedef struct McGetMaxSetVelocity {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    ZMC_DWORD TimeStamp;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL MaxVelocity;
    ZMC_DWORD TimeAtMax;
} McGetMaxSetVelocity;

typedef struct McGetOverride {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Enable;
    // output
    ZMC_BOOL Valid;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
    ZMC_LREAL VelFactor;
    ZMC_LREAL AccFactor;
    ZMC_LREAL JerkFactor;
} McGetOverride;

typedef struct McSetOverride {
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
    ZMC_ERROR ErrorID;
} McSetOverride;

typedef struct McCheckLimits {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bEnable;
    ZMC_BOOL bCheckVel;
    ZMC_BOOL bCheckAccDec;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    ZMC_ERROR iErrorID;
    ZMC_BOOL bLimitsExceeded;
} McCheckLimits;

typedef struct McInPosition {
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
    ZMC_ERROR iErrorID;
} McInPosition;

typedef struct McSetMovementType {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_INT iMovementType;
    ZMC_LREAL fPositionPeriod;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bError;
    ZMC_ERROR eErrorID;
} McSetMovementType;

typedef struct McSetRampType {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_RAMPTYPE eRampType;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bError;
    ZMC_ERROR eErrorID;
} McSetRampType;

typedef struct McChangeDynamicLimits {
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
    ZMC_ERROR ErrorID;
} McChangeDynamicLimits;

typedef struct McChangeGearingRatio {
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
    ZMC_ERROR nErrorID;
} McChangeGearingRatio;

typedef struct McSetSoftwareLimits {
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
    ZMC_ERROR ErrorID;
} McSetSoftwareLimits;

typedef struct McSetControllerMode {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_CONTROLLER_MODE nControllerMode;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    ZMC_ERROR nErrorID;
} McSetControllerMode;

typedef struct McEstimateDeadTime {
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
} McEstimateDeadTime;

typedef struct McVirtualAxis {
    void* pImpl;

    // input
    ZMC_WORD wDriveID;
    ZMC_INT iMovementType;
    ZMC_LREAL fPositionPeriod;
    ZMC_RAMPTYPE eRampType;
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
} McVirtualAxis;

typedef struct McMeasureDistance {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    // output
    ZMC_LREAL fDistance;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    ZMC_ERROR nErrorID;
} McMeasureDistance;

typedef struct McSetAdditionalConversionFactors {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL TorqueFactor;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    ZMC_ERROR ErrorID;
} McSetAdditionalConversionFactors;

typedef struct McReinitDrive {
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL bExecute;
    ZMC_BOOL bVirtual;
    // output
    ZMC_BOOL bDone;
    ZMC_BOOL bBusy;
    ZMC_BOOL bError;
    ZMC_ERROR nErrorID;
} McReinitDrive;

#pragma pack(pop)




ZMC_DLLPORT int MC_Power_create(McPower*);
ZMC_DLLPORT int MC_Power_exec(McPower*);
ZMC_DLLPORT int MC_Power_destroy(McPower*);

ZMC_DLLPORT int MC_Reset_create(McReset*);
ZMC_DLLPORT int MC_Reset_exec(McReset*);
ZMC_DLLPORT int MC_Reset_destroy(McReset*);

ZMC_DLLPORT int MC_SetPosition_create(McSetPosition*);
ZMC_DLLPORT int MC_SetPosition_exec(McSetPosition*);
ZMC_DLLPORT int MC_SetPosition_destroy(McSetPosition*);

ZMC_DLLPORT int MC_ReadStatus_create(McReadStatus*);
ZMC_DLLPORT int MC_ReadStatus_exec(McReadStatus*);
ZMC_DLLPORT int MC_ReadStatus_destroy(McReadStatus*);

ZMC_DLLPORT int MC_ReadActualPosition_create(McReadActualPosition*);
ZMC_DLLPORT int MC_ReadActualPosition_exec(McReadActualPosition*);
ZMC_DLLPORT int MC_ReadActualPosition_destroy(McReadActualPosition*);

ZMC_DLLPORT int MC_ReadActualTorque_create(McReadActualTorque*);
ZMC_DLLPORT int MC_ReadActualTorque_exec(McReadActualTorque*);
ZMC_DLLPORT int MC_ReadActualTorque_destroy(McReadActualTorque*);

ZMC_DLLPORT int MC_ReadActualVelocity_create(McReadActualVelocity*);
ZMC_DLLPORT int MC_ReadActualVelocity_exec(McReadActualVelocity*);
ZMC_DLLPORT int MC_ReadActualVelocity_destroy(McReadActualVelocity*);

ZMC_DLLPORT int MC_ReadAxisError_create(McReadAxisError*);
ZMC_DLLPORT int MC_ReadAxisError_exec(McReadAxisError*);
ZMC_DLLPORT int MC_ReadAxisError_destroy(McReadAxisError*);

ZMC_DLLPORT int MC_ReadCommandPosition_create(McReadCommandPosition*);
ZMC_DLLPORT int MC_ReadCommandPosition_exec(McReadCommandPosition*);
ZMC_DLLPORT int MC_ReadCommandPosition_destroy(McReadCommandPosition*);

ZMC_DLLPORT int MC_ReadSetPosition_create(McReadSetPosition*);
ZMC_DLLPORT int MC_ReadSetPosition_exec(McReadSetPosition*);
ZMC_DLLPORT int MC_ReadSetPosition_destroy(McReadSetPosition*);

ZMC_DLLPORT int MC_ReadBoolParameter_create(McReadBoolParameter*);
ZMC_DLLPORT int MC_ReadBoolParameter_exec(McReadBoolParameter*);
ZMC_DLLPORT int MC_ReadBoolParameter_destroy(McReadBoolParameter*);

ZMC_DLLPORT int MC_ReadParameter_create(McReadParameter*);
ZMC_DLLPORT int MC_ReadParameter_exec(McReadParameter*);
ZMC_DLLPORT int MC_ReadParameter_destroy(McReadParameter*);

ZMC_DLLPORT int MC_WriteBoolParameter_create(McWriteBoolParameter*);
ZMC_DLLPORT int MC_WriteBoolParameter_exec(McWriteBoolParameter*);
ZMC_DLLPORT int MC_WriteBoolParameter_destroy(McWriteBoolParameter*);

ZMC_DLLPORT int MC_WriteParameter_create(McWriteParameter*);
ZMC_DLLPORT int MC_WriteParameter_exec(McWriteParameter*);
ZMC_DLLPORT int MC_WriteParameter_destroy(McWriteParameter*);

ZMC_DLLPORT int MC_Stop_create(McStop*);
ZMC_DLLPORT int MC_Stop_exec(McStop*);
ZMC_DLLPORT int MC_Stop_destroy(McStop*);

ZMC_DLLPORT int MC_Halt_create(McHalt*);
ZMC_DLLPORT int MC_Halt_exec(McHalt*);
ZMC_DLLPORT int MC_Halt_destroy(McHalt*);

ZMC_DLLPORT int MC_MoveAbsolute_create(McMoveAbsolute*);
ZMC_DLLPORT int MC_MoveAbsolute_exec(McMoveAbsolute*);
ZMC_DLLPORT int MC_MoveAbsolute_destroy(McMoveAbsolute*);

ZMC_DLLPORT int MC_MoveAdditive_create(McMoveAdditive*);
ZMC_DLLPORT int MC_MoveAdditive_exec(McMoveAdditive*);
ZMC_DLLPORT int MC_MoveAdditive_destroy(McMoveAdditive*);

ZMC_DLLPORT int MC_MoveRelative_create(McMoveRelative*);
ZMC_DLLPORT int MC_MoveRelative_exec(McMoveRelative*);
ZMC_DLLPORT int MC_MoveRelative_destroy(McMoveRelative*);

ZMC_DLLPORT int MC_MoveVelocity_create(McMoveVelocity*);
ZMC_DLLPORT int MC_MoveVelocity_exec(McMoveVelocity*);
ZMC_DLLPORT int MC_MoveVelocity_destroy(McMoveVelocity*);

ZMC_DLLPORT int MC_MoveContinuousAbsolute_create(McMoveContinuousAbsolute*);
ZMC_DLLPORT int MC_MoveContinuousAbsolute_exec(McMoveContinuousAbsolute*);
ZMC_DLLPORT int MC_MoveContinuousAbsolute_destroy(McMoveContinuousAbsolute*);

ZMC_DLLPORT int MC_MoveContinuousRelative_create(McMoveContinuousRelative*);
ZMC_DLLPORT int MC_MoveContinuousRelative_exec(McMoveContinuousRelative*);
ZMC_DLLPORT int MC_MoveContinuousRelative_destroy(McMoveContinuousRelative*);

ZMC_DLLPORT int MC_MoveSuperImposed_create(McMoveSuperImposed*);
ZMC_DLLPORT int MC_MoveSuperImposed_exec(McMoveSuperImposed*);
ZMC_DLLPORT int MC_MoveSuperImposed_destroy(McMoveSuperImposed*);

ZMC_DLLPORT int MC_HaltSuperImposed_create(McHaltSuperImposed*);
ZMC_DLLPORT int MC_HaltSuperImposed_exec(McHaltSuperImposed*);
ZMC_DLLPORT int MC_HaltSuperImposed_destroy(McHaltSuperImposed*);

ZMC_DLLPORT int MC_PositionProfile_create(McPositionProfile*);
ZMC_DLLPORT int MC_PositionProfile_exec(McPositionProfile*);
ZMC_DLLPORT int MC_PositionProfile_destroy(McPositionProfile*);

ZMC_DLLPORT int MC_AccelerationProfile_create(McAccelerationProfile*);
ZMC_DLLPORT int MC_AccelerationProfile_exec(McAccelerationProfile*);
ZMC_DLLPORT int MC_AccelerationProfile_destroy(McAccelerationProfile*);

ZMC_DLLPORT int MC_VelocityProfile_create(McVelocityProfile*);
ZMC_DLLPORT int MC_VelocityProfile_exec(McVelocityProfile*);
ZMC_DLLPORT int MC_VelocityProfile_destroy(McVelocityProfile*);

ZMC_DLLPORT int MC_Home_create(McHome*);
ZMC_DLLPORT int MC_Home_exec(McHome*);
ZMC_DLLPORT int MC_Home_destroy(McHome*);

ZMC_DLLPORT int MC_Homing_create(McHoming*);
ZMC_DLLPORT int MC_Homing_exec(McHoming*);
ZMC_DLLPORT int MC_Homing_destroy(McHoming*);

ZMC_DLLPORT int MC_Jog_create(McJog*);
ZMC_DLLPORT int MC_Jog_exec(McJog*);
ZMC_DLLPORT int MC_Jog_destroy(McJog*);

ZMC_DLLPORT int MC_Inch_create(McInch*);
ZMC_DLLPORT int MC_Inch_exec(McInch*);
ZMC_DLLPORT int MC_Inch_destroy(McInch*);

ZMC_DLLPORT int MC_GearIn_create(McGearIn*);
ZMC_DLLPORT int MC_GearIn_exec(McGearIn*);
ZMC_DLLPORT int MC_GearIn_destroy(McGearIn*);

ZMC_DLLPORT int MC_GearInPos_create(McGearInPos*);
ZMC_DLLPORT int MC_GearInPos_exec(McGearInPos*);
ZMC_DLLPORT int MC_GearInPos_destroy(McGearInPos*);

ZMC_DLLPORT int MC_GearOut_create(McGearOut*);
ZMC_DLLPORT int MC_GearOut_exec(McGearOut*);
ZMC_DLLPORT int MC_GearOut_destroy(McGearOut*);

ZMC_DLLPORT int MC_Phasing_create(McPhasing*);
ZMC_DLLPORT int MC_Phasing_exec(McPhasing*);
ZMC_DLLPORT int MC_Phasing_destroy(McPhasing*);

ZMC_DLLPORT int MC_SetTorque_create(McSetTorque*);
ZMC_DLLPORT int MC_SetTorque_exec(McSetTorque*);
ZMC_DLLPORT int MC_SetTorque_destroy(McSetTorque*);

ZMC_DLLPORT int MC_FollowPosition_create(McFollowPosition*);
ZMC_DLLPORT int MC_FollowPosition_exec(McFollowPosition*);
ZMC_DLLPORT int MC_FollowPosition_destroy(McFollowPosition*);

ZMC_DLLPORT int MC_FollowPositionVelocity_create(McFollowPositionVelocity*);
ZMC_DLLPORT int MC_FollowPositionVelocity_exec(McFollowPositionVelocity*);
ZMC_DLLPORT int MC_FollowPositionVelocity_destroy(McFollowPositionVelocity*);

ZMC_DLLPORT int MC_FollowVelocity_create(McFollowVelocity*);
ZMC_DLLPORT int MC_FollowVelocity_exec(McFollowVelocity*);
ZMC_DLLPORT int MC_FollowVelocity_destroy(McFollowVelocity*);

ZMC_DLLPORT int MC_FollowSetValues_create(McFollowSetValues*);
ZMC_DLLPORT int MC_FollowSetValues_exec(McFollowSetValues*);
ZMC_DLLPORT int MC_FollowSetValues_destroy(McFollowSetValues*);

ZMC_DLLPORT int MC_TrackAxis_create(McTrackAxis*);
ZMC_DLLPORT int MC_TrackAxis_exec(McTrackAxis*);
ZMC_DLLPORT int MC_TrackAxis_destroy(McTrackAxis*);

ZMC_DLLPORT int MC_TrackSetValues_create(McTrackSetValues*);
ZMC_DLLPORT int MC_TrackSetValues_exec(McTrackSetValues*);
ZMC_DLLPORT int MC_TrackSetValues_destroy(McTrackSetValues*);

ZMC_DLLPORT int MC_CamIn_create(McCamIn*);
ZMC_DLLPORT int MC_CamIn_exec(McCamIn*);
ZMC_DLLPORT int MC_CamIn_destroy(McCamIn*);

ZMC_DLLPORT int MC_CamOut_create(McCamOut*);
ZMC_DLLPORT int MC_CamOut_exec(McCamOut*);
ZMC_DLLPORT int MC_CamOut_destroy(McCamOut*);

ZMC_DLLPORT int MC_CamTableSelect_create(McCamTableSelect*);
ZMC_DLLPORT int MC_CamTableSelect_exec(McCamTableSelect*);
ZMC_DLLPORT int MC_CamTableSelect_destroy(McCamTableSelect*);

ZMC_DLLPORT int MC_GetTappetValue_create(McGetTappetValue*);
ZMC_DLLPORT int MC_GetTappetValue_exec(McGetTappetValue*);
ZMC_DLLPORT int MC_GetTappetValue_destroy(McGetTappetValue*);

ZMC_DLLPORT int MC_CamRegister_create(McCamRegister*);
ZMC_DLLPORT int MC_CamRegister_exec(McCamRegister*);
ZMC_DLLPORT int MC_CamRegister_destroy(McCamRegister*);

ZMC_DLLPORT int MC_CamCalc_create(McCamCalc*);
ZMC_DLLPORT int MC_CamCalc_exec(McCamCalc*);
ZMC_DLLPORT int MC_CamCalc_destroy(McCamCalc*);

ZMC_DLLPORT int MC_CAMBounds_create(McCAMBounds*);
ZMC_DLLPORT int MC_CAMBounds_exec(McCAMBounds*);
ZMC_DLLPORT int MC_CAMBounds_destroy(McCAMBounds*);

ZMC_DLLPORT int MC_CAMBounds_Pos_create(McCAMBounds_Pos*);
ZMC_DLLPORT int MC_CAMBounds_Pos_exec(McCAMBounds_Pos*);
ZMC_DLLPORT int MC_CAMBounds_Pos_destroy(McCAMBounds_Pos*);

ZMC_DLLPORT int MC_GetCamSlaveSetPosition_create(McGetCamSlaveSetPosition*);
ZMC_DLLPORT int MC_GetCamSlaveSetPosition_exec(McGetCamSlaveSetPosition*);
ZMC_DLLPORT int MC_GetCamSlaveSetPosition_destroy(McGetCamSlaveSetPosition*);

ZMC_DLLPORT int MC_AbortTrigger_create(McAbortTrigger*);
ZMC_DLLPORT int MC_AbortTrigger_exec(McAbortTrigger*);
ZMC_DLLPORT int MC_AbortTrigger_destroy(McAbortTrigger*);

ZMC_DLLPORT int MC_TouchProbe_create(McTouchProbe*);
ZMC_DLLPORT int MC_TouchProbe_exec(McTouchProbe*);
ZMC_DLLPORT int MC_TouchProbe_destroy(McTouchProbe*);

ZMC_DLLPORT int MC_DigitalCamSwitch_create(McDigitalCamSwitch*);
ZMC_DLLPORT int MC_DigitalCamSwitch_exec(McDigitalCamSwitch*);
ZMC_DLLPORT int MC_DigitalCamSwitch_destroy(McDigitalCamSwitch*);

ZMC_DLLPORT int MC_ReadCAM_create(McReadCAM*);
ZMC_DLLPORT int MC_ReadCAM_exec(McReadCAM*);
ZMC_DLLPORT int MC_ReadCAM_destroy(McReadCAM*);

ZMC_DLLPORT int MC_WriteCAM_create(McWriteCAM*);
ZMC_DLLPORT int MC_WriteCAM_exec(McWriteCAM*);
ZMC_DLLPORT int MC_WriteCAM_destroy(McWriteCAM*);

ZMC_DLLPORT int MC_ReadSetValues_create(McReadSetValues*);
ZMC_DLLPORT int MC_ReadSetValues_exec(McReadSetValues*);
ZMC_DLLPORT int MC_ReadSetValues_destroy(McReadSetValues*);

ZMC_DLLPORT int MC_ReadActualValues_create(McReadActualValues*);
ZMC_DLLPORT int MC_ReadActualValues_exec(McReadActualValues*);
ZMC_DLLPORT int MC_ReadActualValues_destroy(McReadActualValues*);

ZMC_DLLPORT int MC_ReadSetDriveValues_create(McReadSetDriveValues*);
ZMC_DLLPORT int MC_ReadSetDriveValues_exec(McReadSetDriveValues*);
ZMC_DLLPORT int MC_ReadSetDriveValues_destroy(McReadSetDriveValues*);

ZMC_DLLPORT int MC_ReadActualDriveValues_create(McReadActualDriveValues*);
ZMC_DLLPORT int MC_ReadActualDriveValues_exec(McReadActualDriveValues*);
ZMC_DLLPORT int MC_ReadActualDriveValues_destroy(McReadActualDriveValues*);

ZMC_DLLPORT int MC_GetTrackingError_create(McGetTrackingError*);
ZMC_DLLPORT int MC_GetTrackingError_exec(McGetTrackingError*);
ZMC_DLLPORT int MC_GetTrackingError_destroy(McGetTrackingError*);

ZMC_DLLPORT int MC_GetTravelledDistance_create(McGetTravelledDistance*);
ZMC_DLLPORT int MC_GetTravelledDistance_exec(McGetTravelledDistance*);
ZMC_DLLPORT int MC_GetTravelledDistance_destroy(McGetTravelledDistance*);

ZMC_DLLPORT int MC_GetMaxSetAccDec_create(McGetMaxSetAccDec*);
ZMC_DLLPORT int MC_GetMaxSetAccDec_exec(McGetMaxSetAccDec*);
ZMC_DLLPORT int MC_GetMaxSetAccDec_destroy(McGetMaxSetAccDec*);

ZMC_DLLPORT int MC_GetMaxSetVelocity_create(McGetMaxSetVelocity*);
ZMC_DLLPORT int MC_GetMaxSetVelocity_exec(McGetMaxSetVelocity*);
ZMC_DLLPORT int MC_GetMaxSetVelocity_destroy(McGetMaxSetVelocity*);

ZMC_DLLPORT int MC_GetOverride_create(McGetOverride*);
ZMC_DLLPORT int MC_GetOverride_exec(McGetOverride*);
ZMC_DLLPORT int MC_GetOverride_destroy(McGetOverride*);

ZMC_DLLPORT int MC_SetOverride_create(McSetOverride*);
ZMC_DLLPORT int MC_SetOverride_exec(McSetOverride*);
ZMC_DLLPORT int MC_SetOverride_destroy(McSetOverride*);

ZMC_DLLPORT int MC_CheckLimits_create(McCheckLimits*);
ZMC_DLLPORT int MC_CheckLimits_exec(McCheckLimits*);
ZMC_DLLPORT int MC_CheckLimits_destroy(McCheckLimits*);

ZMC_DLLPORT int MC_InPosition_create(McInPosition*);
ZMC_DLLPORT int MC_InPosition_exec(McInPosition*);
ZMC_DLLPORT int MC_InPosition_destroy(McInPosition*);

ZMC_DLLPORT int MC_SetMovementType_create(McSetMovementType*);
ZMC_DLLPORT int MC_SetMovementType_exec(McSetMovementType*);
ZMC_DLLPORT int MC_SetMovementType_destroy(McSetMovementType*);

ZMC_DLLPORT int MC_SetRampType_create(McSetRampType*);
ZMC_DLLPORT int MC_SetRampType_exec(McSetRampType*);
ZMC_DLLPORT int MC_SetRampType_destroy(McSetRampType*);

ZMC_DLLPORT int MC_ChangeDynamicLimits_create(McChangeDynamicLimits*);
ZMC_DLLPORT int MC_ChangeDynamicLimits_exec(McChangeDynamicLimits*);
ZMC_DLLPORT int MC_ChangeDynamicLimits_destroy(McChangeDynamicLimits*);

ZMC_DLLPORT int MC_ChangeGearingRatio_create(McChangeGearingRatio*);
ZMC_DLLPORT int MC_ChangeGearingRatio_exec(McChangeGearingRatio*);
ZMC_DLLPORT int MC_ChangeGearingRatio_destroy(McChangeGearingRatio*);

ZMC_DLLPORT int MC_SetSoftwareLimits_create(McSetSoftwareLimits*);
ZMC_DLLPORT int MC_SetSoftwareLimits_exec(McSetSoftwareLimits*);
ZMC_DLLPORT int MC_SetSoftwareLimits_destroy(McSetSoftwareLimits*);

ZMC_DLLPORT int MC_SetControllerMode_create(McSetControllerMode*);
ZMC_DLLPORT int MC_SetControllerMode_exec(McSetControllerMode*);
ZMC_DLLPORT int MC_SetControllerMode_destroy(McSetControllerMode*);

ZMC_DLLPORT int MC_EstimateDeadTime_create(McEstimateDeadTime*);
ZMC_DLLPORT int MC_EstimateDeadTime_exec(McEstimateDeadTime*);
ZMC_DLLPORT int MC_EstimateDeadTime_destroy(McEstimateDeadTime*);

ZMC_DLLPORT int MC_VirtualAxis_create(McVirtualAxis*);
ZMC_DLLPORT int MC_VirtualAxis_exec(McVirtualAxis*);
ZMC_DLLPORT int MC_VirtualAxis_destroy(McVirtualAxis*);

ZMC_DLLPORT int MC_MeasureDistance_create(McMeasureDistance*);
ZMC_DLLPORT int MC_MeasureDistance_exec(McMeasureDistance*);
ZMC_DLLPORT int MC_MeasureDistance_destroy(McMeasureDistance*);

ZMC_DLLPORT int MC_SetAdditionalConversionFactors_create(McSetAdditionalConversionFactors*);
ZMC_DLLPORT int MC_SetAdditionalConversionFactors_exec(McSetAdditionalConversionFactors*);
ZMC_DLLPORT int MC_SetAdditionalConversionFactors_destroy(McSetAdditionalConversionFactors*);

ZMC_DLLPORT int MC_ReinitDrive_create(McReinitDrive*);
ZMC_DLLPORT int MC_ReinitDrive_exec(McReinitDrive*);
ZMC_DLLPORT int MC_ReinitDrive_destroy(McReinitDrive*);

ZMC_DLLPORT int MC_ReadValues_create(McReadValues*);
ZMC_DLLPORT int MC_ReadValues_exec(McReadValues*);
ZMC_DLLPORT int MC_ReadValues_destroy(McReadValues*);

ZMC_DLLPORT int MC_ReadPowerParam_create(McReadPowerParam*);
ZMC_DLLPORT int MC_ReadPowerParam_exec(McReadPowerParam*);
ZMC_DLLPORT int MC_ReadPowerParam_destroy(McReadPowerParam*);

#ifdef __cplusplus
}
#endif

#endif