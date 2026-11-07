#ifndef _FBT_AXIS_VIRTUAL_H_
#define _FBT_AXIS_VIRTUAL_H_

#include "AxisRef.h"
#include "fbt_axis_opaque.h"

namespace zmc{

class AxisVirtual:public AxisBase{  
public:
    AxisVirtual();
    void set_config(const AxisCiaConfig* cfg);
    void reload_config();
    void runCycle();
};

class AxisMotEmu:public AxisBase{  
public:
    AxisMotEmu();
    void set_config(const AxisCiaConfig* cfg);
    void reload_config();
    void runCycle();
    int check(double pos, double vel, double acc);
    int check_set_pva();
};

}
#endif