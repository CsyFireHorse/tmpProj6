#include <stdio.h>
#include "fbt_axis_impl.h"
#include "fbt_fb_base.hpp"
#include "fb_cpp_def.h"
#include <cmath>
#include "misc_helper.hpp"
#include "fbt_impl_enum_def.h"
#include "zmc_fb_predef.h"

namespace zmc {

class FbtImplReset : public FbtExeTmpl<FbtImplReset, FbtImplParamTmpl<FbtResetStruct>> {
   public:
    FbtImplReset() { fbType = FB_TYPE_RESET; }
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        auto& ax = get_axis_impl_ref(param.Axis);
        if (ax.axisStat != mcErrorStop) {
            this->onError(ZMC_R_NO_ERROR_TO_RESET);
            return;
        }
        this->fbStat = FB_STAT_BUSY_ACTIVE;
        _cnt = 0;
    }

    void onExecuting() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        if (ax.communicationError || ax.notOperationalError) {
            // Active fieldbus faults cannot be reset until a healthy ECAT snapshot arrives.
            this->fbStat = FB_STAT_ERROR;
            error_code = SMC_R_CANNOT_RESET_COMMUNICATION_ERROR;
            return;
        }
        if (ax.driveError)
            ax.cmd = AXIS_CMD_RESET;
        else
            ax.bClearAxError = true;
        if (_cnt++ > ZMC_FB_RESET_TIMEOUT_COUNT) {
            this->fbStat = FB_STAT_ERROR;
            error_code = ZMC_ERR_R_TIMEOUT;
            return;
        }
        if (!ax.driveError && !ax.axisSwError && ax.getCommunicationErrorId() == ZMC_NO_ERROR &&
            ax.axisStat == mcErrorStop) {
            ax.axisStat =
                (ax.regulatorRealState && ax.driveStartRealState) ? mcStandstill : mcDisabled;
        }

        if (ax.axisStat != mcErrorStop) {
            this->fbStat = FB_STAT_DONE;
        }
    }
    void onCapture() { this->execute = this->pParam->Execute; }
    void onUpdate() {
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }

   private:
    int _cnt = 0;
};

FbtReset::FbtReset() {
    make_impl<FbtImplReset, FbtResetStruct>(*this);
    // all_zero_init<FbtResetStruct>(*this);
    // pImpl = new FbtImplReset();
    // static_cast<FbtImplReset*>(pImpl)->pParam = this;
}

FbtReset::~FbtReset() { delete static_cast<FbtImplReset*>(pImpl); }

void FbtReset::runCycle() {
    auto impl = static_cast<FbtImplReset*>(pImpl);
    impl->runCycle();
}

// struct FbtSetPositionImpl {
//     FbStatEnum fbStat = FB_STAT_IDLE;
//     BOOL_T _executeBak = false;
// };

void axisInfoShift(AxisImplBase& axis, LREAL offset) {
    const auto& fModuloValue = axis.fPositionPeriod;
    if (axis.is_modulo_type()) {
        axis.fActPosition = fmod(axis.fActPosition + offset + fModuloValue, fModuloValue);
        axis.fSetPosition = fmod(axis.fSetPosition + offset + fModuloValue, fModuloValue);
        axis.fSavePosition = fmod(axis.fSavePosition + offset + fModuloValue, fModuloValue);
        axis.fAimPosition = fmod(axis.fAimPosition + offset + fModuloValue, fModuloValue);
        axis.fMarkPosition = fmod(axis.fMarkPosition + offset + fModuloValue, fModuloValue);
        axis.fOffsetPosition = fmod(axis.fOffsetPosition + offset + fModuloValue, fModuloValue);
    } else {
        axis.fActPosition += offset;
        axis.fSetPosition += offset;
        axis.fSavePosition += offset;
        axis.fMarkPosition += offset;
        axis.fAimPosition += offset;
        axis.fOffsetPosition += offset;
    }
}

class FbtSetPositionImpl
    : public FbtExeTmpl<FbtSetPositionImpl, FbtImplParamTmpl<FbtSetPositionStruct>> {
   public:
    FbtSetPositionImpl() { fbType = FB_TYPE_SETPOSITION; }
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        auto& ax = get_axis_impl_ref(param.Axis);
        if (ax.axisStat == mcErrorStop) {
            this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_ERRORSTOP);
            return;
        }
        if (ax.axisStat != mcStandstill && ax.axisStat != mcDisabled) {
            this->onError(ZMC_ERR_SETPOSITION_WHILE_MOVING);
            return;
        }
        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onExecuting() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);
        // Calculate new position
        LREAL ofs;
        if (pParam->Mode) {
            ofs = pParam->Position;
        } else {
            ofs = pParam->Position - ax.fSetPosition;
        }
        axisInfoShift(ax, ofs);
        fbStat = FB_STAT_DONE;
    }
    void onCapture() { this->execute = this->pParam->Execute; }
    void onUpdate() {
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtSetPosition::FbtSetPosition() { make_impl<FbtSetPositionImpl, FbtSetPositionStruct>(*this); }

FbtSetPosition::~FbtSetPosition() { delete static_cast<FbtSetPositionImpl*>(pImpl); }

void FbtSetPosition::runCycle() {
    auto impl = static_cast<FbtSetPositionImpl*>(pImpl);
    impl->runCycle();
}

class FbtImplSetMovementType
    : public FbtExeTmpl<FbtImplSetMovementType, FbtImplParamTmpl<FbtSetMovementTypeStruct>> {
   public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        auto& ax = get_axis_impl_ref(param.Axis);
        // Check axis state - only allow in standstill or disabled state
        if (ax.axisStat != mcStandstill && ax.axisStat != mcDisabled) {
            this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_ERRORSTOP);
            return;
        }

        // Validate movement type (0 = modulo, 1 = linear)
        if (param.iMovementType != MC_MOVEMENT_MODULO &&
            param.iMovementType != MC_MOVEMENT_LINEAR) {
            return this->onError(ZMC_WP_PARAM_INVALID);
        }

        // For modulo movement, validate position period
        if (param.iMovementType == MC_MOVEMENT_MODULO && param.fPositionPeriod <= 0) {
            return this->onError(ZMC_NEGATIVE_MODULO_PERIOD);
        }
        // this->fbStat = FB_STAT_BUSY_ACTIVE;
        // Apply settings to axis
        ax.iMovementType = param.iMovementType;
        if (ax.is_modulo_type()) {
            ax.fPositionPeriod = param.fPositionPeriod;
        }
        this->fbStat = FB_STAT_DONE;
    }

    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->bDone = this->isDone();
        this->pParam->bError = this->isError();
        this->pParam->eErrorID = this->getErrorID();
    }
};

FbtSetMovementType::FbtSetMovementType() {
    make_impl<FbtImplSetMovementType, FbtSetMovementTypeStruct>(*this);
    fPositionPeriod = 1.0;  // Default position period
}

FbtSetMovementType::~FbtSetMovementType() {
    if (pImpl) {
        delete static_cast<FbtImplSetMovementType*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtSetMovementType::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplSetMovementType*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplSetRampType
    : public FbtExeTmpl<FbtImplSetRampType, FbtImplParamTmpl<FbtSetRampTypeStruct>> {
   public:
    FbtImplSetRampType() { fbType = FB_TYPE_SETRAMPTYPE; }
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);
        if (!ax.is_axis_valid()) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        // Check axis state - only allow in standstill or disabled state
        if (ax.axisStat != mcStandstill && ax.axisStat != mcDisabled) {
            return this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_ERRORSTOP);
        }

        // Validate ramp type (0-3 are valid values)
        if (param.eRampType < trapez || param.eRampType > quadratic_smooth_ramp) {
            return this->onError(ZMC_WP_PARAM_INVALID);
        }

        // this->fbStat = FB_STAT_BUSY_ACTIVE;
        // Apply ramp type to axis
        ax.eRampType = param.eRampType;
        this->fbStat = FB_STAT_DONE;
    }
    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->bDone = this->isDone();
        this->pParam->bError = this->isError();
        this->pParam->eErrorID = this->getErrorID();
    }
};

FbtSetRampType::FbtSetRampType() { make_impl<FbtImplSetRampType, FbtSetRampTypeStruct>(*this); }

FbtSetRampType::~FbtSetRampType() {
    if (pImpl) {
        delete static_cast<FbtImplSetRampType*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtSetRampType::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplSetRampType*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplChangeDynamicLimits
    : public FbtExeTmpl<FbtImplChangeDynamicLimits,
                        FbtImplParamTmpl<FbtChangeDynamicLimitsStruct>> {
   public:
    FbtImplChangeDynamicLimits() { fbType = FB_TYPE_CHANGEDYNAMICLIMITS; }
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);

        // Check axis state - only allow in standstill or disabled state
        if (ax.axisStat != mcStandstill && ax.axisStat != mcDisabled) {
            this->onError(ZMC_SDL_INVALID_AXIS_STATE);
            return;
        }

        // Validate dynamic limits - all must be positive
        if (param.fMaxVelocity <= 0 || param.fMaxAcceleration <= 0 ||
            param.fMaxDeceleration <= 0 || param.fMaxJerk <= 0) {
            this->onError(ZMC_SDL_INVALID_VELACC_VALUES);
            return;
        }

        // Apply dynamic limits to axis
        ax.fMaxVelocity = param.fMaxVelocity;
        ax.fMaxAcceleration = param.fMaxAcceleration;
        ax.fMaxDeceleration = param.fMaxDeceleration;
        ax.fSWMaxJerk = param.fMaxJerk;
        if (param.fMaxTorque > 0) {
            ax.fMaxTorque = param.fMaxTorque;
        }

        this->fbStat = FB_STAT_DONE;
    }

    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->bDone = this->isDone();
        this->pParam->bBusy = this->isBusy();
        this->pParam->bError = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtChangeDynamicLimits::FbtChangeDynamicLimits() {
    make_impl<FbtImplChangeDynamicLimits, FbtChangeDynamicLimitsStruct>(*this);
    fMaxTorque = 1e19;  // Default to positive infinity
}

FbtChangeDynamicLimits::~FbtChangeDynamicLimits() {
    if (pImpl) {
        delete static_cast<FbtImplChangeDynamicLimits*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtChangeDynamicLimits::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplChangeDynamicLimits*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplChangeGearingRatio
    : public FbtExeTmpl<FbtImplChangeGearingRatio, FbtImplParamTmpl<FbtChangeGearingRatioStruct>> {
   public:
    FbtImplChangeGearingRatio() { fbType = FB_TYPE_CHANGEGEARINGRATIO; }
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);

        // Check axis state - only allow in standstill or disabled state
        if (ax.axisStat != mcStandstill && ax.axisStat != mcDisabled) {
            this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_ERRORSTOP);
            return;
        }

        // Validate parameters
        if (param.dwRatioTechUnitsDenom == 0) {
            this->onError(ZMC_CGR_ZERO_VALUES);
            return;
        }

        if (param.iMovementType != MC_MOVEMENT_MODULO &&
            param.iMovementType != MC_MOVEMENT_LINEAR) {
            this->onError(ZMC_WP_PARAM_INVALID);
            return;
        }

        // For modulo movement, validate position period
        if (param.iMovementType == MC_MOVEMENT_MODULO && param.fPositionPeriod <= 0) {
            this->onError(ZMC_NEGATIVE_MODULO_PERIOD);
            return;
        }

        if (1) {
            // fSetPos = dSetPos /num*den + ofs;
            // fSetPos2 = dSetPos / num2*den2 + ofs;
            double ofsBak = ax.fOffsetPosition;
            double diPos = (ax.fSavePosition - ax.fOffsetPosition) * ax.iRatioTechUnitsNum /
                           ax.dwRatioTechUnitsDenom;
            double posNew = diPos / param.iRatioTechUnitsNum * param.dwRatioTechUnitsDenom +
                            ax.fOffsetPosition;
            double diff = posNew - ax.fSavePosition;
            // Apply gearing ratio and movement type to axis
            ax.iRatioTechUnitsNum = param.iRatioTechUnitsNum;
            ax.dwRatioTechUnitsDenom = param.dwRatioTechUnitsDenom;
            axisInfoShift(ax, diff);
            ax.fOffsetPosition = ofsBak;
            // printf("diPos calc=%f, diPos=%d\n", diPos, ax.diSetPosition);
            // printf("diff=%f, ofsBak=%f\n", diff, ofsBak);
        } else {
            // fSetPos = dSetPos /num*den + ofs;
            // fSetPos = dSetPos / num2*den2 + ofs2;
        }
        // only use for gearing modulo
        if (!this->bFlagSpec) {
            ax.iMovementType = param.iMovementType;
            if (ax.is_modulo_type()) {
                ax.fPositionPeriod = param.fPositionPeriod;
            }
        }
        this->fbStat = FB_STAT_DONE;
    }

    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->bDone = this->isDone();
        this->pParam->bBusy = this->isBusy();
        this->pParam->bError = this->isError();
        this->pParam->nErrorID = this->getErrorID();
    }
};

FbtChangeGearingRatio::FbtChangeGearingRatio() {
    make_impl<FbtImplChangeGearingRatio, FbtChangeGearingRatioStruct>(*this);
    dwRatioTechUnitsDenom = 1;  // Default denominator
    iRatioTechUnitsNum = 1;     // Default numerator
    fPositionPeriod = 1.0;      // Default position period
}

FbtChangeGearingRatio::~FbtChangeGearingRatio() {
    if (pImpl) {
        delete static_cast<FbtImplChangeGearingRatio*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtChangeGearingRatio::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplChangeGearingRatio*>(pImpl);
        impl->runCycle();
    }
}

FbtChangeRatio::FbtChangeRatio() {
    make_impl<FbtImplChangeGearingRatio, FbtChangeGearingRatioStruct>(
        (FbtChangeGearingRatio&)*this);
    dwRatioTechUnitsDenom = 1;  // Default denominator
    iRatioTechUnitsNum = 1;     // Default numerator
    fPositionPeriod = 1.0;      // Default position period
    setFbImplFlagSpec(pImpl, true);
}

FbtChangeRatio::~FbtChangeRatio() {
    if (pImpl) {
        delete static_cast<FbtImplChangeGearingRatio*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtChangeRatio::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplChangeGearingRatio*>(pImpl);
        impl->runCycle();
    }
}

// Note The effective error deceleration is the maximum of the configured maximum deceleration of
// the axis, the value of SWL_Error_Deceleration, and the value derived from SWL_Error_MaxDistance
// and the current velocity.
//(SWL_Error_MaxDistance is only taken into account if the value is positive.)
class FbtImplSetSoftwareLimits
    : public FbtExeTmpl<FbtImplSetSoftwareLimits, FbtImplParamTmpl<FbtSetSoftwareLimitsStruct>> {
   public:
    FbtImplSetSoftwareLimits() { fbType = FB_TYPE_SETSOFTWARELIMITS; }
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);
        if (ax.axisStat != mcStandstill && ax.axisStat != mcDisabled) {
            return this->onError(ZMC_SDL_INVALID_AXIS_STATE);
        }

        // Validate software limits - positive limit should be greater than negative limit
        if (param.SWL_Activated && param.SWL_Positive <= param.SWL_Negative) {
            this->onError(ZMC_WP_PARAM_INVALID);
            return;
        }

        // Validate error deceleration - must be positive if specified
        if (param.SWL_Error_Deceleration < 0) {
            this->onError(ZMC_SDL_INVALID_VELACC_VALUES);
            return;
        }

        // Apply software limits to axis
        ax.bSWLimitEnable = param.SWL_Activated;
        if (param.SWL_Activated) {
            ax.fSWLimitPositive = param.SWL_Positive;
            ax.fSWLimitNegative = param.SWL_Negative;
        }

        if (param.SWL_Error_Deceleration > 0) {
            ax.fSWLimitDeceleration = param.SWL_Error_Deceleration;
        }

        if (param.SWL_Error_MaxDistance > 0) {
            ax.fSWErrorMaxDistance = param.SWL_Error_MaxDistance;
        }

        this->fbStat = FB_STAT_DONE;
    }

    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->bDone = this->isDone();
        this->pParam->bError = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtSetSoftwareLimits::FbtSetSoftwareLimits() {
    make_impl<FbtImplSetSoftwareLimits, FbtSetSoftwareLimitsStruct>(*this);
}

FbtSetSoftwareLimits::~FbtSetSoftwareLimits() {
    if (pImpl) {
        delete static_cast<FbtImplSetSoftwareLimits*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtSetSoftwareLimits::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplSetSoftwareLimits*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplSetControllerMode
    : public FbtExeTmpl<FbtImplSetControllerMode, FbtImplParamTmpl<FbtSetControllerModeStruct>> {
   public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);

        // Check axis state - not allowed in errorstop, stopping or homing
        if (ax.axisStat == mcErrorStop || ax.axisStat == mcStopping || ax.axisStat == mcHoming) {
            this->onError(ZMC_SCM_AXIS_IN_WRONG_STATE);
            return;
        }

        // Validate controller mode
        if (param.nControllerMode > 11) {  // ZMC_current
            this->onError(ZMC_SCM_NOT_SUPPORTED);
            return;
        }

        // Set controller mode and start waiting
        ax.byControllerMode = param.nControllerMode;
        _waitCycles = 0;

        // Calculate expected set values for higher level control modes
        calculateExpectedSetValues(ax);

        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onExecuting() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);

        // Check if mode switch is complete
        if (ax.byRealControllerMode == param.nControllerMode) {
            this->fbStat = FB_STAT_DONE;
            return;
        }

        // Check for timeout (1000 cycles)
        if (++_waitCycles >= ZMC_FB_CTRL_MODE_TIMEOUT_COUNT) {
            this->onError(ZMC_ERR_SCM_MODE_SWITCH_TIMEOUT);
            return;
        }

        // Continue to set the controller mode
        ax.byControllerMode = param.nControllerMode;
    }

    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->bDone = this->isDone();
        this->pParam->bBusy = this->isBusy();
        this->pParam->bError = this->isError();
        this->pParam->nErrorID = this->getErrorID();
    }

   private:
    void calculateExpectedSetValues(AxisImplBase& ax) {
        auto& param = *this->pParam;

        // When switching from lower to higher level control mode,
        // calculate expected set values to compensate for time lag

        return;
        switch (param.nControllerMode) {
            case ZMC_position:
                // For position mode, calculate expected position based on current motion
                if (ax.fSetActTimeLagCycles > 0) {
                    double expectedDistance =
                        ax.fActVelocity * ax.fSetActTimeLagCycles * ax.cycleTime;
                    ax.fSetPosition = ax.fActPosition + expectedDistance;
                } else {
                    ax.fSetPosition = ax.fActPosition;
                }
                break;
            case ZMC_velocity:
                // For velocity mode, use current velocity
                ax.fSetVelocity = ax.fActVelocity;
                break;
            case ZMC_torque:
                // For torque mode, use current torque
                ax.fSetTorque = ax.fActTorque;
                break;
        }
    }

   private:
    uint16_t _waitCycles = 0;
};

FbtSetControllerMode::FbtSetControllerMode() {
    make_impl<FbtImplSetControllerMode, FbtSetControllerModeStruct>(*this);
    nControllerMode = ZMC_position;  // Default to position mode
}

FbtSetControllerMode::~FbtSetControllerMode() {
    if (pImpl) {
        delete static_cast<FbtImplSetControllerMode*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtSetControllerMode::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplSetControllerMode*>(pImpl);
        impl->runCycle();
    }
}

// Empty implementation classes for interface-only function blocks
class FbtImplSetCustomRampType
    : public FbtExeTmpl<FbtImplSetCustomRampType, FbtImplParamTmpl<FbtSetCustomRampTypeStruct>> {
   public:
    void onStarted() {
        // Empty implementation - interface only
        this->fbStat = FB_STAT_DONE;
    }

    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->bDone = this->isDone();
        this->pParam->bError = this->isError();
        this->pParam->eErrorID = this->getErrorID();
    }
};

FbtSetCustomRampType::FbtSetCustomRampType() {
    make_impl<FbtImplSetCustomRampType, FbtSetCustomRampTypeStruct>(*this);
}

FbtSetCustomRampType::~FbtSetCustomRampType() {
    if (pImpl) {
        delete static_cast<FbtImplSetCustomRampType*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtSetCustomRampType::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplSetCustomRampType*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplSetAdditionalConversionFactors
    : public FbtExeTmpl<FbtImplSetAdditionalConversionFactors,
                        FbtImplParamTmpl<FbtSetAdditionalConversionFactorsStruct>> {
   public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);

        // Check axis state - only allow in standstill or disabled state
        if (ax.axisStat != mcStandstill && ax.axisStat != mcDisabled) {
            this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_ERRORSTOP);
            return;
        }

        // Validate torque factor - must not be 0
        if (param.TorqueFactor == 0.0) {
            this->onError(ZMC_WP_PARAM_INVALID);
            return;
        }

        // Apply conversion factor to axis
        // The factor is multiplied by set values before sending to drive
        // and actual values are divided by the factor after receiving from drive
        ax.fFactorTor = param.TorqueFactor;

        this->fbStat = FB_STAT_DONE;
    }

    void onCapture() { this->execute = this->pParam->Execute; }

    void onUpdate() {
        this->pParam->Done = this->isDone();
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtSetAdditionalConversionFactors::FbtSetAdditionalConversionFactors() {
    make_impl<FbtImplSetAdditionalConversionFactors, FbtSetAdditionalConversionFactorsStruct>(
        *this);
    TorqueFactor = 1.0;  // Default torque factor
}

FbtSetAdditionalConversionFactors::~FbtSetAdditionalConversionFactors() {
    if (pImpl) {
        delete static_cast<FbtImplSetAdditionalConversionFactors*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtSetAdditionalConversionFactors::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplSetAdditionalConversionFactors*>(pImpl);
        impl->runCycle();
    }
}

// Empty implementation classes for MeasureDistance and AxisDiagnosticLog
class FbtImplMeasureDistance
    : public FbtExeTmpl<FbtImplMeasureDistance, FbtImplParamTmpl<FbtMeasureDistanceStruct>> {
   public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);

        // Check if axis is a rotary axis (modulo type)
        if (!ax.is_modulo_type()) {
            this->onError(ZMC_WP_PARAM_INVALID);  // Not a rotary axis
            return;
        }

        // Initialize measurement
        _startPosition = ax.fActPosition;
        _totalDistance = 0.0;
        _lastPosition = ax.fActPosition;
        _wraps = 0;

        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onExecuting() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);

        // Calculate distance considering wraps
        double currentPosition = ax.fActPosition;
        double positionDiff = currentPosition - _lastPosition;

        // Handle wrap-around for modulo axis - only to count wraps
        if (ax.is_modulo_type() && ax.fPositionPeriod > 0) {
            // Check for wrap-around in positive direction
            if (positionDiff > ax.fPositionPeriod / 2) {
                _wraps--;
            }
            // Check for wrap-around in negative direction
            else if (positionDiff < -ax.fPositionPeriod / 2) {
                _wraps++;
            }
        }

        _lastPosition = currentPosition;

        // Calculate total distance: (current_position - start_position) + wraps * period
        double positionDistance = currentPosition - _startPosition;
        double wrapDistance = _wraps * ax.fPositionPeriod;
        _totalDistance = positionDistance + wrapDistance;

        // Update output
        param.fDistance = _totalDistance;
    }
    void onClear() {
        _startPosition = 0.0;
        _totalDistance = 0.0;
        _lastPosition = 0.0;
        _wraps = 0;
    }
    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->fDistance = _totalDistance;
        this->pParam->bBusy = this->isBusy();
        this->pParam->bError = this->isError();
        this->pParam->nErrorID = this->getErrorID();
    }

   private:
    double _startPosition = 0.0;
    double _totalDistance = 0.0;
    double _lastPosition = 0.0;
    int _wraps = 0;
};

class FbtImplAxisDiagnosticLog
    : public FbtExeTmpl<FbtImplAxisDiagnosticLog, FbtImplParamTmpl<FbtAxisDiagnosticLogStruct>> {
   public:
    void onStarted() {
        // Empty implementation - interface only
        this->fbStat = FB_STAT_DONE;
    }

    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->bDone = this->isDone();
        this->pParam->bBusy = this->isBusy();
        this->pParam->bError = this->isError();
        this->pParam->ErrorID = this->getErrorID();
        this->pParam->bRecording = false;  // Default not recording
    }
};

FbtMeasureDistance::FbtMeasureDistance() {
    make_impl<FbtImplMeasureDistance, FbtMeasureDistanceStruct>(*this);
    fDistance = 0.0;  // Default distance
}

FbtMeasureDistance::~FbtMeasureDistance() {
    if (pImpl) {
        delete static_cast<FbtImplMeasureDistance*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtMeasureDistance::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplMeasureDistance*>(pImpl);
        impl->runCycle();
    }
}

FbtAxisDiagnosticLog::FbtAxisDiagnosticLog() {
    make_impl<FbtImplAxisDiagnosticLog, FbtAxisDiagnosticLogStruct>(*this);
    eMode = LOG_CONTINUOUS;  // Default mode
    bySeparatorChar = 9;     // Default tab character
    // Initialize string fields
    // memset(sFileName.data, 0, sizeof(sFileName.data));
    // memset(sRecordSeparatorString.data, 0, sizeof(sRecordSeparatorString.data));
    // strcpy(sRecordSeparatorString.data, "$R$N");
}

FbtAxisDiagnosticLog::~FbtAxisDiagnosticLog() {
    if (pImpl) {
        delete static_cast<FbtImplAxisDiagnosticLog*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtAxisDiagnosticLog::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplAxisDiagnosticLog*>(pImpl);
        impl->runCycle();
    }
}

FbtAbortTrigger::FbtAbortTrigger() {}
FbtAbortTrigger::~FbtAbortTrigger() {}
void FbtAbortTrigger::runCycle() {}

FbtTouchProbe::FbtTouchProbe() {}
FbtTouchProbe::~FbtTouchProbe() {}
void FbtTouchProbe::runCycle() {}

// Empty implementation classes for BrakeStatus and BrakeControl
class FbtImplBrakeStatus {
   public:
    FbtBrakeStatusStruct* pParam = nullptr;

    void runCycle() {
        auto& param = *pParam;

        if (!param.bEnable) {
            param.bValid = false;
            param.bBusy = false;
            param.bError = false;
            param.nErrorID = ZMC_NO_ERROR;
            param.bBrakeClosed = false;
            return;
        }

        if (!param.Axis) {
            param.bValid = false;
            param.bBusy = false;
            param.bError = true;
            param.nErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            param.bBrakeClosed = false;
            return;
        }

        // Empty implementation - interface only
        param.bValid = true;
        param.bBusy = false;
        param.bError = false;
        param.nErrorID = ZMC_NO_ERROR;
        param.bBrakeClosed = false;  // Default to brake open
    }
};

class FbtImplBrakeControl {
   public:
    FbtBrakeControlStruct* pParam = nullptr;

    void runCycle() {
        auto& param = *pParam;

        if (!param.bExecute) {
            param.bDone = false;
            param.bBusy = false;
            param.bError = false;
            param.nErrorID = ZMC_NO_ERROR;
            return;
        }

        if (!param.Axis) {
            param.bDone = false;
            param.bBusy = false;
            param.bError = true;
            param.nErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            return;
        }

        // Empty implementation - interface only
        param.bDone = true;
        param.bBusy = false;
        param.bError = false;
        param.nErrorID = ZMC_NO_ERROR;
    }
};

FbtBrakeStatus::FbtBrakeStatus() { make_impl<FbtImplBrakeStatus, FbtBrakeStatusStruct>(*this); }

FbtBrakeStatus::~FbtBrakeStatus() {
    if (pImpl) {
        delete static_cast<FbtImplBrakeStatus*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtBrakeStatus::runCycle() {
    if (pImpl) {
        static_cast<FbtImplBrakeStatus*>(pImpl)->runCycle();
    }
}

FbtBrakeControl::FbtBrakeControl() {
    make_impl<FbtImplBrakeControl, FbtBrakeControlStruct>(*this);

    // Initialize default values
    this->eSetBrake = MC_BRAKE_AUTO;
    this->fTimeOut = 0.0;
}

FbtBrakeControl::~FbtBrakeControl() {
    if (pImpl) {
        delete static_cast<FbtImplBrakeControl*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtBrakeControl::runCycle() {
    if (pImpl) {
        static_cast<FbtImplBrakeControl*>(pImpl)->runCycle();
    }
}

}  // namespace zmc
