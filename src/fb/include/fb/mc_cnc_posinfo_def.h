#ifndef _MC_CNC_POSINFO_DEF_H_
#define _MC_CNC_POSINFO_DEF_H_

#include "zmc_types_def.h"
// #include "mc_cnc_enum_def.h"


// SMC_Vector3D structure
typedef struct _SMC_Vector3D_t {
    ZMC_LREAL dX;
    ZMC_LREAL dY;
    ZMC_LREAL dZ;
} SMC_Vector3D;

// SMC_POSINFO structure - Position information
typedef struct _SMC_POSINFO_t {
    ZMC_INT iFrameNo;              // Additional information not relevant for the SoftMotion modules
    ZMC_UINT wAuxData;          // SMC_ADDAXIS: Bit by bit description of position axis to be calculated
    ZMC_UINT wSProfile;         // Additional axes that have their bit set are interpolated in sigmoid (S-) shape
    ZMC_LREAL dX;                  // X-position in coordinate system
    ZMC_LREAL dY;                  // Y-position in coordinate system
    ZMC_LREAL dZ;                  // Z-position in coordinate system
    ZMC_LREAL dA;                  // Position of additional axis A
    ZMC_LREAL dB;                  // Position of additional axis B
    ZMC_LREAL dC;                  // Position of additional axis C
    ZMC_LREAL dA1;                 // Position of additional axis P
    ZMC_LREAL dA2;                 // Position of additional axis Q
    ZMC_LREAL dA3;                 // Position of additional axis U
    ZMC_LREAL dA4;                 // Position of additional axis V
    ZMC_LREAL dA5;                 // Position of additional axis W
    ZMC_LREAL dA6;                 // Position of additional axis A6 (Cannot be programmed with G-code!)
} SMC_POSINFO;

// SMC_GCodeObjectId structure - Identifies a path object
typedef struct _SMC_GCodeObjectId_t {
    ZMC_DINT id;                   // The unique identifier of the path object
} SMC_GCodeObjectId;


typedef struct SMC_GEOINFO SMC_GEOINFO;
typedef struct SMC_OUTQUEUE SMC_OUTQUEUE;

// SMC_GCODE_TEXT structure - G-code text information
typedef struct _SMC_GCODE_TEXT_t {
    ZMC_STRING str;        // String representing the current line (STRING(80))
    ZMC_DINT iLineNumber;  // Line number (from N word)
    ZMC_BOOL bNewLine;     // TRUE when a new line has been decoded in this cycle; FALSE else
    ZMC_BOOL bClearList;   // TRUE when the Decoder has been started from new, what implies that a buffer that may store the last lines should be emptied
} SMC_GCODE_TEXT;

// SMC_GCODE_WORD structure - G-code word data
typedef struct _SMC_GCODE_WORD_t {
    ZMC_LREAL fValue;      // Value as LREAL. This value is used by SMC_NCDecoder
    ZMC_DINT diValue;      // Value as DINT. Not used any more
    ZMC_BYTE* pAdr;        // If <> 0, the value of this word is specified by a variable. pAdr points to this variable
    ZMC_BYTE byLetter;     // ASCII-code of the letter of the word, e.g. 69 for 'F'
    ZMC_BYTE byVarType;    // If < 32, contains the variable type as defined in SMC_VARTYPE
} SMC_GCODE_WORD;


#endif 