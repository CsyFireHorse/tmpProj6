#include "fb_cnc_toy_def.h"
#include "fbt_fb_base.hpp"

namespace zmc {

class FbtInterpolatorToyImpl:public FbtImplParamTmpl<FbtInterpolatorToyStruct> {
   public:
    uint8_t iStatus = 0;
    bool bWorking_ = false;
    bool hasInit_ = false;
    enum LAST_PT:uint8_t{
        LAST_PT_NULL = 0,
        LAST_PT_EMIT,
        LAST_PT_WAIT,
        LAST_PT_EMIT_DONE,
        LAST_PT_WORK_DONE,
        LAST_PT_DONE,
    };
    LAST_PT lastPosStat_ = LAST_PT_NULL;
    uint8_t lastPosCnt = 0;
    void _runCycle() {
        if (execute && !_executeBak) {
            fbStat = FB_STAT_BUSY_ACTIVE;
            iStatus = IPO_INIT;
        }
        if (isBusy()) {
            if (pParam->bEmergency_Stop) {
                iStatus = IPO_WAIT;
                if (lastPosStat_ == LAST_PT_EMIT) 
                    lastPosStat_ = LAST_PT_WAIT;
                return;  // Emergency stop
            }
            if (pParam->poqDataIn->curIndex < pParam->poqDataIn->nCapacity) {
                pParam->piSetPosition = pParam->poqDataIn->pData[pParam->poqDataIn->curIndex] * pParam->poqDataIn->ratio;
                if (hasEmitFirst)
                    pParam->poqDataIn->curIndex++;
                if (pParam->poqDataIn->curIndex == pParam->poqDataIn->nCapacity && !lastPosStat_){
                    lastPosStat_ = LAST_PT_EMIT;
                    lastPosCnt = 2;
                }
                iStatus = IPO_CONSTANT;
            } else {
                if (lastPosStat_ == LAST_PT_WAIT || lastPosStat_ == LAST_PT_EMIT) {
                    if (lastPosCnt-- == 0){
                        lastPosStat_ = LAST_PT_EMIT_DONE;
                        iStatus = IPO_FINISHED;
                    }
                }else if (lastPosStat_ == LAST_PT_EMIT_DONE ) {
                    lastPosStat_ = LAST_PT_WORK_DONE;
                    bWorking_ = false;
                }else if (lastPosStat_ == LAST_PT_WORK_DONE ){
                    fbStat = FB_STAT_DONE; 
                    lastPosStat_ = LAST_PT_DONE; iStatus = IPO_FINISHED;
                }
            }
            if (!hasEmitFirst){
                hasEmitFirst = true;
                bWorking_ = true;
            }
        } else if (isDoneErrorAborted()) {
            if (!execute) {
                this->onClear();
            }
        }
    }
    void onClear(){
        FbtBase::onClear();
        this->iStatus = IPO_UNKNOWN;
        this->lastPosStat_ = LAST_PT_NULL;
        this->bWorking_ = false;
        this->lastPosCnt = 0;
    }
    void runCycle() {
        execute = pParam->bExecute;
        _runCycle();
        _executeBak = execute;
        this->onUpdate();
    }
    void onUpdate(){
        pParam->iStatus = this->iStatus;
        pParam->bDone = this->isDone();
        pParam->bBusy = this->isBusy();
        pParam->bError = this->isError();
        pParam->wErrorID = this->getErrorID();
        pParam->bWorking = this->bWorking_;
    }
};

FbtInterpolatorToy::FbtInterpolatorToy() {
    make_impl<FbtInterpolatorToyImpl, FbtInterpolatorToyStruct>(*this); 
}
FbtInterpolatorToy::~FbtInterpolatorToy() {
    if (pImpl) {
        delete (FbtInterpolatorToyImpl*)pImpl;
        pImpl = nullptr;
    }
}
void FbtInterpolatorToy::runCycle() { 
    if (!pImpl) return;
    auto impl = static_cast<FbtInterpolatorToyImpl*>(pImpl);
    impl->runCycle();
}

}  // namespace zmc