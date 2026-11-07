#ifndef _ZMC_LOOKAHEAD_IMPL_HPP_
#define _ZMC_LOOKAHEAD_IMPL_HPP_

#include "base/include/solver.h"
#include "math.h"

namespace zmc {

inline static double cal_reachable_trapz_vel(double S, double V_0, double A_m) {
    return sqrt(V_0 * V_0 + A_m * 2 * S);
}

inline static double cal_reachable_dist(double V_d, double V_0, double A_m, double J) {
    double acc = sqrt(J * (V_d - V_0));
    return 2 * A_m * V_0 / J + acc * acc * acc / J / J;
}
inline static double cal_reachable_vel_s21(double S, double V_0, double A_m, double J) {
    double acc = cube_equation_solver(2 * V_0 * J, -J * J * S);
    double V_s = V_0 + acc * acc / J;
    return V_s;
}
inline static double cal_reachable_vel_s31(double S, double V_0, double A_m, double J) {
    double V_s =
        quadtric_equation_solver2(1, A_m * A_m / J, V_0 * A_m * A_m / J - V_0 * V_0 - 2 * S * A_m);
    return V_s;
}
inline static double cal_reachable_scurve_vel(double S_s, double V_0, double A_m, double J) {
    double VM_exact = V_0 + A_m * A_m / J;
    if (cal_reachable_dist(VM_exact, V_0, A_m, J) >= S_s)
        return cal_reachable_vel_s21(S_s, V_0, A_m, J);
    else
        return cal_reachable_vel_s31(S_s, V_0, A_m, J);
}

class LookaheadVel {
   public:
    int _len;
    double* sArr;
    double* accArr;
    double* decArr;
    double* jerkArr;
    // sizeof(vArr) === sizeof(sArr) +1
    int relaxation(double* vArr) const {
        for (int i = 0; i < _len; i++) {
            double v = cal_reachable_pro_vel(i, vArr[i]);
            if (v < vArr[i + 1] - 1e-6) {
                if (i == _len - 1) return -1;
                vArr[i + 1] = v;
                // self._vvFlag[i+1] = LOOKAHEAD_NTYPE_LEFT
            } else if (v < vArr[i + 1] + 1e-6)
                ;  // self._vvFlag[i+1] |= LOOKAHEAD_NTYPE_LEFT
        }
        for (int i = _len - 1; i >= 0; i--) {
            double v = cal_reachable_pre_vel(i, vArr[i + 1]);
            if (v < vArr[i] - 1e-6) {
                if (i == 0) return -1;
                vArr[i] = v;
                // self._vvFlag[i] = LOOKAHEAD_NTYPE_RIGHT
            } else if (v < vArr[i] + 1e-6)
                ;  // self._vvFlag[i] |= LOOKAHEAD_NTYPE_RIGHT
        }
        return 0;
    }
    inline double cal_reachable_pre_vel(int idx, double V_d) const {
        if (jerkArr[idx] < 1e-6)
            return cal_reachable_trapz_vel(sArr[idx], V_d, decArr[idx]);
        else
            return cal_reachable_scurve_vel(sArr[idx], V_d, decArr[idx], jerkArr[idx]);
    }
    inline double cal_reachable_pro_vel(int idx, double V_0) const {
        if (jerkArr[idx] < 1e-6)
            return cal_reachable_trapz_vel(sArr[idx], V_0, accArr[idx]);
        else
            return cal_reachable_scurve_vel(sArr[idx], V_0, accArr[idx], jerkArr[idx]);
    }
};
template <typename T>
class LookaheadVelTmplObj {
   public:
    T dat;
    inline double cal_reachable_pre_vel(int idx, double V_d) const {
        return dat[idx].cal_reachable_pre_vel(V_d);
    }
    inline double cal_reachable_pro_vel(int idx, double V_0) const {
        return dat[idx].cal_reachable_pro_vel(V_0);
    }
};

template <typename T>
int lkhd_relax_tmpl(const T& vels, int len, double* vArr) {
    for (int i = 0; i < len; i++) {
        double v = vels.cal_reachable_pro_vel(i, vArr[i]);
        if (v < vArr[i + 1] - 1e-6) {
            if (i == len - 1) return -1;
            vArr[i + 1] = v;
        } else if (v < vArr[i + 1] + 1e-6)
            ;
    }
    for (int i = len - 1; i >= 0; i--) {
        double v = vels.cal_reachable_pre_vel(i, vArr[i + 1]);
        if (v < vArr[i] - 1e-6) {
            if (i == 0) return -1;
            vArr[i] = v;
        } else if (v < vArr[i] + 1e-6)
            ;
    }
    return 0;
}

template <typename T>
int relaxation2(const T*(&vels), int len, double* vArr) {
    return 0;  // lkhd_relax_tmpl(LookaheadVelTmplObj(vels), len, vArr);
}

}  // namespace zmc
#endif
