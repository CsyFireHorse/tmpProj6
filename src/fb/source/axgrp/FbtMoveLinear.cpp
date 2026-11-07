#include "FbtMoveLinear.h"

#include "FbtMoveGroupImplTmpl.hpp"

namespace zmc {

class FbtMoveLinearImpl : public FbtMoveGroupImplTmpl<FbtMoveLinearImpl, FbtMoveLinearStruct> {
   public:
    void onTraj() { trajGeom.set_line_param(pParam->Position.v, pParam->MotionType); }
};

FbtMoveLinear::FbtMoveLinear() {
    all_zero_init<FbtMoveLinearStruct>(*this);
    pImpl = new FbtMoveLinearImpl();
    static_cast<FbtMoveLinearImpl*>(pImpl)->pParam = this;
}

FbtMoveLinear::~FbtMoveLinear() {
    delete static_cast<FbtMoveLinearImpl*>(pImpl);
    pImpl = nullptr;
}

void FbtMoveLinear::runCycle() {
    if (!pImpl) return;
    static_cast<FbtMoveLinearImpl*>(pImpl)->runCycle();
}

void FbtMoveLinear::reset() {
    if (!pImpl) return;
    auto* impl = static_cast<FbtMoveLinearImpl*>(pImpl);
    Execute = false;
    impl->execute = false;
    impl->_executeBak = false;
    impl->onClear();
    impl->onUpdate();
}

uint8_t FbtMoveLinear::getStat() const {
    if (!pImpl) return FB_STAT_IDLE;
    return static_cast<const FbtMoveLinearImpl*>(pImpl)->fbStat;
}

uint16_t FbtMoveLinear::getErrorId() const {
    if (!pImpl) return 0;
    return static_cast<uint16_t>(static_cast<const FbtMoveLinearImpl*>(pImpl)->error_code);
}

}  // namespace zmc
