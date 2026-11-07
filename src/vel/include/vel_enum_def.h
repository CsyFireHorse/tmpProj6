#ifndef _VEL_ENUM_DEF_H_
#define _VEL_ENUM_DEF_H_
#include "stdint.h"

enum VelInnerParam{
    VEL_INNER_NUM_SEG,
    VEL_INNER_TOTAL_DISTANCE,
    VEL_INNER_BEGIN_POS,
    VEL_INNER_BEGIN_VEL,
    VEL_INNER_BEGIN_ACC,
    VEL_INNER_END_POS,
    VEL_INNER_END_VEL,
    VEL_INNER_END_ACC,
    VEL_INNER_MAX_VEL,
    VEL_INNER_MAX_ACC,
    VEL_INNER_MAX_DEC,
    VEL_INNER_MAX_JERK,
    VEL_INNER_MAX_JERK2,
    VEL_INNER_TOTAL_TIME = 16,
    VEL_INNER_REAL_END_TIME,
    VEL_INNER_REAL_MAX_VEL,
    VEL_INNER_REAL_MAX_ACC,
    VEL_INNER_REAL_MAX_DEC,
};

enum EnumVelPolicy{
    // E_VEL_POLICY_NOP = 0,

    E_VEL_POLICY_DISCETE = 0, // use for discrete
    E_VEL_POLICY_CONSTINUOUS = 1, // use for move continuous
    E_VEL_POLICY_NO_DEST = 2,   // use for stopper
    E_VEL_POLICY_ACCLER = 3,   // use for movev: NO_DEST + CONSTINUOUS
    E_VEL_POLICY_SLAVE = 4,   // slave
    E_VEL_POLICY_SCURVE = 8, // use for scurve or trapz

    E_VEL_POLICY_SINSQUARE = 1<< 4, // use for sinsquare
    //  = 2,   // , acceler
    E_VEL_POLICY_ONLINE = 1<< 14, // 

    // allow inverse move 
    E_VEL_POLICY_DISABLE_INVERT = 64,    // use for nseg23
    // change vd
    E_VEL_POLICY_DISABLE_VD_RELAX = 128, // use for nseg11
    E_VEL_POLICY_DISABLE_VD_INC = 1<<8,    // use for nseg12

    // change vd to zero & allow inverse move 
    E_VEL_POLICY_DISABLE_VD_ZERO_INVERT = 1<<9,
    E_VEL_POLICY_DISABLE_VD_MAX_RELAX = 1<<10,
};

typedef enum _EnumVelCovered_t{
    E_VEL_COVERED,
    E_VEL_SEG_COVERED,
    E_VEL_UNCOVERED,
    E_VEL_INXXX,
    E_VEL_NOT_READY = 8,
}EnumVelCovered;


enum EnumGearStatus : uint8_t {
    GEAR_STATUS_IDLE = 0,
    GEAR_STATUS_CLUTCH = 1,
    GEAR_STATUS_GEAR_IN = 2,
    GEAR_STATUS_GEAR_OUT = 3,
};

enum EnumVelMode : uint8_t {
    VEL_MODE_TRAPEZOID = 0,
    VEL_MODE_SINSQUARE = 1,
    // SIGMOID_LIMIT
    VEL_MODE_QUADRATIC = 2,
    VEL_MODE_QUADRATIC_SMOOTH = 3,
    // VEL_MODE_NUM,
};

#endif