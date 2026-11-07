#ifndef _ZMC_MOT_MISC_HPP_
#define _ZMC_MOT_MISC_HPP_

#include "fbt_impl_enum_def.h"
#include "fbt_errorcode_def.h"

namespace zmc {


inline bool isAxisStateUpdatable(MC_AXIS_STATES axis_state_, MC_AXIS_STATES set_state) {
    switch (axis_state_) {
        case mcHoming:
            switch (set_state) {
                case mcContinuousMotion:
                case mcSynchronizedMotion:
                case mcDiscreteMotion:
                    return false;
                default:
                    break;
            }
            break;
        case mcContinuousMotion:
        case mcSynchronizedMotion:
            if (set_state == mcStandstill || set_state == mcHoming) {
                return false;
            }
            break;
        case mcDiscreteMotion:
            if (set_state == mcHoming) {
                return false;
            }
            break;
        case mcStopping:
            switch (set_state) {
                case mcHoming:
                case mcContinuousMotion:
                case mcSynchronizedMotion:
                case mcDiscreteMotion:
                    return false;
                default:
                    break;
            }
            break;
        case mcErrorStop:
            switch (set_state) {
                case mcStopping:
                case mcHoming:
                case mcContinuousMotion:
                case mcSynchronizedMotion:
                case mcDiscreteMotion:
                    return false;
                default:
                    break;
            }
            break;
        case mcDisabled:
            switch (set_state) {
                case mcStopping:
                case mcHoming:
                case mcContinuousMotion:
                case mcSynchronizedMotion:
                case mcDiscreteMotion:
                    return false;
                    break;
                default:
                    break;
            }
            break;
        case mcStandstill:
            break;
        default:
            break;
    }
    return true;
}

#if 0
inline MC_ERROR_CODE axisStateUpdate(MC_AXIS_STATES& axis_state_, MC_AXIS_STATES set_state) {
    switch (axis_state_) {
        case mcHoming:
            switch (set_state) {
                case mcContinuousMotion:
                case mcSynchronizedMotion:
                case mcDiscreteMotion:
                    return ZMC_ERR_AXIS_STATE_VIOLATION;
                default:
                    break;
            }
            break;
        case mcContinuousMotion:
        case mcSynchronizedMotion:
            if (set_state == mcStandstill || set_state == mcHoming) {
                return ZMC_ERR_AXIS_STATE_VIOLATION;
            }
            break;
        case mcDiscreteMotion:
            if (set_state == mcHoming) {
                return ZMC_ERR_AXIS_STATE_VIOLATION;
            }
            break;
        case mcStopping:
            switch (set_state) {
                case mcHoming:
                case mcContinuousMotion:
                case mcSynchronizedMotion:
                case mcDiscreteMotion:
                    return ZMC_ERR_INVALIDSTATE_FROMSTOPPING;
                default:
                    break;
            }
            break;
        case mcErrorStop:
            switch (set_state) {
                case mcStopping:
                case mcHoming:
                case mcContinuousMotion:
                case mcSynchronizedMotion:
                case mcDiscreteMotion:
                    return ZMC_ERR_INVALIDSTATE_FROMERRORSTOP;
                default:

                    break;
            }
            break;
        case mcDisabled:
            switch (set_state) {
                case mcStopping:
                case mcHoming:
                case mcContinuousMotion:
                case mcSynchronizedMotion:
                case mcDiscreteMotion:
                    return ZMC_ERR_INVALIDSTATE_FROMDISABLED;
                    break;
                default:
                    break;
            }
            break;
        case mcStandstill:
            break;
        default:
            break;
    }
    axis_state_ = set_state;
    return ZMC_NO_ERROR;
}

inline
MC_ERROR_CODE changeAxisStates(MC_AXIS_STATES* current_state, MC_AXIS_STATES set_state) {
    switch (*current_state) {
        case mcDisabled:
        case mcErrorStop: {
            return ZMC_ERR_AXIS_STATE_VIOLATION;
        } break;
        case mcStandstill:
            switch (set_state) {
                case mcErrorStop:
                case mcStandstill:
                case mcHoming:
                case mcStopping:
                case mcDiscreteMotion:
                case mcSynchronizedMotion:
                case mcContinuousMotion: {
                    *current_state = set_state;
                    return ZMC_NO_ERROR;
                } break;
                default:
                    return ZMC_ERR_AXIS_STATE_VIOLATION;
                    break;
            }
            break;
        case mcHoming:
            switch (set_state) {
                case mcErrorStop:
                case mcHoming:
                case mcStopping:
                case mcStandstill: {
                    *current_state = set_state;
                    return ZMC_NO_ERROR;
                } break;
                default:
                    return ZMC_ERR_AXIS_STATE_VIOLATION;
                    break;
            }
            break;
        case mcStopping:
            switch (set_state) {
                case mcErrorStop:
                case mcStopping:
                case mcStandstill: {
                    *current_state = set_state;
                    return ZMC_NO_ERROR;
                } break;
                default:
                    return ZMC_ERR_AXIS_STATE_VIOLATION;
                    break;
            }
            break;
        case mcDiscreteMotion:
            switch (set_state) {
                case mcErrorStop:
                case mcStopping:
                case mcStandstill:
                case mcDiscreteMotion:
                case mcSynchronizedMotion:
                case mcContinuousMotion: {
                    *current_state = set_state;
                    return ZMC_NO_ERROR;
                } break;
                default:
                    return ZMC_ERR_AXIS_STATE_VIOLATION;
                    break;
            }
            break;
        case mcContinuousMotion:
            switch (set_state) {
                case mcErrorStop:
                case mcStopping:
                case mcContinuousMotion:
                case mcSynchronizedMotion:
                case mcDiscreteMotion: {
                    *current_state = set_state;
                    return ZMC_NO_ERROR;
                } break;
                default:
                    return ZMC_ERR_AXIS_STATE_VIOLATION;
                    break;
            }
            break;
        case mcSynchronizedMotion:
            switch (set_state) {
                case mcErrorStop:
                case mcStopping:
                case mcContinuousMotion:
                case mcDiscreteMotion:
                case mcSynchronizedMotion: {
                    *current_state = set_state;
                    return ZMC_NO_ERROR;
                } break;
                default:
                    return ZMC_ERR_AXIS_STATE_VIOLATION;
                    break;
            }
            break;
        default:
            break;
    }
    return ZMC_NO_ERROR;
}
#endif
}
#endif