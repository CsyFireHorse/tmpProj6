#include "poly_spline.h"
#include "common/zmc/fbt_errorcode_def.h"

#include <memory>

#include "cstring"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void cubicSpline_build(double* matr, int N, int mode, const double* pts);
namespace zmc {

LinearSplineProfileImpl::LinearSplineProfileImpl(int len, const double* tArr, const double* pArr) {
    _len = len;
    xx = (double*)tArr;
    yy = (double*)pArr;
    this->set_ptype(SPLINE_LINE_FULL);
}

int LinearSplineProfileImpl::build() {
    if(_len < 2) return ZMC_ERR_TRAJ_INVALID_SPLINE_DATA;
    memcpy(tt, xx, sizeof(double) * _len);
    pp1 = tt + _len;  //  (double*)(tt + sizeof(double) * _len);
    memcpy(pp1, yy, sizeof(double) * _len);
    CubicPoly cp;
    for (int i = 0; i < _len - 1; i++) {
        if (tt[i] == tt[i + 1]) return ZMC_ERR_TRAJ_INVALID_SPLINE_DATA;
        // cp.init(tt[i], tt[i + 1], yy[i], yy[i + 1]);
        // pp4[i][0] = cp.dat[0];
        // pp4[i][1] = cp.dat[1];
    }
    return 0;
};
double LinearSplineProfileImpl::get_pos(double tm) const {
    int i = find_idx(tm);
    // double x = (tm - tt[i]) / (tt[i + 1] - tt[i]);
    //  cp->val(x);
    double rt = (tm - tt[i]) / (tt[i + 1] - tt[i]);
    return pp1[i] * (1 - rt) + pp1[i + 1] * rt;
};
double LinearSplineProfileImpl::get_velocity(double tm) const {
    int i = find_idx(tm);
    return (pp1[i + 1] - pp1[i]) / (tt[i + 1] - tt[i]);
};

double LinearSplineProfileImpl::get_acc(double tm) const { return 0; };

void LinearSplineProfileImpl::get_pos_vel_acc(double tm, double* pos, double* vel,
                                              double* acc) const {
    int i = find_idx(tm);
    CubicPolyOrg* cp = (CubicPolyOrg*)(&this->pp4[i][0]);
    // double sc = 1. / (tt[i + 1] - tt[i]);
    // double x = (tm - tt[i]) * sc;

    if (pos) {
        double rt = (tm - tt[i]) / (tt[i + 1] - tt[i]);
        *pos = pp1[i] * (1 - rt) + pp1[i + 1] * rt;
    }
    if (vel) *vel = (pp1[i + 1] - pp1[i]) / (tt[i + 1] - tt[i]);
    if (acc) *acc = 0;
};


void LinearSplineProfileImpl::get_pva(double tm, PosVelAcc& pva) const{
    int i = find_idx(tm);
    double rt = (tm - tt[i]) / (tt[i + 1] - tt[i]);
    pva.pos = pp1[i] * (1 - rt) + pp1[i + 1] * rt;
    pva.vel = (pp1[i + 1] - pp1[i]) / (tt[i + 1] - tt[i]);
    pva.acc = 0;
}

CubicSplineProfileImpl::CubicSplineProfileImpl(int len, const double* tArr, const double* pArr,
                                               const double* vArr) {
    _len = len;
    xx = (double*)tArr;
    yy = (double*)pArr;
    vv = (double*)vArr;
    this->set_ptype(SPLINE_CUBIC_FULL);
}

int SplineProfileBase::find_idx(double tm) const {
    if (tm >= this->tt[_len - 1])
        return _len - 2;
    else if (tm < this->tt[0])
        return 0;
    int i;
    for (i = 0; i < _len; i++) {
        if (tm < tt[i]) break;
    }
    i--;
    return i;  // np.where(this->tt > tm)[0][0]-1
}
double CubicSplineProfileImpl::get_pos(double tm) const {
    int i = find_idx(tm);
    CubicPolyOrg* cp = (CubicPolyOrg*)(&this->pp4[i][0]);
    double x = (tm - tt[i]) / (tt[i + 1] - tt[i]);
    return cp->val(x);
}

double CubicSplineProfileImpl::get_velocity(double tm) const {
    int i = find_idx(tm);
    CubicPolyOrg* cp = (CubicPolyOrg*)(&this->pp4[i][0]);
    double sc = 1. / (tt[i + 1] - tt[i]);
    double x = (tm - tt[i]) * sc;
    return cp->diff(x) * sc;
}
double CubicSplineProfileImpl::get_acc(double tm) const {
    int i = find_idx(tm);
    CubicPolyOrg* cp = (CubicPolyOrg*)(&this->pp4[i][0]);
    double sc = 1. / (tt[i + 1] - tt[i]);
    double x = (tm - tt[i]) * sc;
    return cp->ddiff(x) * sc * sc;
}

void CubicSplineProfileImpl::get_pva(double tm, PosVelAcc& pva) const{
    int i = find_idx(tm);
    CubicPolyOrg* cp = (CubicPolyOrg*)(&this->pp4[i][0]);
    double sc = 1. / (tt[i + 1] - tt[i]);
    double x = (tm - tt[i]) * sc;

    pva.pos = cp->val(x); 
    pva.vel = cp->diff(x) * sc;
    pva.acc = cp->ddiff(x) * sc * sc;
}

void CubicSplineProfileImpl::get_pos_vel_acc(double tm, double* pos, double* vel,
                                             double* acc) const {
    PosVelAcc pva;
    get_pva(tm, pva);
    if (pos) *pos = pva.pos;
    if (vel) *vel = pva.vel;
    if (acc) *acc = pva.acc;
}

int CubicSplineProfileImpl::build() {
    // tt = (double*)malloc(sizeof(double) * _len);
    // pp4 = (double(*)[4])malloc(sizeof(double) * (_len - 1) * 4);
    memcpy(tt, xx, sizeof(double) * _len);
    pp4 = (double(*)[4])get_poly_addr();
    CubicPoly cp;
    for (int i = 0; i < _len - 1; i++) {
        if (tt[i] == tt[i + 1]) return ZMC_ERR_TRAJ_INVALID_SPLINE_DATA;
        cp.init(tt[i], tt[i + 1], yy[i], yy[i + 1], vv[i], vv[i + 1]);
        pp4[i][0] = cp.dat[0];
        pp4[i][1] = cp.dat[1];
        pp4[i][2] = cp.dat[2];
        pp4[i][3] = cp.dat[3];
    }
    return 0;
}
CubicSplineRecursionImpl::CubicSplineRecursionImpl(int len, const double* tArr, const double* pArr,
                                               const double* vArr):CubicSplineProfileImpl(len, tArr, pArr, vArr){
    pts[0] = vArr[0];
    pts[1] = vArr[1];
    this->set_ptype(SPLINE_CUBIC_COND);
}

int CubicSplineRecursionImpl::build() {
    memcpy(tt, xx, sizeof(double) * _len);
    pp4 = (double(*)[4])get_poly_addr();
    CubicPoly2 cp;
    double vc = pts[0], ac = pts[1];
    for (int i = 0; i < _len - 1; i++) {
        if (tt[i] == tt[i + 1]) return ZMC_ERR_TRAJ_INVALID_SPLINE_DATA;
        cp.init(tt[i], tt[i + 1], yy[i], yy[i + 1], vc, ac);
        pp4[i][0] = cp.dat[0];
        pp4[i][1] = cp.dat[1];
        pp4[i][2] = cp.dat[2];
        pp4[i][3] = cp.dat[3];
        vc = cp.diff(tt[i + 1]);
        ac = cp.ddiff(tt[i + 1]);
    }
    return 0;
}

CubicSplineRecursion2Impl::CubicSplineRecursion2Impl(int len, const double* tArr,
                                               const double* vArr):CubicSplineProfileImpl(len/2, tArr, tArr, vArr){
    pts[0] = vArr[0];
    pts[1] = vArr[1];
    this->set_ptype(SPLINE_CUBIC_COND2);
}

int CubicSplineRecursion2Impl::build() {
    for (int i = 0; i < _len; i++) tt[i] = ((int64_t*)xx)[2 * i];
    pp4 = (double(*)[4])get_poly_addr();
    CubicPoly2 cp;
    double vc = pts[0], ac = pts[1];
    for (int i = 0; i < _len - 1; i++) {
        if (tt[i] == tt[i + 1]) return ZMC_ERR_TRAJ_INVALID_SPLINE_DATA;
        cp.init(tt[i], tt[i + 1], yy[2*i+1], yy[2*i + 3], vc, ac);
        pp4[i][0] = cp.dat[0];
        pp4[i][1] = cp.dat[1];
        pp4[i][2] = cp.dat[2];
        pp4[i][3] = cp.dat[3];
        vc = cp.diff(tt[i + 1]);
        ac = cp.ddiff(tt[i + 1]);
    }
    return 0;
}

CubicSplineConiditonImpl::CubicSplineConiditonImpl(int len, const double* tArr, const double* pArr,
                                               const double* vArr, int mode):CubicSplineProfileImpl(len, tArr, pArr, vArr){
    _mode = mode;                                                
    pts[0] = vArr[0];
    pts[1] = vArr[1];
    this->set_ptype(SPLINE_CUBIC_COND);
}

int CubicSplineConiditonImpl::build() {
    memcpy(tt, xx, sizeof(double) * _len);
    memcpy(tt + _len, yy, sizeof(double) * _len);
    pp4 = (double(*)[4]) (tt + 2*_len);
    
    cubicSpline_build(tt, _len, _mode, pts);
    return 0;
}


void SplineProfileBase::set_addr(void* ptr, unsigned int len) {
    tt = (double*)ptr;  // realloc(tt, );
    _capacity = len;
}

void* SplineProfileBase::get_addr() const { return tt; }
int SplineProfileBase::get_addr_len() const { return _capacity; }

FifthOrderSplineProfileBase::FifthOrderSplineProfileBase(int len, const double* tArr,
                                                         const double* pArr, const double* vArr,
                                                         const double* aArr) {
    _len = len;
    xx = (double*)tArr;
    yy = (double*)pArr;
    vv = (double*)vArr;
    aa = (double*)aArr;
    this->_mode = SPLINE_FIFTH_FULL;
}
FifthOrderSplineProfileBase::FifthOrderSplineProfileBase() {
    _len = 0;
    this->_mode = SPLINE_FIFTH_FULL;
}
double FifthOrderSplineProfileBase::get_pos(double tm) const {
    int i = find_idx(tm);
    FifthOrderPolyOrg* cp = (FifthOrderPolyOrg*)(&this->pp6[i][0]);
    double x = (tm - tt[i]) / (tt[i + 1] - tt[i]);
    return cp->val(x);
}
double FifthOrderSplineProfileBase::get_velocity(double tm) const {
    int i = find_idx(tm);
    FifthOrderPolyOrg* cp = (FifthOrderPolyOrg*)(&this->pp6[i][0]);
    double sc = 1. / (tt[i + 1] - tt[i]);
    double x = (tm - tt[i]) * sc;
    return cp->diff(x) * sc;
}
double FifthOrderSplineProfileBase::get_acc(double tm) const {
    int i = find_idx(tm);
    FifthOrderPolyOrg* cp = (FifthOrderPolyOrg*)(&this->pp6[i][0]);
    double sc = 1. / (tt[i + 1] - tt[i]);
    double x = (tm - tt[i]) * sc;
    return cp->ddiff(x) * sc * sc;
}

void FifthOrderSplineProfileBase::get_pva(double tm, PosVelAcc& pva) const{
    int i = find_idx(tm);
    FifthOrderPolyOrg* cp = (FifthOrderPolyOrg*)(&this->pp6[i][0]);
    double sc = 1. / (tt[i + 1] - tt[i]);
    double x = (tm - tt[i]) * sc;
    pva.pos = cp->val(x);
    pva.vel = cp->diff(x) * sc;
    pva.acc = cp->ddiff(x) * sc * sc;
}

void FifthOrderSplineProfileBase::get_pos_vel_acc(double tm, double* pos, double* vel,
                                                  double* acc) const {
    PosVelAcc pva;
    get_pva(tm, pva);
    if (pos) *pos = pva.pos;
    if (vel) *vel = pva.vel;
    if (acc) *acc = pva.acc;
}

int FifthOrderSplineProfileBase::build() {
    FifthOrderPoly cp;
    memcpy(tt, xx, sizeof(double) * _len);
    pp6 = (double(*)[6])get_poly_addr();
    for (int i = 0; i < _len - 1; i++) {
        if (tt[i] == tt[i + 1]) return ZMC_ERR_TRAJ_INVALID_SPLINE_DATA;
        cp.init(tt[i], tt[i + 1], yy[i], yy[i + 1], vv[i], vv[i + 1], aa[i], aa[i + 1]);
        pp6[i][0] = cp.dat[0];
        pp6[i][1] = cp.dat[1];
        pp6[i][2] = cp.dat[2];
        pp6[i][3] = cp.dat[3];
        pp6[i][4] = cp.dat[4];
        pp6[i][5] = cp.dat[5];
    }
    return 0;
}

FifthOrderSplineProfileBase2::FifthOrderSplineProfileBase2(int len, const double* xyvaArr) {
    _len = len / 4;
    xx = (double*)xyvaArr;
    yy = (double*)xyvaArr;
    vv = (double*)xyvaArr;
    aa = (double*)xyvaArr;
    xyva = (double*)xyvaArr;
    this->_mode = SPLINE_FIFTH_XYVA;
}

int FifthOrderSplineProfileBase2::build() {
    FifthOrderPoly cp;
    for (int i = 0; i < _len; i++) tt[i] = xx[4 * i];
    pp6 = (double(*)[6])get_poly_addr();
    for (int i = 0; i < _len - 1; i++) {
        if (tt[4 * i] == tt[4 * i + 4]) return ZMC_ERR_TRAJ_INVALID_SPLINE_DATA;
        cp.init(aa[4 * i], aa[4 * i + 4], aa[4 * i + 1], aa[4 * i + 5], aa[4 * i + 2],
                aa[4 * i + 6], aa[4 * i + 3], aa[4 * i + 7]);
        pp6[i][0] = cp.dat[0];
        pp6[i][1] = cp.dat[1];
        pp6[i][2] = cp.dat[2];
        pp6[i][3] = cp.dat[3];
        pp6[i][4] = cp.dat[4];
        pp6[i][5] = cp.dat[5];
    }
    return 0;
}

}  // namespace zmc
