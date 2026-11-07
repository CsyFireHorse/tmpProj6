#ifndef _MC_CNC_ENUM_DEF_H_
#define _MC_CNC_ENUM_DEF_H_

#include "stdint.h"

// SMC_INT_STATUS enumeration
typedef enum SMC_INT_STATUS: uint16_t {
    IPO_UNKNOWN = 0,           // Internal state that may not occur after a complete pass of the SMC_Interpolator
    IPO_INIT = 1,              // Initialization state; movement not started yet
    IPO_ACCEL = 2,             // Interpolation ongoing; currently accelerating
    IPO_CONSTANT = 3,          // Interpolation ongoing with constant velocity
    IPO_DECEL = 4,             // Interpolation ongoing; currently decelerating
    IPO_FINISHED = 5,          // Interpolation done
    IPO_WAIT = 6,              // Interpolation ongoing; currently waiting for example by a stop input or a M-command
    IPO_INCREASING_ACCEL = 7,  // Interpolation ongoing; currently building up the acceleration
    IPO_DECREASING_ACCEL = 8,  // Interpolation ongoing; currently ramping down the acceleration
    IPO_INCREASING_DECEL = 9,  // Interpolation ongoing; currently building up the deceleration
    IPO_DECREASING_DECEL = 10  // Interpolation ongoing; currently ramping down the deceleration
} SMC_INT_STATUS;


// SMC_DEC_STATUS enumeration - Decoder status
typedef enum _SMC_DEC_STATUS_t {
    WAIT_PROG = 0,     // Program not found yet
    READ_WORD = 1,     // Word read
    PROG_READ = 2      // End of program reached
} SMC_DEC_STATUS;

// SMC_ORI_CONVENTION enumeration - Orientation convention
typedef enum _SMC_ORI_CONVENTION_t {
    ADDAXES = 0,       // A/B/C are treated as additional axes and not as an orientation
    ZYZ = 1,           // Rotation around the axes z, y', z''
    ZYX = 2,           // Rotation around the axes z, y', x''; also known as Yaw-Pitch-Roll
    XYZ = 3            // Rotation around the axes x, y', z''
} SMC_ORI_CONVENTION;


// SMC_INT_VELMODE enum - Represents the velocity profile
typedef enum _SMC_INT_VELMODE_t: uint16_t {
    SMC_VELMODE_TRAPEZOID = 0,        // Velocity profile with trapezoid shape
    SMC_VELMODE_SIGMOID = 1,          // Equal to the TRAPEZOID profile but rising and falling edges of the velocity profile are replaced by sin2 functions with same area
    SMC_VELMODE_SIGMOID_LIMIT = 2,    // Velocity profile: Equal to mode SIGMOID with the difference that the same time is taken for interpolating one path in trapezoid and sigmoid mode
    SMC_VELMODE_QUADRATIC = 3,        // Acceleration profile in a trapezoidal form with jerk limitation
    SMC_VELMODE_QUADRATIC_SMOOTH = 4, // It works like mode QUADRATIC but creates a jerk profile without jumps
} SMC_INT_VELMODE;

// SMC_ADDAXIS enum - Additional axis types
typedef enum _SMC_ADDAXIS_t: uint16_t {
    ADDAXIS_NONE = 0,
    ADDAXIS_X = 1,
    ADDAXIS_Y = 2,
    ADDAXIS_Z = 4,
    ADDAXIS_A = 8,
    ADDAXIS_B = 16,
    ADDAXIS_C = 32,
    ADDAXIS_P = 64,
    ADDAXIS_Q = 128,
    ADDAXIS_U = 256,
    ADDAXIS_V = 512,
    ADDAXIS_W = 1024,
    ADDAXIS_A6 = 2048
} SMC_ADDAXIS;

// SMC_MOVTYP enum - Movement types
// Indicates the geometrical type of the object
typedef enum _SMC_MOVTYP_t: uint16_t {
    SMC_MOVTYP_LIN = 1,           // Straight movement (G01)
    SMC_MOVTYP_CLW = 2,           // Circle in clockwise direction (G02)
    SMC_MOVTYP_CCLW = 3,          // Circle in counterclockwise direction (G03)
    SMC_MOVTYP_SPLINE = 5,        // Spline, parabola (G05, G06)
    SMC_MOVTYP_ELLCLW = 8,        // Clockwise ellipse (G08)
    SMC_MOVTYP_ELLCCLW = 9,       // Counterclockwise ellipse (G09)
    SMC_MOVTYP_SPLINE3D = 10,     // 3D Spline
    SMC_MOVTYP_SPLINE3D_5 = 11,   // 3D Spline 5
    SMC_MOVTYP_LINPOS = 100,      // Straight positioning (G00)
    SMC_MOVTYP_LINNODELTA = 101,  // Straight movement without movement in X/Y but Z
    SMC_MOVTYP_LINADDAXES = 102,  // Straight movement without movement in X/Y/Z
    SMC_MOVTYP_INITPOS = 110,     // Blind positioning (start point still unknown, will be added by SMC_Interpolator)
    SMC_MOVTYP_MCOMMAND = 120     // Additional option, M-option
} SMC_MOVTYP;

// SMC_VARTYPE enum - Variable types for G-code words
typedef enum _SMC_VARTYPE_t: uint16_t {
    SMC_VARTYPE_NONE = 0,         // No variable type
    SMC_VARTYPE_BOOL = 1,         // Boolean type
    SMC_VARTYPE_BYTE = 2,         // Byte type
    SMC_VARTYPE_WORD = 3,         // Word type
    SMC_VARTYPE_DWORD = 4,        // Double word type
    SMC_VARTYPE_SINT = 5,         // Short integer type
    SMC_VARTYPE_INT = 6,          // Integer type
    SMC_VARTYPE_DINT = 7,         // Double integer type
    SMC_VARTYPE_LINT = 8,         // Long integer type
    SMC_VARTYPE_USINT = 9,        // Unsigned short integer type
    SMC_VARTYPE_UINT = 10,        // Unsigned integer type
    SMC_VARTYPE_UDINT = 11,       // Unsigned double integer type
    SMC_VARTYPE_ULINT = 12,       // Unsigned long integer type
    SMC_VARTYPE_REAL = 13,        // Real type
    SMC_VARTYPE_LREAL = 14,       // Long real type
    SMC_VARTYPE_TIME = 15,        // Time type
    SMC_VARTYPE_DATE = 16,        // Date type
    SMC_VARTYPE_TIME_OF_DAY = 17, // Time of day type
    SMC_VARTYPE_DATE_AND_TIME = 18, // Date and time type
    SMC_VARTYPE_STRING = 19,      // String type
    SMC_VARTYPE_WSTRING = 20,     // Wide string type
    SMC_VARTYPE_ARRAY = 21,       // Array type
    SMC_VARTYPE_STRUCT = 22,      // Structure type
    SMC_VARTYPE_POINTER = 23,     // Pointer type
    SMC_VARTYPE_REFERENCE = 24,   // Reference type
    SMC_VARTYPE_FUNCTION = 25,    // Function type
    SMC_VARTYPE_FUNCTION_BLOCK = 26, // Function block type
    SMC_VARTYPE_PROGRAM = 27,     // Program type
    SMC_VARTYPE_RESOURCE = 28,    // Resource type
    SMC_VARTYPE_TASK = 29,        // Task type
    SMC_VARTYPE_CONFIGURATION = 30, // Configuration type
    SMC_VARTYPE_ACCESS_PATH = 31  // Access path type
} SMC_VARTYPE;


#endif