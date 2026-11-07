#ifndef _MC_GEOINFO_DEF_H_
#define _MC_GEOINFO_DEF_H_

#include "zmc_types_def.h"
#include "mc_cnc_posinfo_def.h"

// Constants for CNC definitions
#define MAX_IPOSWITCHES 8
#define SMC_MAX_ADDITIONAL_PARAMS 10

// SMC_GEOINFO structure - Path object information
typedef struct SMC_GEOINFO {
    ZMC_DINT iObjNo;               // Arbitrary object number (irrelevant for current path description)
    ZMC_DINT iSourceLine_No;       // Stores the 0-based line number inside the CNC source file
    ZMC_DINT diSentenceNo;         // Stores the G-code sentence number (value of the N-word)
    ZMC_UINT iMoveType;          // Value defining the geometrical type of the object
    SMC_POSINFO piStartPos;        // The start position of the object
    SMC_POSINFO piDestPos;         // The end position of the object
    ZMC_LREAL dP1;                 // Parameter 1 (varies by move type)
    ZMC_LREAL dP2;                 // Parameter 2 (varies by move type)
    ZMC_LREAL dP3;                 // Parameter 3 (varies by move type)
    ZMC_LREAL dP4;                 // Parameter 4 (varies by move type)
    ZMC_LREAL dP5;                 // Parameter 5 (varies by move type)
    ZMC_LREAL dP6;                 // Parameter 6 (varies by move type)
    ZMC_LREAL dP7;                 // Parameter 7 (varies by move type)
    ZMC_LREAL dP8;                 // Parameter 8 (varies by move type)
    ZMC_LREAL dP9;                 // Parameter 9 (varies by move type)
    ZMC_LREAL dP10;                // Parameter 10 (varies by move type)
    ZMC_LREAL dP11;                // Parameter 11 (varies by move type)
    ZMC_LREAL dP12;                // Parameter 12 (varies by move type)
    ZMC_LREAL dP13;                // Parameter 13 (varies by move type)
    ZMC_LREAL dP14;                // Parameter 14 (varies by move type)
    ZMC_LREAL dP15;                // Parameter 15 (varies by move type)
    ZMC_LREAL dP16;                // Parameter 16 (varies by move type)
    ZMC_LREAL dP17;                // Parameter 17 (varies by move type)
    ZMC_LREAL dP18;                // Parameter 18 (varies by move type)
    SMC_Vector3D vX;               // X-axis of the local coordinate system, expressed in world coordinates
    SMC_Vector3D vY;               // Y-axis of the local coordinate system, expressed in world coordinates
    SMC_Vector3D vN;               // Z-axis of the local coordinate system, expressed in world coordinates
    ZMC_LREAL dT1;                 // Start parameter for circle/spline
    ZMC_LREAL dT2;                 // Apex angle/end parameter for circle/spline
    ZMC_LREAL dToolRadius;         // The tool radius. Necessary for path processing with SMC_ToolRadiusCorr
    ZMC_LREAL dSmoothRadius;       // The smooth radius. Necessary for path processing with SMC_RoundPath and SMC_SmoothPath
    ZMC_LREAL dVel;                // Target velocity to be reached (in path units/sec)
    ZMC_LREAL dVelEnd;             // Prescribed velocity at end of object (path units/sec)
    ZMC_LREAL dVelEndStored;       // Maximum speed at end of segment before call to SMC_CheckVelocities
    ZMC_LREAL dVelEndSafe;         // Computed for the quadratic velocity profile: maximum <= dVelEnd that allows to reach dVelEndSafe of the next element
    ZMC_LREAL dAccel;              // Maximum allowable acceleration (in path units/sec²)
    ZMC_LREAL dDecel;              // Maximum allowable deceleration (in path units/sec²)
    ZMC_LREAL dLength;             // Length of object in path units
    ZMC_WORD wInternMark2;         // Intern marks for tool correction, rounding, etc.
    ZMC_BYTE byInternMark;         // Start and end of path preprocessing actions
    ZMC_DWORD dwFeatureFlags;      // If TRUE, the object is a 3D-object
    ZMC_BOOL b3DMode;              // Stores feature bits set by G38/G39
    ZMC_LREAL dHelpPos[MAX_IPOSWITCHES];  // Relative position of auxiliary switches
    ZMC_INT iHelpID[MAX_IPOSWITCHES];     // ID (G-code: H) of auxiliary switches
    ZMC_LREAL adVelAddAx[8];       // Velocity values for additional axes
    ZMC_LREAL adAccAddAx[8];       // Acceleration values for additional axes
    ZMC_LREAL adDecAddAx[8];       // Deceleration values for additional axes
    ZMC_LREAL aAdditionalParams[SMC_MAX_ADDITIONAL_PARAMS];  // Additional parameters set by G38 I/J/K
    ZMC_LREAL adToolLength[3];     // Parameters for tool length compensation (set by G43 I/J/K)
    ZMC_LREAL dPrecision;          // The precision of the G-Code
} SMC_GEOINFO;


#endif