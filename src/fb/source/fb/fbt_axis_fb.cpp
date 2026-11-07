
#include <stdio.h>
#include "fbt_fb_base.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_impl.h"
#include "common/misc/TrapzVelToyForwarder.hpp"
#include "fbt_move_impl.hpp"
// #include "VelGearFollower.h"
// #include "VelPosFollower.h"
#include "mc_enum_def_spec.h"
#include "fbt_axis_virtual.h"
#include "fbt_axis_opaque.h"


namespace zmc {


// Empty implementation classes for VirtualAxis and LogicalAxis
class FbtImplVirtualAxis:public FbtEnBase<FB_TYPE_VIRTUALAXIS>  {
public:
    FbtVirtualAxisStruct* pParam = nullptr;
    AxisVirtual* pAxis;
    FbtImplVirtualAxis(){
        pAxis = new AxisVirtual();
    }
    ~FbtImplVirtualAxis(){
        if (pAxis)
            delete pAxis;
        pAxis = nullptr;
    }
    void runCycle() {
        auto& param = *pParam;
        auto& cfg = pAxis->get_config_ref();
        // Configure virtual axis with input parameters

        // Set basic axis properties
        cfg.is_axis_virtual = true;
        cfg.axis_type = param.iMovementType; //  AXIS_TYPE_MODULO
        cfg.modulo.modulo_value = param.fPositionPeriod;
        cfg.id = param.wDriveID;
        // Set velocity ramp type
        cfg.velocity_ramp_option = param.eRampType;
        
        // Set dynamic limits
        cfg.dynamic_limits.velocity = param.fSWMaxVelocity;
        cfg.dynamic_limits.acceleration = param.fSWMaxAcceleration;
        cfg.dynamic_limits.deceleration = param.fSWMaxDeceleration;
        cfg.dynamic_limits.jerk = param.fSWMaxJerk;
        
        // Set software limits for finite axis
        // if (param.iMovementType == AXIS_TYPE_FINITE) {
        cfg.finite.software_limits.is_activate = param.bSWLimitEnable;
        cfg.finite.software_limits.negative = param.fSWLimitNegative;
        cfg.finite.software_limits.positive = param.fSWLimitPositive;
        cfg.software_error_reaction.deceleration = param.fSWLimitDeceleration;
        cfg.software_error_reaction.max_distance = param.fSWErrorMaxDistance;
        
        // Apply configuration to virtual axis
        pAxis->set_config(&cfg);
        pAxis->reload_config();
        
        // Set axis reference output
        param.Axis = pAxis;
        param.bError = false;
    }
};

class FbtImplLogicalAxis:public FbtEnBase<FB_TYPE_LOGICALAXIS>  {
public:
    FbtLogicalAxisStruct* pParam = nullptr;
    AxisVirtual* pAxis;
    bool _isInitialized = false;
    int _filterCycles = 0;

    void runCycle() {
        auto& param = *pParam;
        
        if (!param.Execute) {
            param.Initialized = false;
            param.InSteadyState = false;
            param.Error = false;
            _isInitialized = false;
            return;
        }

        if (!param.RealAxis) {
            param.Initialized = false;
            param.InSteadyState = false;
            param.Error = true;
            _isInitialized = false;
            return;
        }

        // Initialize logical axis on first execution
        if (!_isInitialized) {
            _isInitialized = true;
            _filterCycles = 0;
        }
        
        // Update filter cycles for steady state detection
        if (_isInitialized) {
            _filterCycles++;
            // Consider steady state after filter depth cycles
            bool isSteadyState = _filterCycles >= param.usiFilterDepthPosition;
            param.InSteadyState = isSteadyState;
        }
        
        // For now, use the real axis as the logical axis output
        // In a real implementation, this would create a filtered/logical axis
        param.Axis = pAxis;
        param.Initialized = _isInitialized;
        param.Error = false;
    }
};

FbtVirtualAxis::FbtVirtualAxis() {
    make_impl<FbtImplVirtualAxis, FbtVirtualAxisStruct>(*this);
    
    // Initialize default values
    this->wDriveID = 1;
    this->iMovementType = AXIS_TYPE_FINITE; // SMC_AXIS_TYPE_finite
    this->fPositionPeriod = 360.0;
    // this->eRampType = VEL_RAMP_TYPE_TRAPEZOID;
    this->fSWLimitPositive = 1000.0;
}

FbtVirtualAxis::~FbtVirtualAxis() {
    if (pImpl) {
        delete static_cast<FbtImplVirtualAxis*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtVirtualAxis::runCycle() {
    if (pImpl) {
        static_cast<FbtImplVirtualAxis*>(pImpl)->runCycle();
    }
}

FbtLogicalAxis::FbtLogicalAxis() {
    make_impl<FbtImplLogicalAxis, FbtLogicalAxisStruct>(*this);
    // Initialize default values
    this->wDriveID = 1;
    // this->iMovementType = AXIS_TYPE_MODULO; // SMC_AXIS_TYPE_modulo
    this->fPositionPeriod = 360.0;
    this->eValueReference = ZMC_VALUESELECT_ACTUAL;
    this->fTimeShiftCycles = 3.0;
}

FbtLogicalAxis::~FbtLogicalAxis() {
    if (pImpl) {
        delete static_cast<FbtImplLogicalAxis*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtLogicalAxis::runCycle() {
    if (pImpl) {
        static_cast<FbtImplLogicalAxis*>(pImpl)->runCycle();
    }
}


// Empty implementation classes for ChangeAxisScalingLinear and ChangeAxisScalingRotary
class FbtImplChangeAxisScalingLinear : public FbtExeTmpl<FbtImplChangeAxisScalingLinear, FbtImplParamTmpl<FbtChangeAxisScalingLinearStruct>> {
public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        
        auto& ax = get_axis_impl_ref(param.Axis);
        
        // Check if drive is powered - should not be enabled
        if (ax.axisStat != mcDisabled) {
            this->onError(ZMC_RD_AXIS_IN_WRONG_STATE);
            return;
        }

        // Validate parameters
        if (param.Increments == 0 || param.TechnicalUnits == 0) {
            this->onError(ZMC_WP_PARAM_INVALID);
            return;
        }

        // Get axis configuration and update scaling parameters
        auto& cfg = param.Axis->get_config_ref();
        
        // Update linear scaling parameters
        cfg.linear.increments_units_in_application_num = param.Increments;
        cfg.linear.increments_units_in_application_den = param.TechnicalUnits;
        
        // Set direction inversion
        cfg.is_invert_direction = param.InvertDirection;
        
        // Set axis type (finite/modulo)
        cfg.axis_type = param.AxisType;
        
        // Set modulo period if axis is modulo type
        if (param.AxisType == AXIS_TYPE_MODULO) {
            cfg.modulo.modulo_value = param.ModuloPeriod;
        }
        
        // Reload configuration to apply changes
        param.Axis->reload_config();
        
        this->fbStat = FB_STAT_DONE;
    }
    
    void onCapture() {
        this->execute = this->pParam->Execute;
    }
    
    void onUpdate() {
        this->pParam->Done = this->isDone();
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

class FbtImplChangeAxisScalingRotary : public FbtExeTmpl<FbtImplChangeAxisScalingRotary, FbtImplParamTmpl<FbtChangeAxisScalingRotaryStruct>> {
public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        
        auto& ax = get_axis_impl_ref(param.Axis);
        
        // Check if drive is powered - should not be enabled
        if (ax.axisStat != mcDisabled) {
            this->onError(ZMC_RD_AXIS_IN_WRONG_STATE);
            return;
        }

        // Validate parameters
        if (param.Increments == 0 || param.MotorTurns1 == 0 || 
            param.MotorTurns2 == 0 || param.GearOutput1 == 0 || 
            param.GearOutput2 == 0 || param.TechnicalUnits == 0) {
            this->onError(ZMC_WP_PARAM_INVALID);
            return;
        }

        // Create new configuration with updated rotary scaling parameters
        auto& cfg = param.Axis->get_config_ref();
        
        // Set basic axis properties
        cfg.is_axis_virtual = false; // Real axis
        cfg.axis_type = param.AxisType;
        cfg.modulo.modulo_value = param.ModuloPeriod;
        
        // Set motor type to rotary
        // cfg.motor_type = 1; // 1 = rotary, 0 = linear
        
        // Update rotary gearing parameters
        cfg.rotary.increments_motor_turns_num = param.Increments;
        cfg.rotary.increments_motor_turns_den = param.MotorTurns1;
        cfg.rotary.motor_gear_num = param.MotorTurns2;
        cfg.rotary.motor_gear_den = param.GearOutput1;
        cfg.rotary.gear_application_num = param.GearOutput2;
        cfg.rotary.gear_application_den = param.TechnicalUnits;
        
        // Set direction inversion
        cfg.is_invert_direction = param.InvertDirection;
        
        param.Axis->reload_config();
        
        this->fbStat = FB_STAT_DONE;
    }
    
    void onCapture() {
        this->execute = this->pParam->Execute;
    }
    
    void onUpdate() {
        this->pParam->Done = this->isDone();
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

FbtChangeAxisScalingLinear::FbtChangeAxisScalingLinear() {
    make_impl<FbtImplChangeAxisScalingLinear, FbtChangeAxisScalingLinearStruct>(*this);
    
    // Initialize default values
    this->Increments = 0x10000;  // 16#10000
    this->TechnicalUnits = 360;
    // this->ModuloPeriod = 0.0;
    // this->AxisType = AXIS_TYPE_MODULO;  // Default axis type
}

FbtChangeAxisScalingLinear::~FbtChangeAxisScalingLinear() {
    if (pImpl) {
        delete static_cast<FbtImplChangeAxisScalingLinear*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtChangeAxisScalingLinear::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplChangeAxisScalingLinear*>(pImpl);
        impl->runCycle();
    }
}

FbtChangeAxisScalingRotary::FbtChangeAxisScalingRotary() {
    make_impl<FbtImplChangeAxisScalingRotary, FbtChangeAxisScalingRotaryStruct>(*this);
    
    // Initialize default values
    this->Increments = 0x10000;  // 16#10000
    this->MotorTurns1 = 1;
    this->MotorTurns2 = 1;
    this->GearOutput1 = 1;
    this->GearOutput2 = 1;
    this->TechnicalUnits = 360;
    // this->ModuloPeriod = 0.0;
    // this->AxisType = AXIS_TYPE_MODULO;  // Default axis type
}

FbtChangeAxisScalingRotary::~FbtChangeAxisScalingRotary() {
    if (pImpl) {
        delete static_cast<FbtImplChangeAxisScalingRotary*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtChangeAxisScalingRotary::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplChangeAxisScalingRotary*>(pImpl);
        impl->runCycle();
    }
}

    
class FbtImplReinitDrive : public FbtExeTmpl<FbtImplReinitDrive, FbtImplParamTmpl<FbtReinitDriveStruct>> {
public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!param.Axis || !get_axis_impl_ref(param.Axis).is_axis_valid()) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        
        auto& ax = get_axis_impl_ref(param.Axis);
        
        // Check axis state - only allow in disabled state
        if (ax.axisStat != mcDisabled) {
            this->onError(ZMC_RD_AXIS_IN_WRONG_STATE);
            return;
        }

        // Check if virtual mode is already set as requested
        // if (ax.bSimulation == param.bVirtual) {
        //     this->onError(ZMC_RD_VIRTUAL_MODE_ALREADY_SET);
        //     return;
        // }

        // Set virtual mode
        ax.bSimulation = param.bVirtual;
        
        this->fbStat = FB_STAT_DONE;
    }
    
    void onCapture() {
        this->execute = this->pParam->bExecute;
    }
    
    void onUpdate() {
        this->pParam->bDone = this->isDone();
        this->pParam->bBusy = this->isBusy();
        this->pParam->bError = this->isError();
        this->pParam->nErrorID = this->getErrorID();
    }
};


FbtReinitDrive::FbtReinitDrive() {
    make_impl<FbtImplReinitDrive, FbtReinitDriveStruct>(*this);
}

FbtReinitDrive::~FbtReinitDrive() {
    if (pImpl) {
        delete static_cast<FbtImplReinitDrive*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtReinitDrive::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplReinitDrive*>(pImpl);
        impl->runCycle();
    }
}
}  // namespace zmc