#include "FbtMoveCircular.h"

#include "FbtMoveGroupImplTmpl.hpp"
#include "traj/include/TrajEnumDef.h"

namespace zmc {

class FbtMoveCircularImpl : public FbtMoveGroupImplTmpl<FbtMoveCircularImpl, FbtMoveCircularStruct> {
   public:
    void onTraj() {
        trajGeom.set_circ_param(pParam->AuxPoint.v, pParam->EndPosition.v, pParam->PathChoice != 0,
                                pParam->MotionType != 0, static_cast<SMC_CIRC_MODE>(pParam->CircMode));
    }
};

class FbtMoveMixImpl : public FbtMoveGroupImplTmpl<FbtMoveMixImpl, FbtMoveMixStruct> {
   public:
    void onTraj() {
        if (pParam->MMode == 0) {
            trajGeom.set_line_param(pParam->EndPosition.v, pParam->MotionType != 0);
            return;
        }
        trajGeom.set_circ_param(pParam->AuxPoint.v, pParam->EndPosition.v, pParam->PathChoice != 0,
                                pParam->MotionType != 0, static_cast<SMC_CIRC_MODE>(pParam->CircMode));
    }
};

FbtMoveCircular::FbtMoveCircular() {
    all_zero_init<FbtMoveCircularStruct>(*this);
    pImpl = new FbtMoveCircularImpl();
    static_cast<FbtMoveCircularImpl*>(pImpl)->pParam = this;
}

FbtMoveCircular::~FbtMoveCircular() {
    delete static_cast<FbtMoveCircularImpl*>(pImpl);
    pImpl = nullptr;
}

void FbtMoveCircular::runCycle() {
    if (!pImpl) return;
    static_cast<FbtMoveCircularImpl*>(pImpl)->runCycle();
}

void FbtMoveCircular::reset() {
    if (!pImpl) return;
    auto* impl = static_cast<FbtMoveCircularImpl*>(pImpl);
    Execute = false;
    impl->execute = false;
    impl->_executeBak = false;
    impl->onClear();
    impl->onUpdate();
}

uint8_t FbtMoveCircular::getStat() const {
    if (!pImpl) return FB_STAT_IDLE;
    return static_cast<const FbtMoveCircularImpl*>(pImpl)->fbStat;
}

uint16_t FbtMoveCircular::getErrorId() const {
    if (!pImpl) return 0;
    return static_cast<uint16_t>(static_cast<const FbtMoveCircularImpl*>(pImpl)->error_code);
}

FbtMoveMix::FbtMoveMix() {
    all_zero_init<FbtMoveMixStruct>(*this);
    pImpl = new FbtMoveMixImpl();
    static_cast<FbtMoveMixImpl*>(pImpl)->pParam = this;
}

FbtMoveMix::~FbtMoveMix() {
    delete static_cast<FbtMoveMixImpl*>(pImpl);
    pImpl = nullptr;
}

void FbtMoveMix::runCycle() {
    if (!pImpl) return;
    static_cast<FbtMoveMixImpl*>(pImpl)->runCycle();
}

void FbtMoveMix::reset() {
    if (!pImpl) return;
    auto* impl = static_cast<FbtMoveMixImpl*>(pImpl);
    Execute = false;
    impl->execute = false;
    impl->_executeBak = false;
    impl->onClear();
    impl->onUpdate();
}

uint8_t FbtMoveMix::getStat() const {
    if (!pImpl) return FB_STAT_IDLE;
    return static_cast<const FbtMoveMixImpl*>(pImpl)->fbStat;
}

uint16_t FbtMoveMix::getErrorId() const {
    if (!pImpl) return 0;
    return static_cast<uint16_t>(static_cast<const FbtMoveMixImpl*>(pImpl)->error_code);
}

}  // namespace zmc
