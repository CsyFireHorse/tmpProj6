/*
 * @Author: ChenSy
 * @Date: 2024-07-25 17:00:17
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-29 13:59:34
 */
#ifndef _POLY_SPLINE_H_
#define _POLY_SPLINE_H_

#include <memory>
#include "spline_def.h"
#include "common/misc/polyval.h"
#include "vel/VelWrapCommon.h"
#include "cubic_poly.h"

namespace zmc {


class SplineProfileStruct {
   public:
    unsigned char _mode; // SPLINE_TYPE
    unsigned char _cond_mode; // ENUM_SPLINE_COND
    unsigned int _capacity = 0;
    unsigned int _len; // number of points
    double pts[4]; // condition value
    // data pointer
    double* xx = nullptr;
    double* yy = nullptr;
    double* vv = nullptr;
    double* aa = nullptr;
    double* xyva = nullptr;
    // allocated memory
    double* tt = nullptr;
    union {
        double (*pp4)[4] = nullptr; // use for cubic
        double (*pp6)[6];           // use for fifth pooly
        // double (*pp2)[2];           
        double* pp1;                // use for linear poly
        // double** pp;
    };
};

class SplineProfileBase:public SplineProfileStruct {
   public:
    void set_ptype(SPLINE_TYPE stype) { _mode = stype; };
    void set_condition_type(int ctype) { _cond_mode = ctype; };
    void set_xx_yy(int len, const double* px, const double* py) {
        xx = (double*)px;
        yy = (double*)py;
        _len = len;
    };
    void set_xx_yy_vv(int len, const double* px, const double* py, const double* pv = nullptr) {
        xx = (double*)px;
        yy = (double*)py;
        vv = (double*)pv;
        _len = len;
    };
    void set_xx_yy_vv_aa(int len, const double* px, const double* py, const double* pv = nullptr,
                         const double* pa = nullptr) {
        xx = (double*)px;
        yy = (double*)py;
        vv = (double*)pv;
        aa = (double*)pa;
        _len = len;
    };
    void set_xy(int len, const double* pDat) {
        xyva = (double*)pDat;
        _len = len / 2;
    };
    void set_xyva(int len, const double* pDat) {
        xyva = (double*)pDat;
        _len = len/4;
    };

    void set_condition_value(const double* pDat, int len) {
        for (int i = 0; i < len; i++) {
            pts[i] = pDat[i];
        }
    }
    int find_idx(double tm) const;

    void set_addr(void*, unsigned int);
    void* get_addr() const;
    int get_addr_len() const;
    inline void* get_poly_addr() const { return (void*)(tt + _len); }
};

// static_assert(sizeof(SplineProfileBase)<112);

// you has to call set_addr() to use allocated memory
class LinearSplineProfileImpl : public SplineProfileBase {
   public:
    enum { POLY_NUM = 2 };
    LinearSplineProfileImpl(int len, const double* tArr, const double* pArr);
    int build();

    double get_pos(double tm) const;
    double get_velocity(double tm) const;
    double get_acc(double tm) const;
    void get_pos_vel_acc(double tm, double* pos, double* vel, double* acc) const;
    void get_pva(double tm, PosVelAcc& pva) const;
    template <int P_NUM = POLY_NUM>
    inline int get_addr_sz(int sz) const {
        return sizeof(double) * (sz + sz);
    }
};

class CubicSplineProfileImpl : public SplineProfileBase {
   public:
    enum { POLY_NUM = 4 };
    CubicSplineProfileImpl(int len, const double* tArr, const double* pArr, const double* vArr);
    int build();
    void get_pva(double tm, PosVelAcc& pva) const;
    double get_pos(double tm) const;
    double get_velocity(double tm) const;
    double get_acc(double tm) const;
    void get_pos_vel_acc(double tm, double* pos, double* vel, double* acc) const;

    template <int P_NUM = POLY_NUM>
    inline int get_addr_sz(int sz) const {
        return sizeof(double) * (sz + (sz - 1) * P_NUM);
    }
};

class CubicSplineRecursionImpl : public CubicSplineProfileImpl {
   public:
    enum { POLY_NUM = 4 };
    CubicSplineRecursionImpl(int len, const double* tArr, const double* pArr, const double* vArr);
    int build();
    template <int P_NUM = POLY_NUM>
    inline int get_addr_sz(int sz) const {
        return sizeof(double) * (sz + (sz - 1) * P_NUM);
    }
};
class CubicSplineRecursion2Impl : public CubicSplineProfileImpl {
   public:
    enum { POLY_NUM = 4 };
    CubicSplineRecursion2Impl(int len, const double* xyArr, const double* vArr);
    int build();
    template <int P_NUM = POLY_NUM>
    inline int get_addr_sz(int sz) const {
        return sizeof(double) * (sz + (sz - 1) * P_NUM);
    }
};

class CubicSplineConiditonImpl : public CubicSplineProfileImpl {
   public:
    enum { POLY_NUM = 8 };
    CubicSplineConiditonImpl(int len, const double* tArr, const double* pArr, const double* vArr, int mode);
    int build();
    template <int P_NUM = POLY_NUM>
    inline int get_addr_sz(int sz) const {
        return sizeof(double) * (sz * P_NUM);
    }
};

class FifthOrderSplineProfileBase : public SplineProfileBase {
   public:
    enum { POLY_NUM = 6 };
    FifthOrderSplineProfileBase(int len, const double* tArr, const double* pArr,
                                const double* vArr, const double* aArr);
    FifthOrderSplineProfileBase();
    int build();
    void get_pva(double tm, PosVelAcc& pva) const;
    double get_pos(double tm) const;
    double get_velocity(double tm) const;
    double get_acc(double tm) const;
    void get_pos_vel_acc(double tm, double* pos, double* vel, double* acc) const;

    template <int POLY_NUM = 6>
    inline int get_addr_sz(int sz) const {
        return sizeof(double) * (sz + (sz - 1) * POLY_NUM);
    }
};

class FifthOrderSplineProfileBase2 : public FifthOrderSplineProfileBase {
   public:
    FifthOrderSplineProfileBase2(int len, const double* xyvaArr);
    int build();
};

template <typename T>
class MyAllocate {
   public:
    T* allocate(int size) { return new T[size]; }
    void deallocate(T* ptr, int size) { delete[] ptr; }
};

template <typename T, typename Alloc = MyAllocate<char>>
class PolyOrderSplineTmpl : public T, public Alloc {
   public:
    template <typename... Args>
    PolyOrderSplineTmpl(Args... args) : T(args...) {
        // Alloc alloc;
        // auto use Alloc to allocate memory
        T::_capacity =  T::template get_addr_sz<T::POLY_NUM>(T::_len);
        T::set_addr(Alloc::allocate(T::_capacity), T::_capacity);
    }
    ~PolyOrderSplineTmpl() {
        // Alloc alloc;
        Alloc::deallocate((char*)T::get_addr(), 0);  // get_addr_sz<T::POLY_NUM>(_len)
    }
};
using LinearSplineProfile = PolyOrderSplineTmpl<LinearSplineProfileImpl>;
using CubicSplineProfile = PolyOrderSplineTmpl<CubicSplineProfileImpl>;
using CubicRecursionProfile = PolyOrderSplineTmpl<CubicSplineRecursionImpl>;
using CubicRecursion2Profile = PolyOrderSplineTmpl<CubicSplineRecursion2Impl>;
using CubicConditionProfile = PolyOrderSplineTmpl<CubicSplineConiditonImpl>;
using FifthOrderSplineProfile = PolyOrderSplineTmpl<FifthOrderSplineProfileBase>;
using FifthOrderSplineProfile2 = PolyOrderSplineTmpl<FifthOrderSplineProfileBase2>;

}  // namespace zmc
#endif