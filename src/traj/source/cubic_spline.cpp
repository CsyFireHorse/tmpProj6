#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "cstring"
#include "cubic_spline.h"

typedef struct Arr{double arr[15];}Arr;



void double_diff(const double* arr, double* out, int len){
    for (int i=0; i<len-1; i++){
        out[i] = arr[i+1] - arr[i];
    }
}

void double_inp_diff(double* arr, int len){
    for (int i=0; i<len-1; i++){
        arr[i] = arr[i+1] - arr[i];
    }
}

void double_dotdivide(double* arr, const double* div, int len){
    for (int i=0; i<len; i++){
        arr[i] /= div[i];
    }
}

void double_scale(double* arr, double scale, int len){
    for (int i=0; i<len; i++){
        arr[i] *= scale;
    }
}


/**
 *  solve diag3 matrix equation
 *  diag(a, b, c) * M = D
 * 
 *  \param[in] N  int     sizeof(bb)
 *  \param[in] aa   const double[N-1]
 *  \param[in] bb   double[N]
 *  \param[in] cc   const double[N-1]
 *  \param[in] dd  double[N]
 *  \param[out] mm   double[N]
 * 
 *  pp   double[N-1][4]
 */
static int _TomsonLu(int N, const double* aa, double* bb, const double* cc, double* mm){
    for(int i=1; i<N; i++){
        double c = cc[i-1] / bb[i-1];
        bb[i] -= c * aa[i-1];
        mm[i] -= c * mm[i-1];
    }
    mm[N-1] /= bb[N-1];
    for (int i=N-2;i>-1;i--){
        mm[i] = (mm[i] - aa[i]*mm[i+1])/bb[i];
    }
    return 0;
}

int CubicSpline::find_idx(double tm)const{
    if (tm >= this->tt[N-1])
        return N-2;
    else if (tm < this->tt[0])
        return 0;
    int i;
    for(i=0; i< N; i++){
        if (tm < tt[i])
            break;
    }
    i--;
    return i; // np.where(this->tt > tm)[0][0]-1  
}

double CubicSpline::get_pos(double tm)
{
    int i = find_idx(tm);
    Cubic_poly* cp = (Cubic_poly*) (&this->pp[i][0]);
    return cp->val(tm - this->tt[i]);
}

double CubicSpline::get_velocity(double tm)
{
    int i = find_idx(tm);
    Cubic_poly* cp = (Cubic_poly*) (&this->pp[i][0]);
    return cp->diff(tm - this->tt[i]);
}

double CubicSpline::get_acc(double tm)
{
    int i = find_idx(tm);
    Cubic_poly* cp = (Cubic_poly*) (&this->pp[i][0]);
    return cp->ddiff(tm - this->tt[i]);
}
void CubicSpline::get_pos_vel_acc(double tm, double* pos, double* vel, double* acc) const{
    int i = find_idx(tm);
    Cubic_poly* cp = (Cubic_poly*) (&this->pp[i][0]);
    if (pos) *pos = cp->val(tm - this->tt[i]);
    if (vel) *vel = cp->diff(tm - this->tt[i]);
    if (acc) *acc = cp->ddiff(tm - this->tt[i]);
}

CubicSpline::CubicSpline(int N, const double* tls, const double* yls, const double* cond, int mode){
    this->N = N;
    tt = (double*) malloc(sizeof(double)* N * 8);
    memcpy(tt, tls, sizeof(double)*N);
    yy = (double*)  &tt[N];
    memcpy(yy, yls, sizeof(double)*N);
    this->mode = mode;
    pts[0] = cond[0];
    pts[1] = cond[1];

    // double *pd = (double*) ;
    pp = (double(*)[4]) (&tt[2*N]);
    _build();
}

CubicSpline::~CubicSpline(){
    free(tt);
}

void cubicSpline_build(double* matr, int N, int mode, const double* pts);

void CubicSpline::_build(){
    cubicSpline_build(tt, N,  mode, pts);
}

void cubicSpline_build(double* tt, int N, int mode, const double* pts){
    double* yy = (double*)  &tt[N];
    auto pp = (double(*)[4]) (&tt[2*N]);
    double* matrx = (double*) pp;
    double* hs = &matrx[N*4]; // len=N-1
    double* dyh = &matrx[N*5];  // len=N-1
    double* aa = &matrx[0];  // len=N-1
    double* bb = &matrx[N]; // len=N
    double* cc = &matrx[N*2];  // len=N-1
    double* dd = &matrx[N*3];  // len=N

    double_diff(tt, hs, N);
    double_diff(yy, dyh, N);
    double_dotdivide(dyh, hs, N-1);

    memcpy(aa, hs, N*sizeof(double));
    memcpy(cc, hs, N*sizeof(double));
    if (mode == E_SPLINE_COND_CLAMPPED){
        bb[0] = hs[0]*2;
        for (int i=1; i<N-1; i++){
            bb[i] = 2*(hs[i] + hs[i-1]);
        }
        bb[N-1] = hs[N-2]*2;

        dd[0] = (dyh[0] - pts[0])*6;
        for (int i=1; i<N-1; i++){
            dd[i] = 6*(dyh[i]-dyh[i-1]);
        }
        dd[N-1] = (pts[1] - dyh[N-2])*6;
    }else if (mode == E_SPLINE_COND_NATURE){
        aa[0] = 0;
        cc[N-2] = 0;

        bb[0] = 1;
        for (int i=1; i<N-1; i++){
            bb[i] = 2*(hs[i] + hs[i-1]);
        }
        bb[N-1] = 1;

        dd[0] = pts[0];
        for (int i=1; i<N-1; i++){
            dd[i] = 6*(dyh[i]-dyh[i-1]);
        }
        dd[N-1] = pts[1];
    }else{
        // const double pts[2] = {0, 0}; //
        aa[0] = 0;
        cc[N-2] = 0;

        bb[0] = 1;
        for (int i=1; i<N-1; i++){
            bb[i] = 2*(hs[i] + hs[i-1]);
        }
        bb[N-1] = 1;

        dd[0] = pts[0];
        for (int i=1; i<N-1; i++){
            dd[i] = 6*(dyh[i]-dyh[i-1]);
        }
        dd[N-1] = pts[1] *6;
    }
    _TomsonLu(N, aa, bb, cc, dd);
    double *mm = dd;

    for (int i=0; i<N-1;i++){
        pp[i][0] = (mm[i+1]- mm[i])/6/hs[i];
        pp[i][1] = mm[i]/2;
        pp[i][2] = (yy[i+1]- yy[i])/hs[i] - hs[i]* mm[i]/2 - (mm[i+1]- mm[i])*hs[i]/6;
        pp[i][3] = yy[i];
    }
    // free(hs);
}


int spline_smooth(int type, double* arr, int len, double* out, double ratio)
{
    int len2 = len* ratio;
    double cond[2] = {0, 0};
    double *tls = (double*) malloc(sizeof(double)* len);
    for (int i=0; i< len; i++){
        tls[i] = i;
    }
    CubicSpline csp = CubicSpline(len, tls, arr, cond, E_SPLINE_COND_CYCLIC);
    for (int i=0; i< len2; i++){
        double tm = 1.0f* i* (len-1)/(len2-1);
        out[i] = csp.get_pos(tm);
    }
    free(tls);
    return 0;
}