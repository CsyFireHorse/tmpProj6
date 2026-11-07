/*
 * @Author: ChenSy
 * @Date: 2024-06-19 11:17:24
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-05-16 17:46:38
 */
#ifndef _MC_CAM_REF_IMPL_H_
#define _MC_CAM_REF_IMPL_H_

#include "stdint.h"
#include "zmc_types_def.h"

typedef struct SMC_CAMTable_LREAL_128_1_t {
    ZMC_LREAL Table[128];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_LREAL_128_1;

typedef struct SMC_CAMTable_LREAL_128_2_t {
    ZMC_LREAL Table[128][2];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_LREAL_128_2;

typedef struct SMC_CAMTable_LREAL_256_1_t {
    ZMC_LREAL Table[256];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_LREAL_256_1;

typedef struct SMC_CAMTable_LREAL_256_2_t {
    ZMC_LREAL Table[256][2];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_LREAL_256_2;

//////////
typedef struct SMC_CAMTable_REAL_128_1_t {
    ZMC_REAL Table[128];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_REAL_128_1;

typedef struct SMC_CAMTable_REAL_128_2_t {
    ZMC_REAL Table[128][2];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_REAL_128_2;

typedef struct SMC_CAMTable_REAL_256_1_t {
    ZMC_REAL Table[256];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_REAL_256_1;

typedef struct SMC_CAMTable_REAL_256_2_t {
    ZMC_REAL Table[256][2];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_REAL_256_2;

typedef struct SMC_CAMTable_UINT_128_1_t {
    ZMC_UINT Table[128];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_UINT_128_1;

typedef struct SMC_CAMTable_UINT_128_2_t {
    ZMC_UINT Table[128][2];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_UINT_128_2;

typedef struct SMC_CAMTable_UINT_256_1_t {
    ZMC_UINT Table[256];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_UINT_256_1;

typedef struct SMC_CAMTable_UINT_256_2_t {
    ZMC_UINT Table[256][2];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_UINT_256_2;

typedef struct SMC_CAMTable_UDINT_128_1_t {
    ZMC_UDINT Table[128];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_UDINT_128_1;

typedef struct SMC_CAMTable_UDINT_128_2_t {
    ZMC_UDINT Table[128][2];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_UDINT_128_2;

typedef struct SMC_CAMTable_UDINT_256_1_t {
    ZMC_UDINT Table[256];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_UDINT_256_1;

typedef struct SMC_CAMTable_UDINT_256_2_t {
    ZMC_UDINT Table[256][2];  //
    ZMC_REAL fEditorMasterMin;
    ZMC_REAL fEditorMasterMax;
    ZMC_REAL fEditorSlaveMin;
    ZMC_REAL fEditorSlaveMax;
    ZMC_REAL fTableMasterMin;
    ZMC_REAL fTableMasterMax;
    ZMC_REAL fTableSlaveMin;
    ZMC_REAL fTableSlaveMax;
} SMC_CAMTable_UDINT_256_2;

// use in MC_DigitalCamSwitch MC_CAMSWITCH_REF MC_CAMSWITCH_TR 
typedef enum _SMC_CAM_AXIS_DIECTION_t {
    SMC_CAM_AXIS_TWO_DIECTION = 0,  // The output is switched in two directions,
    SMC_CAM_AXIS_POS_DIECTION = 1,  // The output is only switched in positive direction,
    SMC_CAM_AXIS_NEG_DIECTION = 2,  // The output is only switched in negative direction
} SMC_CAM_AXIS_DIECTION;

typedef enum _SMC_CAM_SWITCH_MODE_t {
    SMC_CAM_SWITCH_BASE_POS = 0,  // The tappets are calculated position-based;
    SMC_CAM_SWITCH_BASE_TIME =
        1,  // The tappets are calculated time-based, whereby only the value of FirstOnPosition
            // will be used and the output stays TRUE for the given time (Duration).
} SMC_CAM_SWITCH_MODE;


typedef enum _ZMC_TAPPET_ACTION_t {
    NO_ACTION,
    SWITCH_ON,
    SWITCH_OFF,
    SWITCH_INVERT
} ZMC_TAPPET_ACTION;


#endif