/*
 * @Author: ChenShengyong
 * @Date: 2024-06-03 17:16:56
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-05-21 09:58:49
 */
#ifndef _AXIS_MIX_CHECKER_H_
#define _AXIS_MIX_CHECKER_H_
#include "fbt_axis_base.h"
#include "math.h"

namespace zmc {

template<typename T>
int AxisDirectChecker(AxisImplBase& ax, const T& cfg){
    if(cfg.debug.limit_flag.bPosDirectionDisable && ax.fSetVelocity > 0) {
        return ZMC_ERR_INVALID_DIRECTION_POSITIVE;
    } else if(cfg.debug.limit_flag.bNegDirectionDisable && ax.fSetVelocity < 0) {
        return ZMC_ERR_INVALID_DIRECTION_NEGATIVE;
    }
    return 0;
}

#define MAX(a,b) ((a)>=(b)?(a):(b))
inline int axisCheckSwLimit(const AxisImplBase& ax) {
    if (1){
        double fSWLimitDeceleration = ax.fSWLimitDeceleration > 0 ? ax.fSWLimitDeceleration: ax.fSWMaxDeceleration;
        // double fSWLimitDeceleration = MAX(ax.fSWLimitDeceleration, ax.fSWMaxDeceleration);
        double stop_dist = ax.fSetVelocity * ax.fSetVelocity / 2/ fSWLimitDeceleration;
        if (ax.fSWErrorMaxDistance> 0 && ax.fSWErrorMaxDistance < stop_dist) {
            stop_dist = ax.fSWErrorMaxDistance;
        }
        double step_dist = ax.fSetVelocity * ax.cycleTime * 0; 
        // double step_dist = fSWLimitDeceleration * ax.cycleTime * ax.cycleTime/2;
        if (ax.fSetVelocity > 0 && ax.fSWLimitPositive < ax.fSetPosition + stop_dist + step_dist) {
            return ZMC_DI_SWLIMITS_EXCEEDED;
        }
        if (ax.fSetVelocity < 0 && ax.fSWLimitNegative > ax.fSetPosition - stop_dist + step_dist) {
            return ZMC_DI_SWLIMITS_EXCEEDED;
        }
    }
    return 0;
}
inline bool axisCheckActSwLimit(const AxisImplBase& ax) {
    if (ax.fSWLimitPositive + 1e-6 < ax.fActPosition) {
        return true;
    }else if (ax.fSWLimitNegative - 1e-6 > ax.fActPosition) {
        return true;
    }
    return false;
}

inline int axisCheckDynamicLimit(const AxisImplBase& ax){
    bool bSWMaxVelocity = true, bSWMaxAcceleration = true, bSWMaxDeceleration = true;
    

    if (bSWMaxVelocity && fabs(ax.fSetVelocity) > ax.fSWMaxVelocity) {
        return ZMC_VD_MAX_VELOCITY_EXCEEDED;
    }

    if (bSWMaxAcceleration &&
        ((ax.fSetVelocity > 0 && ax.fSetAcceleration > ax.fSWMaxAcceleration) ||
         (ax.fSetVelocity < 0 && ax.fSetAcceleration < -ax.fSWMaxAcceleration))) {
        return ZMC_VD_MAX_ACCELERATION_EXCEEDED;
    }
    if (bSWMaxDeceleration &&
        ((ax.fSetVelocity > 0 && ax.fSetAcceleration < -ax.fSWMaxDeceleration) ||
         (ax.fSetVelocity < 0 && ax.fSetAcceleration > ax.fSWMaxDeceleration))) {
        return ZMC_VD_MAX_DECELERATION_EXCEEDED;
    }
    return 0;
}
inline int axisCheckPosVelAccLimit(const AxisImplBase& ax) {
    // isnan
    #if ZMC_AXIS_CHECK_PVA_NAN
    if (isnan(ax.fSetPosition) || isnan(ax.fSetVelocity) || isnan(ax.fSetAcceleration)) {
        return ZMC_ERR_VEL_POS_JUMP; // ZMC_ERR_Invalid_SetValue;
    }
    #endif
    if (ax.fSWLimitDeceleration <= 0)
        return 0;
    if (ax.bSWLimitEnable){
        int ret = axisCheckSwLimit(ax);
        if (ret) return ret;
    }
    if (ax.bControlAxis || ax.bDynamicLimitEnable){
        return axisCheckDynamicLimit(ax);
    }
    return 0;
}

inline int axisCheckLag(const AxisImplBase& ax) {
    // check op mode first
    if (ax.bPositionLagActive && ax.eCheckPositionLag &&
        fabs(ax.fSetPosition - ax.fActPosition) > ax.fMaxPositionLag) {
        return ZMC_DI_POSITIONLAGERROR;
    }
    return 0;
}

// FIXME
// check incontinuity of position and velocity
inline int axisCheckDiscontinuity(const AxisImplBase& ax) {
    // check op mode first
    if ( ax.fSetPosition - ax.fSavePosition > ax.fSaveVelocity * ax.cycleTime) {
        return ZMC_ERR_VEL_POS_JUMP;
    }
    return 0;
}

} /*  namespace zmc */
#endif