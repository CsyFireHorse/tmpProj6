#include "VelPosFollower.h"
#include "common/misc/TrapzVelToyForwarder.hpp"
#include "stdio.h"
#include <utility> 
using namespace zmc;

template <typename T, typename T2>
class VelPosFollowerTmpl {
    public:
    T mot;
    T2 gear;
    int count_to_exit = 40;
    int idx = 0;
    double Ts = 0.1;
    void runCycle() {
        mot.runCycle();
        gear.runCycle();
    }
    int mRet;
    int sRet;
    PosVelAcc mPos;
    PosVelAcc sPva;

    template<typename F>
    void run(F&& f){
        printf("idx,axId,tm,st,pos,vel,acc\n");
        for (idx = 0; idx < count_to_exit; idx++) {
            double tm = idx * Ts;
            std::forward<F>(f)();   
            printf("%d,%d,%f,%d,%f,%f,%f\n", idx, 0, tm, mRet, mPos.pos, mPos.vel, mPos.acc);
            printf("%d,%d,%f,%d,%f,%f,%f\n", idx, 1, tm, sRet, sPva.pos, sPva.vel, sPva.acc);
        }
    }
};


// Test 1: Track move with velocity
void _VelPosFollower_v0_demo(double v0, double p0=0) {
    ToyTrapzVelPlanner vw;
    vw.set_config(5, 3, 3, 0);
    vw.set_begin_pva(p0, v0);
    vw.set_end_pva(20, 0);
    vw.build();

    const double Ts = 0.1;
    VelPosFollower vpf;
    vpf.set_config(6, 3, 3, 0);
    vpf.set_begin_pva(0, 0, 0);
    vpf.set_num_den(1, 1);
    vpf.set_Ts(Ts);

    PosVelAcc mPos;
    PosVelAcc pva;
    printf("idx,axId,tm,St,pos,vel,acc\n");
    for (int i = 0; i < 82; i++) {
        double tm = i * Ts;
        vw.get_slice(tm, mPos);
        vpf.follow(mPos);
        vpf.runCycle();
        int ret = vpf.get_cur_slice(pva);
        printf("%d,%d,%f,%d,%f,%f,%f\n", i, 0, tm, 0, mPos.pos, mPos.vel, mPos.acc);
        printf("%d,%d,%f,%d,%f,%f,%f\n", i, 1, tm, ret, pva.pos, pva.vel, pva.acc);
    }
}

// Test 2: Track move with zero initial velocity
void VelPosFollower_demo() {
    _VelPosFollower_v0_demo(0);
}
void VelPosFollower_nonzero_pos_demo() {
    _VelPosFollower_v0_demo(0, 2);
}
// Test 3: Track move with non-zero initial velocity
void VelPosFollower_nonzero_vel_demo() {
    _VelPosFollower_v0_demo(1, 2);
}

// Test 4: Track with low acceleration
void VelPosFollower_low_acc_demo() {
    VelPosFollowerTmpl<ToyTrapzVelPlanner, VelPosFollower> vpt;
    vpt.mot.set_config(5, 4, 3, 0);
    vpt.mot.set_begin_pva(10, 0);
    vpt.mot.set_end_pva(22, 0);
    vpt.mot.build();
    vpt.gear.set_config(2, 3, 3, 0);
    vpt.gear.set_begin_pva(0, 0, 0);
    vpt.gear.set_num_den(1, 1);
    vpt.gear.set_Ts(0.1);

    vpt.run([&](){
        vpt.mot.get_slice(vpt.idx * vpt.Ts, vpt.mPos);
        if (vpt.idx >= 2){
            vpt.gear.follow(vpt.mPos);
            vpt.gear.runCycle();
            vpt.sRet = vpt.gear.get_cur_slice(vpt.sPva);
        }        
    });
}

// Test 5: Track move with constant velocity
void VelPosFollower_vel_demo() {
    const double Ts = 0.1;
    VelPosFollower vpf;
    vpf.set_config(2, 3, 3, 0);
    vpf.set_begin_pva(0, 0, 0);
    vpf.set_num_den(1, 1);
    vpf.set_Ts(Ts);

    PosVelAcc mPos = {5, -4, 0};  // Starting with constant velocity
    printf("idx,tm,mpos,mvel,spos,svel,sacc\n");
    for (int i = 0; i < 18; i++) {
        double tm = i * Ts;
        PosVelAcc pva;
        mPos.pos += mPos.vel * Ts;  // Update master position with constant velocity
        vpf.follow(mPos);
        vpf.runCycle();
        int ret = vpf.get_cur_slice(pva);
        printf("%d,%f,%f,%f,%f,%f,%f\n", i, tm, mPos.pos, mPos.vel, pva.pos, pva.vel, pva.acc);
    }
}

// Test 6: Position and velocity tracking with gear ratio
void VelPosFollower_ratio_demo() {
    const double Ts = 0.1;
    VelPosFollower vpf;
    vpf.set_config(5, 4, 3, 0);
    vpf.set_begin_pva(0, 0, 0);
    vpf.set_num_den(2, 1);  // 2:1 gear ratio
    vpf.set_Ts(Ts);

    // Master moves in a trapezoid velocity profile
    ToyTrapzVelPlanner master;
    master.set_config(5, 3, 3, 0);
    master.set_begin_pva(0, 0);
    master.set_end_pva(30, 0);
    master.build();

    PosVelAcc mPos, pva;
    printf("idx,tm,mpos,mvel,spos,svel,sacc\n");
    for (int i = 0; i < 60; i++) {
        double tm = i * Ts;
        master.get_slice(tm, mPos);
        vpf.follow(mPos);
        vpf.runCycle();
        int ret = vpf.get_cur_slice(pva);
        printf("%d,%f,%f,%f,%f,%f,%f\n", i, tm, mPos.pos, mPos.vel, pva.pos, pva.vel, pva.acc);
    }
} 