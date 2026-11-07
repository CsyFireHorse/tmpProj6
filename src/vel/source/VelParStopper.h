/*
 * @Author: ChenShengyong
 * @Date: 2024-07-03 17:17:24
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-07-03 17:44:09
 */
#ifndef _VEL_PAR_STOPPER_H_
#define _VEL_PAR_STOPPER_H_

#include "VelBase.h"
#include "math.h"
#include "zal_common.h"

namespace zmc {

struct VelParamsStopper : public VelParam {
   public:
    inline static double calc_stop_dist(double v0, double a0, double dec, double jerk) {
        if (jerk > 1e-6) {
            if (v0 > 0)
                return _calc_stop_distance_s(v0, a0, dec, jerk);
            else {
                return -_calc_stop_distance_s(-v0, -a0, dec, jerk);
            }
        } else {
            return calc_stop_dist(v0, dec);
        }
    }
    inline double static calc_stop_dist(double v0, double dec) {
        if (v0 > 1e-6) {
            return _calc_stop_distance_t(v0, dec);
        } else if (v0 < -1e-6) {
            return -_calc_stop_distance_t(v0, dec);
        } else {
            return 0;
        }
    }
    inline double static _calc_stop_distance_t(double v0, double dec) { return v0 * v0 / dec / 2; }
    double static _calc_stop_distance_s(double v0, double a0, double dec, double jerk) {
        VelParamsStopper vel;
        vel.V_0 = v0;
        vel.V_d = 0;
        vel.A_0 = a0;
        vel.A_m = dec;
        vel.A_n = dec;
        vel.J = jerk;

        if (vel.vd2v0_nseg22() <= v0)
            return vel.v2s_nseg32();
        else
            return vel.v2s_nseg22();
    }
    inline double vd2v0_nseg22() const { return (A_n * A_n - A_0 * A_0 / 2) / J; };
    inline double v2s_nseg32() const {
        if (A_0 == 0) return (V_d + V_0) * A_n / (2 * J) + (V_0 * V_0 - V_d * V_d) / (2 * A_n);
        double s = A_0 * (A_0 * A_0 / 3 / J + V_0) / J + (V_d + V_0) * A_n / (2 * J) +
                   (V_0 * V_0 - V_d * V_d) / (2 * A_n);
        return s;
    };
    double vd2an_nseg22(double vd) const { return sqrt(A_0 * A_0 / 2 + J * (V_0 - vd)); }
    inline double an2s_nseg22(double an) const {
        if (A_0 == 0)
            return (2 * J * V_0 - an * an) * an / J * J;
        else
            return (A_0 * A_0 * A_0 / 3 + A_0 * A_0 * an + J * V_0 * A_0 - an * an * an +
                    2 * J * V_0 * an) /
                   (J * J);
    }

    inline double v2s_nseg22() const {
        if (A_0 == 0) {
            double an = sqrt(J * (V_0 - V_d));
            // # assert self.vd2an_s22(0)  <= A_n
            return (V_0 + V_d) * an / J;
        } else {
            double an = vd2an_nseg22(V_d);
            return an2s_nseg22(an);
        }
    }
};
}  // namespace zmc
#endif