#ifndef _ZMC_PFORM_DEF_H_
#define _ZMC_PFORM_DEF_H_

#include "zmc_types_def.h"

typedef struct MC_TP_t {
    ZMC_TIME DeltaTime;
    ZMC_LREAL Position;
} MC_TP;

#define MC_TP_REF_LENGTH (100)

// use in MC_PositionProfile
typedef struct MC_TP_ARRAY {
    ZMC_INT Number_of_pairs;
    ZMC_BOOL IsAbsolute;
    MC_TP MC_TP_Array[MC_TP_REF_LENGTH];
} MC_TP_ARRAY;


typedef struct MC_TV_t {
    ZMC_TIME DeltaTime;
    ZMC_LREAL velocity;
} MC_TV;

// use in MC_VelocityProfile
typedef struct MC_TV_ARRAY {
    ZMC_INT Number_of_pairs;  //
    ZMC_BOOL IsAbsolute;
    MC_TV MC_TV_Array[MC_TP_REF_LENGTH];
} MC_TV_ARRAY;


typedef struct MC_TA_t {
    ZMC_TIME DeltaTime;
    ZMC_LREAL acceleration;
} MC_TA;

// use in MC_AcclerationProfile
typedef struct MC_TA_ARRAY {
    ZMC_INT Number_of_pairs;
    ZMC_BOOL IsAbsolute;
    MC_TA MC_TA_Array[MC_TP_REF_LENGTH];
} MC_TA_ARRAY;
// typedef struct MC_TA_ARRAY* MC_TA_REF;

#endif