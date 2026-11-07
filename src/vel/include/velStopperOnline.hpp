#ifndef _ZMC_VEL_STOPPER_ONLINE_HPP_
#define _ZMC_VEL_STOPPER_ONLINE_HPP_

namespace zmc {
// assert reachable in T_s
inline double getS_ByTc_t2x(double V_0, double V_d, double T_s, double A_m) {
    if (V_0 > V_d)
        return V_d * T_s + (V_0 - V_d) * (V_0 - V_d) / A_m / 2;  // T24
    else
        return V_d * T_s - (V_0 - V_d) * (V_0 - V_d) / A_m / 2;  // T21
}

class VelStopperOnline {
   public:
    int stat = E_VEL_NOT_READY;
    double A_m = 1e6;
    double A_n = 1e6;
    // double V_m;
    double V_d = 0;
    double Ts = 1e-3;
    double S_c;
    double V_c;
    double A_c;
    void set_config(double V_m, double A_m, double A_n, double J){
        this->A_m = A_m;
        this->A_n = A_n;
    }
    inline void set_pos_vel_acc(double sc, double vc, double ac){
        S_c = sc;
        V_c = vc;
        A_c = ac;
    }
    int build() {
        double vel = V_c;
        if (V_c == V_d) {
            S_c += V_c * Ts;
            stat = E_VEL_INXXX;
        } else if (V_c > V_d) {
            vel -= A_n * Ts;
            if (vel < V_d) {
                A_c = 0;
                S_c += getS_ByTc_t2x(V_c, V_d, Ts, A_n);
                V_c = V_d;
            } else {
                A_c = -A_n;
                S_c += V_c * Ts - A_n * Ts * Ts / 2;
                V_c += -A_n * Ts;
            }
            stat = E_VEL_UNCOVERED;
        } else {
            vel += A_m * Ts;
            if (vel > V_d) {
                S_c += getS_ByTc_t2x(V_c, V_d, Ts, A_m);
                A_c = 0;
                V_c = 0;
            } else {
                S_c += V_c * Ts + A_m * Ts * Ts / 2;
                A_c = A_m;
                V_c += A_m * Ts;
            }
            stat = E_VEL_UNCOVERED;
        }
        return 0;
    }
    int get_slice(double tm, double* pos, double* vel, double* acc) const {
        return get_cur_slice(pos, vel, acc);
    }
    inline int get_cur_slice(double* pos, double* vel, double* acc) const {
        *pos = S_c;
        *vel = V_c;
        *acc = A_c;
        return stat;
    }
};

inline double calc_stop_pos_online(double posCur, double posBak, double A_m, double Ts) {
    double velBak = (posCur - posBak) / Ts;
    double vel = velBak;
    if (vel == 0) {
        return posCur;
    } else if (vel > 0) {
        vel -= A_m * Ts;
        if (vel < 0) {
            vel = 0;
            return posCur + getS_ByTc_t2x(velBak, vel, Ts, A_m);
        } else {
            posCur += vel * Ts - A_m * Ts * Ts / 2;
            return posCur;
        }
    } else {
        vel += A_m * Ts;
        if (vel > 0) {
            vel = 0;
            return posCur + getS_ByTc_t2x(velBak, vel, Ts, A_m);
        } else {
            posCur += vel * Ts + A_m * Ts * Ts / 2;
            return posCur;
        }
    }
};
}  // namespace zmc
#endif