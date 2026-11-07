#ifndef _ZMC_FB_REF_DEF_H_
#define _ZMC_FB_REF_DEF_H_

#include "zmc_types_def.h"

typedef struct MC_TP_ARRAY MC_TP_ARRAY;
typedef struct MC_TP_ARRAY* MC_TP_REF;
typedef struct MC_TV_ARRAY MC_TV_ARRAY;
typedef struct MC_TV_ARRAY* MC_TV_REF;
typedef struct MC_TA_ARRAY MC_TA_ARRAY;
typedef struct MC_TA_ARRAY* MC_TA_REF;

typedef struct MC_CAM_TABLE MC_CAM_TABLE;
typedef MC_CAM_TABLE* MC_CAM_REF;


#ifndef AW_PLC_NATIVE_ALIGN
#define AW_PLC_NATIVE_ALIGN 8
#endif

#pragma pack(push, AW_PLC_NATIVE_ALIGN)
// STRUCT
typedef struct MC_CAM_ID {
    ZMC_BYTE* pCT;
    ZMC_BOOL Periodic;
    ZMC_BOOL MasterAbsolute;
    ZMC_BOOL SlaveAbsolute;
    ZMC_LREAL StartMaster;
    ZMC_LREAL EndMaster;
    ZMC_LREAL StartSlave;
    ZMC_LREAL EndSlave;
    ZMC_BYTE byCompatibilityMode;
} MC_CAM_ID;

typedef struct SMC_CAMTappet SMC_CAMTappet;
typedef struct SMC_TappetData {
	SMC_CAMTappet* pTaps[3];
	ZMC_DWORD dwCycleTime;
	ZMC_BYTE byChannels;
	ZMC_BOOL bRestart;
}SMC_TappetData;
typedef SMC_TappetData* SMC_TappetData_REF;


// #define MAX_NUM_TAPPETS (64)
#define MAX_NUM_TAPPETS (512)
typedef bool TAPPET_BITS[MAX_NUM_TAPPETS];
// ARRAY [1..MAX_NUM_TAPPETS] OF BOOL
// typedef TAPPET_BITS* TAPPET_BITS_REF;
typedef struct TAPPET_BITS_STRUCT{
    uint8_t arr[MAX_NUM_TAPPETS];
}TAPPET_BITS_STRUCT;
typedef TAPPET_BITS_STRUCT* TAPPET_BITS_REF;


// use in MC_DigitalCamSwitch
// This STRUCT describes the switch positions of the tappets.
typedef struct _MC_CAMSWITCH_TR_t {
    ZMC_INT TrackNumber;  // Number of the output; several position ranges per output are possible.
                          // [1..32]
    ZMC_LREAL FirstOnPosition;  // If moving in positive direction, the first position where the
                                // switch is on, otherwise the last; may be > LastOnPosition which
                                // results in an "inverted" switch
    ZMC_LREAL
    LastOnPosition;  // ZMC_LREAL If moving in positive direction, the last position where the
                     // switch is on, otherwise the first; Used only in case CamSwitchMode = 0
    ZMC_INT AxisDirection;  // see SMC_CAM_AXIS_DIECTION
    ZMC_INT CamSwitchMode;
    ZMC_TIME Duration;   // Period of time for which the tappet output stays TRUE in case of
                         // CAMSwitchMode = 1 .
    ZMC_BOOL bOn;        // internal; used by MC_DigitalCamSwitch
    ZMC_INT CounterOff;  // internal; used by MC_DigitalCamSwitch
} MC_CAMSWITCH_TR;

// This STRUCT describes a list of cam switches and is used as an input of MC_DigitalCamSwitch.
typedef struct MC_CAMSWITCH {
    ZMC_BYTE NoOfSwitches;  //  the number of MC_CAMSWITCH_TR to be evaluated
    MC_CAMSWITCH_TR *CamSwitchPtr;
} MC_CAMSWITCH;
typedef MC_CAMSWITCH* MC_CAMSWITCH_REF;

// typedef ZMC_BOOL MC_OUTPUT_REF[32];

typedef struct MC_OUTPUT_STRUCT{
    ZMC_BOOL arr[32];
}MC_OUTPUT_STRUCT;
typedef MC_OUTPUT_STRUCT* MC_OUTPUT_STRUCT_REF;
typedef MC_OUTPUT_STRUCT_REF MC_OUTPUT_REF;

typedef struct _MC_TRACK_TR_t {
    ZMC_LREAL
    OnCompensation;  // A delay (value positive) or an early switch-on (value negative) can be
                     // set. The time is given in seconds. May affect whether the switch is
                     // inverted or not. See the output SwitchCorrupted of MC_DigitalCamSwitch.
                     // Always used for switching ON, regardless the direction of the axis.
    ZMC_LREAL OffCompensation;  // A delay (value positive) or an early switch-off (value negative)
                                // can be set. The time is given in seconds. May affect whether the
                                // switch is inverted or not. See the output SwitchCorrupted of
                                // MC_DigitalCamSwitch. Not used in time-based mode. Always used
                                // for switching OFF, regardless the direction of the axis.
    ZMC_LREAL
    Hysteresis;  //  Additionally, a hysteresis can be set, which avoids a permanent
                 //  switching of the output, which might occur e.g. if the servo-controller
                 //  is exactly in switch-position and slightly swinging around that position
                 //  due to the position control. A track won't switch if the distance to the
                 //  last switching position is smaller than this variable.
} MC_TRACK_TR;

// typedef MC_TRACK_TR MC_TRACK_REF[32];

typedef struct MC_TRACK_TR_STRUCT{
    MC_TRACK_TR arr[32];
}MC_TRACK_TR_STRUCT;
typedef MC_TRACK_TR_STRUCT* MC_TRACK_TR_STRUCT_REF;
typedef MC_TRACK_TR_STRUCT_REF MC_TRACK_REF;

// MC_TouchProbe. The axis driver decides how the trigger number is mapped to hardware triggers.
typedef struct _TRIGGER_INPUT_t {
    ZMC_INT
    iTriggerNumber;  //    Trigger channel; defined by driver (only used when bFastLatching ``
                     //    = ``TRUE). Must be in the range [0, SMC_MAX_NUM_TRIGGERS-1]. Only
                     //    used if bFastLatching is TRUE. Please check the driver documentation
                     //    (PDF file inside driver library) for details on the used driver.
    ZMC_BOOL bFastLatching;  //  TRUE: Latching is done in drive (precise).
    // FALSE: Latching is done with bInput in the cycle of the motion task (unprecise).
    ZMC_BOOL bInput;  // Trigger signal when bFastLatching = FALSE.
} _TRIGGER_INPUT_t;
typedef _TRIGGER_INPUT_t*  TRIGGER_REF;


typedef struct ZMC_STRING_80{
    char value[81];
}ZMC_STRING_80;

typedef ZMC_STRING_80* ZMC_STRING_80_REF;

typedef struct ZMC_STRING_255{
    char value[256];
}ZMC_STRING_255;

typedef ZMC_STRING_255* ZMC_STRING_255_REF;

#pragma pack(pop)

#endif
