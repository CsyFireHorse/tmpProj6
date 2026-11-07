#ifndef _FBT_AXIS_CIA402_H_
#define _FBT_AXIS_CIA402_H_

#include "pdo_struct.h"
#include "AxisRef.h"
#include "fbt_axis_opaque.h"
#include "AxisCiaConfig.hpp"

namespace zmc{

class AxisCia402:public AxisBase{  
public:
    AxisCia402();
    void set_config(const AxisCiaConfig* config);
    void reload_config();
    void set_input(const servo_stat* inp);
    void runCycle();
    void get_output(servo_ctrl* oup) const;
};

class AxisCia402Fak:public AxisCia402{
public:
    AxisCia402Fak();
    void set_config(const AxisCiaConfig* config);
    void reload_config();
    void runCycle();
};

class AxisCia402Virt:public AxisCia402{
public:
    AxisCia402Virt();
};


}

#endif