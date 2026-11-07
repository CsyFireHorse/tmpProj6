#include "FbRepeaterDbg.h"

#include "fbt_fb_base.hpp"
#include "misc_helper.hpp"

namespace zmc {

class FbRepeaterDbgImpl
    : public FbtExeTmpl<FbRepeaterDbgImpl, FbtImplParamTmpl<FbRepeaterDbgStruct>> {
   public:
    void onStarted() {
        auto& param = *this->pParam;
        if (!this->isBusy()) this->fbStat = FB_STAT_BUSY_ACTIVE;
        _count = param.Count;
    }

    void onExecuting() {
        auto& param = *this->pParam;

        if (_idxCur < _count) {
            _idxCur += 1;
            param.Value = _idxCur;
            if (param.TrigAbort) {
                this->fbStat = FB_STAT_ABORTED;
                return;
            }    
        }
        if (_idxCur == _count) {
            this->fbStat = FB_STAT_DONE;
        } else if (_idxCur > _count) {
            this->fbStat = FB_STAT_ERROR;
            // param.ErrorID = ZMC_ERR_REPDBG_OVERFLOW_ERROR;
        } else {
            this->fbStat = FB_STAT_BUSY_ACTIVE;
        }
    }

    void onClear() {
        _idxCur = 0;
        _count = 0;
        this->pParam->Value = 0;
        this->fbStat = FB_STAT_IDLE;
    }
    void onCapture(){
        this->execute = pParam->Execute; 
    }
    void onUpdate(){
        this->pParam->Done = this->isDone();
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
   private:
    int _count = 0;
    int _idxCur = 0;
};

FbRepeaterDbg::FbRepeaterDbg() {
    all_zero_init<FbRepeaterDbgStruct>(*this);
    pImpl = new FbRepeaterDbgImpl();
    static_cast<FbRepeaterDbgImpl*>(pImpl)->pParam = this;
}

FbRepeaterDbg::~FbRepeaterDbg() {
    delete (FbRepeaterDbgImpl*)pImpl;
    pImpl = nullptr;
}   

void FbRepeaterDbg::runCycle() {
    static_cast<FbRepeaterDbgImpl*>(pImpl)->runCycle();
}

}  // namespace zmc