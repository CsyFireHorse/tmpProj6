#ifndef _FB_CNC_TOY_DEF_H_
#define _FB_CNC_TOY_DEF_H_

#include "zmc_types_def.h"
#include "fbt_common.h"
#include "mc_ref_def.h"
#include "mc_cnc_enum_def.h"
// #include "mc_cnc_geoinfo_def.h"
// #include "mc_cnc_posinfo_def.h"

namespace zmc {

class FbtOutqueueToy {
   public:
    double* pData;
    int nCapacity;
    int curIndex = 0;
    double ratio = 1;
};

class FbtInterpolatorToyStruct {
   public:
    void* pImpl;  // Implementation pointer

    ZMC_BOOL bExecute;
    FbtOutqueueToy* poqDataIn;
    ZMC_BOOL bEmergency_Stop;
    ZMC_DWORD dwIpoTime;

    ZMC_BOOL bDone;      // TRUE when input data has been processed completely
    ZMC_BOOL bBusy;      // TRUE while execution is not finished
    ZMC_BOOL bError;     // TRUE when error has occurred
    ZMC_ERROR wErrorID;  // Error identification
    uint16_t iStatus;    // Current status of the function block
    ZMC_BOOL bWorking;   // TRUE if processing has started and not finished
    double piSetPosition;
    // uint8_t fbStat;
};

class FbtInterpolatorToy:public FbtInterpolatorToyStruct {
   public:
    FbtInterpolatorToy();
    ~FbtInterpolatorToy();
    void runCycle();
};

}  // namespace zmc
#endif