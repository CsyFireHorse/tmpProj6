#ifndef _ZMC_SIMPLE_SERVO_SIM_HPP_
#define _ZMC_SIMPLE_SERVO_SIM_HPP_

#include <stdint.h>
#include "common/ecat/Cia402Easy.hpp"

namespace zmc {

// simple servo simulator parameters
#define SIMPLE_SERVO_MIN_CYCLE   2u
#define SIMPLE_SERVO_QS_DECEL    1000.0

struct SimpleServoIn {
    uint16_t ctrlWord;
    uint16_t statusWord;
    uint8_t opMode;
    int32_t diSetPosition;
    int32_t diSetVelocity;
    int32_t diSetTorque;
    double cycleTime;       // seconds
    uint32_t cycleCounter;
    bool isSimulation;
};

struct SimpleServoOut {
    uint16_t statusWord;
    uint8_t opModeDisplay;
    int32_t diActPosition;
    int32_t diActVelocity;
    int32_t diActTorque;
    bool driveError;
    bool driveStartRealState;
    bool regulatorRealState;
};

// lightweight CIA402-based servo simulation.
// stateless: all state is carried by input/output parameters.
inline void SimpleServoRunCycle(const SimpleServoIn& in, SimpleServoOut& out) {
    // guard: simulation disabled or not yet started
    if (!in.isSimulation) {
        return;
    }
    if (in.cycleCounter < SIMPLE_SERVO_MIN_CYCLE) {
        return;
    }

    // compute new statusword using CIA402 state machine
    Cia402Easy::axis_flags flg;
    flg.is_moved = (uint8_t)(in.diSetPosition != out.diActPosition);
    flg.is_moving = (uint8_t)(out.diActVelocity != 0);
    flg.is_home_mode = 0;
    flg.is_home_error = 0;

    uint16_t newStatus = Cia402Easy::get_new_stateEx(in.ctrlWord, in.statusWord, flg);
    Cia402Easy::axis_sm_t smNew = Cia402Easy::get_sm(newStatus);

    // update drive flags from state machine
    out.driveError = (smNew == Cia402Easy::axis_sm_fault_reaction_active) ||
                     (smNew == Cia402Easy::axis_sm_fault);
    out.regulatorRealState = (smNew == Cia402Easy::axis_sm_operation_enabled) ||
                             (smNew == Cia402Easy::axis_sm_quick_stop_active);
    out.driveStartRealState = (smNew == Cia402Easy::axis_sm_operation_enabled);

    // quick stop deceleration
    if (smNew == Cia402Easy::axis_sm_quick_stop_active && in.cycleTime > 0.0) {
        double v = static_cast<double>(out.diActVelocity);
        if (v != 0.0) {
            double sign = (v > 0.0) ? 1.0 : -1.0;
            double a = SIMPLE_SERVO_QS_DECEL;
            double vNext = v - sign * a * in.cycleTime;
            if ((sign > 0.0 && vNext < 0.0) || (sign < 0.0 && vNext > 0.0)) {
                vNext = 0.0;
            }
            double pos = static_cast<double>(out.diActPosition);
            double posNext = pos + (v + vNext) * 0.5 * in.cycleTime;
            out.diActVelocity = static_cast<int32_t>(vNext);
            out.diActPosition = static_cast<int32_t>(posNext);
        }
    } else if (smNew == Cia402Easy::axis_sm_operation_enabled) {
        // simple follow behavior in enabled state
        out.diActPosition = in.diSetPosition;
        out.diActVelocity = in.diSetVelocity;
        out.diActTorque = in.diSetTorque;
    }

    out.statusWord = newStatus;
    out.opModeDisplay = in.opMode;
}

} // namespace zmc

#endif

