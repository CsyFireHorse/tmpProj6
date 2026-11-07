/*
 * @ArtifactStatus draft
 * @ArtifactStatusDate 2026-08-31
 * @ArtifactStatusAuthority GPT-5 Codex
 */

#ifndef _ZMC_VEL_STOP_ONLINE_HPP_
#define _ZMC_VEL_STOP_ONLINE_HPP_

namespace zmc {

struct PosVelAcc {
    double pos;
    double vel;
    double acc;
};

inline PosVelAcc calc_next_decel_pva(const PosVelAcc& pva, double dec, double dt) {
    static const double kStopTol = 1e-6;
    if (pva.vel >= dec * dt) {
        const double v_new = pva.vel - dec * dt;
        if (v_new <= kStopTol)
            return PosVelAcc{pva.pos + pva.vel * pva.vel / 2 / dec, 0, 0};
        return PosVelAcc{pva.pos + pva.vel * dt - dec * dt * dt / 2, v_new, -dec};
    } else if (pva.vel <= -dec * dt) {
        const double v_new = pva.vel + dec * dt;
        if (v_new >= -kStopTol)
            return PosVelAcc{pva.pos + pva.vel * pva.vel / 2 / (-dec), 0, 0};
        return PosVelAcc{pva.pos + pva.vel * dt + dec * dt * dt / 2, v_new, dec};
    } else if (pva.vel < kStopTol && pva.vel > -kStopTol)
        return PosVelAcc{pva.pos, 0, 0};
    else {
        dec = pva.vel >= 0 ? dec : -dec;
        // Complete the constant-deceleration stop inside this cycle, then report
        // the stationary PVA at the cycle endpoint without extending the stop distance.
        return PosVelAcc{pva.pos + pva.vel * pva.vel / 2 / dec, 0, 0};
    }
}

}  // namespace zmc

#endif  // _ZMC_VEL_STOP_ONLINE_HPP_
