/*
 * @Author: ChenShengyong 
 * @Date: 2024-06-24 17:47:01 
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-20 09:32:25
 */
#ifndef ZS_VELOCITY_WRAP_COMMON_H_
#define ZS_VELOCITY_WRAP_COMMON_H_


struct PosVelAcc{
    double pos;
    double vel;
    double acc;
};

struct PosVelAccTime{
    double tm;
    double pos;
    double vel;
    double acc;
};
inline void getPva(const PosVelAccTime& pvat, PosVelAcc& pva){
    pva.pos = pvat.pos;
    pva.vel = pvat.vel;
    pva.acc = pvat.acc;
}

struct PosVelAccJerk{
    double pos;
    double vel;
    double acc;
    double jerk;
};

struct Xyva{
    double x;
    double y;
    double v;
    double a;
};

#endif  // ZS_VELOCITY_WRAP_COMMON_H_
/* end of file */