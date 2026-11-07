#ifndef _VEL_FORWARD_TRAPZ_REV_H_
#define _VEL_FORWARD_TRAPZ_REV_H_

#include "vel_forward_trapz.h"

namespace zmc {

class FuncVelTrapzRev:public VelFunctorBase{
public:
    int operator()(const VelParam& param, VelForwarderTrapzUn& fw);
};


inline
int FuncVelTrapzRev::operator()(const VelParam& param, VelForwarderTrapzUn& fwUn){
    TrapzParams& par = * (TrapzParams*) &param;
    return velBuild<TrapzRelaxParam>(par, fwUn);
}

// support inverse distance
class VelTrapzRev:public VelTmplBase<FuncVelTrapzRev, VelForwarderTrapzUn>{
};


} /* namespace zmc */
#endif
