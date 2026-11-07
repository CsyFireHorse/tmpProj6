#ifndef ZS_VELOCITY_FORWARD_SJ_H_
#define ZS_VELOCITY_FORWARD_SJ_H_

#include "stdint.h"
#include "scurve_base.h"
#include "VelBase.h"

namespace zmc
{
class VelForwardSj:public VelForwarderBase{
public:
    enum {NUM_SLADDER_SEGMENT = 7};
    int get_plan(double* total_time, double* rVmax, double* rAcc=nullptr, double* rDec=nullptr)const;
    int get_slice(double slice, double *position, double *velocity, double *acc=nullptr) const;
    double get_distance(double tm)const;
    double get_rate(double tm)const;
    double get_acc(double tm)const;
    double get_jerk(double tm) const;

    int build();
    int check() const;
    double get_prefer_insertime(double tm)const;
    inline double get_begin_time()const { return cTarr[0];}
    inline double get_end_time()const { return cTarr[NUM_SLADDER_SEGMENT];}
    inline double get_rmax_vel() const { return vm; }
private:
    void _buildTimeSegment(void);
    void _buildTimeSegment_s70(void);
    void _buildTimeSegment_s60(void);
    void _buildTimeSegment_s61(void);
    void _buildTimeSegment_s62(void);
    void _buildTimeSegment_s50(void);
    void _buildTimeSegment_s51(void);
    void _buildTimeSegment_s52(void);
    void _buildTimeSegment_s40(void);
    void _buildTimeSegment_s41(void);
    void _buildTimeSegment_s42(void);
    void _buildTimeSegment_s31(void);
    void _buildTimeSegment_s32(void);
    void _buildTimeSegment_s21(void);
    void _buildTimeSegment_s22(void);
    void _buildTimeSegment_s10(void);

    void _buildPosVecSegment(void);
    void _buildPosVecSegment_s70(void);
    void _buildPosVecSegment_s60(void);
    void _buildPosVecSegment_s61(void);
    void _buildPosVecSegment_s62(void);
    void _buildPosVecSegment_s50(void);
    void _buildPosVecSegment_s51(void);
    void _buildPosVecSegment_s52(void);
    void _buildPosVecSegment_s40(void);
    void _buildPosVecSegment_s31(void);
    void _buildPosVecSegment_s32(void);
    void _buildPosVecSegment_s21(void);
    void _buildPosVecSegment_s22(void);

    void _cumsum();
    int _check();

    double _get_distance_s70(double tm)const;
    double _get_distance_s60(double tm)const;
    double _get_distance_s50(double tm)const;
    double _get_distance_s51(double tm)const;
    double _get_distance_s52(double tm)const;
    double _get_distance_s40(double tm)const;
    double _get_distance_s31(double tm)const;
    double _get_distance_s21(double tm)const;

    double _get_rate_s70(double tm)const;
    double _get_rate_s60(double tm)const;
    double _get_rate_s50(double tm)const;
    double _get_rate_s51(double tm)const;
    double _get_rate_s52(double tm)const;
    double _get_rate_s40(double tm)const;
    double _get_rate_s31(double tm)const;
    double _get_rate_s21(double tm)const;

    double _get_acc_rate_s70(double tm)const;
    double _get_acc_rate_s60(double tm)const;
    double _get_acc_rate_s50(double tm)const;
    double _get_acc_rate_s51(double tm)const;
    double _get_acc_rate_s52(double tm)const;
    double _get_acc_rate_s40(double tm)const;
    double _get_acc_rate_s31(double tm)const;
    double _get_acc_rate_s21(double tm)const;
  
    double _get_jerk_s70(double tm)const;
public:

    double am; // 可松弛
    double vm; // 可松弛
    double an; // 可松弛

private:
    union{
        double Tarr[NUM_SLADDER_SEGMENT+1];
        double cTarr[NUM_SLADDER_SEGMENT+1];
    };
    union{
        double Sarr[NUM_SLADDER_SEGMENT+1];
        double cSarr[NUM_SLADDER_SEGMENT+1];
    };
    double Varr[NUM_SLADDER_SEGMENT+1];
};

}
#endif