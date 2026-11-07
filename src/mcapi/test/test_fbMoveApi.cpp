#include <gtest/gtest.h>

#include "axis/AxisRunTmpl.hpp"
#include "common/misc/AxisChecker.hpp"
#include "fbt_axis.h"
#include "fbt_axis_impl.h"
#include "math.h"
#include "stdio.h"
#include "velocity_utils.hpp"
#include "mc_fb_api.h"
#include "McAxis.hpp"

using namespace zmc;

inline int get_move_stat(const McMoveAbsolute& fbMov){
    return fbMov.Active + fbMov.Busy + fbMov.Done * 3 + fbMov.Error * 4 + fbMov.CommandAborted * 5;
}
class AxMoveApiTestCase : public AxisRunTmpl<McAxis, 1> {
   public:
    double targetPos = 10;
    double begPos = 1;
    McPower fbPow;
    McMoveAbsolute fbMov;
    VelOnlineChecker chk;
    AxMoveApiTestCase() {
        auto& cfg = get_config_ref(0);
        cfg.debug.servo_fak_pos = 1;
        cfg.cycle_time = 50000;
        cfg.is_axis_virtual = true;
        reload_config(0);

        this->count_to_exit = 70;
        chk.Ts = cfg.cycle_time / 1e6;

        MC_Power_create(&fbPow);
        MC_MoveAbsolute_create(&fbMov);
    }
    ~AxMoveApiTestCase() {
        MC_Power_destroy(&fbPow);
        MC_MoveAbsolute_destroy(&fbMov);
    }
    void preRunCycle() override {
        auto mstat = get_move_stat(fbMov);
        auto& idx = this->idx;
        if (idx == 2) {
            fbPow.Axis = axisRef(0);
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
            EXPECT_NEAR(axRef(0).fActPosition, begPos, 1e-6);
        } else if (idx == 8) {
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
            fbMov.Axis = axisRef(0);
            fbMov.Velocity = 20;
            fbMov.Acceleration = 40;
            fbMov.Deceleration = 40;
            fbMov.Execute = true;
        } else if (idx == 9) {
            EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(axRef(0).axisStat, mcDiscreteMotion);
        } else if (idx == 60) {
            EXPECT_EQ(mstat, FB_STAT_DONE);
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
            fbMov.Execute = false;
            EXPECT_NEAR(axRef(0).fSetPosition, targetPos, 1e-6);
            EXPECT_NEAR(axRef(0).fActPosition, targetPos, 1e-6);
        } else if (idx == 61) {
            EXPECT_EQ(mstat, FB_STAT_IDLE);
        }
        MC_Power_exec(&fbPow);
        MC_MoveAbsolute_exec(&fbMov);
        if (idx >= 5) {
            int ret = check_set_pva(0);
            EXPECT_EQ(ret, 0);
        }
    }
    int check_set_pva(int idx){
        return chk.check(axRef(idx).fSetPosition, axRef(idx).fSetVelocity, axRef(idx).fSetAcceleration);
    }
};

TEST(mc_api, moveabs) {
    AxMoveApiTestCase axm;
    axm.fbMov.Position = axm.targetPos;
    // axm.verbose_flag = true;
    axm.run();
}
