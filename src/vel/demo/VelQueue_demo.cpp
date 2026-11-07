#include "vel/include/VelQueue.h"
#include "vel/include/TrapzVel.h"
#include <iostream>
#include <iomanip>
#include <memory>

using namespace zmc;

void velQueueDemo() {
    VelQueue velQueue;
    velQueue.setTimeStep(0.005);
    
    TrapzVel vel1;
    vel1.set_config(1.0, 10.0, 10.0, 0.0);
    vel1.set_begin_pva(0.0, 0.0);
    vel1.set_end_pva(10.0, 3.0);
    vel1.build();
    
    TrapzVel vel2;
    vel2.set_config(3.0, 10.0, 10.0, 0.0);
    vel2.set_begin_pva(10.0, 3.0);
    vel2.set_end_pva(20.0, 0.0);
    vel2.build();
    
    
    velQueue.add_velocity_plan(vel1);
    velQueue.add_velocity_plan(vel2);
    // velQueue.add_velocity_plan(vel3);

    
    std::cout << "\nTime,Index,Position,Velocity,Acceleration" << std::endl;
    
    int stepCount = 0;
    int maxSteps = 2500;
    
    while (stepCount < maxSteps && !velQueue.isFinished()) {
        velQueue.runCycle();
        
        PosVelAcc pva;
        if (velQueue.get_cur_slice(pva) == 0) {
            std::cout << std::fixed << std::setprecision(6)
                      << velQueue.getCurrentTime() << ","
                      << velQueue.getCurrentIndex() << ","
                      << pva.pos << ","
                      << pva.vel << ","
                      << pva.acc << std::endl;
        }
        
        stepCount++;
    }

}
