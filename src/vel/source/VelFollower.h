#ifndef _VEL_FOLLOWER_H_
#define _VEL_FOLLOWER_H_

#include "VelBase.h"
#include "math.h"

namespace zmc {

// class FuncVelTrapzRev:public VelFunctorBase{
// public:
//     int operator()(const VelParam& param, VelForwarderTrapzUn& fw);
// };
// // support inverse distance
// class VelTrapzRev:public VelTmplBase<FuncVelTrapzRev, VelForwarderTrapzUn>{
// };

typedef PosVelAcc PosInfo;

template <typename T>
inline T sat(T val, T low, T up) {
    if (val < low)
        return low;
    else if (val > up)
        return up;
    else
        return val;
}

class VelGearFollower : public VelBase {
   public:
    inline bool is_scurve() const { return param.is_scurve(); }
    void set_begin_pos(double s0, double v0, double a0) {
        _curPos.pos = s0;
        _curPos.vel = v0;
        _curPos.acc = a0;
    }
    void set_num_den(int n, int d) { num = n; den = d; }
    void set_time_sample(double v) { Ts = v;}
    void init(){
        is_gearin = false;
        is_pos_mode = true;
        _has_begin = false;
        num = 1;
        den = 1;
        Ts = 0.01;
    }
    void follow(double sc, double vc, double ac) { _mPos.pos = sc; _mPos.vel = vc; _mPos.acc = ac; }
    void follow(PosInfo& mPos) { _mPos = mPos; }
    /**
     * v1      _________
     *        /        |
     *       /         |
     * V0   /  |       |
     *     /           |
     *    |            |
     *    |    |       |
     *    |            |
     *    0    tc      Ts
     */
    double _calc_dist(double v0, double vd) {
        double dv = vd - v0;
        double tc = (dv > 0) ? (dv / param.A_m) : (dv / -param.A_n);
        if (tc <= Ts)
            //  (v0 + vd) * Ts/2;
            return (v0 + vd) * tc / 2 + vd * (Ts - tc);
        else {
            double Ts2 = ceil(tc / Ts) * Ts;
            return (v0 + vd) * tc / 2 + vd * (Ts2 - tc);
        }
    }
    inline double calc_sync_pos() const {
        return synSlavePos + (_mPos.pos - synMasterPos) * num / den;
    }
    int build() {
        double vn = _mPos.vel * num / den;
        double dv = vn - _curPos.vel;
        if (!_has_begin) {
            set_begin_pos(param.S_0, param.V_0, param.A_0);
            is_gearin = false;
            _has_begin = true;
            // _mPos.pos = 0;
            // _mPos.vel = 0;
            // _mPos.acc = 0;
        }
        if (param.A_m * Ts + 1e-6  > dv && dv > -param.A_n * Ts - 1e-6) {
            if (is_gearin) {
                _curPos.vel = vn;  // in gear level
                if (1) {
                    _curPos.pos = calc_sync_pos();
                    if (!is_pos_mode){
                        synMasterPos = _mPos.pos;
                        synSlavePos = _curPos.pos;
                    }
                } else {
                    _curPos.pos += _curPos.vel * Ts;
                }
            } else {  // in gear edge
                is_gearin = true;
                _curPos.pos += _calc_dist(_curPos.vel, vn);
                _curPos.vel = vn;
                _curPos.acc = sat(_mPos.acc * num / den, -param.A_n, param.A_m);
                if (1) {
                    synMasterPos = _mPos.pos;
                    synSlavePos = _curPos.pos;
                }
            }
        } else {  // not in gear
            _curPos.acc = (dv > 0) ? param.A_m : -param.A_n;
            _curPos.vel = _curPos.vel + _curPos.acc * Ts;
            _curPos.pos += (_curPos.vel - _curPos.acc * Ts / 2) * Ts;
            is_gearin = false;
        }
        return 0;
    }
    int build0() {
        double vn = _mPos.vel * num / den;
        double dv = vn - _curPos.vel;
        if (!_has_begin) {
            set_begin_pos(param.S_0, param.V_0, param.A_0);
            is_gearin = false;
            _has_begin = true;
            // _mPos.pos = 0;
            // _mPos.vel = 0;
            // _mPos.acc = 0;
        }
        if (param.A_m * Ts > dv && dv > -param.A_n * Ts) {
            if (is_gearin) {
                _curPos.vel = vn;  // in gear level
                if (1) {
                    _curPos.pos = calc_sync_pos();
                } else {
                    _curPos.pos += _curPos.vel * Ts;
                }
            } else {  // in gear edge
                is_gearin = true;
                _curPos.pos += _calc_dist(_curPos.vel, vn);
                _curPos.vel = vn;
                _curPos.acc = sat(_mPos.acc * num / den, -param.A_n, param.A_m);
                if (is_pos_mode) {
                    synMasterPos = _mPos.pos;
                    synSlavePos = _curPos.pos;
                }
            }
        } else {  // not in gear
            _curPos.acc = (dv > 0) ? param.A_m : -param.A_n;
            _curPos.vel = _curPos.vel + _curPos.acc * Ts;
            _curPos.pos += (_curPos.vel - _curPos.acc * Ts / 2) * Ts;
            is_gearin = false;
        }
        return 0;
    }
    int check() const { return 0; }
    int get_plan(double* total_time, double* rVmax, double* rAcc, double* rDec) const {
        return 0;
    };
    int get_slice(double time, double* pos, double* vel, double* acc) const {
        (void)time;
        if (pos) *pos = _curPos.pos;
        if (vel) *vel = _curPos.vel;
        if (acc) *acc = _curPos.acc;
        return !is_gearin;
    };
    int get_slice(double time, PosVelAcc& pva) const {
        (void)time;
        pva = _curPos;
        return !is_gearin;
    };
    int get_cur_slice(PosVelAcc& pva) const {
        pva = _curPos;
        return !is_gearin;
    };
    inline int get_nseg() const { return 0; }
    inline double get_distance(double) const { return _curPos.pos; }
    inline double get_rate(double) const { return _curPos.vel; }
    inline double get_acc(double) const { return _curPos.acc; }
    bool is_gearin = false;
    bool is_pos_mode = false;  // true for intergral, false for sum
    bool _has_begin = false;
    // unsigned int _idx = 0;
    int num = 1;
    unsigned int den = 1;
    double Ts = 0.01;
    double synMasterPos;
    double synSlavePos;
    // double synTime;
    PosInfo _curPos;
    PosInfo _mPos;
};

} /* namespace zmc */
#endif