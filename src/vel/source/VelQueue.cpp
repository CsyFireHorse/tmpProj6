#include "vel/include/VelQueue.h"
#include "misc/cpp14_backport.hpp"
#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace zmc {

VelQueue::VelQueue() 
    : curIdx(0)
    , curTime(0.0)
    , coverTime(0.0)
    , timeStep(0.01)
    , finished(false)
{
    pva.pos = 0.0;
    pva.vel = 0.0;
    pva.acc = 0.0;
}

VelQueue::~VelQueue() {
}

void VelQueue::add_velocity_plan(const TrapzVel& vel) {
    velArr.push_back(make_unique<TrapzVel>(vel));
}

void VelQueue::clear() {
    velArr.clear();
    curIdx = 0;
    curTime = 0.0;
    coverTime = 0.0;
    finished = false;
    pva.pos = 0.0;
    pva.vel = 0.0;
    pva.acc = 0.0;
}

bool VelQueue::hasNext() const {
    return curIdx < static_cast<int>(velArr.size()) - 1;
}

bool VelQueue::isEmpty() const {
    return velArr.empty();
}

int VelQueue::getCurrentIndex() const {
    return curIdx;
}

size_t VelQueue::size() const {
    return velArr.size();
}

void VelQueue::runCycle() {
    if (isEmpty() || finished) {
        return;
    }

    double dt = curTime - coverTime;
    
    if (dt < velArr[curIdx]->get_end_time()) {
        velArr[curIdx]->get_slice(dt, pva);
    } else if (hasNext()) {
        coverTime += velArr[curIdx]->get_end_time();
        curIdx++;
        dt = curTime - coverTime;
        
        if (dt < 0) {
            dt = 0;
        }
        
        velArr[curIdx]->get_slice(dt, pva);
    } else {
        velArr[curIdx]->get_slice(dt, pva);
        finished = true;
    }
    
    curTime += timeStep;
}

int VelQueue::get_cur_slice(PosVelAcc& pva) const {
    if (isEmpty()) {
        return -1;
    }
    
    pva = this->pva;
    return 0;
}

void VelQueue::reset() {
    curIdx = 0;
    curTime = 0.0;
    coverTime = 0.0;
    finished = false;
    pva.pos = 0.0;
    pva.vel = 0.0;
    pva.acc = 0.0;
}

double VelQueue::getCurrentTime() const {
    return curTime;
}

bool VelQueue::isFinished() const {
    return finished;
}

void VelQueue::setTimeStep(double dt) {
    timeStep = dt;
}

double VelQueue::getTimeStep() const {
    return timeStep;
}

void VelQueue::switchToNext() {
    if (hasNext()) {
        coverTime += velArr[curIdx]->get_end_time();
        curIdx++;
    }
}

TrapzVel& VelQueue::getVelocityPlan(size_t index) {
    if (index >= velArr.size()) {
        throw std::out_of_range("Index out of range");
    }
    return *velArr[index];
}

const TrapzVel& VelQueue::getVelocityPlan(size_t index) const {
    if (index >= velArr.size()) {
        throw std::out_of_range("Index out of range");
    }
    return *velArr[index];
}

TrapzVel& VelQueue::getCurrentVelocityPlan() {
    if (isEmpty()) {
        throw std::runtime_error("Queue is empty");
    }
    return *velArr[curIdx];
}

const TrapzVel& VelQueue::getCurrentVelocityPlan() const {
    if (isEmpty()) {
        throw std::runtime_error("Queue is empty");
    }
    return *velArr[curIdx];
}

TrapzVel& VelQueue::operator[](size_t index) {
    return getVelocityPlan(index);
}

const TrapzVel& VelQueue::operator[](size_t index) const {
    return getVelocityPlan(index);
}

}  // namespace zmc
