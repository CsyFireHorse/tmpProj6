#ifndef _VEL_4_WRAP_H_
#define _VEL_4_WRAP_H_

#include "VelUnCppWrap.h"

namespace zmc {

class Vel4Wrap:public VelocityWrap{
public:
    void switchVelMode(int velMode);
};

// typedef Vel4Wrap ToyTrapzVelPlanner;

}

#endif