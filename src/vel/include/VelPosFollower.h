#ifndef _VEL_POS_FOLLOWER_H_
#define _VEL_POS_FOLLOWER_H_

#include "vel/VelWrapCommon.h"
#include "vel_enum_def.h"

namespace zmc {

class VelPosFollower {
private:
    void* pImpl;  // Implementation pointer
public:
    double masterPos;  // Master axis setpoint

    VelPosFollower();
    ~VelPosFollower();

    // Configuration methods
    void set_config(double vm, double acc, double dec, double jerk);
    void set_begin_pva(double s0, double v0, double a0);
    void set_num_den(int num, unsigned int den);
    void set_Ts(double Ts);

    // Operation methods
    void follow(const PosVelAcc& mPva);
    void follow(double mPos);
    void runCycle();
    int get_cur_slice(PosVelAcc& pva) const;
    void shift_pos(double pos);
};


class VelPvFollower {
private:
    void* pImpl;  // Implementation pointer
public:
    double masterPos;
    double masterVel;

    VelPvFollower();
    ~VelPvFollower();

    // Configuration methods
    void set_config(double vm, double acc, double dec, double jerk);
    void set_begin_pva(double s0, double v0, double a0);
    void set_num_den(int num, unsigned int den);
    void set_Ts(double Ts);

    // Operation methods
    void follow(const PosVelAcc& mPva);
    void follow(double mPos, double mVel);
    void runCycle();
    int get_cur_slice(PosVelAcc& pva) const;
    void shift_pos(double pos);
};

    
class VelPvaFollower {
private:
    void* pImpl;  // Implementation pointer
public:
    PosVelAcc masterSet;  // Master axis setpoint

    VelPvaFollower();
    ~VelPvaFollower();

    // Configuration methods
    void set_config(double vm, double acc, double dec, double jerk);
    void set_begin_pva(double s0, double v0, double a0);
    void set_num_den(int num, unsigned int den);
    void set_Ts(double Ts);

    // Operation methods
    void follow(const PosVelAcc& mPva);
    void runCycle();
    int get_cur_slice(PosVelAcc& pva) const;
    void shift_pos(double pos);
};


} // namespace zmc

#endif // _VEL_POS_FOLLOWER_H_ 