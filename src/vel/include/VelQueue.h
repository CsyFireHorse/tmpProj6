#ifndef _ZMC_VEL_QUEUE_H_
#define _ZMC_VEL_QUEUE_H_

#include "vel/VelWrapCommon.h"
#include "vel/include/TrapzVel.h"
#include <vector>
#include <memory>

namespace zmc {

class VelQueue {
public:
    VelQueue();
    ~VelQueue();

    void add_velocity_plan(const TrapzVel& vel);
    void clear();
    bool hasNext() const;
    bool isEmpty() const;
    int getCurrentIndex() const;
    size_t size() const;
    void runCycle();
    int get_cur_slice(PosVelAcc& pva) const;
    void reset();
    double getCurrentTime() const;
    bool isFinished() const;

    void setTimeStep(double dt);
    double getTimeStep() const;

    // Direct access to velocity plan array
    TrapzVel& getVelocityPlan(size_t index);
    const TrapzVel& getVelocityPlan(size_t index) const;
    TrapzVel& getCurrentVelocityPlan();
    const TrapzVel& getCurrentVelocityPlan() const;
    TrapzVel& operator[](size_t index);
    const TrapzVel& operator[](size_t index) const;

private:
    std::vector<std::unique_ptr<TrapzVel>> velArr;
    int curIdx;
    double curTime;
    double coverTime;
    double timeStep;
    PosVelAcc pva;
    bool finished;

    void switchToNext();
};

}  // namespace zmc

#endif  // _ZMC_VEL_QUEUE_H_
