/*
 * @Author: ChenShengyong
 * @Date: 2024-06-21 10:50:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-11-28 10:16:47
 */
#ifndef _MC_MOTION_DEF_H_
#define _MC_MOTION_DEF_H_

#include "mc_enum_def.h"

using namespace zmc;

/* Motion direction. use in MC_MoveAbsolute*/
// typedef enum _ZMC_DIRECTION_T {
//     ZMC_DIR_NEGATIVE,
//     ZMC_DIR_SHORTEST_WAY,  // only modulo axes
//     ZMC_DIR_POSITIVE,
//     ZMC_DIR_CURRENT,      // only modulo axes
//     ZMC_DIR_FASTEST_WAY,  // only modulo axes (codesys only)
// } ZMC_DIRECTION;

typedef enum _MC_MOTION_MODE_t {
    mcNoMoveType = 0,
    mcMovePower,
    mcMoveHome,
    mcMoveHoming,
    mcStop,
    mcHalt,
    mcMoveAbsolute,
    mcMoveRelative,
    mcMoveAdditive,
    mcMoveVelocity,
    mcMoveContinuousAbsolute,
    mcMoveContinuousRelative,
    mcMoveJog,
    mcMoveInch,
    mcMovePositionProfile,
    mcMoveVelocityProfile,
    mcMoveAccelerationProfile,
    mcMoveSuperimposed,
    mcHaltSuperimposed,

    mcMovePhasing,
    mcMovePhasingAbsolute,
    mcMovePhasingRelative,
    mcMoveTrackAxis,
    mcMoveTrackSetValues,
    mcMoveCombineAxes,
    mcMoveGearIn,
    mcMoveGearInPos,
    mcMoveGearOut,
    mcMoveCamIn,
    mcMoveCamOut,
    mcMoveFollowPosition,
    mcMoveFollowVelocity,
    mcMoveFollowPositionVelocity,
    mcMoveFollowSetValues

} MC_MOTION_MODE;

typedef enum _ZMC_MAIN_TYPE_T {
    ZMC_MTYPE_NOP = 0,
    ZMC_MTYPE_UNMOVE,

    ZMC_MTYPE_HOMING,
    ZMC_MTYPE_STOP,
    ZMC_MTYPE_HALT,

    ZMC_MTYPE_DISCRETE,
    ZMC_MTYPE_DISCRETE_PROFILE,
    ZMC_MTYPE_CONTINUOUS,
    ZMC_MTYPE_CONTINUOUS_VEL,
    ZMC_MTYPE_CONTINUOUS_PROFILE,
    ZMC_MTYPE_JOG,
    ZMC_MTYPE_SYNC,
    ZMC_MTYPE_SYNC_IN,
    ZMC_MTYPE_SYNC_CAM,
    ZMC_MTYPE_SYNC_OUT,
    ZMC_MTYPE_SUPERIMPOSED,
    ZMC_MTYPE_PHASING,
    ZMC_MTYPE_SPECIAL,
} ZMC_MAIN_TYPE;

inline ZMC_MAIN_TYPE motionMde2Mtype(MC_MOTION_MODE mod) {
    switch (mod) {
        case mcNoMoveType:
            return ZMC_MTYPE_NOP;
        case mcMovePower:
            return ZMC_MTYPE_UNMOVE;
        case mcMoveHome:
            return ZMC_MTYPE_HOMING;
        case mcMoveHoming:
            return ZMC_MTYPE_HOMING;
        case mcStop:
            return ZMC_MTYPE_STOP;
        case mcHalt:
            return ZMC_MTYPE_HALT;
        case mcMoveAbsolute:
        case mcMoveRelative:
        case mcMoveAdditive:
            return ZMC_MTYPE_DISCRETE;
        case mcMoveContinuousAbsolute:
        case mcMoveContinuousRelative:
            return ZMC_MTYPE_CONTINUOUS;
        case mcMoveVelocity:
            return ZMC_MTYPE_CONTINUOUS_VEL;
        case mcMoveJog:
        case mcMoveInch:
            return ZMC_MTYPE_JOG;
        case mcMoveVelocityProfile:
        case mcMoveAccelerationProfile:
            return ZMC_MTYPE_CONTINUOUS_PROFILE;
        case mcMovePositionProfile:
            return ZMC_MTYPE_DISCRETE_PROFILE;
        case mcMoveSuperimposed:
        case mcHaltSuperimposed:
            return ZMC_MTYPE_SUPERIMPOSED;
        case mcMovePhasing:
        case mcMovePhasingAbsolute:
        case mcMovePhasingRelative:
            return ZMC_MTYPE_PHASING;
        case mcMoveTrackAxis:
        case mcMoveTrackSetValues:
        case mcMoveCombineAxes:
            return ZMC_MTYPE_SYNC;
        case mcMoveGearIn:
        case mcMoveGearInPos:
            return ZMC_MTYPE_SYNC_IN;
        case mcMoveCamIn:
            return ZMC_MTYPE_SYNC_CAM;
        case mcMoveGearOut:
        case mcMoveCamOut:
            return ZMC_MTYPE_SYNC_OUT;
        default:
            return ZMC_MTYPE_SPECIAL;
    }
}

inline bool isMotionModeDeactivateByDone(MC_MOTION_MODE mod) {
    switch (mod) {
        case mcMoveHome:
        case mcMoveHoming:
        case mcStop:
        case mcHalt:
        case mcMoveAbsolute:
        case mcMoveRelative:
        case mcMoveAdditive:
        case mcMoveVelocityProfile:
        case mcMoveAccelerationProfile:
        case mcMovePositionProfile:
        case mcMoveGearOut:
        case mcMoveCamOut:
            return true;
        default:
            return false;
    }
}

inline bool isMotionModeDeactivateByInxxx(MC_MOTION_MODE mod) {
    switch (mod) {
        case mcMovePower:
        case mcMoveContinuousAbsolute:
        case mcMoveContinuousRelative:
        case mcMoveVelocity:
        case mcMoveCombineAxes:
        case mcMoveGearIn:
        case mcMoveGearInPos:
        case mcMoveCamIn:
        case mcMoveTrackAxis:
        case mcMoveTrackSetValues:
            return true;
        default:
            return false;
            // case mcMoveJog:
            // case mcMoveInch:
            // case mcMoveSuperimposed:
            // case mcHaltSuperimposed:
            // case mcMovePhasing:
            // case mcMovePhasingAbsolute:
            // case mcMovePhasingRelative:
    }
}


#endif