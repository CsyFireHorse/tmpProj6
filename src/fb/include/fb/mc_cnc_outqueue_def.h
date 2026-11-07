
#ifndef _MC_CNC_OUTQUEUE_DEF_H_
#define _MC_CNC_OUTQUEUE_DEF_H_

#include "zmc_types_def.h"
#include "mc_cnc_geoinfo_def.h"

// SMC_OUTQUEUE structure - Output queue for managing SMC_GEOINFO objects
typedef struct _SMC_OUTQUEUE_t {
    ZMC_WORD wOUTQUEUEStructID;    // Fixed value to check if structure is of type SMC_OUTQUEUE
    SMC_GEOINFO* pbyBuffer;        // Address of coherent memory buffer allocated for SMC_GEOINFO objects
    ZMC_UDINT nCapacity;           // Contains the capacity in multiples of SMC_GEOINFO objects
    ZMC_UDINT nReadPos;            // Contains the position of the first object in the list
    ZMC_BOOL bEndOfList;           // Set to TRUE when the last SMC_GEOINFO object has been stored
    ZMC_BYTE byGenerator;          // Originator of the queue
    ZMC_BOOL bReCheckVelocities;   // Internal variable
    ZMC_STRING strProgramName;     // Name of the CNC program
    ZMC_UDINT nWritePos;           // Contains the next free position
    ZMC_BOOL xPartofLM;            // If TRUE, the function block is maintained by managers
    ZMC_UDINT nNumReservedEntries; // The number of reserved entries
    ZMC_DINT nPastMarker;          // Points to the first element that has been stored in the queue
} SMC_OUTQUEUE;

#endif 