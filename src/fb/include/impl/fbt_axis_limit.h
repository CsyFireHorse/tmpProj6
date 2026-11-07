#ifndef _ZMC_FBT_AXIS_LIMIT_H_
#define _ZMC_FBT_AXIS_LIMIT_H_

#include "fbt_common.h"

namespace zmc {

template<typename Base>
class AxLimitLayer : public Base {
public:
    bool bSWLimitEnable;
    bool bHWLimitEnable;
    bool bDynamicLimitEnable;
    bool bPositionLagActive = false;
    uint8_t eCheckPositionLag; // see Position_Lag_Supervision
    uint8_t usiSWEndSwitchState = false;

    LREAL fSWLimitPositive = 1e19;
    LREAL fSWLimitNegative = -1e19;
    LREAL fMaxPositionLag;
    LREAL fSWLimitDeceleration = 1000;
    LREAL fSWErrorMaxDistance = 0;

    LREAL fSWMaxVelocity = 1e9; 
    LREAL fSWMaxAcceleration = 1e9; 
    LREAL fSWMaxDeceleration = 1e9;
    LREAL fSWMaxJerk = 1e19;
    LREAL fSWMaxTorque;

    LREAL fMaxVelocity; 
    LREAL fMaxAcceleration; 
    LREAL fMaxDeceleration;
    LREAL fMaxTorque;
};

} // namespace zmc

#endif // _ZMC_FBT_AXIS_LIMIT_H_ 