#include "fbt_axis_impl.h"
#include "fbt_fb_base.hpp"
#include <stdio.h>
#include <new>
#include "fbt_axis_opaque.h"
#include "fbt_axis_impl_cfg.hpp"
#include "common/misc/math_gcd.hpp"
#include "errorCode2str.hpp"
#include "AxVerboseFlag.h"
#include "axLogFactory.h"

namespace zmc{

void axisImpl_set_config(AxisImpl* pAx, const AxisCiaConfig& cfg){
    int axis_error_ = AxisCiaConfig_check(&cfg);
    if (axis_error_){
        pAx->dwErrorID = axis_error_;
        // _WARN_(");
        printf("set_config error=%d,%s\n", axis_error_, MC_ERROR_CODEToString((MC_ERROR_CODE) axis_error_) );
        // return;
    }
    static_cast<AxisInnerContent*>(pAx->native)->axisCfg = cfg;
    pAx->bSimulation = cfg.is_axis_virtual;
    if (cfg.debug.eAxisMtype){
        pAx->eAxisMtype = cfg.debug.eAxisMtype;
    }
    pAx->bDeaf = cfg.is_axis_virtual && (cfg.debug.eAxisMtype !=  AXIS_TYPE_CIA402_SIMULATED && cfg.debug.eAxisMtype !=  AXIS_TYPE_ENCODER);

    pAx->wAxisStructID = cfg.id;

    int direction = cfg.is_invert_direction ? -1 : 1;
    if (cfg.motor_type == MOTOR_TYPE_LINEAR){
        pAx->iRatioTechUnitsNum = cfg.linear.increments_units_in_application_num;
        pAx->dwRatioTechUnitsDenom = cfg.linear.increments_units_in_application_den;
    }else{
        pAx->iRatioTechUnitsNum = cfg.rotary.gear_application_num * cfg.rotary.increments_motor_turns_num * cfg.rotary.motor_gear_num;
        pAx->dwRatioTechUnitsDenom = cfg.rotary.gear_application_den * cfg.rotary.increments_motor_turns_den * cfg.rotary.motor_gear_den;
    }
    if (pAx->iRatioTechUnitsNum != 0 && pAx->dwRatioTechUnitsDenom != 0){
        int gcd_ = gcd(pAx->iRatioTechUnitsNum , pAx->dwRatioTechUnitsDenom);
        pAx->iRatioTechUnitsNum /= gcd_;
        pAx->dwRatioTechUnitsDenom /= gcd_;
    }
    pAx->iRatioTechUnitsNum *= direction;

    pAx->bMotorRotaryType = cfg.motor_type;
    pAx->iMovementType = cfg.axis_type;
    pAx->fPositionPeriod = cfg.modulo.modulo_value;

    pAx->cycleTime = cfg.cycle_time*1e-6;
    pAx->eRampType = (Velocity_Ramp_Type)cfg.velocity_ramp_option;

    pAx->bSWLimitEnable = !cfg.debug.limit_flag.bSWLimitDisable && cfg.axis_type == AXIS_TYPE_FINITE && cfg.finite.software_limits.is_activate;
    pAx->bHWLimitEnable = cfg.debug.limit_flag.bHWLimitEnable;
    pAx->bDynamicLimitEnable = cfg.debug.limit_flag.bDynamicLimitEnable;
    
    pAx->fSWLimitPositive = cfg.finite.software_limits.positive;
    pAx->fSWLimitNegative = cfg.finite.software_limits.negative;
    pAx->fSWMaxVelocity = cfg.dynamic_limits.velocity;
    pAx->fSWMaxAcceleration = cfg.dynamic_limits.acceleration;
    pAx->fSWMaxDeceleration = cfg.dynamic_limits.deceleration;
    pAx->fSWMaxJerk = cfg.dynamic_limits.jerk;

    pAx->eCheckPositionLag = cfg.position_lag_option;
    pAx->fMaxPositionLag = cfg.position_lag_supervision.value;
    pAx->bPositionLagActive = cfg.position_lag_supervision.is_changeable;
    pAx->fSWLimitDeceleration = cfg.software_error_reaction.deceleration;
    pAx->fSWErrorMaxDistance = cfg.software_error_reaction.max_distance;

    pAx->usiEncoderBitWidth = cfg.bit_width;
    
    char logFileName[100] = {0};
    int id = cfg.id?cfg.id:cfg.mapping_id;
    sprintf(logFileName, "axis_%d.log", id);
    pAx->installLogger((AxLoggerType) cfg.debug.log_type, logFileName);
    // printf("create axis log file %s\n", logFileName);
}

AxisOpaque::AxisOpaque() {
    static_assert(sizeof(AxisImpl) <= sizeof(storage_), "AxisImpl is too large");
    auto pAx = new (storage_) AxisImpl();

    AxisCiaConfig cfg;
    AxisCiaConfig_default_init(&cfg);
    axisImpl_set_config(pAx, cfg);
}

AxisOpaque::~AxisOpaque() {
    ((AxisImpl*)storage_)->~AxisImpl();
}


inline AxisImpl* to_axis_impl(AxisOpaque* pAxis){
    return (AxisImpl*) pAxis;
}
inline const AxisImpl* to_axis_impl(const AxisOpaque* pAxis){
    return (AxisImpl*) pAxis;
}

void AxisOpaque::set_config(const AxisCiaConfig* config) {
    axisImpl_set_config(to_axis_impl(this), *config);
}
void AxisOpaque::reload_config(){
    set_config(&get_config_ref());
}

AxisCiaConfig AxisOpaque::get_config() const {
    return static_cast<AxisInnerContent*>(to_axis_impl(this)->native)->axisCfg;
}
AxisCiaConfig& AxisOpaque::get_config_ref(){
    return static_cast<AxisInnerContent*>(to_axis_impl(this)->native)->axisCfg;
}
AxisCiaConfig* AxisOpaque::get_config_addr(){
    return &static_cast<AxisInnerContent*>(to_axis_impl(this)->native)->axisCfg;
}
const AxisCiaConfig* AxisOpaque::get_config_addr()const{
    return &static_cast<const AxisInnerContent*>(to_axis_impl(this)->native)->axisCfg;
}

AXIS_REF AxisOpaque::axis() {
    return (AXIS_REF)this;
}

void AxisOpaque::runCycle() {
    ((AxisImpl*)storage_)->runCycle();
}
void AxisOpaque::print(int verboseFlag)const{
    return to_axis_impl(this)->print(verboseFlag);
}

bool AxisOpaque::is_valid()const{
    return to_axis_impl(this)->is_axis_valid();
}

uint32_t AxisOpaque::get_count()const{
    return to_axis_impl(this)->m_cycleCounter;
}

void AxisOpaque::reset_runtime(){
    to_axis_impl(this)->reset_runtime();
}

void AxisOpaque::bind_extra_data(int32_t* pDwExtraData){
    to_axis_impl(this)->bind_extra_data(pDwExtraData);
}

void assign_axis_id(AxisBase* ax, int id){
    to_axis_impl(ax)->wAxisStructID = id;
}

void printAxisHeader(int verbose_flag) {
    return printVerboseHeader((AxVerboseFlag)verbose_flag);
}
// uint16_t get_fb_stat(FbBase* fb) { return (uint16_t)((implPtr*)fb)->ptr->_fbOStat.ostat; }

void axisRequestSwError(AXIS_REF axis, uint16_t error_id){
    to_axis_impl(axis)->requestSwError(error_id);
}
void axisSetCommunError(AXIS_REF axis, bool val){
    to_axis_impl(axis)->communicationError = val;
}
void axisSetNotOperationalError(AXIS_REF axis, bool val){
    to_axis_impl(axis)->notOperationalError = val;
}
bool axisGetCommunError(AXIS_REF axis){
    return to_axis_impl(axis)->communicationError;
}
void axisSetDriveInterfaceError(AXIS_REF axis, uint16_t error){
    to_axis_impl(axis)->uiDriveInterfaceError = error;
}
uint16_t axisGetDriveInterfaceError(AXIS_REF axis){
    return to_axis_impl(axis)->uiDriveInterfaceError;
}
void axisSetCycleTime(AXIS_REF axis, double val){
    to_axis_impl(axis)->cycleTime = val;
}
double axisGetCycleTime(AXIS_REF axis){
	return to_axis_impl(axis)->cycleTime;
}
}  // namespace zmc
