#include <gtest/gtest.h>
#include "AxisRunTmpl.hpp"
#include "fb_cpp_def.h"
#include "fbt_motion_traits.hpp"
#include "math.h"
#include "stdio.h"

using namespace zmc;

// Base template for safety limit tests
template<typename T>
struct AxSafetyTestTmpl : public AxSgMotTmpl<T, 1> {
    AxSafetyTestTmpl() {
        auto& cfg = this->get_config_ref(0);
        cfg.debug.servo_fak_pos = 0;
        cfg.cycle_time = 100000; // 0.1s
        cfg.debug.limit_flag.bDynamicLimitEnable = true; // Enable dynamic limits
        cfg.debug.limit_flag.bSWLimitDisable = false; // Enable SW limits
        
        // Set default safety limits using correct configuration structure
        cfg.dynamic_limits.velocity = 10.0;
        cfg.dynamic_limits.acceleration = 20.0; 
        cfg.dynamic_limits.deceleration = 20.0;
        cfg.dynamic_limits.jerk = 0.0;
        
        cfg.finite.software_limits.is_activate = true;
        cfg.finite.software_limits.positive = 100.0;
        cfg.finite.software_limits.negative = -100.0;
        
        cfg.software_error_reaction.deceleration = 15.0;
        cfg.software_error_reaction.max_distance = 5.0;
        
        this->reload_config(0);
        this->bindAxis(0);
        this->count_to_exit = 50;
    }
    int err_idx = 0;
};

// Test SW Position Limits - Positive direction
TEST(motNodeLimit, sw_limit_positive) {
    AxSafetyTestTmpl<FbtMoveAbsolute> axFb;
    
    // Move to position that exceeds positive SW limit
    axFb.mov.Position = 150.0; // Exceeds SW limit of 100.0
    set_fb_vel_config(axFb.mov, 9, 10, 10, 0);
    // axFb.verbose_flag = 1;
    axFb.count_to_exit = 150; 
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (ax.dwErrorID) {
            // Should trigger SW limit error    
            if (axFb.err_idx == 0) axFb.err_idx = i;
            EXPECT_EQ(ax.dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
            EXPECT_EQ(ax.axisStat, mcErrorStop);
            if (axFb.err_idx + 1 == i) {
                EXPECT_TRUE(axFb.mov.Error);
                EXPECT_NEAR(axFb.axRef(0).fSetAcceleration, -15, 1e-6);
                // EXPECT_EQ(axFb.mov.ErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
            }
        }
        EXPECT_EQ(axFb.check_set_pva(0), 0);
        EXPECT_LE(axFb.axRef(0).fActPosition, 100.0);
        EXPECT_LE(axFb.axRef(0).fSetPosition, 100.0);
    });
    EXPECT_TRUE(axFb.mov.Error);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcErrorStop);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
    EXPECT_NEAR(axFb.axRef(0).fActVelocity, 0.0, 1e-6);
}

// Test SW Position Limits - Negative direction  
TEST(motNodeLimit, sw_limit_negative) {
    AxSafetyTestTmpl<FbtMoveAbsolute> axFb;
    
    // Move to position that exceeds negative SW limit
    axFb.mov.Position = -150.0; // Exceeds SW limit of -100.0
    set_fb_vel_config(axFb.mov, 9, 10, 10, 0);
    // axFb.verbose_flag = 1;
    axFb.count_to_exit = 150;
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (ax.dwErrorID) {
            // Should trigger SW limit error
            if (axFb.err_idx == 0) axFb.err_idx = i;
                EXPECT_EQ(ax.dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
                EXPECT_EQ(ax.axisStat, mcErrorStop);
            if (axFb.err_idx + 1 == i) {
                EXPECT_TRUE(axFb.mov.Error);
                EXPECT_NEAR(axFb.axRef(0).fSetAcceleration, 15, 1e-6);
                
                // EXPECT_EQ(axFb.mov.ErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
            }
        }
        EXPECT_GE(axFb.axRef(0).fActPosition, -100.0);
        EXPECT_GE(axFb.axRef(0).fSetPosition, -100.0);
        EXPECT_EQ(axFb.check_set_pva(0), 0);
    });
    EXPECT_TRUE(axFb.mov.Error);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcErrorStop);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
    EXPECT_GE(axFb.axRef(0).fActPosition, -100.0);
    EXPECT_NEAR(axFb.axRef(0).fActVelocity, 0.0, 1e-6);
}


// Test SW Position Limits - Negative direction  
TEST(motNodeLimit, sw_limit_neg_rerun) {
    AxSafetyTestTmpl<FbtMoveAbsolute> axFb;
    
    // Move to position that exceeds negative SW limit
    axFb.mov.Position = -150.0; // Exceeds SW limit of -100.0
    set_fb_vel_config(axFb.mov, 9, 10, 10, 0);
    FbtReset fbRst;
    fbRst.Axis = axFb.axisRef(0);
    // axFb.verbose_flag = 1;
    axFb.count_to_exit = 180;
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (ax.dwErrorID) {
            // Should trigger SW limit error
            if (axFb.err_idx == 0) axFb.err_idx = i;
                EXPECT_EQ(ax.dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
                EXPECT_EQ(ax.axisStat, mcErrorStop);
            if (axFb.err_idx + 1 == i) {
                EXPECT_TRUE(axFb.mov.Error);
                EXPECT_NEAR(axFb.axRef(0).fSetAcceleration, 15, 1e-6);
                // EXPECT_EQ(axFb.mov.ErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
            }else if (axFb.err_idx + 7 == i) {
                fbRst.Execute = true;
                axFb.mov.Execute = false;
                axFb.mov.Position = -70.0;
                // EXPECT_EQ(axFb.mov.ErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
            }
        }else if(fbRst.Done){
            if (axFb.axRef(0).axisStat == mcStandstill)
                axFb.mov.Execute = true;
            axFb.powerOn(0);
        }
        fbRst.runCycle();
        int ret = axFb.check_set_pva(0);
        if (axFb.axRef(0).axisStatBak != mcDisabled)
            EXPECT_EQ(ret, 0);
    });
    EXPECT_TRUE(axFb.mov.Done);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcStandstill);
    EXPECT_NEAR(axFb.axRef(0).fActPosition, -70.0, 1e-6);
    EXPECT_NEAR(axFb.axRef(0).fActVelocity, 0.0, 1e-6);
}

// Test: Start position out of positive SW limit, move back into range
TEST(motNodeLimit, sw_limit_start_outside_move_in) {
    AxSafetyTestTmpl<FbtMoveRelative> axFb;
    auto& cfg = axFb.get_config_ref(0);
    // Set start position outside positive SW limit
    cfg.debug.servo_fak_pos = 120.0; // SW limit is 100.0
    axFb.reload_config(0);

    // Target position inside SW limit
    axFb.mov.Distance = -100.0;
    set_fb_vel_config(axFb.mov, 8, 10, 10, 0);
    axFb.count_to_exit = 180;

    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);

        if (i == 5) {
            axFb.mov.Execute = true;
        }
        // Should allow move back into range, not trigger SW limit error
        // EXPECT_LE(ax.fActPosition, cfg.finite.software_limits.positive + 1e-6);
        // EXPECT_GE(ax.fActPosition, cfg.finite.software_limits.negative - 1e-6);
        EXPECT_EQ(axFb.check_set_pva(0), 0);
        EXPECT_EQ(ax.dwErrorID, 0);
    });
    // Should finish normally, not error stop
    EXPECT_TRUE(axFb.mov.Done);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcStandstill);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, 0);
    EXPECT_NEAR(axFb.axRef(0).fActPosition, 20.0, 1e-6);
}


// Test Maximum Velocity Dynamic Limit
TEST(motNodeDynamic, max_velocity_exceeded) {
    AxSafetyTestTmpl<FbtMoveVelocity> axFb;
    // axFb.verbose_flag = 1;
    // Set velocity that exceeds SW max velocity limit (10.0)
    set_fb_vel_config(axFb.mov, 15.0, 10, 10, 0); // 15.0 > 10.0 limit
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (ax.dwErrorID) {
            // Should trigger max velocity error
            if (axFb.err_idx == 0) axFb.err_idx = i;
            EXPECT_EQ(ax.dwErrorID, ZMC_VD_MAX_VELOCITY_EXCEEDED);
            EXPECT_EQ(ax.axisStat, mcErrorStop);
            if (axFb.err_idx + 1 == i) {
                EXPECT_TRUE(axFb.mov.Error);
            }
        }
        EXPECT_EQ(axFb.check_set_pva(0), 0);
    });
    EXPECT_TRUE(axFb.mov.Error);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcErrorStop);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, ZMC_VD_MAX_VELOCITY_EXCEEDED);
}

// Test Maximum Acceleration Limit
TEST(motNodeDynamic, max_acceleration_exceeded) {
    AxSafetyTestTmpl<FbtMoveAbsolute> axFb;

    // Set acceleration that exceeds SW max acceleration limit (20.0)
    axFb.mov.Position = 50.0;
    set_fb_vel_config(axFb.mov, 8, 30, 10, 0); // 30 > 20.0 limit
    
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (ax.dwErrorID != 0) {
            // Should trigger max acceleration error
            EXPECT_EQ(ax.dwErrorID, ZMC_VD_MAX_ACCELERATION_EXCEEDED);
            EXPECT_EQ(ax.axisStat, mcErrorStop);
            if (axFb.err_idx == 0) axFb.err_idx = i;
            if (axFb.err_idx + 1 == i) {
                EXPECT_TRUE(axFb.mov.Error);
            }
        }
        EXPECT_EQ(axFb.check_set_pva(0), 0);
    });
    EXPECT_TRUE(axFb.mov.Error);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcErrorStop);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, ZMC_VD_MAX_ACCELERATION_EXCEEDED);
}

// Test Maximum Deceleration Limit
TEST(motNodeDynamic, max_deceleration_exceeded) {
    AxSafetyTestTmpl<FbtMoveAbsolute> axFb;
    // axFb.verbose_flag = 1;
    // Set deceleration that exceeds SW max deceleration limit (20.0)
    axFb.mov.Position = 30.0;
    set_fb_vel_config(axFb.mov, 8, 10, 30, 0); // 30 > 20.0 limit
    
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (ax.dwErrorID != 0) {
            // Should trigger max deceleration error during deceleration phase
            EXPECT_EQ(ax.dwErrorID, ZMC_VD_MAX_DECELERATION_EXCEEDED);
            EXPECT_EQ(ax.axisStat, mcErrorStop);
            if (axFb.err_idx == 0) axFb.err_idx = i;
            if (axFb.err_idx + 1 == i) {
                EXPECT_TRUE(axFb.mov.Error);
            }
        }
        EXPECT_EQ(axFb.check_set_pva(0), 0);
    });
    EXPECT_TRUE(axFb.mov.Error);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcErrorStop);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, ZMC_VD_MAX_DECELERATION_EXCEEDED);
}

// Test Invalid Velocity/Acceleration Values (3SH Error)
TEST(motNodeLimit, invalid_vel_acc_values) {
    AxSafetyTestTmpl<FbtMoveAbsolute> axFb;
    
    axFb.mov.Position = 50.0;
    
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto mstat = get_fb_impl_stat(axFb.mov);
        
        if (i == 5) {
            // Set invalid parameters (zero or negative values)
            set_fb_vel_config(axFb.mov, 0, 0, 0, 0);
            axFb.mov.Execute = true;
        } else if (i >= 8 && i <= 15) {
            // Should trigger invalid vel/acc/dec error
            if (mstat == FB_STAT_ERROR) {
                EXPECT_EQ(axFb.mov.ErrorID, ZMC_INVALID_VELACCJERK_VALUES);
            }
        }
    });
    EXPECT_TRUE(axFb.mov.Error);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcErrorStop);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, ZMC_INVALID_VELACCJERK_VALUES);
}

// TODO: fill this case after stopper by max distance implementation
TEST(motNodeLimit, sw_limit_max_distance) {
    // GTEST_SKIP() << "Test implementation pending";
    AxSafetyTestTmpl<FbtMoveAbsolute> axFb;
    auto& cfg = axFb.get_config_ref(0);
    cfg.software_error_reaction.max_distance = 1;
    axFb.reload_config(0);
    // Move to position that exceeds negative SW limit
    axFb.mov.Position = -150.0; // Exceeds SW limit of -100.0
    set_fb_vel_config(axFb.mov, 9, 10, 10, 0);
    // axFb.verbose_flag = 1;
    axFb.count_to_exit = 150;
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (ax.dwErrorID) {
            // Should trigger SW limit error
            if (axFb.err_idx == 0) axFb.err_idx = i;
                EXPECT_EQ(ax.dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
                EXPECT_EQ(ax.axisStat, mcErrorStop);
            if (axFb.err_idx + 1 == i) {
                EXPECT_TRUE(axFb.mov.Error);
                EXPECT_NEAR(axFb.axRef(0).fSetAcceleration, 9.0*9/2, 1e-6);
                // EXPECT_EQ(axFb.mov.ErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
            }
        }
        EXPECT_EQ(axFb.check_set_pva(0), 0);
    });
    EXPECT_TRUE(axFb.mov.Error);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcErrorStop);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
    EXPECT_GE(axFb.axRef(0).fActPosition, -100.0);
    EXPECT_NEAR(axFb.axRef(0).fActVelocity, 0.0, 1e-6);
}

// Test SW Limits with Moving Axis (Stop Distance Calculation)
TEST(motNodeLimit, sw_limit_stop_distance) {
    AxSafetyTestTmpl<FbtMoveVelocity> axFb;
    
    // Start moving towards positive limit
    set_fb_vel_config(axFb.mov, 8, 10, 15, 0);
    axFb.mov.Direction = mcPositiveDirection;
    
    // Set position close to SW limit to test stop distance calculation
    auto& cfg = axFb.get_config_ref(0);
    cfg.debug.servo_fak_pos = 95; // Close to positive limit of 100
    axFb.reload_config(0);
    
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (i >= 15) {
            // Should trigger SW limit error when stop distance calculation
            // determines axis cannot stop before limit
            if (ax.dwErrorID != 0) {
                EXPECT_EQ(ax.dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
                EXPECT_EQ(ax.axisStat, mcErrorStop);
            }
        }
        
        if (i > 5) {
            EXPECT_EQ(axFb.check_set_pva(0), 0);
        }
    });
    EXPECT_TRUE(axFb.mov.Error);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcErrorStop);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
}

// Test Disabled Safety Limits
TEST(motNodeLimit, disabled_safety_limits) {
    AxSafetyTestTmpl<FbtMoveAbsolute> axFb;
    
    // Disable all safety checks
    auto& cfg = axFb.get_config_ref(0);
    cfg.debug.limit_flag.bDynamicLimitEnable = false;
    cfg.debug.limit_flag.bSWLimitDisable = true;
    axFb.reload_config(0);
    
    // Use values that would normally trigger limits
    axFb.mov.Position = 150.0; // Would exceed SW limit if enabled
    set_fb_vel_config(axFb.mov, 15, 30, 30, 0); // Would exceed all limits if enabled
    
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (i >= 10) {
            // Should NOT trigger any limit errors since they're disabled
            EXPECT_EQ(ax.dwErrorID, 0);
            EXPECT_NE(ax.axisStat, mcErrorStop);
        }
        
        if (i > 5) {
            EXPECT_EQ(axFb.check_set_pva(0), 0);
        }
    });
}

// Test Multiple FB Safety Scenario
TEST(motNodeLimit, multiple_fb_safety) {
    AxDbMotTmpl<FbtMoveAbsolute, FbtMoveVelocity> axFb;
    
    auto& cfg = axFb.get_config_ref(0);
    cfg.debug.servo_fak_pos = 0;
    cfg.cycle_time = 100000;
    cfg.debug.limit_flag.bDynamicLimitEnable = true;
    cfg.dynamic_limits.velocity = 10.0;
    cfg.dynamic_limits.acceleration = 20.0;
    axFb.reload_config(0);
    axFb.bindAxis(0);
    axFb.count_to_exit = 40;
    
    // First FB: Normal move
    axFb.mov.Position = 30.0;
    set_fb_vel_config(axFb.mov, 5, 10, 10, 0);
    
    // Second FB: Exceeds velocity limit
    set_fb_vel_config(axFb.mov2, 15, 10, 10, 0); // 15 > 10 limit
    
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
        } else if (i == 15) {
            axFb.mov2.Execute = true; // This should trigger velocity limit error
        } else if (i >= 18 && i <= 25) {
            if (ax.dwErrorID != 0) {
                // EXPECT_EQ(ax.dwErrorID, ZMC_VD_MAX_VELOCITY_EXCEEDED);
                // EXPECT_TRUE(axFb.mov2.Error);
                // EXPECT_EQ(axFb.mov2.ErrorID, ZMC_VD_MAX_VELOCITY_EXCEEDED);
            }
        }
        
        EXPECT_EQ(axFb.check_set_pva(0), 0);
    });
    EXPECT_TRUE(axFb.mov2.Error);
    EXPECT_EQ(axFb.axRef(0).axisStat, mcErrorStop);
    EXPECT_EQ(axFb.axRef(0).dwErrorID, ZMC_VD_MAX_VELOCITY_EXCEEDED);
}

#if 1
// Test Axis Configuration Validation
TEST(motNodeLimit, axis_config_validation) {
    AxSafetyTestTmpl<FbtMoveAbsolute> axFb;
    
    // Test case 1: Invalid cycle time (too small)
    {
        auto& cfg = axFb.get_config_ref(0);
        uint32_t original_cycle_time = cfg.cycle_time;
        cfg.cycle_time = 0; // Invalid cycle time
        axFb.reload_config(0);
        
        auto& ax = axFb.axRef(0);
        // {
            // Should trigger configuration error
            EXPECT_EQ(ax.dwErrorID, ZMC_P_FTASKCYCLE_EMPTY);
            // printf("Config validation error (cycle_time=0): %d\n", ax.dwErrorID);
        // }
        
        // Restore valid cycle time
        cfg.cycle_time = original_cycle_time;
        axFb.reload_config(0);
    }
    
    // Test case 2: Invalid scaling parameters
    {
        auto& cfg = axFb.get_config_ref(0);
        uint32_t original_num = cfg.linear.increments_units_in_application_num;
        uint32_t original_den = cfg.linear.increments_units_in_application_den;
        
        cfg.linear.increments_units_in_application_num = 0; // Invalid numerator
        cfg.linear.increments_units_in_application_den = 0; // Invalid denominator
        axFb.reload_config(0);
        
        auto& ax = axFb.axRef(0);
        // if (ax.dwErrorID != 0) {
        // Should trigger configuration error
        EXPECT_EQ(ax.dwErrorID, ZMC_CGR_ZERO_VALUES);
        // printf("Config validation error (scaling=0/0): %d\n", ax.dwErrorID);
        // }
        
        // Restore valid scaling
        cfg.linear.increments_units_in_application_num = original_num;
        cfg.linear.increments_units_in_application_den = original_den;
        axFb.reload_config(0);
    }
    
    // Test case 3: Inconsistent software limits
    {
        auto& cfg = axFb.get_config_ref(0);
        double original_pos = cfg.finite.software_limits.positive;
        double original_neg = cfg.finite.software_limits.negative;
        
        cfg.finite.software_limits.positive = -50.0; // Positive limit < 0
        cfg.finite.software_limits.negative = 50.0;  // Negative limit > 0
        axFb.reload_config(0);
        
        auto& ax = axFb.axRef(0);
        if (ax.dwErrorID != 0) {
            // Should trigger configuration error for inconsistent limits
            EXPECT_EQ(ax.dwErrorID, ZMC_DI_SWLIMITS_EXCEEDED);
            printf("Config validation error (inconsistent limits): %d\n", ax.dwErrorID);
        }
        
        // Restore valid limits
        cfg.finite.software_limits.positive = original_pos;
        cfg.finite.software_limits.negative = original_neg;
        axFb.reload_config(0);
    }
    
    // Test case 4: Invalid dynamic limits (negative values)
    {
        auto& cfg = axFb.get_config_ref(0);
        double original_vel = cfg.dynamic_limits.velocity;
        double original_acc = cfg.dynamic_limits.acceleration;
        
        cfg.dynamic_limits.velocity = -10.0;     // Negative velocity limit
        cfg.dynamic_limits.acceleration = -20.0; // Negative acceleration limit
        axFb.reload_config(0);
        
        auto& ax = axFb.axRef(0);
        // if (ax.dwErrorID != 0) {
            // Should trigger configuration error for negative dynamic limits
            EXPECT_EQ(ax.dwErrorID, ZMC_SDL_INVALID_VELACC_VALUES);
            printf("Config validation error (negative dynamics): %d\n", ax.dwErrorID);
        // }
        
        // Restore valid dynamic limits
        cfg.dynamic_limits.velocity = original_vel;
        cfg.dynamic_limits.acceleration = original_acc;
        printf("original_vel: %f, original_acc: %f\n", original_vel, original_acc);
        printf("cfg.dynamic_limits.velocity: %f, cfg.dynamic_limits.acceleration: %f\n", cfg.dynamic_limits.velocity, cfg.dynamic_limits.acceleration);
        axFb.reload_config(0);
    }
    axFb.axRef(0).dwErrorID = 0;
    // Final test: Valid configuration should work without errors
    axFb.mov.Position = 20.0;
    set_fb_vel_config(axFb.mov, 5, 10, 10, 0);
    axFb.count_to_exit = 30;
    
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 5) {
            axFb.mov.Execute = true;
            // After valid config reload, should have no errors
            EXPECT_EQ(ax.dwErrorID, 0);
        } else if (i >= 10 && i <= 25) {
            // Normal operation should proceed without configuration errors
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        }
        EXPECT_EQ(axFb.check_set_pva(0), 0);
    });
}

#endif