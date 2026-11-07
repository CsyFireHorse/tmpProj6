#ifndef _ZMC_MOTION_NODE_SUPER_H_
#define _ZMC_MOTION_NODE_SUPER_H_

#include "fbt_common.h"
#include "mc_enum_def_spec.h"
#include "fbt_impl_enum_def.h"
#include "fbt_fb_base.hpp"
#include "motion_node_list.h"

namespace zmc{

class MotionNodeSuper: public MotionNodeList {
   public:
    void clearFbByAborting();
    void clearFbByAxisStat(MC_AXIS_STATES);  
    void pushSuperimposedNode(FbtImplParam* pNode);
    void runCycle();
    FbtImplParam* popSuperimposedNode();
    FbtImplParam* pSuperimposedNode = nullptr;
};


}
#endif