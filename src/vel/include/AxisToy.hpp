#ifndef _ZMC_AXIS_TOY_HPP_
#define _ZMC_AXIS_TOY_HPP_
#include <vector>

#include "stdio.h"
#include "stdint.h"
#include "vel/VelWrapCommon.h"
#include "common/misc/pou_base.hpp"

namespace zmc {

class AxisToy {
   public:
    int axStat = 0;
    double pos = 0;
    double vel = 0;
    double acc = 0;
    int idx = 0;
    int id = 0;
    double Ts = 0.1;
    void set_pos_vel_acc(double pos, double vel, double acc) {
        this->pos = pos;
        this->vel = vel;
        this->acc = acc;
    }
    double get_cur_time() const { return idx * Ts; }
    void runCycle() { idx++; }
    void print(int verboseFlag = 1) const {
        if (verboseFlag > 0) {
            printf("%d,%d,%f,%d,%f,%f,%f\n", idx, id, get_cur_time(), axStat, pos, vel, acc);
        }
    }
    void printHeader(int verboseFlag = 1) const {
        if (verboseFlag > 0) {
            printf("idx,axId,time,axStat,pos,vel,acc\n");
        }
    }
};

template <typename T>
struct ImplExt : public T {
    bool ExecuteBak_ = false;
    uint8_t stat_ = 0;
};

template <typename T>
class VelToyMoveTmpl {
   public:
    VelToyMoveTmpl() { pImpl = new ImplExt<T>(); };
    ~VelToyMoveTmpl() { delete pImpl; };
    ImplExt<T>* pImpl;
    AxisToy* Axis;
    bool Execute = false;
    double Position;
    double Velocity;
    double Acceleration;
    double Deceleration;
    double Jerk;
    double EndVelocity = 0;
    // output
    uint8_t State = 0;
    void set_config(double vm, double acc, double dec, double jerk) {
        Velocity = vm;
        Acceleration = acc;
        Deceleration = dec;
        Jerk = jerk;
    }
    // void set_begin_pva(double s0, double v0, double a0);
    void set_end_pva(double S_d, double EndVelocity) {
        Position = S_d;
        this->EndVelocity = EndVelocity;
    }
    void runCycle() {
        if (Execute && !pImpl->ExecuteBak_) {
            pImpl->set_config(Velocity, Acceleration, Deceleration, Jerk);
            pImpl->set_begin_pva(Axis->pos, Axis->vel, Axis->acc);
            pImpl->set_end_pva(Position, EndVelocity);
            if (pImpl->build()) {
                printf("build failed\n");
                pImpl->stat_ = 4;
                State = pImpl->stat_;
                return;
            }
            // double planTime;
            // planTime = vt.get_end_time();
            Axis->axStat = 3;
            pImpl->stat_ = 1;
        }
        if (pImpl->stat_ == 1) {
            PosVelAcc pva;
            int ret = pImpl->get_slice(Axis->get_cur_time(), pva);
            Axis->pos = pva.pos;
            Axis->vel = pva.vel;
            Axis->acc = pva.acc;
            if (ret == 2) {
                pImpl->stat_ = 2;
                Axis->axStat = 1;
            }
        } else if (pImpl->stat_ > 1) {
            if (!Execute && pImpl->ExecuteBak_) {
                pImpl->stat_ = 0;  // onClear
            }
        }
        pImpl->ExecuteBak_ = Execute;
        State = pImpl->stat_;
    }
};

template <typename T>
class VelToyGearTmpl {
   public:
    VelToyGearTmpl() { pImpl = new ImplExt<T>(); }
    ~VelToyGearTmpl() { delete pImpl; }
    ImplExt<T>* pImpl;
    AxisToy* Master;
    AxisToy* Slave;
    bool Execute = false;
    double Velocity;
    double Acceleration;
    double Deceleration;
    double Jerk;
    int Numerator = 1;
    int Denominator = 1;
    // output
    uint8_t State = 0;
    void set_num_den(int num, unsigned int den) {
        this->Numerator = num;
        this->Denominator = den;
    }
    void set_config(double vm, double acc, double dec, double jerk) {
        Velocity = vm;
        Acceleration = acc;
        Deceleration = dec;
        Jerk = jerk;
    }
    void runCycle() {
        if (Execute && !pImpl->ExecuteBak_) {
            if (pImpl->stat_ == 0) {
                pImpl->set_config(Velocity, Acceleration, Deceleration, Jerk);
                pImpl->set_begin_pva(Slave->pos, Slave->vel, Slave->acc);
                pImpl->set_num_den(Numerator, Denominator);
                pImpl->set_Ts(Slave->Ts);
                // if (pImpl->build()){
                //     printf("build failed\n");
                //     stat_ = 4;
                //     return;
                // }
                pImpl->stat_ = 1;
                Slave->axStat = 5;
            }
        }
        if (pImpl->stat_ == 1) {
            PosVelAcc mPos = {Master->pos, Master->vel, Master->acc};
            pImpl->follow(mPos);
            pImpl->runCycle();

            PosVelAcc pva;
            int ret = pImpl->get_cur_slice(pva);
            Slave->pos = pva.pos;
            Slave->vel = pva.vel;
            Slave->acc = pva.acc;
        } else if (pImpl->stat_ > 1) {
            if (!Execute && pImpl->ExecuteBak_) {
                pImpl->stat_ = 0;
                pImpl->ExecuteBak_ = false;
            }
        }
        pImpl->ExecuteBak_ = Execute;
        State = pImpl->stat_;
    }
};

class AxToyPou:public RunnerBase {
   public:
    AxToyPou(int axNum = 1) { 
        axisArr.resize(axNum);
        for (int i=0; i<axNum; i++)
        this->axisArr[i].id = i;
    }
    virtual ~AxToyPou() = default;
    void preRun() override { this->printHeader(); }
    virtual void postRunCycle() {
        axisRunCycle();
        this->print(); 
    }
    void runCycle() override {
        if (pTask) {
            pTask->runCycle();
        }
    }
    void axisRunCycle() {
        for (auto& ax : axisArr) {
            ax.runCycle();
        }
    }
    void print() const {
        for (auto& ax : axisArr) ax.print(verbose_flag);
    }
    void printHeader() const {
        for (auto& ax : axisArr) {
            ax.printHeader(verbose_flag);
            break;
        }
    }
    void bind(int i, PouTaskBase* pObj) {
        pTask = (pObj);
    }
   public:
    std::vector<AxisToy> axisArr;
    PouTaskBase* pTask = nullptr;
};




}  // namespace zmc
#endif