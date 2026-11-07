#ifndef ZMC_MC_ROBOT_DEF_H
#define ZMC_MC_ROBOT_DEF_H

#include "stdint.h"

namespace zmc {

static constexpr int kZmotMaxGroupAxes = 8;

// Stores the position of each axis used in an axis group.
// Depending on the used kinematics, only a prefix of the values is used,
// e.g. a0, a1, and a2 for a kinematics with three degrees of freedom.
struct AXISPOS_REF {
    double a0; // Position of axis 0
    double a1; // Position of axis 1
    double a2; // Position of axis 2
    double a3; // Position of axis 3
    double a4; // Position of axis 4
    double a5; // Position of axis 5
};

// Cartesian coordinate and orientation structure
struct MC_COORD_REF {
    double X; // X-coordinate of position
    double Y; // Y-coordinate of position
    double Z; // Z-coordinate of position
    double A; // Angle of rotation around z-axis [deg]
    double B; // Angle of rotation around y'-axis [deg]
    double C; // Angle of rotation around z''-axis [deg]
};

typedef union ZMC_POS_REF {
    MC_COORD_REF c;                 // Cartesian position
    AXISPOS_REF a;                  // Axis coordinates
    double v[kZmotMaxGroupAxes];   // Interpretation depends on coordinate system
} ZMC_POS_REF;

// Unit conversion factors for an axis group (user units -> SI units)
struct ZMC_GroupUnits {
    double factorPos;
    double factorVel;
    double factorAcc;
};

// Coordinate system used by MC_GroupSetPosition
typedef enum ZMC_COORD_SYSTEM : uint16_t {
    ZMC_CS_ACS = 0,
    ZMC_CS_MCS = 1,
    ZMC_CS_PCS = 2,
} ZMC_COORD_SYSTEM;

}  // namespace zmc

#endif
