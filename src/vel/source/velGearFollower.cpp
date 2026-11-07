#include "math.h"
#include "stdint.h"
#include "VelGearFollower.h"
#include "TrapzVel.h"
#include "VelFollowerImpl.hpp"

namespace zmc {


struct VelGearFollowerImpl:public GearFollowerImplBase {
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
    double _calc_dist(double v0, double vd) const {
        double dv = vd - v0;
        double tc = (dv > 0) ? (dv / config.A_m) : (dv / -config.A_n);
        if (tc <= config.Ts) { // in gear edge
            //  (v0 + vd) * Ts/2;
            return (v0 + vd) * tc / 2 + vd * (config.Ts - tc);
        }else { // not in gear 
            double Ts2 = ceil(tc / config.Ts) * config.Ts;
            return (v0 + vd) * tc / 2 + vd * (Ts2 - tc);
        }
    }

    void runCycle(const PosVelAcc& mPos) {
        double vn = mPos.vel * config.num / config.den;
        double dv = vn - slavePva.vel;
        if (nGearStatus == GEAR_STATUS_IDLE) {
            nGearStatus = GEAR_STATUS_CLUTCH;
        }
        if (nGearStatus == GEAR_STATUS_GEAR_OUT) {
            slavePva.acc = 0;
            slavePva.pos += slavePva.vel * config.Ts;
            return;
        }
        const double& Ts = config.Ts;
        if (config.A_m * Ts + 1e-6 > dv && dv > -config.A_n * Ts - 1e-6) {
            // can follow master in this cycle
            if (nGearStatus == GEAR_STATUS_GEAR_IN) {
                // in gear level
                if (!is_vel_mode) {
                    slavePva.pos =
                        synSlavePos + (mPos.pos - synMasterPos) * config.num / config.den;
                    synMasterPos = mPos.pos;
                    synSlavePos = slavePva.pos;
                } else {
                    slavePva.pos += slavePva.vel * Ts;
                }
                slavePva.vel = vn;  // in gear level
                slavePva.acc = mPos.acc * config.num / config.den;
            } else {  // in gear edge
                nGearStatus = GEAR_STATUS_GEAR_IN;
                slavePva.pos += _calc_dist(slavePva.vel, vn);
                slavePva.vel = vn;
                slavePva.acc = sat(mPos.acc * config.num / config.den, -config.A_n, config.A_m);
                if (!is_vel_mode) {
                    synMasterPos = mPos.pos;
                    synSlavePos = slavePva.pos;
                }
            }
        } else { // cant follow master in this cycle
            slavePva.acc = (dv > 0) ? config.A_m : -config.A_n;
            slavePva.vel = slavePva.vel + slavePva.acc * Ts;
            slavePva.pos += (slavePva.vel - slavePva.acc * Ts / 2) * Ts;
            nGearStatus = GEAR_STATUS_CLUTCH;
        }
    }
    void shift_pos(double pos){
        slavePva.pos += pos;
        synSlavePos += pos;
    }
};

VelGearFollower::VelGearFollower() { pImpl = new VelGearFollowerImpl(); }
VelGearFollower::~VelGearFollower() { delete (VelGearFollowerImpl*)pImpl; }
void VelGearFollower::set_config(double vm, double acc, double dec, double jerk) {
    ((VelGearFollowerImpl*)pImpl)->set_config(vm, acc, dec, jerk);
}
void VelGearFollower::set_begin_pva(double s0, double v0, double a0) {
    ((VelGearFollowerImpl*)pImpl)->set_begin_pva(s0, v0, a0);
}
void VelGearFollower::set_num_den(int num, unsigned int den) {
    ((VelGearFollowerImpl*)pImpl)->set_num_den(num, den);
}
void VelGearFollower::set_Ts(double Ts) { ((VelGearFollowerImpl*)pImpl)->config.Ts = Ts; }

void VelGearFollower::follow(const PosVelAcc& mPos) { masterSet = mPos; }

void VelGearFollower::runCycle() {
    ((VelGearFollowerImpl*)pImpl)->runCycle(masterSet);
    // GearStatus = ((VelGearFollowerImpl*)pImpl)->nGearStatus;
    // slaveSet = ((VelGearFollowerImpl*)pImpl)->slavePva;
}
int VelGearFollower::get_cur_slice(PosVelAcc& pva) const {
    pva = ((VelGearFollowerImpl*)pImpl)->slavePva;
    return ((VelGearFollowerImpl*)pImpl)->nGearStatus;
};
void VelGearFollower::shift_pos(double pos){
    ((VelGearFollowerImpl*)pImpl)->shift_pos(pos);
}
}  // namespace zmc