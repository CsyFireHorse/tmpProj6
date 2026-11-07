// FifthOrderPolyWrap.cpp
#include "FifthOrderPolyWrap.h"
#include "cubic_poly.h"
#include <new>

namespace zmc {

inline FifthOrderPoly& get_poly(FifthOrderPolyWrap& self) {
    return *static_cast<FifthOrderPoly*>((void*)&self);
}

inline const FifthOrderPoly& get_poly(const FifthOrderPolyWrap& self) {
    return *static_cast<const FifthOrderPoly*>((void*) &self);
}

FifthOrderPolyWrap::FifthOrderPolyWrap() {
    new (storage_) FifthOrderPoly();
    static_assert(sizeof(FifthOrderPoly) <= STORAGE_SIZE, "FifthOrderPoly is too large");
}

FifthOrderPolyWrap::~FifthOrderPolyWrap() {
    get_poly(*this).~FifthOrderPoly();
}




void FifthOrderPolyWrap::init(double t0, double t1, double p0, double p1, 
                            double v0, double v1, double a0, double a1) {
    get_poly(*this).init2(t0, t1, p0, p1, v0, v1, a0, a1);
}

PosVelAcc FifthOrderPolyWrap::get_pva(double x) const {
    return get_poly(*this).get_pva(x);
}

void FifthOrderPolyWrap::get_pva(double t, PosVelAcc* pva) const {
    get_poly(*this).get_pva(t, *pva);
}

void FifthOrderPolyWrap::get_pvaj(double t, PosVelAccJerk* pvaj) const {
    get_poly(*this).get_pvaj(t, *pvaj);
}

} // namespace zmc