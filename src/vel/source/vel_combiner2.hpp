#ifndef _ZMC_VEL_COMBINER2_H_
#define _ZMC_VEL_COMBINER2_H_

#include <deque>
#include <list>
#include <vector>

// #include "common/misc/zal_utils.h"

namespace zmc {
template <typename T>
class VelCombinerOnline {
   public:
    void set_config(double vm, double acc, double dec, double jerk) {
        V_m = vm;
        A_m = acc;
        A_n = dec;
        J = jerk;
    }
    inline int size() const { return _vels.size(); }
    // Copy and append to the end.
    inline void append(const T& v) { _vels.push_back(v); }
    inline T& back() { return _vels.back(); };
    T& operator[](int idx) { return _vels[idx]; };
    const T& operator[](int idx) const { return _vels[idx]; };
    inline const T& get_vel(int idx) const { return _vels[idx]; };
    inline T& get_vel(int idx) { return _vels[idx]; };
    inline int _build() {
        int errCode = 0;
        for (int i = 0; i < size(); i++) {
            errCode |= get_vel(i).build();
        }
        return errCode;
    }
    inline int build() { return _build(); }
    // int chop_left_time(int fd, double tm) { _vels[fd].chop_left_time(tm); }
    void timeToAuto() {
        _vels[_cur_idx].chop_left_time(_cur_reltime);
        _cur_reltime = 0;
    }
    void append_tail(double dist) {
        _vels.push_back(T());
        T& vel = _vels[_vels.size() - 1];
        vel.set_config(V_m, A_m, A_n, J);
        vel.set_begin_end_pos(0, dist);
    }

    int forward() {
        double Td = _vels[_cur_idx].get_end_time();
        if (_cur_reltime <= Td) {
            _vels[_cur_idx].get_slice(_cur_reltime, &_pva.pos, &_pva.vel, &_pva.acc);
            return E_VEL_UNCOVERED;
        } else if (_cur_reltime > Td && _cur_idx < size() - 1) {
            _vels[_cur_idx + 1].get_slice(_cur_reltime - Td, &_pva.pos, &_pva.vel, &_pva.acc);
            return E_VEL_SEG_COVERED;
        } else {
            _vels[_cur_idx].get_slice(_cur_reltime, &_pva.pos, &_pva.vel, &_pva.acc);
            return E_VEL_COVERED;
        }
    }
    void runCycle() {
        if (1) {
        } else {
            if (1) {
            }
        }
        _ret_stat = forward();
        if (_ret_stat == E_VEL_SEG_COVERED) {
            _cur_reltime -= _vels[_cur_idx].get_end_time();
            _vels.pop_front();
        }
        _cur_reltime += Ts;
        _cur_abstime += Ts;
    }
    int get_commands(PosVelAcc& pva) const {
        pva = _pva;
        return _ret_stat;
    }

   private:
    double V_m;
    double A_m;
    double A_n;
    double J;

    double Ts = 0.1;
    int _cur_idx = 0;
    double _cur_reltime = 0;
    double _cur_abstime = 0;
    std::deque<T> _vels;
    int _ret_stat = 0;
    PosVelAcc _pva;
};
}  // namespace zmc
#endif
