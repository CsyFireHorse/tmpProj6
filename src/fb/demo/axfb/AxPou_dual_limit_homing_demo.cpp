#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "pou/AxisManager.h"
#include "fb_cpp_def.h"
#include "common/misc/CmdArgsToy.hpp"
#include "fbt_axis_base.h"
#include "fbt_axis_opaque.h"
#include "mc_parameter_enum_def.h"
#include <iostream>
#include <cmath>

using namespace zmc;

enum DualLimitHomingState {
    STATE_IDLE,
    STATE_POWER_ON,
    STATE_HOMING_NEGATIVE,      // Search negative limit
    STATE_READ_NEG_OFFSET,      // Read offset at negative limit
    STATE_HOMING_POSITIVE,      // Search positive limit
    STATE_READ_POS_OFFSET,      // Read offset at positive limit
    STATE_CALCULATE_MIDPOINT,   // Calculate midpoint and distance
    STATE_SET_ZERO_OFFSET,      // Set zero point at midpoint
    STATE_MOVE_TO_ZERO,         // Move to zero position
    STATE_DONE,
    STATE_ERROR
};

class DualLimitHomingTask : public PouTaskBase {
public:
    DualLimitHomingTask() {
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        power.Axis = axis;
        homing.Axis = axis;
        readParam.Axis = axis;
        setPos.Axis = axis;
        moveAbs.Axis = axis;

        auto& args = CmdArgsToySingleton::get();
        
        // Configurable limit points (default: -100 and 100)
        limitNegative = args["limitNegative"].get<double>(-100.0);
        limitPositive = args["limitPositive"].get<double>(100.0);
        
        // Homing parameters
        homing.fVelocityFast = args["homing.fVelocityFast"].get<double>(100.0);
        homing.fVelocitySlow = args["homing.fVelocitySlow"].get<double>(20.0);
        homing.fAcceleration = args["homing.fAcceleration"].get<double>(500.0);
        homing.fDeceleration = args["homing.fDeceleration"].get<double>(500.0);
        homing.fJerk = args["homing.fJerk"].get<double>(0.0);
        
        moveAbs.Velocity = homing.fVelocityFast;
        moveAbs.Acceleration = homing.fAcceleration;
        moveAbs.Deceleration = homing.fDeceleration;
        moveAbs.Jerk = homing.fJerk;
        
        // Initialize state machine
        state = STATE_IDLE;
        offsetNegative = 0.0;
        offsetPositive = 0.0;
        distance = 0.0;
        midpoint = 0.0;
        
        // Simulate limit switches based on position
        limitSwitchWidth = args["limitSwitchWidth"].get<double>(0.5);
    }

    void runCycle() override {
        auto ax = (AxisImplBase*) axis;
        double pos = ax->diActPosition;
        
        // Simulate limit switches
        bool negLimitActive = pos >= (limitNegative - limitSwitchWidth);
        bool posLimitActive = pos <= (limitPositive + limitSwitchWidth);
        
        // Simulate reference switch (for homing)
        homing.bReferenceSwitch = negLimitActive || posLimitActive;
        
        // Power on
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();
        
        if (!power.Status) {
            state = STATE_POWER_ON;
            return;
        }
        
        // State machine
        switch (state) {
            case STATE_IDLE:
            case STATE_POWER_ON:
                if (power.Status) {
                    state = STATE_HOMING_NEGATIVE;
                    std::cout << "Starting negative limit search..." << std::endl;
                }
                break;
                
            case STATE_HOMING_NEGATIVE:
                // Configure homing to search negative direction
                homing.fHomePosition = 0;
                homing.nDirection = mcNegativeDirection;
                homing.nHomingMode = FAST_BSLOW_S_STOP;
                homing.bReferenceSwitch = negLimitActive;
                homing.bReturnToZero = false;
                homing.bExecute = true;
                homing.runCycle();
                
                if (homing.bDone) {
                    state = STATE_READ_NEG_OFFSET;
                    std::cout << "Negative limit found, reading offset..." << std::endl;
                } else if (homing.bError) {
                    state = STATE_ERROR;
                    std::cerr << "Error during negative limit search: " << homing.nErrorID << std::endl;
                }
                break;
                
            case STATE_READ_NEG_OFFSET:
                // Read actual position at negative limit using readParameter
                readParam.Enable = true;
                readParam.ParameterNumber = PARA_fOffsetPosition;
                readParam.runCycle();
                
                if (readParam.Valid) {
                    offsetNegative = readParam.Value;  // Store actual position at negative limit
                    std::cout << "Negative limit position: " << -offsetNegative << std::endl;
                    // Reset homing for next search
                    homing.bExecute = false;
                    homing.runCycle();
                    // Wait one cycle for homing to reset
                    state = STATE_HOMING_POSITIVE;
                    std::cout << "Starting positive limit search..." << std::endl;
                } else if (readParam.Error) {
                    state = STATE_ERROR;
                    std::cerr << "Error reading negative position: " << readParam.ErrorID << std::endl;
                }
                break;
                
            case STATE_HOMING_POSITIVE:
                // Configure homing to search positive direction
                homing.fHomePosition = 0;
                homing.nDirection = mcPositiveDirection;
                homing.nHomingMode = FAST_BSLOW_S_STOP;
                homing.bReferenceSwitch = posLimitActive;
                homing.bReturnToZero = false;
                homing.bExecute = true;
                homing.runCycle();
                
                if (homing.bDone) {
                    state = STATE_READ_POS_OFFSET;
                    std::cout << "Positive limit found, reading offset..." << std::endl;
                } else if (homing.bError) {
                    state = STATE_ERROR;
                    std::cerr << "Error during positive limit search: " << homing.nErrorID << std::endl;
                }
                break;
                
            case STATE_READ_POS_OFFSET:
                // Read actual position at positive limit using readParameter
                readParam.Enable = true;
                readParam.ParameterNumber = PARA_fOffsetPosition;
                readParam.runCycle();
                
                if (readParam.Valid) {
                    offsetPositive = readParam.Value;  // Store actual position at positive limit
                    std::cout << "Positive limit position: " << -offsetPositive << std::endl;
                    // Reset homing
                    homing.bExecute = false;
                    homing.runCycle();
                    state = STATE_CALCULATE_MIDPOINT;
                } else if (readParam.Error) {
                    state = STATE_ERROR;
                    std::cerr << "Error reading positive position: " << readParam.ErrorID << std::endl;
                }
                break;
                
            case STATE_CALCULATE_MIDPOINT:
                // Calculate distance and midpoint
                distance = std::abs(offsetPositive - offsetNegative);
                midpoint = distance / 2.0;
                
                std::cout << "Distance between limits: " << distance << std::endl;
                std::cout << "Midpoint: " << midpoint << std::endl;
                std::cout << "Configuring zero point at midpoint..." << std::endl;
                
                state = STATE_SET_ZERO_OFFSET;
                // break;
                
            case STATE_SET_ZERO_OFFSET:
                // Write offset parameter to set zero point at midpoint
                // Offset = -midpoint to shift coordinate system so midpoint becomes zero
                setPos.Execute = true;
                setPos.Position = midpoint;

                // writeParam.ParameterNumber = PARA_fOffsetPosition;
                // writeParam.Value = offsetPositive + midpoint;  // Negative to shift zero to midpoint
                setPos.runCycle();
                
                if (setPos.Done) {
                    std::cout << "Zero point configured at midpoint (offset = " << midpoint << ")" << std::endl;
                    // Reset write parameter
                    setPos.Execute = false;
                    setPos.runCycle();
                    state = STATE_MOVE_TO_ZERO;
                } else if (setPos.Error) {
                    state = STATE_ERROR;
                    std::cerr << "Error setting zero offset: " << setPos.ErrorID << std::endl;
                }
                break;
                
            case STATE_MOVE_TO_ZERO:
                // Move to zero position (midpoint)
                moveAbs.Execute = true;
                moveAbs.Position = 0.0;
                moveAbs.Direction = mcCurrentDirection;
                moveAbs.BufferMode = mcAborting;
                moveAbs.runCycle();
                
                if (moveAbs.Done) {
                    state = STATE_DONE;
                    std::cout << "Successfully moved to zero position (midpoint)" << std::endl;
                    std::cout << "Homing sequence completed!" << std::endl;
                    std::cout << "Final configuration:" << std::endl;
                    std::cout << "  Negative limit position: " << -offsetNegative << std::endl;
                    std::cout << "  Positive limit position: " << -offsetPositive << std::endl;
                    std::cout << "  Distance: " << distance << std::endl;
                    std::cout << "  Midpoint diPos, fPos " << pos << "," << ax->fSetPosition << std::endl;
                } else if (moveAbs.Error) {
                    state = STATE_ERROR;
                    std::cerr << "Error moving to zero: " << moveAbs.ErrorID << std::endl;
                }
                break;
                
            case STATE_DONE:
                // Sequence completed, keep axis at zero
                break;
                
            case STATE_ERROR:
                // Error state, stop all operations
                homing.bExecute = false;
                moveAbs.Execute = false;
                break;
        }
    }

    AXIS_REF axis;
    FbtPower power;
    FbtHoming homing;
    FbtReadParameter readParam;
    // FbtWriteParameter writeParam;
    FbtMoveAbsolute moveAbs;
    FbtSetPosition setPos;
    // Configurable parameters
    double limitNegative = -100.0;
    double limitPositive = 100.0;
    double limitSwitchWidth = 0.5;
    
    // State machine
    DualLimitHomingState state = STATE_IDLE;
    
    // Measured values
    double offsetNegative = 0.0;
    double offsetPositive = 0.0;
    double distance = 0.0;
    double midpoint = 0.0;
};

int pou_dual_limit_homing_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<DualLimitHomingTask>(argc, argv, iniContentDefault);
}

