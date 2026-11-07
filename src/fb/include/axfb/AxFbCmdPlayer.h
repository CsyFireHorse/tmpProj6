#ifndef _ZMC_FB_CMD_PLAYER_HPP_
#define _ZMC_FB_CMD_PLAYER_HPP_

#include "AxFbCmdManager.h"
#include <chrono>

namespace zmc {

class AxFbCmdPlayer:public AxFbCmdManager {
public:
    AxFbCmdPlayer();
    ~AxFbCmdPlayer();
    
    // Manual time control mode
    void step();                
    void steps(int num);        
    void tickto(int targetTick);
    void delay(double time);   
    void timeto(double targetTime);
    
    // Real-time simulation mode
    void set_real_time_num(int num);
    void set_real_time_den(int den);
    // void delay_by_real_time_off();  
    // void delay_by_real_time_on();  
    void set_delay_by_real_time(bool );  
    void delay_by_real_time();      
    
    // Time conversion utilities
    int realTimeToTick(double real_time)const;
    int simuTimeToTick(double simu_time)const;
    double realTimeToSimuTime(double real_time)const;

private:
    void preRunCycle();
    void updateSimTimeByRealTime();
    int _real_time_num = 1;      
    int _real_time_den = 1;      
    bool _delay_by_real_time = false;
    bool bLastTimeValid_ = false;
    std::chrono::time_point<std::chrono::steady_clock> _last_real_time; 
};

}  // namespace zmc

#endif  // _ZMC_FB_CMD_PLAYER_HPP_