#ifndef _VEL_GEAR_FOLLOWER_H_
#define _VEL_GEAR_FOLLOWER_H_

#include "vel/VelWrapCommon.h"
#include "vel_enum_def.h"
#include "stdint.h"

namespace zmc {


struct VelGearFollowerStruct {
    void* pImpl;
    // input
    PosVelAcc masterSet;  // set position
};

class VelGearFollower : public VelGearFollowerStruct {
   public:
    VelGearFollower();
    ~VelGearFollower();
    void set_config(double vm, double acc, double dec, double jerk);
    void set_begin_pva(double s0, double v0, double a0);
    void set_num_den(int num, unsigned int den);
    void set_Ts(double Ts);
    void follow(const PosVelAcc& mPos);

    void runCycle();
    int get_cur_slice(PosVelAcc& pva) const;
    void shift_pos(double pos);
};

}  // namespace zmc
#endif