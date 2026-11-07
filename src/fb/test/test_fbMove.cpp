#include <gtest/gtest.h>
#include "axMoveTestTmpl.hpp"
// #include "AxisFbCompose.hpp"
#include "common/misc/AxisChecker.hpp"
#include "fbt_axis.h"
#include "fbt_axis_impl.h"
#include "fb_cpp_def.h"
#include "fbt_fb_base.hpp"
#include "math.h"
#include "stdio.h"
#include "velocity_utils.hpp"
#include "fbt_motion_traits.hpp"

using namespace zmc;

template <typename T>
class AxMoveTestCase : public PouTaskBase {
   public:
    AXIS_REF axis;
    FbtPower fbPow;
    T fbMov;
    VelOnlineChecker chk;
    double begPos = 1;
    double targetPos = 10;
    int cnt = 0;
    
    AxMoveTestCase() {
        const auto& config = AxMoveTestCaseConfig::get_config();
        begPos = config.get_double("axis_0.debug.servo_fak_pos", 1.0);
        targetPos = config.get_double("endPos", 10.0);
        double posOrDist = config.get_double("posOrDist", 10.0);
        setFbDistance(&fbMov, posOrDist);
        
        // Get cycle_time from axis_0.cycle_time or use default
        int cycle_time = config.get_int("axis_0.cycle_time", 50000);
        chk.Ts = cycle_time / 1e6;
    }
    
    void runCycle() override {
        if (!cnt) {
            fbPow.Axis = axis;
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
            
            fbMov.Axis = axis;
            fbMov.Velocity = 20;
            fbMov.Acceleration = 40;
            fbMov.Deceleration = 40;
        }

        if (cnt == 8) {
            fbMov.Execute = true;
        } else if (cnt == 60) {
            fbMov.Execute = false;
        }

        auto mstat = get_move_stat(fbMov);
        AxisImpl& ax = *((AxisImpl*)axis);

        if (cnt == 2) {
            EXPECT_NEAR(ax.fActPosition, begPos, 1e-6);
        } else if (cnt == 8) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
        } else if (cnt == 9) {
            EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        } else if (cnt == 60) {
            EXPECT_EQ(mstat, FB_STAT_DONE);
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_NEAR(ax.fSetPosition, targetPos, 1e-6);
            EXPECT_NEAR(ax.fActPosition, targetPos, 1e-6);
        } else if (cnt == 61) {
            EXPECT_EQ(mstat, FB_STAT_IDLE);
        }
        
        fbPow.runCycle();
        fbMov.runCycle();
        // axis->print(1);
        if (cnt >= 5) {
            int ret = chk.check(ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
            EXPECT_EQ(ret, 0);
        }
        
        cnt++;
    }
};

template<typename T, typename R>
void AxMoveTestTmpl(const ConfigDict& config){
    // Merge default configuration
    ConfigDict merged_config = config;
    ConfigDict default_config = {
        {"axis_0.cycle_time", "50000"},
        {"axis_0.is_axis_virtual", "true"},
        {"base.count_to_exit", "70"}
    };
    merged_config.merge(default_config);    
    // Call ax_move_test_tmpl_dict with test case type
    ax_move_test_tmpl_dict<AxMoveTestCase<T>, R>(merged_config);
}

TEST(moveabs, sg) {
    ConfigDict config = {
        {"axis_0.debug.servo_fak_pos", "1"},
        {"endPos", "10"},
        {"posOrDist", "10"}
    };
    AxMoveTestTmpl<FbtMoveAbsolute, AxisCia402Fak>(config);
}

TEST(moverel, sg) {
    ConfigDict config = {
        {"axis_0.debug.servo_fak_pos", "1"},
        {"endPos", "10"},
        {"posOrDist", "9"}
    };
    AxMoveTestTmpl<FbtMoveRelative, AxisCia402Fak>(config);
}

TEST(moverel, sg2) {
    ConfigDict config = {
        {"axis_0.debug.servo_fak_pos", "1"},
        {"endPos", "10"},
        {"posOrDist", "9"}
    };
    AxMoveTestTmpl<FbtMoveRelative, AxisCia402Virt>(config);
}

TEST(moveadd, sg) {
    ConfigDict config = {
        {"axis_0.debug.servo_fak_pos", "-1"},
        {"endPos", "-10"},
        {"posOrDist", "-9"}
    };
    AxMoveTestTmpl<FbtMoveRelative, AxisCia402Fak>(config);
}
