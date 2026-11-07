#ifndef _ZMC_FB_REPEATER_DBG_H_
#define _ZMC_FB_REPEATER_DBG_H_

#include "fbt_common.h"

struct FbRepeaterDbgStruct {
    void* pImpl;
    // input
    ZMC_BOOL Execute;
    ZMC_BOOL TrigAbort;
    int Count;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
    int Value;
};

namespace zmc {
class FbRepeaterDbg : public FbRepeaterDbgStruct {
   public:
    FbRepeaterDbg();
    ~FbRepeaterDbg();
    void runCycle();
};

} /* namespace zmc */
#endif /* _ZMC_FB_REPEATER_DBG_H_ */
