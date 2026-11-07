/**
 * History:
 * ================================================================
 * 2025-07-30 ChenSy <ChenSy@zlg.cn> created.
 *
 */
	
#ifndef _FB_CNC_INT_DEF_H_
#define _FB_CNC_INT_DEF_H_

#include "fbt_common.h"
#include "mc_ref_def.h"
#include "mc_cnc_enum_def.h"
// #include "mc_cnc_geoinfo_def.h"
#include "mc_cnc_posinfo_def.h"


namespace zmc{
    
#pragma pack(push, AW_PLC_NATIVE_ALIGN)

// SMC_Interpolator function block structure
// This function block is used to convert a continuous path described by SMC_GEOINFO objects 
// into discrete path position points taking into account a defined velocity profile and time pattern
typedef struct FbtInterpolatorStruct{
    void* pImpl;                   // Implementation pointer

    // Input parameters
    ZMC_BOOL bExecute;             // Execution starts on the rising edge
    SMC_OUTQUEUE* poqDataIn;       // Pointer to SMC_OUTQUEUE structure containing SMC_GEOINFO objects
    ZMC_BOOL bSlow_Stop;           // If FALSE, path will be passed non-stop; if TRUE, reduce velocity to 0
    ZMC_BOOL bEmergency_Stop;      // Immediate stop when TRUE, position will be retained
    ZMC_BOOL bWaitAtNextStop;      // If TRUE, pause at next regular stop position
    ZMC_LREAL dOverride;           // Override factor (default: 1.0), scales scheduled velocity
    SMC_INT_VELMODE iVelMode;      // Velocity profile mode (default: TRAPEZOID)
    ZMC_DWORD dwIpoTime;           // Cycle time in micro sec
    ZMC_LREAL dLastWayPos;         // Last way position for path measurement
    ZMC_BOOL bAbort;               // Abort function block when TRUE
    ZMC_BOOL bSingleStep;          // Stop at transition between path objects when TRUE
    ZMC_BOOL bAcknM;               // Acknowledge M-function when TRUE
    ZMC_BOOL bQuick_Stop;          // Quick stop when TRUE, reduce velocity to zero
    ZMC_LREAL dQuickDeceleration;  // Deceleration value for quick stop
    ZMC_LREAL dJerkMax;            // Maximum allowed jerk magnitude for quadratic velocity modes
    ZMC_LREAL dQuickStopJerk;      // Jerk magnitude for quick stop in quadratic modes
    ZMC_BOOL bSuppressSystemMFunctions; // Suppress system M-functions when TRUE

    // Output parameters
    ZMC_BOOL bDone;                // TRUE when input data has been processed completely
    ZMC_BOOL bBusy;                // TRUE while execution is not finished
    ZMC_BOOL bError;               // TRUE when error has occurred
    ZMC_ERROR wErrorID;            // Error identification
    SMC_POSINFO piSetPosition;     // Calculated set position with cartesian coordinates
    SMC_INT_STATUS iStatus;        // Current status of the function block
    ZMC_BOOL bWorking;             // TRUE if processing has started and not finished
    ZMC_DINT iActObjectSourceNo;   // Source line number of active SMC_GEOINFO object
    ZMC_LREAL dActObjectLength;    // Length of current object (valid if bWorking = TRUE)
    ZMC_LREAL dActObjectLengthRemaining; // Remaining length of current object (valid if bWorking = TRUE)
    ZMC_LREAL dVel;                // Current path velocity
    SMC_Vector3D vecActTangent;    // Path tangent unit vector
    ZMC_INT iLastSwitch;           // Number of last switch passed
    ZMC_DWORD dwSwitches;          // Current switch status of all switches 1-32
    ZMC_LREAL dWayPos;             // Current way position (sum of dLastWayPos and distance covered)
    ZMC_WORD wM;                   // M-function value when passing M-function
    ZMC_LREAL adToolLength[3];     // Parameters for tool length compensation
    SMC_GEOINFO* Act_Object;       // Pointer to currently interpolated path element
    SMC_GCodeObjectId actObjectId; // Identifier of currently interpolated path object
}FbtInterpolatorStruct;

// SMC_TRAFO_Gantry2 function block structure
// Reverse transformation for two-dimensional portal systems
typedef struct FbtTrafoGantry2Struct{
    void* pImpl;                   // Implementation pointer

    // Input parameters
    SMC_POSINFO pi;                // Target vector position (x,y), output of interpolator
    ZMC_LREAL dOffsetX;            // Additional offset for x-axis
    ZMC_LREAL dOffsetY;            // Additional offset for y-axis

    // Output parameters
    ZMC_LREAL dx;                  // Resulting position for x-axis
    ZMC_LREAL dy;                  // Resulting position for y-axis
}FbtTrafoGantry2Struct;

// SMC_TRAFOF_Gantry2 function block structure
// Forward transformation for two-dimensional gantries (X/Y)
typedef struct FbtTrafoFGantry2Struct{
    void* pImpl;                   // Implementation pointer

    // InOut parameters
    AXIS_REF DriveX;               // Reference to x-axis
    AXIS_REF DriveY;               // Reference to y-axis

    // Input parameters
    ZMC_LREAL dOffsetX;            // Offset x-position. Equivalent to SMC_TRAFO_Gantry2
    ZMC_LREAL dOffsetY;            // Offset y-position. Equivalent to SMC_TRAFO_Gantry2
    ZMC_LREAL minX;                // Lower bound of move range in x-direction (for visualization purpose)
    ZMC_LREAL maxX;                // Upper bound of move range in x-direction (for visualization purpose)
    ZMC_LREAL minY;                // Lower bound of move range in y-direction (for visualization purpose)
    ZMC_LREAL maxY;                // Upper bound of move range in y-direction (for visualization purpose)

    // Output parameters
    ZMC_LREAL dx;                  // X-position
    ZMC_LREAL dy;                  // Y-position
    ZMC_LREAL dnx;                 // Normed x-position (with value in [0,1])
    ZMC_LREAL dny;                 // Normed y-position (with value in [0,1])
    ZMC_LREAL ratio;               // Ratio x-interval / y-interval
    ZMC_LREAL dnOffsetX;           // X-offset for visualization
    ZMC_LREAL dnOffsetY;           // Y-offset for visualization
}FbtTrafoFGantry2Struct;

// SMC_CNC_REF function block structure
// This function block manages the parsed G-code files
typedef struct FbtCncRefStruct{
    void* pImpl;                   // Implementation pointer

    // Input parameters
    ZMC_WORD wCNCREFStructID;      // The fix value allows the module to check internally if the consigned structure variable is of type SMC_CNC_REF
    ZMC_BOOL bRestart;             // In case of jumps implemented within the IEC program (G20), bRestart will be set by SMC_NCDecoder
    ZMC_DINT diJmpTarget;          // The jump target (sentence number); only valid, if bRestart = TRUE
    ZMC_UDINT nElements;           // Number of SMC_GCODE_WORD structures at pgc
    ZMC_UDINT diReadPos;           // For internal use
    ZMC_UDINT udiBuffer;           // For internal use
    SMC_GCODE_WORD* pgc;          // Pointer on first structure of type SMC_GCODE_WORD
    SMC_POSINFO piStartPosition;   // Start position of CNC program
    ZMC_STRING strProgramName;     // Name of CNC program
    ZMC_BOOL xPartofLM;           // If the input is TRUE, the function block is maintained by managers
    ZMC_BOOL bEndOfData;          // If TRUE, no further data is supposed to be appended
}FbtCncRefStruct;

// SMC_CheckVelocities function block structure
// This function block checks velocities for path segments
typedef struct FbtCheckVelocitiesStruct{
    void* pImpl;                   // Implementation pointer

    // Input parameters
    ZMC_BOOL bExecute;             // Execution starts on the rising edge
    ZMC_BOOL bAbort;               // If TRUE, the current processing of this function block is aborted
    SMC_OUTQUEUE* poqDataIn;       // This input points to the SMC_OUTQUEUE structure object, which describes the SMC_GEOINFO objects of the path
    ZMC_LREAL dAngleTol;           // This input describes the tolerance angle (angle leeway) up to which at a sharp bend of the path no stop shall be executed
    ZMC_BOOL bCheckAddAxVelJump;   // Check additional axis velocity jump
    ZMC_LREAL dMaxAddAxVelDifference; // Maximum additional axis velocity difference

    // Output parameters
    ZMC_BOOL bBusy;                // TRUE, while execution of function block is not finished and the process on the path object is still running
    ZMC_BOOL bError;               // TRUE when error has occurred
    ZMC_ERROR wErrorID;            // Error identification
    SMC_OUTQUEUE* poqDataOut;      // This output points on the SMC_OUTQUEUE structure object, which contains the path with the permissible velocity values
}FbtCheckVelocitiesStruct;

// SMC_NCDecoder function block structure
// This function block decodes G-code programs
typedef struct FbtNCDecoderStruct{
    void* pImpl;                   // Implementation pointer

    // InOut parameters
    FbtCncRefStruct* ncprog;         // This variable holds the CNC program. The program may have been created by the IEC-program or within the CNC-Editor.

    // Input parameters
    ZMC_BOOL bExecute;             // Execution starts on the rising edge
    ZMC_BOOL bAbort;               // If TRUE, the current processing of this function block is aborted
    ZMC_BOOL bAppend;              // The input bAppend is intended for the use-case of appending a further G-Code file to the last one, keeping the decoder state
    ZMC_BOOL bStepSuppress;        // When this input is TRUE, lines of the CNC program starting with '/' will be ignored
    SMC_POSINFO piStartPosition;   // Start position of the path. Only the position of the axes is used
    SMC_Vector3D vStartToolLength; // Start tool length
    ZMC_UDINT nSizeOutQueue;       // This variable contains the size of the data buffer
    SMC_GEOINFO* pbyBufferOutQueue; // This input must point to the first byte of the memory area that is allocated for the SMC_OUTQUEUE structure
    ZMC_BOOL bEnableSyntaxChecks;  // Turns on syntax checks that will detect invalid G-code and stop with an error in this case
    SMC_ORI_CONVENTION eOriConv;   // Defines the way the orientation in the A/B/C words is interpreted
    ZMC_LREAL dCircleTolerance;    // Tolerance to decide whether the definition of a circle makes sense
    ZMC_LREAL dPrecision;          // The precision of the path in technical units

    // Output parameters
    ZMC_BOOL bDone;                // TRUE when input data has been processed completely
    ZMC_BOOL bBusy;                // TRUE while execution is not finished
    ZMC_BOOL bError;               // TRUE when error has occurred
    ZMC_ERROR wErrorID;            // Error identification
    SMC_OUTQUEUE* poqDataOut;      // Pointer to SMC_OUTQUEUE structure with decoded G-code
    SMC_DEC_STATUS iStatus;        // Current decoder status
    ZMC_DINT iLineNumberDecoded;   // The variable contains the 0-based line number of the CNC file
    SMC_GCODE_TEXT GCodeText;      // The G-Code text
}FbtNCDecoderStruct;

#pragma pack(pop)

class FbtInterpolator : public FbtInterpolatorStruct {
public:
    FbtInterpolator();
    ~FbtInterpolator();
    void runCycle();
};

class FbtTrafoGantry2 : public FbtTrafoGantry2Struct {
public:
    FbtTrafoGantry2();
    ~FbtTrafoGantry2();
    void runCycle();
};

class FbtTrafoFGantry2 : public FbtTrafoFGantry2Struct {
public:
    FbtTrafoFGantry2();
    ~FbtTrafoFGantry2();
    void runCycle();
};

class FbtCncRef : public FbtCncRefStruct {
public:
    FbtCncRef();
    ~FbtCncRef();
    void runCycle();
};

// class FbtOutQueue : public FbtOutQueueStruct {
// public:
//     FbtOutQueue();
//     ~FbtOutQueue();
//     void runCycle();
// };

class FbtCheckVelocities : public FbtCheckVelocitiesStruct {
public:
    FbtCheckVelocities();
    ~FbtCheckVelocities();
    void runCycle();
};

class FbtNCDecoder : public FbtNCDecoderStruct {
public:
    FbtNCDecoder();
    ~FbtNCDecoder();
    void runCycle();
};

}
#endif 