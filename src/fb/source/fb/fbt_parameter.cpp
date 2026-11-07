/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#include "fb_cpp_def.h"
#include "fbt_axis_impl.h"
#include "fbt_fb_base.hpp"
#include "mc_parameter_enum_def.h"
#include "axis/axis_coe_channel.h"
#include "fb/smc_parameter_utils.h"

namespace zmc{
typedef AxisBase Axis;
int mcReadParameter(Axis& Ax, ZMC_DINT paraNo, LREAL& value) {
    auto& axis = get_axis_impl_ref(&Ax);
    switch(paraNo) {
        // State parameters
        case PARA_nAxisState:
            value = static_cast<LREAL>(axis.axisStat);
            break;
        case PARA_wDriveId:
            value = static_cast<LREAL>(axis.wAxisStructID);
            break;
        case PARA_iOwner:
        case PARA_iNoOwner:
            // TODO: Implement when owner management is added
            value = 0.0;  // Return default value
            break;

        // Position parameters
        case PARA_fSetPosition:
            value = axis.fSetPosition;
            break;
        case PARA_fActPosition:
            value = axis.fActPosition;
            break;
        case PARA_fAimPosition:
            value = axis.fAimPosition;
            break;
        case PARA_fMarkPosition:
            value = axis.fMarkPosition;
            break;
        case PARA_fSavePosition:
            value = axis.fSavePosition;
            break;

        // Velocity parameters
        case PARA_fSetVelocity:
            value = axis.fSetVelocity;
            break;
        case PARA_fActVelocity:
            value = axis.fActVelocity;
            break;
        case PARA_fMaxVelocity:
            value = axis.fMaxVelocity;
            break;
        case PARA_fSWMaxVelocity:
            value = axis.fSWMaxVelocity;
            break;
        case PARA_fMarkVelocity:
            value = axis.fMarkVelocity;
            break;
        case PARA_fSaveVelocity:
            value = axis.fSaveVelocity;
            break;

        // Acceleration parameters
        case PARA_fSetAcceleration:
            value = axis.fSetAcceleration;
            break;
        case PARA_fActAcceleration:
            value = axis.fActAcceleration;
            break;
        case PARA_fMaxAcceleration:
            value = axis.fMaxAcceleration;
            break;
        case PARA_fSWMaxAcceleration:
            value = axis.fSWMaxAcceleration;
            break;
        case PARA_fMarkAcceleration:
            value = axis.fMarkAcceleration;
            break;
        case PARA_fSaveAcceleration:
            value = axis.fSaveAcceleration;
            break;
        case PARA_fMaxDeceleration:
            value = axis.fMaxDeceleration;
            break;
        case PARA_fSWMaxDeceleration:
            value = axis.fSWMaxDeceleration;
            break;
        case PARA_fSaveDeceleration:
            value = 0.0;  // Default value, to be implemented later
            break;

        // Jerk parameters
        case PARA_fSetJerk:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fActJerk:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fMaxJerk:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fSWMaxJerk:
            value = axis.fSWMaxJerk;
            break;
        case PARA_fRampJerk:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fMarkJerk:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fSaveJerk:
            value = 0.0;  // Default value, to be implemented later
            break;

        // Current parameters
        case PARA_fSetCurrent:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fActCurrent:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fMaxCurrent:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fSWMaxCurrent:
            value = 0.0;  // Default value, to be implemented later
            break;

        // Torque parameters
        case PARA_fSetTorque:
            value = axis.fSetTorque;
            break;
        case PARA_fActTorque:
            value = axis.fActTorque;
            break;
        case PARA_fMaxTorque:
            value = axis.fMaxTorque;
            break;
        case PARA_fSWMaxTorque:
            value = axis.fSWMaxTorque;
            break;

        // Limit parameters
        case PARA_fSWLimitPositive:
            value = axis.fSWLimitPositive;
            break;
        case PARA_fSWLimitNegative:
            value = axis.fSWLimitNegative;
            break;
        case PARA_usiSWEndSwitchState:
            value = static_cast<LREAL>(axis.usiSWEndSwitchState);
            break;
        case PARA_bSWLimitEnable:
            value = axis.bSWLimitEnable ? 1.0 : 0.0;
            break;
        case PARA_bHWLimitEnable:
            value = axis.bHWLimitEnable ? 1.0 : 0.0;
            break;

        // Reference and homing parameters
        case PARA_bStartReference:
            value = axis.bStartReference ? 1.0 : 0.0;
            break;
        case PARA_fReference:
            value = 0.0;  // Default value, field commented out in axis structure
            break;
        case PARA_bStartReferenceRealState:
            value = axis.bStartReferenceRealState ? 1.0 : 0.0;
            break;
        case PARA_fOffsetPosition:
            value = axis.fOffsetPosition;
            break;

        // Position offset and transform parameters
        case PARA_dwPosOffsetForResiduals:
            value = static_cast<LREAL>(axis.dwPosOffsetForResiduals);
            break;
        case PARA_dwOneTurn:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_dwLastPosition:
            value = static_cast<LREAL>(axis.dwLastPosition);
            break;
        case PARA_bySwitchingState:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_iRestNumerator:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_iTurn:
            value = static_cast<LREAL>(axis.iTurn);
            break;
        case PARA_dwPosOffsetForResidualsHoming:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_dwActPosition:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_dwBusBandWidth:
            value = 0.0;  // Default value, to be implemented later
            break;

        // Brake control parameters
        case PARA_eBrakeControl:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_bBrakeClosedRealState:
            value = 0.0;  // Default value, to be implemented later
            break;

        // Error and status parameters
        case PARA_bOldError:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_bVirtual:
            value = 0.0;  // Default value, field commented out in axis structure
            break;
        case PARA_bLogical:
            value = axis.bLogical ? 1.0 : 0.0;
            break;

        // Time lag parameter
        case PARA_fSetActTimeLagCycles:
            value = axis.fSetActTimeLagCycles;
            break;

        // Integer position/velocity/acceleration/current/torque parameters (DINT)
        case PARA_diSetPosition:
            value = static_cast<LREAL>(axis.diSetPosition);
            break;
        case PARA_diActPosition:
            value = static_cast<LREAL>(axis.diActPosition);
            break;
        case PARA_diSetVelocity:
            value = static_cast<LREAL>(axis.diSetVelocity);
            break;
        case PARA_diActVelocity:
            value = static_cast<LREAL>(axis.diActVelocity);
            break;
        case PARA_diSetAcceleration:
            value = static_cast<LREAL>(axis.diSetAcceleration);
            break;
        case PARA_diActAcceleration:
            value = static_cast<LREAL>(axis.diActAcceleration);
            break;
        case PARA_diSetCurrent:
            value = static_cast<LREAL>(axis.diSetCurrent);
            break;
        case PARA_diActCurrent:
            value = static_cast<LREAL>(axis.diActCurrent);
            break;
        case PARA_diSetTorque:
            value = static_cast<LREAL>(axis.diSetTorque);
            break;
        case PARA_diActTorque:
            value = static_cast<LREAL>(axis.diActTorque);
            break;

        // Software limit and error parameters
        case PARA_fSWLimitDeceleration:
            value = axis.fSWLimitDeceleration;
            break;
        case PARA_eCheckPositionLag:
            value = static_cast<LREAL>(axis.eCheckPositionLag);
            break;
        case PARA_fMaxPositionLag:
            value = axis.fMaxPositionLag;
            break;
        case PARA_bPositionLagActive:
            value = axis.bPositionLagActive ? 1.0 : 0.0;
            break;
        case PARA_xWaitForHaltWhenStopInterruptsHome:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_bModuloDoneByDrive:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_diFollowingError:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fFollowingError:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fSWErrorMaxDistance:
            value = axis.fSWErrorMaxDistance;
            break;
        case PARA_aCaptDesc:
            value = 0.0;  // Default value, array type not supported
            break;

        // Scale factors and ratios
        case PARA_fScalefactor:
            value = axis.fScalefactor;
            break;
        case PARA_iRatioTechUnitsNum:
            value = static_cast<LREAL>(axis.iRatioTechUnitsNum);
            break;
        case PARA_dwRatioTechUnitsDenom:
            value = static_cast<LREAL>(axis.dwRatioTechUnitsDenom);
            break;
        case PARA_fFactorVel:
            value = axis.fFactorVel;
            break;
        case PARA_fFactorAcc:
            value = axis.fFactorAcc;
            break;
        case PARA_fFactorTor:
            value = axis.fFactorTor;
            break;
        case PARA_fFactorJerk:
            value = axis.fFactorJerk;
            break;
        case PARA_fFactorCur:
            value = 1.0;  // Default value, to be implemented later
            break;
        case PARA_iMovementType:
            // Convert from iMovementType: 0 = modulo, 1 = finite
            value = axis.iMovementType ? 1.0 : 0.0;  // Default to modulo (0)
            break;
        case PARA_nDirection:
            value = static_cast<LREAL>(axis.nDirection);
            break;
        case PARA_fPositionPeriod:
            value = axis.fPositionPeriod;
            break;

        // Boolean status parameters (convert to LREAL)
        case PARA_bRegulatorOn:
            value = axis.bRegulatorOn ? 1.0 : 0.0;
            break;
        case PARA_bDriveStart:
            value = axis.bDriveStart ? 1.0 : 0.0;
            break;
        case PARA_bCommunication:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_wCommunicationState:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_uiDriveInterfaceError:
            value = static_cast<LREAL>(axis.uiDriveInterfaceError);
            break;
        case PARA_bRegulatorRealState:
            value = axis.regulatorRealState ? 1.0 : 0.0;
            break;
        case PARA_bDriveStartRealState:
            value = axis.driveStartRealState ? 1.0 : 0.0;
            break;
        case PARA_fCycleTimeSpent:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_fTaskCycle:
            value = axis.cycleTime;
            break;
        case PARA_bError:
            value = axis.driveError ? 1.0 : 0.0;
            break;
        case PARA_bErrorAckn:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_bDisableErrorLogging:
            value = 0.0;  // Default value, to be implemented later
            break;
        case PARA_bConstantVelocity:
            value = axis.ConstantVelocity ? 1.0 : 0.0;
            break;
        case PARA_bAccelerating:
            value = axis.Accelerating ? 1.0 : 0.0;
            break;
        case PARA_bDecelerating:
            value = axis.Decelerating ? 1.0 : 0.0;
            break;

        // Error handling parameters
        case PARA_dwErrorID:
            value = static_cast<LREAL>(axis.dwErrorID);
            break;
        case PARA_fbeFBError:
            value = static_cast<LREAL>(axis.fbeFBError);
            break;

        // Controller mode parameters
        case PARA_byControllerMode:
            value = static_cast<LREAL>(axis.byControllerMode);
            break;
        case PARA_byRealControllerMode:
            value = static_cast<LREAL>(axis.byRealControllerMode);
            break;

        // Ramp type parameter
        case PARA_eRampType:
            value = static_cast<LREAL>(axis.eRampType);
            break;

        default:
            return -1;  // Parameter not supported
    }
    return 0;  // Success
}



// ... existing code ...

int mcWriteParameter(Axis& Ax, ZMC_DINT paraNo, LREAL value) {
    auto& axis = get_axis_impl_ref(&Ax);
    switch(paraNo) {
        // Read-only parameters (return error)
        case PARA_wDriveId:
        case PARA_iOwner:
        case PARA_iNoOwner:
        case PARA_bCommunication:
        case PARA_wCommunicationState:
        case PARA_uiDriveInterfaceError:
        case PARA_fCycleTimeSpent:
        case PARA_fTaskCycle:
        case PARA_bErrorAckn:
        case PARA_bDisableErrorLogging:
        case PARA_fbeFBError:
        case PARA_fFactorCur:
        case PARA_fSaveDeceleration:
        case PARA_fMaxJerk:

        case PARA_fRampJerk:
        case PARA_fMarkJerk:
        case PARA_fSaveJerk:
        case PARA_fSetJerk:
        case PARA_fMaxCurrent:
        case PARA_fSWMaxCurrent:
        case PARA_fSetCurrent:
        case PARA_usiSWEndSwitchState:
        case PARA_fReference:
        case PARA_bStartReferenceRealState:
        case PARA_dwOneTurn:

        case PARA_bySwitchingState:
        case PARA_iRestNumerator:
        case PARA_dwPosOffsetForResidualsHoming:
        case PARA_dwBusBandWidth:
        case PARA_eBrakeControl:
        case PARA_bBrakeClosedRealState:
        case PARA_bOldError:
        case PARA_bVirtual:
        case PARA_dwActPosition:
        case PARA_xWaitForHaltWhenStopInterruptsHome:
        case PARA_bModuloDoneByDrive:
        case PARA_diFollowingError:
        case PARA_fFollowingError:
        case PARA_aCaptDesc:
            return -1;  // not implenment
        case PARA_nAxisState:
            if (value < 0 || value > mcErrorStop) return -2;
            axis.axisStat = static_cast<MC_AXIS_STATES>(static_cast<int>(value));
            break;
        // Position parameters
        case PARA_fSetPosition:
            axis.fSetPosition = value;
            break;
        case PARA_fAimPosition:
            axis.fAimPosition = value;
            break;
        case PARA_fMarkPosition:
            axis.fMarkPosition = value;
            break;

        // Velocity parameters
        case PARA_fSetVelocity:
            axis.fSetVelocity = value;
            break;
        case PARA_fMaxVelocity:
            if (value <= 0) return -1;  // Validate velocity
            axis.fMaxVelocity = value;
            break;
        case PARA_fSWMaxVelocity:
            if (value <= 0) return -1;  // Validate velocity
            axis.fSWMaxVelocity = value;
            break;
        case PARA_fMarkVelocity:
            axis.fMarkVelocity = value;
            break;

        // Acceleration parameters
        case PARA_fSetAcceleration:
            axis.fSetAcceleration = value;
            break;
        case PARA_fMaxAcceleration:
            if (value <= 0) return -1;  // Validate acceleration
            axis.fMaxAcceleration = value;
            break;
        case PARA_fSWMaxAcceleration:
            if (value <= 0) return -1;  // Validate acceleration
            axis.fSWMaxAcceleration = value;
            break;
        case PARA_fMarkAcceleration:
            axis.fMarkAcceleration = value;
            break;

        // Torque parameters
        case PARA_fSetTorque:
            axis.fSetTorque = value;
            break;
        case PARA_fMaxTorque:
            if (value <= 0) return -1;  // Validate torque
            axis.fMaxTorque = value;
            break;
        case PARA_fSWMaxTorque:
            if (value <= 0) return -1;  // Validate torque
            axis.fSWMaxTorque = value;
            break;

        // Scale factors and ratios
        case PARA_fScalefactor:
            if (value <= 0) return -1;  // Validate scale factor
            axis.fScalefactor = value;
            break;
        case PARA_fFactorVel:
            if (value <= 0) return -1;  // Validate velocity factor
            axis.fFactorVel = value;
            break;
        case PARA_fFactorAcc:
            if (value <= 0) return -1;  // Validate acceleration factor
            axis.fFactorAcc = value;
            break;
        case PARA_fFactorTor:
            if (value <= 0) return -1;  // Validate torque factor
            axis.fFactorTor = value;
            break;
        case PARA_fFactorJerk:
            if (value <= 0) return -1;  // Validate jerk factor
            axis.fFactorJerk = value;
            break;

        // Boolean parameters (convert from LREAL)
        case PARA_bRegulatorOn:
            axis.bRegulatorOn = (value != 0.0);
            break;
        case PARA_bDriveStart:
            axis.bDriveStart = (value != 0.0);
            break;

        // Controller mode parameters
        case PARA_byControllerMode:
            axis.byControllerMode = static_cast<int>(value);
            break;

        // Ramp type parameter
        case PARA_eRampType:
            if (value < 0 || value > VEL_RAMP_TYPE_QUADRATIC_SMOOTH) return -2;
            axis.eRampType = static_cast<Velocity_Ramp_Type>(static_cast<int>(value));
            break;

        // Period and ratio parameters
        case PARA_fPositionPeriod:
            if (value <= 0) return -1;  // Validate period
            axis.fPositionPeriod = value;
            break;
        case PARA_iRatioTechUnitsNum:
            axis.iRatioTechUnitsNum = static_cast<int>(value);
            break;
        case PARA_dwRatioTechUnitsDenom:
            if (value <= 0) return -1;  // Validate denominator
            axis.dwRatioTechUnitsDenom = static_cast<uint32_t>(value);
            break;
        case PARA_fSavePosition:
            axis.fSavePosition = value;
            break;
        case PARA_fSaveVelocity:
            axis.fSaveVelocity = value;
            break;
        case PARA_fSaveAcceleration:
            axis.fSaveAcceleration = value;
            break;
        case PARA_fMaxDeceleration:
            if (value <= 0) return -1;  // Validate deceleration
            axis.fMaxDeceleration = value;
            break;
        case PARA_fSWMaxDeceleration:
            if (value <= 0) return -1;  // Validate deceleration
            axis.fSWMaxDeceleration = value;
            break;
        case PARA_fSWLimitPositive:
            axis.fSWLimitPositive = value;
            break;
        case PARA_fSWLimitNegative:
            axis.fSWLimitNegative = value;
            break;
        case PARA_bSWLimitEnable:
            axis.bSWLimitEnable = (value != 0.0);
            break;
        case PARA_bHWLimitEnable:
            axis.bHWLimitEnable = (value != 0.0);
            break;
        case PARA_bStartReference:
            axis.bStartReference = (value != 0.0);
            break;
        case PARA_fOffsetPosition:
            axis.fOffsetPosition = value;
            break;
        case PARA_dwPosOffsetForResiduals:
            axis.dwPosOffsetForResiduals = static_cast<int32_t>(value);
            break;
        case PARA_iTurn:
            axis.iTurn = static_cast<int>(value);
            break;
        case PARA_bLogical:
            axis.bLogical = (value != 0.0);
            break;
        case PARA_fSetActTimeLagCycles:
            axis.fSetActTimeLagCycles = value;
            break;
        case PARA_diSetPosition:
            axis.diSetPosition = static_cast<int32_t>(value);
            break;
        case PARA_diSetVelocity:
            axis.diSetVelocity = static_cast<int32_t>(value);
            break;
        case PARA_diSetAcceleration:
            axis.diSetAcceleration = static_cast<int32_t>(value);
            break;
        case PARA_diSetCurrent:
            axis.diSetCurrent = static_cast<int32_t>(value);
            break;
        case PARA_diSetTorque:
            axis.diSetTorque = static_cast<int32_t>(value);
            break;
        case PARA_fSWLimitDeceleration:
            if (value < 0) return -2;  // Validate deceleration
            axis.fSWLimitDeceleration = value;
            break;

        // Actual position/velocity/acceleration/torque parameters (writable)
        case PARA_fActPosition:
            axis.fActPosition = value;
            break;
        case PARA_fActVelocity:
            axis.fActVelocity = value;
            break;
        case PARA_fActAcceleration:
            axis.fActAcceleration = value;
            break;
        case PARA_fActTorque:
            axis.fActTorque = value;
            break;
        case PARA_fActJerk:
        case PARA_fActCurrent:
            return -1;
            break;
        // Real state parameters (writable)
        case PARA_bRegulatorRealState:
            axis.regulatorRealState = (value != 0.0);
            break;
        case PARA_bDriveStartRealState:
            axis.driveStartRealState = (value != 0.0);
            break;

        // Error and controller mode parameters (writable)
        case PARA_dwErrorID:
            axis.dwErrorID = static_cast<uint16_t>(value);
            break;
        case PARA_byRealControllerMode:
            axis.byRealControllerMode = static_cast<uint8_t>(value);
            break;

        // Movement type and direction parameters (writable)
        case PARA_iMovementType:
            // Convert to iMovementType: 0 = finite, 1 = modulo
            axis.iMovementType = (static_cast<int>(value) != 0);
            break;
        case PARA_nDirection:
            axis.nDirection = static_cast<int8_t>(value);
            break;

        // Integer actual position/velocity/acceleration/current/torque parameters (writable)
        case PARA_diActPosition:
            axis.diActPosition = static_cast<int32_t>(value);
            break;
        case PARA_diActVelocity:
            axis.diActVelocity = static_cast<int32_t>(value);
            break;
        case PARA_diActAcceleration:
            axis.diActAcceleration = static_cast<int32_t>(value);
            break;
        case PARA_diActCurrent:
            axis.diActCurrent = static_cast<int32_t>(value);
            break;
        case PARA_diActTorque:
            axis.diActTorque = static_cast<int32_t>(value);
            break;

        // Software error max distance parameter (writable)
        case PARA_fSWErrorMaxDistance:
            if (value < 0) return -2;  // Validate distance
            axis.fSWErrorMaxDistance = value;
            break;

        // Jerk and position parameters (writable)
        case PARA_fSWMaxJerk:
            if (value <= 0) return -2;  // Validate jerk
            axis.fSWMaxJerk = value;
            break;
        case PARA_dwLastPosition:
            axis.dwLastPosition = static_cast<uint32_t>(value);
            break;

        // Position lag parameters (writable)
        case PARA_eCheckPositionLag:
            axis.eCheckPositionLag = static_cast<uint8_t>(value);
            break;
        case PARA_fMaxPositionLag:
            if (value < 0) return -2;  // Validate position lag
            axis.fMaxPositionLag = value;
            break;
        case PARA_bPositionLagActive:
            axis.bPositionLagActive = (value != 0.0);
            break;

        // Motion state parameters (writable)
        case PARA_bAccelerating:
            axis.Accelerating = (value != 0.0);
            break;
        case PARA_bDecelerating:
            axis.Decelerating = (value != 0.0);
            break;
        case PARA_bConstantVelocity:
            axis.ConstantVelocity = (value != 0.0);
            break;
        case PARA_bError:
            axis.bError = (value != 0);
            break;
        default:
            return -1;  // Parameter not supported or not writable
    }

    return 0;  // Success
}

FbtReadParameter::FbtReadParameter(){
    all_zero_init<FbtReadParameterStruct>(*this);
}
FbtReadParameter::~FbtReadParameter(){}
void FbtReadParameter::runCycle() {
    if (Enable) {
        if (Axis) {
            double ParameterValue;
            int ret = mcReadParameter(*Axis, ParameterNumber, ParameterValue);
            if (ret)
                return synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
            Value = ParameterValue;
            synEnFbStat(*this, FB_STAT_VALID);
        }else{
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    }else{
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}
FbtReadBoolParameter::FbtReadBoolParameter(){
    all_zero_init<FbtReadBoolParameterStruct>(*this);
}
FbtReadBoolParameter::~FbtReadBoolParameter(){}
void FbtReadBoolParameter::runCycle(){
    if (Enable) {
        if (Axis) {
            LREAL tmpValue;
            int ret = mcReadParameter(*Axis, ParameterNumber, tmpValue);
            if (ret)
                return synEnFbError(*this, ZMC_RP_PARAM_UNKNOWN);
            Value = tmpValue;
            synEnFbStat(*this, FB_STAT_VALID);
        }else{
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    }else{
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}


class FbtWriteParameterImpl : public FbtExeTmpl<FbtWriteParameterImpl, FbtImplParamTmpl<FbtWriteParameterStruct>> {
   public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onExecuting() {
        auto& param = *this->pParam;
        int ret = mcWriteParameter(*param.Axis, pParam->ParameterNumber, pParam->Value);
        if (ret) 
            return this->onError(ret == -1 ? ZMC_WP_PARAM_INVALID : ZMC_ERR_WP_VALUE_INVALID);
        this->fbStat = FB_STAT_DONE;
    }
    void onCapture(){
        this->execute = this->pParam->Execute;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

struct FbtWriteParameterImpl0 {
    FbStatEnum fbStat = FB_STAT_IDLE;
    BOOL_T _executeBak = false;
    BOOL_T isBusy() const { return fbStat == FB_STAT_BUSY_ACTIVE || fbStat == FB_STAT_VALID; }
    BOOL_T isDoneErrorAborted() const { return fbStat == FB_STAT_DONE || fbStat == FB_STAT_EN_ERROR || fbStat == FB_STAT_ABORTED; }
};

template <typename T, typename POD, typename R>
inline void make_param_impl(R& p) {
    all_zero_init<POD>(p);
    auto ptr = new T();
    p.pImpl = ptr;
    ptr->pParam = &p;
}

FbtWriteParameter::FbtWriteParameter() { 
    make_param_impl<FbtWriteParameterImpl, FbtWriteParameterStruct>(*this);
}
FbtWriteParameter::~FbtWriteParameter() { delete static_cast<FbtWriteParameterImpl*>(pImpl); }

void FbtWriteParameter::runCycle() {
    auto impl = static_cast<FbtWriteParameterImpl*>(pImpl);
    impl->runCycle();
}
#if 0
void FbtWriteParameter::runCycle() {
    auto impl = static_cast<FbtWriteParameterImpl*>(pImpl);
    bool exeBak = impl->_executeBak;

    if (this->Execute && !exeBak) {
        if (!Axis) {
            synExeFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        impl->fbStat = FB_STAT_BUSY_ACTIVE;
    }
    impl->_executeBak = this->Execute;
    if (impl->isBusy()) {
        int ret = mcWriteParameter(*Axis, ParameterNumber, Value);
        if (ret) return synExeFbError(*this, ZMC_WP_PARAM_INVALID);
        synExeFbStat(*this, FB_STAT_DONE);
    } else if (impl->isDoneErrorAborted() && !this->Execute) {
        synExeFbStat(*this, FB_STAT_IDLE);
        ErrorID = ZMC_NO_ERROR;
    }
}
#endif


class FbtWriteBoolParameterImpl : public FbtExeTmpl<FbtWriteBoolParameterImpl, FbtImplParamTmpl<FbtWriteBoolParameterStruct>> {
   public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onExecuting() {
        auto& param = *this->pParam;
        LREAL val = pParam->Value? 1.0 : 0.0;
        int ret = mcWriteParameter(*param.Axis, pParam->ParameterNumber, val);
        if (ret) return this->onError(ZMC_WP_PARAM_INVALID);
        this->fbStat = FB_STAT_DONE;
    }
    void onCapture(){
        this->execute = this->pParam->Execute;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtWriteBoolParameter::FbtWriteBoolParameter() { 
    make_param_impl<FbtWriteBoolParameterImpl, FbtWriteBoolParameterStruct>(*this);
}

FbtWriteBoolParameter::~FbtWriteBoolParameter() { 
    delete static_cast<FbtWriteBoolParameterImpl*>(pImpl); 
}

void FbtWriteBoolParameter::runCycle() {
    if (!pImpl) return;
    auto impl = static_cast<FbtWriteBoolParameterImpl*>(pImpl);
    impl->runCycle();
    // bool exeBak = impl->_executeBak;

    // if (this->Execute && !exeBak) {
    //     if (!Axis) {
    //         synExeFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
    //         return;
    //     }
    //     impl->fbStat = FB_STAT_BUSY_ACTIVE;
    // }
    // impl->_executeBak = this->Execute;
    // if (impl->isBusy()) {
    //     BOOL_T bValue = false;
    //     int ret = mcWriteParameter(*Axis, ParameterNumber, bValue);
    //     if (ret) return synExeFbError(*this, ZMC_WP_PARAM_INVALID);
    //     Value = bValue;
    //     synExeFbStat(*this, FB_STAT_DONE);
    // } else if (impl->isDoneErrorAborted() && !this->Execute) {
    //     synExeFbStat(*this, FB_STAT_IDLE);
    //     ErrorID = ZMC_NO_ERROR;
    // }
}

// Default watchdog budget for one CoE request: covers FIFO wait, one mailbox
// round trip at the worker timeout, and the result pump divider delay.
static const uint32_t kCoeTimeoutDefaultCycles = 1000u;

// Narrow one LREAL to the signed integer width selected by len (1/2/4 bytes).
// Returns 0 on success (truncating toward zero like SoftMotion) or -1 when
// the value lies outside the target range.
static int coe_narrow_value(ZMC_LREAL value, uint8_t len, int32_t* out) {
    if (!out) return -1;
    // Truncate the fractional part before the range check, never round it in.
    const double truncated = (value >= 0.0) ? std::floor(value) : std::ceil(value);
    switch (len) {
        case 1u:
            if (truncated < -128.0 || truncated > 127.0) return -1;
            *out = static_cast<int32_t>(truncated);
            return 0;
        case 2u:
            if (truncated < -32768.0 || truncated > 32767.0) return -1;
            *out = static_cast<int32_t>(truncated);
            return 0;
        case 4u:
            if (truncated < -2147483648.0 || truncated > 2147483647.0) return -1;
            *out = static_cast<int32_t>(truncated);
            return 0;
        default:
            return -1;
    }
}

// One-shot CoE SDO write through the axis CoE channel. Only parameter
// numbers encoded by zmc_parameter_number_coe are accepted; this block
// never writes local axis fields, so local PARA_ numbers are rejected.
class FbtWriteCoeParameterImpl
    : public FbtExeTmpl<FbtWriteCoeParameterImpl, FbtImplParamTmpl<FbtWriteCoeParameterStruct>> {
   public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        // Decode the CoE layout; decode is the single validation authority,
        // so a zero return covers local parameter numbers and bad lengths.
        uint16_t index = 0;
        uint8_t subIndex = 0;
        uint8_t dataLength = 0;
        if (!zmc_parameter_coe_decode(param.ParameterNumber, &index, &subIndex, &dataLength)) {
            this->onError(ZMC_WP_PARAM_INVALID);
            return;
        }
        // Narrow the LREAL value into the signed width named by dataLength.
        int32_t value = 0;
        if (coe_narrow_value(param.Value, dataLength, &value) != 0) {
            this->onError(ZMC_ERR_WP_VALUE_INVALID);
            return;
        }
        // Queue on the axis FIFO; queued FBs simply wait as PENDING.
        uint32_t tag = 0;
        const int32_t enq =
            axis_coe_req_enqueue(param.Axis, index, subIndex, dataLength, value, &tag);
        if (enq != 0) {
            // -1 marks a full axis FIFO; anything else is an invalid argument.
            this->onError(enq == -1 ? ZMC_ERR_WP_SDO_QUEUE_FULL : ZMC_WP_PARAM_INVALID);
            return;
        }
        tag_ = tag;
        age_ = 0;
        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onExecuting() {
        auto& param = *this->pParam;
        ++age_;
        // Watchdog covers queue wait, mailbox transfer and pump divider delay.
        const uint32_t budget =
            param.TimeoutCycles > 0 ? static_cast<uint32_t>(param.TimeoutCycles)
                                    : kCoeTimeoutDefaultCycles;
        if (age_ > budget) {
            return this->onError(ZMC_ERR_WP_SDO_TIMEOUT);
        }
        int32_t code = 0;
        const int32_t state = axis_coe_result_query(param.Axis, tag_, &code);
        if (state == AXIS_COE_RESULT_PENDING) {
            return;  // Still queued or in flight; stay busy and retry next cycle
        }
        if (state == AXIS_COE_RESULT_SUPERSEDED) {
            return this->onError(ZMC_ERR_WP_SDO_TIMEOUT);
        }
        if (code != 0) {
            // The raw int32 transport code stays in the axis result ring for
            // diagnostics; ErrorID carries the ZMC semantic code only.
            return this->onError(ZMC_ERR_WP_SDO_ABORTED);
        }
        this->fbStat = FB_STAT_DONE;
    }

    void onCapture() { this->execute = this->pParam->Execute; }

    void onUpdate() {
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }

   private:
    uint32_t tag_ = 0;  // Axis-local request id; 0 while idle
    uint32_t age_ = 0;  // Cycles since the Execute rising edge
};

// One-shot CoE SDO read through the axis CoE channel. This block only forwards
// the completed transport payload into Value; it does not interpret object data.
class FbtReadCoeParameterImpl
    : public FbtExeTmpl<FbtReadCoeParameterImpl, FbtImplParamTmpl<FbtReadCoeParameterStruct>> {
   public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        uint16_t index = 0;
        uint8_t subIndex = 0;
        uint8_t dataLength = 0;
        if (!zmc_parameter_coe_decode(param.ParameterNumber, &index, &subIndex, &dataLength)) {
            this->onError(ZMC_WP_PARAM_INVALID);
            return;
        }
        uint32_t tag = 0;
        const int32_t enq =
            axis_coe_req_enqueue_read(param.Axis, index, subIndex, dataLength, &tag);
        if (enq != 0) {
            this->onError(enq == -1 ? ZMC_ERR_WP_SDO_QUEUE_FULL : ZMC_WP_PARAM_INVALID);
            return;
        }
        tag_ = tag;
        age_ = 0;
        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onExecuting() {
        auto& param = *this->pParam;
        ++age_;
        const uint32_t budget =
            param.TimeoutCycles > 0 ? static_cast<uint32_t>(param.TimeoutCycles)
                                    : kCoeTimeoutDefaultCycles;
        if (age_ > budget) {
            return this->onError(ZMC_ERR_WP_SDO_TIMEOUT);
        }
        int32_t code = 0;
        uint32_t data = 0;
        const int32_t state = axis_coe_result_query(param.Axis, tag_, &code, &data);
        if (state == AXIS_COE_RESULT_PENDING) {
            return;
        }
        if (state == AXIS_COE_RESULT_SUPERSEDED) {
            return this->onError(ZMC_ERR_WP_SDO_TIMEOUT);
        }
        if (code != 0) {
            return this->onError(ZMC_ERR_WP_SDO_ABORTED);
        }
        param.Value = static_cast<ZMC_LREAL>(data);
        this->fbStat = FB_STAT_DONE;
    }

    void onCapture() { this->execute = this->pParam->Execute; }

    void onUpdate() {
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }

   private:
    uint32_t tag_ = 0;  // Axis-local request id; 0 while idle
    uint32_t age_ = 0;  // Cycles since the Execute rising edge
};

FbtWriteCoeParameter::FbtWriteCoeParameter() {
    make_param_impl<FbtWriteCoeParameterImpl, FbtWriteCoeParameterStruct>(*this);
}

FbtWriteCoeParameter::~FbtWriteCoeParameter() {
    delete static_cast<FbtWriteCoeParameterImpl*>(pImpl);
}

void FbtWriteCoeParameter::runCycle() {
    if (!pImpl) return;
    static_cast<FbtWriteCoeParameterImpl*>(pImpl)->runCycle();
}

FbtReadCoeParameter::FbtReadCoeParameter() {
    make_param_impl<FbtReadCoeParameterImpl, FbtReadCoeParameterStruct>(*this);
}

FbtReadCoeParameter::~FbtReadCoeParameter() {
    delete static_cast<FbtReadCoeParameterImpl*>(pImpl);
}

void FbtReadCoeParameter::runCycle() {
    if (!pImpl) return;
    static_cast<FbtReadCoeParameterImpl*>(pImpl)->runCycle();
}

}
