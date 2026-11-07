/*
 * @Author: ChenShengyong 
 * @Date: 2024-05-16 17:49:06 
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-12 16:38:28
 */
#ifndef ZS_VELOCITY_CPP_WRAP_H_
#define ZS_VELOCITY_CPP_WRAP_H_
#include "vel/VelWrapCommon.h"
#include "vel/include/vel_enum_def.h"

namespace zmc
{
union VelocityWrapImpl;

class VelocityWrap{
public:
    VelocityWrap();
    VelocityWrap(VelocityWrapImpl*);
    // Copy mirrors TrajWrap: placement-new / assign the storage-backed impl.
    VelocityWrap(const VelocityWrap &other);
    VelocityWrap &operator=(const VelocityWrap &other);
    ~VelocityWrap();
    // VelocityWrap* clone()const;
    void set_config(double vm, double acc, double dec, double jerk);
    void set_begin_end_pos_vel(double s0, double sd, double v0, double vd);
    void set_begin_end_pos(double s0, double sd);
    void set_begin_end_vel(double v0, double vd);
    void set_begin_end_acc(double a0=0, double ad=0);

    void set_begin_pva(double s0, double v0, double a0=0);
    void set_end_pva(double sd, double vd, double ad=0);
    void set_begin_pos(double s0);
    void set_begin_vel(double v0);
    void set_end_pos(double);

    void set_end_vel(double vd);
    void set_total_time(double Tc);
    void init();
    void set_num_den(int n, int d);
    void set_time_sample(double v);

    bool is_scurve()const;
    bool is_continuous_vel()const;
    bool is_no_dest()const;
    bool is_slave()const;
    void set_scurve(bool);
    void set_continuous_vel(bool);
    void set_no_dest(bool);
    void set_inversable(bool);
    void set_slave(bool);
    void set_sinsquare(bool);
    void set_mode(int);

    double cal_reachable_pre_vel(double vd)const;
    double cal_reachable_pro_vel(double v0)const;
    double cal_reachable_forward_vel(double current_vel) const;
    double cal_reachable_backward_vel(double target_vel) const;

    double get_end_pos()const;
    double get_delta_pos()const;
    int get_nseg()const;
    int check()const;
    void follow(double, double, double);
    void shift_time(double tm);
    void chop_left_time(double tm);
    int build();
    void shift_pos(double ofs);
    double get_begin_rate()const;
    double get_end_rate()const;
    double get_begin_acc() const;
    double get_end_acc() const;
    double get_rend_time()const;
    double get_max_vel()const;
    double get_rmax_vel()const;

    double get_end_time()const;
    int get_plan(double* total_time, double* rVmax, double* rAcc=nullptr, double* rDec=nullptr)const;
    int get_slice(double time, PosVelAcc&)const;
    int get_slice(double time, double* pos, double* vel, double* acc=nullptr)const;
    int get_slice_continuous(double time, PosVelAcc&)const;
    int get_plan2(double& total_time, double& rVmax, double& rAcc, double& rDec)const;
    int get_slice2(double time, double& pos, double& vel, double& acc)const;
    int get_slice4(double time, double* pva)const;
    double get_param(VelInnerParam)const;
    PosVelAcc get_slice3(double time)const;
    void get_config(double& vm, double& acc, double& dec, double& jerk);
    static double calc_stop_dist(double v0, double a0, double dec, double jerk);
    static double calc_stop_dist(double v0, double dec);
    static double calc_vel_dist(double v0, double vd, double acc, double dec);
    double cal_stop_pos()const;
private:
    static constexpr int STORAGE_SIZE = 384; // 344;
    char storage_[STORAGE_SIZE];
};

}


#endif  // ZS_VELOCITY_SLADDER_T_IMPL_H_
/* end of file */