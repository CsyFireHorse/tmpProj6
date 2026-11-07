#include "SplineCppWrap.h"
#include <new>
#include "common/misc/ZmcAllocator.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "cubic_spline.h"
#include "poly_spline.h"


namespace zmc {

struct PoliFitterWrapStorage {
    SplineProfileBase base;
    PmAllocator* pAlloc = nullptr;
};


// inline PoliFitterWrapStorage& getStorage(char* storage) {
//     return *static_cast<PoliFitterWrapStorage*>((void*) storage);
// }
// inline const SplineProfileBase& getBase(const char* storage) {
//     return static_cast<const PoliFitterWrapStorage*>((void*) storage)->base;
// }

static inline PoliFitterWrapStorage& getStorage(PoliFitterWrap& self) {
    return ((PoliFitterWrapStorage&)self);
}

static inline SplineProfileBase& getBase(PoliFitterWrap& self) {
    return ((PoliFitterWrapStorage&)self).base;
}

static inline const SplineProfileBase& getBase(const PoliFitterWrap& self) {
    return ((const PoliFitterWrapStorage&)self).base;
}

template <typename T>
inline const T& getSpline(const PoliFitterWrap& self) {
    return static_cast<const T&>(getBase(self));
}

int tryAllocate(PoliFitterWrap& self, int cap) {
    auto* pAlloc = getStorage(self).pAlloc;
    auto& base = getStorage(self).base;
    if (base._capacity >= cap) return 0;
    if (base._capacity > 0) {
        pAlloc->deallocate(base.get_addr(), base._capacity);
    }
    base._capacity = cap;
    void* ptr = pAlloc->allocate(cap);
    if (!ptr) return ZMC_ERR_TRAJ_ALLOC_FAILED;
    base.set_addr(ptr, cap);
    return 0;
}


PoliFitterWrap::PoliFitterWrap() {
    new (storage_) PoliFitterWrapStorage();
    static_assert(sizeof(PoliFitterWrapStorage) <= STORAGE_SIZE, "PoliFitterWrapStorage is too large");
}

PoliFitterWrap::~PoliFitterWrap() {
    getBase(*this).~SplineProfileBase();
}

void PoliFitterWrap::set_allocator(void* pAlloc) {
    getStorage(*this).pAlloc = static_cast<PmAllocator*>(pAlloc);
}

void PoliFitterWrap::set_default_allocator() {
    static AllocatorDefault alloc;
    getStorage(*this).pAlloc = &alloc;
}
void* PoliFitterWrap::get_allocator(){
    return getStorage(*this).pAlloc;
}

void PoliFitterWrap::set_xx_yy(int len, const double* tArr, const double* pArr) {
    getBase(*this).set_xx_yy(len, tArr, pArr);
}


void PoliFitterWrap::set_xy(int len, const double* xyArr) {
    getBase(*this).set_xy(len, xyArr);
}

void PoliFitterWrap::set_xx_yy_vv(int len, const double* tArr, const double* pArr,
                                  const double* vArr) {
    getBase(*this).set_xx_yy_vv(len, tArr, pArr, vArr);
}
void PoliFitterWrap::set_xx_yy_vv_aa(int len, const double* tArr, const double* pArr,
                                     const double* vArr, const double* aArr) {
    getBase(*this).set_xx_yy_vv_aa(len, tArr, pArr, vArr, aArr);
}

void PoliFitterWrap::set_xyva(int len, const double* pXy) {
    getBase(*this).set_xyva(len, pXy);
}

void PoliFitterWrap::set_ptype(SPLINE_TYPE ptype) { getBase(*this).set_ptype(ptype); }

void PoliFitterWrap::set_condition_type(int ctype) { getBase(*this).set_condition_type(ctype); }
void PoliFitterWrap::set_condition_value(const double* cond, int len) {
    getBase(*this).set_condition_value(cond, len);
}


int PoliFitterWrap::build() {
    // auto call Alloc to allocate memory
    PmAllocator* pAlloc = getStorage(*this).pAlloc;
    auto& spl = getBase(*this);
    if (!pAlloc) return ZMC_ERR_TRAJ_ALLOC_FAILED;
    switch (spl._mode) {
        case SPLINE_LINE_FULL: {
            LinearSplineProfileImpl& fsp = *((LinearSplineProfileImpl*)&spl);
            new (this) LinearSplineProfileImpl(spl._len, spl.xx, spl.yy);
            if (tryAllocate(*this, fsp.get_addr_sz(fsp._len))) return ZMC_ERR_TRAJ_ALLOC_FAILED;
            return fsp.build();
        }
        case SPLINE_CUBIC_COND: {
            if (spl._cond_mode == E_SPLINE_BEG_VEL_ACC_COND) {
                CubicSplineRecursionImpl& fsp = *((CubicSplineRecursionImpl*)&spl);
                new (this) CubicSplineRecursionImpl(spl._len, spl.xx, spl.yy, spl.pts);
                if (tryAllocate(*this, fsp.get_addr_sz(fsp._len))) return ZMC_ERR_TRAJ_ALLOC_FAILED;
                return fsp.build();
            } else {
                CubicSplineConiditonImpl& fsp = *((CubicSplineConiditonImpl*)&spl);
                new (this)
                    CubicSplineConiditonImpl(spl._len, spl.xx, spl.yy, spl.pts, spl._cond_mode);
                if (tryAllocate(*this, fsp.get_addr_sz(fsp._len))) return ZMC_ERR_TRAJ_ALLOC_FAILED;
                return fsp.build();
            }
        }
        case SPLINE_CUBIC_COND2: {
            if (spl._cond_mode == E_SPLINE_BEG_VEL_ACC_COND) {
                CubicSplineRecursion2Impl& fsp = *((CubicSplineRecursion2Impl*)&spl);
                new (this) CubicSplineRecursion2Impl(spl._len * 2, spl.xyva, spl.pts);
                if (tryAllocate(*this, fsp.get_addr_sz(fsp._len))) return ZMC_ERR_TRAJ_ALLOC_FAILED;
                return fsp.build();
            }
        }
        case SPLINE_CUBIC_FULL: {
            CubicSplineProfileImpl& fsp = *((CubicSplineProfileImpl*)&spl);
            new (this) CubicSplineProfileImpl(spl._len, spl.xx, spl.yy, spl.vv);
            if (tryAllocate(*this, fsp.get_addr_sz(fsp._len))) return ZMC_ERR_TRAJ_ALLOC_FAILED;
            return fsp.build();
        }
        case SPLINE_FIFTH_COND:
            break;
        case SPLINE_FIFTH_XYVA: {
            FifthOrderSplineProfileBase2& fsp = *((FifthOrderSplineProfileBase2*)&spl);
            new (this) FifthOrderSplineProfileBase2(spl._len * 4, spl.xyva);
            if (tryAllocate(*this, fsp.get_addr_sz(fsp._len))) return ZMC_ERR_TRAJ_ALLOC_FAILED;
            return fsp.build();
        }
        case SPLINE_FIFTH_FULL: {
            FifthOrderSplineProfileBase& fsp = *((FifthOrderSplineProfileBase*)&spl);
            new (this) FifthOrderSplineProfileBase(spl._len, spl.xx, spl.yy, spl.vv, spl.aa);
            if (tryAllocate(*this, fsp.get_addr_sz(fsp._len))) return ZMC_ERR_TRAJ_ALLOC_FAILED;
            return fsp.build();
        }
        default:
            break;
    }
    return ZMC_ERR_TRAJ_INVALID_MODE;
}

// double PoliFitterWrap::get_pos(double tm) const { return 0;}
// double PoliFitterWrap::get_vel(double tm) const { return 0;}
// double PoliFitterWrap::get_acc(double tm) const { return 0;}
#if 1
double PoliFitterWrap::get_pos(double tm) const {
    // auto& spl = getBase(*this);
    // const auto& impl = static_cast<const PoliFitterWrapStorage*>((const void*) storage_)->base;
    switch (getBase(*this)._mode) {
        case SPLINE_LINE_FULL:
            return getSpline<LinearSplineProfileImpl>(*this).get_pos(tm);
        case SPLINE_CUBIC_COND:
            return getSpline<CubicSplineRecursionImpl>(*this).get_pos(tm);
        case SPLINE_CUBIC_FULL:
            return getSpline<CubicSplineProfile>(*this).get_pos(tm);
        case SPLINE_FIFTH_XYVA:
            return getSpline<FifthOrderSplineProfile>(*this).get_pos(tm);
        case SPLINE_FIFTH_COND:
        case SPLINE_FIFTH_FULL:
            return getSpline<FifthOrderSplineProfile2>(*this).get_pos(tm);
        default:
            return 0;
    }
}

double PoliFitterWrap::get_vel(double tm) const {
    switch (getBase(*this)._mode) {
        case SPLINE_LINE_FULL:
            return getSpline<LinearSplineProfileImpl>(*this).get_velocity(tm);
        case SPLINE_CUBIC_COND:
            return getSpline<CubicSplineRecursionImpl>(*this).get_velocity(tm);
        case SPLINE_CUBIC_FULL:
            return getSpline<CubicSplineProfile>(*this).get_velocity(tm);
        case SPLINE_FIFTH_XYVA:
            return getSpline<FifthOrderSplineProfile>(*this).get_velocity(tm);
        case SPLINE_FIFTH_COND:
        case SPLINE_FIFTH_FULL:
            return getSpline<FifthOrderSplineProfile2>(*this).get_velocity(tm);
        default:
            return 0;
    }
}

double PoliFitterWrap::get_acc(double tm) const {
    switch (getBase(*this)._mode) {
        case SPLINE_LINE_FULL:
            return getSpline<LinearSplineProfileImpl>(*this).get_acc(tm);
        case SPLINE_CUBIC_COND:
            return getSpline<CubicSplineRecursionImpl>(*this).get_acc(tm);
        case SPLINE_CUBIC_FULL:
            return getSpline<CubicSplineProfile>(*this).get_acc(tm);
        case SPLINE_FIFTH_XYVA:
            return getSpline<FifthOrderSplineProfile>(*this).get_acc(tm);
        case SPLINE_FIFTH_COND:
        case SPLINE_FIFTH_FULL:
            return getSpline<FifthOrderSplineProfile2>(*this).get_acc(tm);
        default:
            return 0;
    }
}

#endif
void PoliFitterWrap::get_pos_vel_acc(double tm, double* pos, double* vel, double* acc) const {
    PosVelAcc pva;
    get_pva(tm, &pva);
    if (pos) *pos = pva.pos;
    if (vel) *vel = pva.vel;
    if (acc) *acc = pva.acc;
}

void PoliFitterWrap::get_pva(double tm, PosVelAcc* pva) const {
    auto& spl = getBase(*this);
    switch (spl._mode) {
        case SPLINE_LINE_FULL:
            return getSpline<LinearSplineProfileImpl>(*this).get_pva(tm, *pva);
        case SPLINE_CUBIC_COND:
        case SPLINE_CUBIC_COND2:
            return getSpline<CubicSplineRecursionImpl>(*this).get_pva(tm, *pva);
        case SPLINE_CUBIC_FULL:
            return getSpline<CubicSplineProfile>(*this).get_pva(tm, *pva);
        case SPLINE_FIFTH_XYVA:
            return getSpline<FifthOrderSplineProfile>(*this).get_pva(tm, *pva);
        case SPLINE_FIFTH_COND:
        case SPLINE_FIFTH_FULL:
            return getSpline<FifthOrderSplineProfile2>(*this).get_pva(tm, *pva);
    }   
}

}  // namespace zmc
