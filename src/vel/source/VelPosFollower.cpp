#include "math.h"
#include "stdint.h"
#include "VelPosFollower.h"
#include "VelFollowerImpl.hpp"
#include "TrapzVel.h"


namespace zmc {

enum GearType {
    GEAR_TYPE_POS = 1,
    GEAR_TYPE_VEL = 2,
    GEAR_TYPE_POS_VEL = 3,
    GEAR_TYPE_POS_VEL_ACC = 7,
};

struct PosVelFollowerImpl:public GearFollowerImplBase {
    TrapzVel trapzVel;  // velocity planner

    // Calculate max velocity and distance for acceleration-deceleration case
    void calcAccDecCase(const PosVelAcc& difPva, double& vm, double& dist) {
        const double& vd = difPva.vel;
        // vm = (Am*Ts + Am*vd/An)/(1 + Am/An)
        vm = (config.A_m * config.Ts + config.A_m * vd / config.A_n) / (1 + config.A_m / config.A_n);
        // s = vm^2/(2*Am) + (vm^2 - vd^2)/(2*An)
        dist = vm * vm / (2 * config.A_m) + (vm * vm - vd * vd) / (2 * config.A_n);
    }

    // Calculate min velocity and distance for deceleration-acceleration case
    void calcDecAccCase(const PosVelAcc& difPva, double& vn, double& dist) {
        const double& vd = difPva.vel;
        // vn = (-An*Ts + An*vd/Am)/(1 + An/Am)
        vn = (-config.A_n * config.Ts + config.A_n * vd / config.A_m) / (1 + config.A_n / config.A_m);
        // s = vn^2/(2*An) + (vd^2 - vn^2)/(2*Am)
        dist = - vn * vn / (2 * config.A_n) + (vd * vd - vn * vn) / (2 * config.A_m);
    }

    bool isReachSyncInCycle(const PosVelAcc& difPva) {
        double dv = difPva.vel;
        double tc = (dv > 0) ? (dv / config.A_m) : (dv / -config.A_n);
        if (tc > config.Ts) return false;
        // Calculate max distance (acceleration then deceleration)
        double vm, smax;
        
        calcAccDecCase(difPva, vm, smax);
        
        // Calculate min distance (deceleration then acceleration)
        double vn, smin;
        calcDecAccCase(difPva, vn, smin);
        
        // Target distance is reachable if it's between min and max
        return difPva.pos >= smin && difPva.pos <= smax;
    }
    void runCycle(double mPos) {
        double& posBak = synMasterPos;
        double& posBak2 = synSlavePos;
        if (nGearStatus == GEAR_STATUS_IDLE) {
            PosVelAcc mPva = {mPos, 0, 0};
            runCycle(mPva);
            posBak = mPos;
        } else {
            double velBak = (posBak - posBak2)/config.Ts;
            double vel = (mPos-posBak)/config.Ts;
            PosVelAcc mPva = {mPos, vel, (vel - velBak)/config.Ts};
            runCycle(mPva);
        }
        posBak2 = posBak;
        posBak = mPos;
    }
    void runCycle(double mPos, double mVel) {
        double& velBak = synMasterPos;
        if (nGearStatus == GEAR_STATUS_IDLE) {
            PosVelAcc mPva = {mPos, mVel, 0};
            runCycle(mPva);
            
        } else {
            PosVelAcc mPva = {mPos, mVel, (mVel - velBak)/config.Ts};
            runCycle(mPva);
        }
        velBak = mVel;
    }

    void runCycle(const PosVelAcc& mPos) {
        PosVelAcc difPva = {0, 0, 0};  // difference between slave and master
        // Calculate position and velocity differences
        difPva.pos = mPos.pos * config.num / config.den - slavePva.pos;
        difPva.vel = mPos.vel * config.num / config.den - slavePva.vel;
        difPva.acc = mPos.acc * config.num / config.den - slavePva.acc;

        // Check if we can reach sync state in this cycle
        bool canReachSync = false;
        if (nGearStatus == GEAR_STATUS_IDLE) {
            nGearStatus = GEAR_STATUS_CLUTCH;
        }
        if (nGearStatus == GEAR_STATUS_GEAR_IN) {
            // If already in sync, check if we can maintain sync by comparing acceleration
            canReachSync = fabs(difPva.acc) <= config.A_m;
            if (!canReachSync) {
                nGearStatus = GEAR_STATUS_CLUTCH;
            }
        } else {
            // Check if we can reach sync in this cycle using max/min distance method
            canReachSync = isReachSyncInCycle(difPva);
        }

        if (canReachSync) {
            // We can reach or maintain sync
            slavePva.pos = mPos.pos * config.num / config.den;
            slavePva.vel = mPos.vel * config.num / config.den;
            slavePva.acc = mPos.acc * config.num / config.den;
            nGearStatus = GEAR_STATUS_GEAR_IN;
        } else {
            // Need to plan trajectory to reach sync
            nGearStatus = GEAR_STATUS_CLUTCH;
            
            // Configure TrapzVel for planning
            trapzVel.set_inversable(true);
            trapzVel.set_config(config.V_m, config.A_m, config.A_n, config.J);

            trapzVel.set_begin_pva(slavePva.pos, slavePva.vel, slavePva.acc);
            trapzVel.set_end_pva(mPos.pos * config.num / config.den, mPos.vel * config.num / config.den); 
            if (trapzVel.build() == 0) {
                PosVelAcc plannedPva = trapzVel.get_slice(config.Ts);
                slavePva.pos = plannedPva.pos;
                slavePva.vel = plannedPva.vel;
                slavePva.acc = plannedPva.acc;
            }
            // trapzVel.set_begin_pos_vel(0, 0);  // Start from current difference
            // trapzVel.set_end_pos_vel(difPva.pos, difPva.vel);  // Target difference
            
            // if (trapzVel.build() == 0) {
            //     // Get planned motion for this cycle
            //     PosVelAcc plannedPva = trapzVel.get_slice(config.Ts);
                
            //     // Apply planned motion
            //     slavePva.pos += plannedPva.pos + (slavePva.vel + plannedPva.vel/2) * config.Ts;
            //     slavePva.vel += plannedPva.vel;
            //     slavePva.acc = plannedPva.acc;
            // }
        }
    }
    void shift_pos(double pos){
        if (uGearType == GEAR_TYPE_POS){
            slavePva.pos += pos;
            synSlavePos += pos;
        }
    }
};


VelPosFollower::VelPosFollower() { 
    pImpl = new PosVelFollowerImpl();
    ((PosVelFollowerImpl*)pImpl)->uGearType = GEAR_TYPE_POS;
}
VelPosFollower::~VelPosFollower() { delete (PosVelFollowerImpl*)pImpl; }
void VelPosFollower::set_config(double vm, double acc, double dec, double jerk) {
    ((PosVelFollowerImpl*)pImpl)->set_config(vm, acc, dec, jerk);
}
void VelPosFollower::set_begin_pva(double s0, double v0, double a0) {
    ((PosVelFollowerImpl*)pImpl)->set_begin_pva(s0, v0, a0);
}
void VelPosFollower::set_num_den(int num, unsigned int den) {
    ((PosVelFollowerImpl*)pImpl)->set_num_den(num, den);
}
void VelPosFollower::set_Ts(double Ts) { ((PosVelFollowerImpl*)pImpl)->config.Ts = Ts; }

void VelPosFollower::follow(double mPos) { masterPos = mPos; }

void VelPosFollower::follow(const PosVelAcc& mPva) {
    masterPos = mPva.pos;
}
void VelPosFollower::runCycle() {
    ((PosVelFollowerImpl*)pImpl)->runCycle(masterPos);
    // GearStatus = ((PosVelFollowerImpl*)pImpl)->nGearStatus;
    // slaveSet = ((PosVelFollowerImpl*)pImpl)->slavePva;
}
int VelPosFollower::get_cur_slice(PosVelAcc& pva) const {
    pva = ((PosVelFollowerImpl*)pImpl)->slavePva;
    return ((PosVelFollowerImpl*)pImpl)->nGearStatus;
};

void VelPosFollower::shift_pos(double pos){
    ((PosVelFollowerImpl*)pImpl)->shift_pos(pos);
}


VelPvFollower::VelPvFollower() { 
    pImpl = new PosVelFollowerImpl(); 
    ((PosVelFollowerImpl*)pImpl)->uGearType = GEAR_TYPE_POS_VEL;
}
VelPvFollower::~VelPvFollower() { delete (PosVelFollowerImpl*)pImpl; }
void VelPvFollower::set_config(double vm, double acc, double dec, double jerk) {
    ((PosVelFollowerImpl*)pImpl)->set_config(vm, acc, dec, jerk);
}
void VelPvFollower::set_begin_pva(double s0, double v0, double a0) {
    ((PosVelFollowerImpl*)pImpl)->set_begin_pva(s0, v0, a0);
}
void VelPvFollower::set_num_den(int num, unsigned int den) {
    ((PosVelFollowerImpl*)pImpl)->set_num_den(num, den);
}
void VelPvFollower::set_Ts(double Ts) { ((PosVelFollowerImpl*)pImpl)->config.Ts = Ts; }

void VelPvFollower::follow(const PosVelAcc& mPva) { masterPos = mPva.pos; masterVel = mPva.vel; }
void VelPvFollower::follow(double mPos, double mVel) { masterPos = mPos; masterVel = mVel; }

void VelPvFollower::runCycle() {
    ((PosVelFollowerImpl*)pImpl)->runCycle(masterPos, masterVel);
    // GearStatus = ((PosVelFollowerImpl*)pImpl)->nGearStatus;
    // slaveSet = ((PosVelFollowerImpl*)pImpl)->slavePva;
}
int VelPvFollower::get_cur_slice(PosVelAcc& pva) const {
    pva = ((PosVelFollowerImpl*)pImpl)->slavePva;
    return ((PosVelFollowerImpl*)pImpl)->nGearStatus;
};

void VelPvFollower::shift_pos(double pos){
    ((PosVelFollowerImpl*)pImpl)->shift_pos(pos);
}



VelPvaFollower::VelPvaFollower() { 
    pImpl = new PosVelFollowerImpl();
    ((PosVelFollowerImpl*)pImpl)->uGearType = GEAR_TYPE_POS_VEL_ACC;
}
VelPvaFollower::~VelPvaFollower() { delete (PosVelFollowerImpl*)pImpl; }
void VelPvaFollower::set_config(double vm, double acc, double dec, double jerk) {
    ((PosVelFollowerImpl*)pImpl)->set_config(vm, acc, dec, jerk);
}
void VelPvaFollower::set_begin_pva(double s0, double v0, double a0) {
    ((PosVelFollowerImpl*)pImpl)->set_begin_pva(s0, v0, a0);
}
void VelPvaFollower::set_num_den(int num, unsigned int den) {
    ((PosVelFollowerImpl*)pImpl)->set_num_den(num, den);
}
void VelPvaFollower::set_Ts(double Ts) { ((PosVelFollowerImpl*)pImpl)->config.Ts = Ts; }

void VelPvaFollower::follow(const PosVelAcc& mPos) { masterSet = mPos; }

void VelPvaFollower::runCycle() {
    ((PosVelFollowerImpl*)pImpl)->runCycle(masterSet);
    // GearStatus = ((PosVelFollowerImpl*)pImpl)->nGearStatus;
    // slaveSet = ((PosVelFollowerImpl*)pImpl)->slavePva;
}
int VelPvaFollower::get_cur_slice(PosVelAcc& pva) const {
    pva = ((PosVelFollowerImpl*)pImpl)->slavePva;
    return ((PosVelFollowerImpl*)pImpl)->nGearStatus;
};

void VelPvaFollower::shift_pos(double pos){
    ((PosVelFollowerImpl*)pImpl)->shift_pos(pos);
}



}  // namespace zmc