/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-20
@ArtifactStatusAuthority GPT-5 Codex
*/

#include "fb_cpp_def.h"
#include "AxisRunTmpl.hpp"
#include "axis/axis_coe_channel.h"
#include "common/ecat/Cia402Easy.hpp"
#include "common/ecat/ecat_pdo_key.h"
#include "vel/include/velocity_utils.hpp"
#include "fbt_motion_traits.hpp"
#include <gtest/gtest.h>

#include <vector>

using namespace zmc;

template <typename FB>
class TfbHomeTmpl : public AxisRunTmpl<> {
   public:
    inline int get_home_stat() const {
        return fbHm.Busy * 3 + fbHm.Done * 4 + fbHm.Error * 5 + fbHm.CommandAborted * 6;
    }
    // enum { SET_POSITION_OFS = OFS };
    TfbHomeTmpl(int axisNum = 1) {
        AxisCiaConfig& cfg = this->axArr[0].get_config_ref();
        cfg.pdo_flag.enable_modes_of_operation = true;
        cfg.debug.servo_fak_pos = -100;
        cfg.cycle_time = 10000;
        cfg.pdo_flag.enable_modes_of_operation_display = true;
        this->reload_config(0);

        fbPow.Enable = true;
        fbPow.bDriveStart = true;
        fbPow.bRegulatorOn = true;
        fbHm.Position = 40;

        fbPow.Axis = (AXIS_REF) & this->axArr[0];
        fbHm.Axis = fbPow.Axis;
        fb.Axis = fbPow.Axis;
        chk.Ts = this->axRef(0).cycleTime;
    }
    void preRunCycle() override {
        processCmd();
        fbPow.runCycle();
        fbHm.runCycle();
        fb.runCycle();
    }
    virtual void processCmd() {}

   public:
    FbtPower fbPow;
    FbtHome fbHm;
    FB fb;
    VelOnlineChecker chk;
    const int scale = 10;
    const int fakPos = -1000;
    const double Pos = 200;
};

class TfbHomeOnly : public TfbHomeTmpl<FbtReset> {
   public:
    void processCmd() override {
        fbPow.Enable = (true);
        if (idx == 6) {
            fbHm.Execute = true;
        } else if (idx == 7) {
            EXPECT_EQ(get_home_stat(), FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(axRef(0).axisStat, mcHoming);
            EXPECT_EQ(axRef(0).getAxisStateEx(), mcHomeEx);
        } else if (idx > 25) {
            EXPECT_EQ(get_home_stat(), FB_STAT_DONE);
            EXPECT_NEAR(axRef(0).fActPosition, 40, 1e-6);
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
        }
    }
};

TEST(home, base) {
    TfbHomeOnly asw;
    asw.count_to_exit = 40;
    asw.verbose_flag = 0;
    // EXPECT_EQ(asw._error_id, 0);
    asw.run();
}

class TfbHomeSdo : public AxisRunTmpl<> {
   public:
    TfbHomeSdo(bool enableSdo, bool skipPosition, bool failOffset) : fail_offset_(failOffset) {
        AxisCiaConfig& cfg = axArr[0].get_config_ref();
        cfg.pdo_flag.enable_modes_of_operation = true;
        cfg.pdo_flag.enable_modes_of_operation_display = true;
        cfg.debug.servo_fak_pos = -100;
        cfg.cycle_time = 10000;
        reload_config(0);

        power.Axis = axisRef(0);
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;

        home.Axis = power.Axis;
        home.Position = 40;
        home.setSkipPosition(skipPosition);
        if (enableSdo) home.configureSdo(-5, 15, 200, 20, 1000);
    }

    void preRunCycle() override {
        if (idx == 6) home.Execute = true;
        power.runCycle();
        home.runCycle();
        completeSdoRequest();
    }

    void postRunCycle() override {
        if (axRef(0).cmd == AXIS_CMD_HOME && requests_.size() < 5) {
            home_started_before_sdo_done_ = true;
        }
        if (axRef(0).cmd == AXIS_CMD_HOME) home_started_ = true;
        if (hold_homing_pending_) axRef(0).swSmEx = 0;
    }

    void completeSdoRequest() {
        if (!complete_sdo_) return;
        AxisCoeRequest request{};
        if (axis_coe_req_head(home.Axis, &request) != 0) return;

        requests_.push_back(request);
        const int32_t code = fail_offset_ && requests_.size() == 2 ? -7 : 0;
        axis_coe_result_push(home.Axis, request.tag, code, request.index, request.subindex,
                             request.len, static_cast<uint32_t>(request.value));
        axis_coe_req_pop(home.Axis);
    }

    FbtPower power;
    FbtHome home;
    std::vector<AxisCoeRequest> requests_;
    bool home_started_before_sdo_done_ = false;
    bool home_started_ = false;
    bool complete_sdo_ = true;
    bool hold_homing_pending_ = false;

   private:
    bool fail_offset_;
};

void expect_home_sdo_request(const AxisCoeRequest& request, uint16_t index, uint8_t subindex,
                             uint8_t len, int32_t value) {
    EXPECT_EQ(AXIS_COE_REQUEST_DOWNLOAD, request.direction);
    EXPECT_EQ(index, request.index);
    EXPECT_EQ(subindex, request.subindex);
    EXPECT_EQ(len, request.len);
    EXPECT_EQ(value, request.value);
}

TEST(home, sdo_config_starts_home_after_all_writes) {
    TfbHomeSdo asw(true, false, false);
    asw.count_to_exit = 80;
    asw.run();

    ASSERT_EQ(5u, asw.requests_.size());
    expect_home_sdo_request(asw.requests_[0], PDO_PO_HOMING_METHOD, 0, 1, -5);
    expect_home_sdo_request(asw.requests_[1], PDO_PO_HOMING_OFFSET, 0, 4, 15);
    expect_home_sdo_request(asw.requests_[2], PDO_PO_HOMING_SPEED, 1, 4, 200);
    expect_home_sdo_request(asw.requests_[3], PDO_PO_HOMING_SPEED, 2, 4, 20);
    expect_home_sdo_request(asw.requests_[4], PDO_PO_HOMING_ACCEL, 0, 4, 1000);
    EXPECT_FALSE(asw.home_started_before_sdo_done_);
    EXPECT_TRUE(asw.home.Done);
    EXPECT_FALSE(asw.home.Error);
    EXPECT_NEAR(asw.axRef(0).fActPosition, 40, 1e-6);
}

TEST(home, sdo_config_can_skip_controller_position_shift) {
    TfbHomeSdo asw(true, true, false);
    asw.count_to_exit = 80;
    asw.run();

    EXPECT_TRUE(asw.home.Done);
    EXPECT_FALSE(asw.home.Error);
    EXPECT_GT(fabs(asw.axRef(0).fActPosition - asw.home.Position), 1e-6);
}

TEST(home, can_skip_controller_position_shift_without_sdo) {
    TfbHomeSdo asw(false, true, false);
    asw.count_to_exit = 80;
    asw.run();

    EXPECT_TRUE(asw.requests_.empty());
    EXPECT_TRUE(asw.home.Done);
    EXPECT_FALSE(asw.home.Error);
    EXPECT_GT(fabs(asw.axRef(0).fActPosition - asw.home.Position), 1e-6);
}

TEST(home, sdo_config_error_prevents_home_start) {
    TfbHomeSdo asw(true, false, true);
    asw.count_to_exit = 40;
    asw.run();

    EXPECT_EQ(2u, asw.requests_.size());
    EXPECT_TRUE(asw.home.Error);
    EXPECT_EQ(ZMC_ERR_WP_SDO_ABORTED, asw.home.ErrorID);
    EXPECT_FALSE(asw.home_started_before_sdo_done_);
    EXPECT_EQ(asw.axRef(0).axisStat, mcStandstill);
    AxisCoeRequest request{};
    EXPECT_NE(0, axis_coe_req_head(asw.home.Axis, &request));
}

TEST(home, sdo_config_timeout_uses_timeout_ms) {
    TfbHomeSdo asw(true, false, false);
    asw.home.setTimeoutMs(20);
    asw.complete_sdo_ = false;
    asw.count_to_exit = 40;
    asw.run();

    EXPECT_TRUE(asw.home.Error);
    EXPECT_EQ(ZMC_ERR_WP_SDO_TIMEOUT, asw.home.ErrorID);
    EXPECT_FALSE(asw.home_started_);
}

TEST(home, homing_timeout_starts_after_home_command) {
    TfbHomeSdo asw(false, false, false);
    asw.home.setTimeoutMs(20);
    asw.hold_homing_pending_ = true;
    asw.count_to_exit = 40;
    asw.run();

    EXPECT_TRUE(asw.home_started_);
    EXPECT_TRUE(asw.home.Error);
    EXPECT_EQ(ZMC_ERR_H_TIMEOUT, asw.home.ErrorID);
    EXPECT_EQ(asw.axRef(0).axisStat, mcStandstill);
}

class TfbSetPosHome : public TfbHomeTmpl<FbtSetPosition> {
   public:
    enum { OFS = -300 };
    void processCmd() override {
        fbPow.Enable = (true);
        // auto axRef = fbPow.getAxis();
        if (idx == 5) {
            fb.Execute = true;
            fb.Position = (OFS);
        } else if (idx == 6) {
            EXPECT_NEAR(axRef(0).fSetPosition, OFS, 1e-6);
        } else if (idx == 8) {
            fbHm.Execute = (true);
        } else if (idx == 9) {
            EXPECT_EQ(get_home_stat(), FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(axRef(0).axisStat, mcHoming);
        } else if (idx > 28) {
            EXPECT_EQ(get_home_stat(), FB_STAT_DONE);
            EXPECT_NEAR(axRef(0).fSetPosition, 40, 1e-6);
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
        }
    }
};
TEST(home, set_pos) {
    TfbSetPosHome asw;
    asw.count_to_exit = 40;
    // EXPECT_EQ(asw._error_id, 0);
    asw.run();
}

class TfbHomeStop : public TfbHomeTmpl<FbtStop> {
   public:
    TfbHomeStop() {
        fb.Deceleration = 100000;
        fb.Jerk = 0;
    }

    void processCmd() override {
        fbPow.Enable = (true);
        if (idx == 8) {
            fbHm.Execute = true;
        } else if (idx == 9) {
            EXPECT_EQ(get_home_stat(), FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(axRef(0).axisStat, mcHoming);
        } else if (idx == 11) {
            fb.Execute = true;
            stopStarted = true;
        }
        if (stopStarted && fb.Done && !stopReleased) {
            fb.Execute = false;
            stopReleased = true;
        }
    }

    void postRunCycle() override {
        if (!stopStarted) return;
        auto& ax = axRef(0);

        EXPECT_FALSE(Cia402Easy::is_ctrlw_halt(ax.wControlWord));
        if (fbHm.CommandAborted) {
            sawHomeAborted = true;
            EXPECT_FALSE(fbHm.Error);
            EXPECT_EQ(fbHm.ErrorID, ZMC_NO_ERROR);
        }

        if (ax.cmd == AXIS_CMD_POWEROFF) {
            if (sawMotion) powerOffAfterMotion = true;
            EXPECT_FALSE(sawPowerOn);
            sawPowerOff = true;
        } else if (ax.cmd == AXIS_CMD_POWERON) {
            EXPECT_TRUE(sawPowerOff);
            sawPowerOn = true;
            if (ax.regulatorRealState && ax.driveStartRealState &&
                ax.byRealControllerMode == E_OP_MODE_TO_CTRL_MODE - OP_MODE_CSP) {
                readyPowerOnCycles++;
                lastReadyPosition = ax.fActPosition;
            }
        } else if (ax.cmd == AXIS_CMD_MOTION && !sawMotion) {
            EXPECT_TRUE(sawPowerOff);
            EXPECT_TRUE(sawPowerOn);
            EXPECT_GT(readyPowerOnCycles, 0u);
            EXPECT_TRUE(ax.regulatorRealState);
            EXPECT_TRUE(ax.driveStartRealState);
            EXPECT_EQ(ax.byRealControllerMode, E_OP_MODE_TO_CTRL_MODE - OP_MODE_CSP);
            EXPECT_NEAR(ax.fMarkPosition, lastReadyPosition, 1e-6);
            sawMotion = true;
        }

        if (fb.Done) {
            sawDone = true;
            EXPECT_EQ(ax.axisStat, mcStopping);
        }
    }

    bool stopStarted = false;
    bool stopReleased = false;
    bool sawHomeAborted = false;
    bool sawPowerOff = false;
    bool sawPowerOn = false;
    bool sawMotion = false;
    bool sawDone = false;
    bool powerOffAfterMotion = false;
    uint32_t readyPowerOnCycles = 0;
    double lastReadyPosition = 0.0;
};

TEST(home, stop) {
    TfbHomeStop asw;
    asw.count_to_exit = 1100;
    asw.verbose_flag = 0;
    asw.run();
    EXPECT_TRUE(asw.sawHomeAborted);
    EXPECT_TRUE(asw.sawPowerOff);
    EXPECT_TRUE(asw.sawPowerOn);
    EXPECT_TRUE(asw.sawMotion);
    EXPECT_TRUE(asw.sawDone);
    EXPECT_FALSE(asw.powerOffAfterMotion);
    EXPECT_TRUE(asw.stopReleased);
    EXPECT_EQ(asw.axRef(0).axisStat, mcStandstill);
}

class TfbHomeStopTimeout : public TfbHomeTmpl<FbtStop> {
   public:
    enum FeedbackBlock { BLOCK_POWER_OFF, BLOCK_CSP };

    explicit TfbHomeStopTimeout(FeedbackBlock block) : block_(block) {
        fb.Deceleration = 100000;
        fb.Jerk = 0;
    }

    void processCmd() override {
        if (idx == 8) {
            fbHm.Execute = true;
        } else if (idx == 11) {
            AxisCiaConfig& cfg = axArr[0].get_config_ref();
            if (block_ == BLOCK_POWER_OFF) {
                cfg.pdo_flag.enable_status_word = false;
            } else {
                cfg.pdo_flag.enable_modes_of_operation_display = false;
            }
            fb.Execute = true;
            stopStarted = true;
        }
    }

    void postRunCycle() override {
        if (!stopStarted) return;
        auto& ax = axRef(0);
        EXPECT_FALSE(Cia402Easy::is_ctrlw_halt(ax.wControlWord));
        if (ax.cmd == AXIS_CMD_MOTION) sawMotion = true;
        if (fbHm.CommandAborted) sawHomeAborted = true;
    }

    FeedbackBlock block_;
    bool stopStarted = false;
    bool sawHomeAborted = false;
    bool sawMotion = false;
};

TEST(home, stop_power_off_timeout) {
    TfbHomeStopTimeout asw(TfbHomeStopTimeout::BLOCK_POWER_OFF);
    asw.count_to_exit = 1100;
    asw.run();
    EXPECT_TRUE(asw.sawHomeAborted);
    EXPECT_FALSE(asw.sawMotion);
    EXPECT_TRUE(asw.fb.Error);
    EXPECT_FALSE(asw.fb.Done);
    EXPECT_EQ(asw.fb.ErrorID, ZMC_ERR_P_TIMEOUT);
    EXPECT_EQ(asw.axRef(0).axisStat, mcErrorStop);
}

TEST(home, stop_csp_timeout) {
    TfbHomeStopTimeout asw(TfbHomeStopTimeout::BLOCK_CSP);
    asw.count_to_exit = 2300;
    asw.run();
    EXPECT_TRUE(asw.sawHomeAborted);
    EXPECT_FALSE(asw.sawMotion);
    EXPECT_TRUE(asw.fb.Error);
    EXPECT_FALSE(asw.fb.Done);
    EXPECT_EQ(asw.fb.ErrorID, ZMC_ERR_SCM_MODE_SWITCH_TIMEOUT);
    EXPECT_EQ(asw.axRef(0).axisStat, mcErrorStop);
}

class TfbSoftwareHomingStop : public AxisRunTmpl<> {
   public:
    TfbSoftwareHomingStop() {
        AxisCiaConfig& cfg = axArr[0].get_config_ref();
        cfg.pdo_flag.enable_modes_of_operation = true;
        cfg.pdo_flag.enable_modes_of_operation_display = true;
        cfg.cycle_time = 10000;
        reload_config(0);

        AXIS_REF axis = axisRef(0);
        power.Axis = axis;
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        homing.Axis = axis;
        homing.fVelocitySlow = 20;
        homing.fVelocityFast = 100;
        homing.fAcceleration = 1000;
        homing.fDeceleration = 1000;
        homing.fJerk = 0;
        homing.nDirection = mcPositiveDirection;
        homing.nHomingMode = FAST_BSLOW_S_STOP;
        homing.bReferenceSwitch = false;
        stop.Axis = axis;
        stop.Deceleration = 1000;
        stop.Jerk = 0;
    }

    void preRunCycle() override {
        if (idx == 8) homing.bExecute = true;
        if (idx == 12) {
            stop.Execute = true;
            stopStarted = true;
        }
        if (stopStarted && stop.Done && !stopReleased) {
            stop.Execute = false;
            stopReleased = true;
        }
        power.runCycle();
        homing.runCycle();
        stop.runCycle();
    }

    void postRunCycle() override {
        if (!stopStarted) return;
        auto& ax = axRef(0);
        if (ax.cmd == AXIS_CMD_POWEROFF) sawPowerOff = true;
        if (ax.cmd == AXIS_CMD_MOTION) sawMotion = true;
        if (homing.bCommandAborted) sawHomingAborted = true;
        if (stop.Done) sawDone = true;
    }

    FbtPower power;
    FbtHoming homing;
    FbtStop stop;
    bool stopStarted = false;
    bool stopReleased = false;
    bool sawPowerOff = false;
    bool sawMotion = false;
    bool sawHomingAborted = false;
    bool sawDone = false;
};

TEST(home, software_homing_stop_does_not_power_cycle) {
    TfbSoftwareHomingStop asw;
    asw.count_to_exit = 120;
    asw.run();
    EXPECT_TRUE(asw.sawHomingAborted);
    EXPECT_FALSE(asw.sawPowerOff);
    EXPECT_TRUE(asw.sawMotion);
    EXPECT_TRUE(asw.sawDone);
    EXPECT_EQ(asw.axRef(0).axisStat, mcStandstill);
}

TEST(home, mov_home) {}

class TfbHomeMove : public TfbHomeTmpl<FbtMoveAdditive> {
   public:
    enum { OFS = -300 };
    int cnt2 = 20;
    TfbHomeMove(int axisNum = 1) {
        fb.Distance = 50;
        fb.Velocity = 500;
        fb.Deceleration = 1500;
        fb.Acceleration = 1500;
        fb.Jerk = 0;
        // verbose_flag = true;
    }
    void runCyclePre() {
        auto& axRef = this->axRef(0);
        int cnt = 60;
        if (idx == 5) {
            fbHm.Execute = true;
        } else if (idx == 6) {
            EXPECT_EQ(get_home_stat(), FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(axRef.axisStat, mcHoming);
        } else if (idx == cnt2) {
            fb.Execute = true;
            EXPECT_EQ(get_home_stat(), FB_STAT_DONE);
        } else if (idx == cnt2 + 1) {
            // fb.setExecute(false);
            // EXPECT_TRUE(fbHm.isCommandAborted());
            // EXPECT_EQ(fb.get_fb_mid_stat(), FB_MID_STAT_EXE_BUSY);
            EXPECT_EQ(axRef.axisStat, mcDiscreteMotion);
        } else if (idx == cnt) {
            // EXPECT_EQ(fb.get_fb_mid_stat(), FB_MID_STAT_EXE_DONE);
            EXPECT_EQ(axRef.axisStat, mcStandstill);
            fb.Execute = false;
        } else if (idx == cnt + 1) {
        } else if (idx == cnt + 2) {
            EXPECT_EQ(fb.Done, false);
            EXPECT_NEAR(axRef.fSetPosition, 90, 1e-6);
        }
        // int ret = chk.check(axRef->dPos(), axRef->dVel(), axRef->dAcc());
        // if (idx > cnt2) {
        //     EXPECT_EQ(ret, 0);
        // }
    }
};

TEST(home, move) {
    TfbHomeMove asw;
    asw.count_to_exit = 60;
    asw.run();
}

class TfbHomeMoveAbort : public TfbHomeTmpl<FbtMoveAdditive> {
   public:
    enum { OFS = -300 };
    int cnt2 = 13;
    int cnt3 = 20;
    TfbHomeMoveAbort(int axisNum = 1) {
        fb.Distance = 50;
        fb.Velocity = 500;
        fb.Deceleration = 1500;
        fb.Acceleration = 1500;
        fb.Jerk = 0;
    }
    void runCyclePre() {
        auto& axRef = this->axRef(0);
        if (idx == 5) {
            fbHm.Execute = true;
        } else if (idx == 6) {
            EXPECT_EQ(get_home_stat(), FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(axRef.axisStat, mcHoming);
        } else if (idx == cnt2) {
            fb.Execute = true;
            EXPECT_EQ(get_home_stat(), FB_STAT_BUSY_ACTIVE);
        } else if (idx == cnt2 + 1) {
            EXPECT_EQ(axRef.axisStat, mcHoming);
            EXPECT_EQ(get_move_stat(fb), FB_STAT_ERROR);
            EXPECT_EQ(fb.ErrorID, ZMC_AXIS_NOT_READY_FOR_MOTION);
        } else if (idx == cnt3) {
            EXPECT_EQ(axRef.axisStat, mcStandstill);
            EXPECT_EQ(get_home_stat(), FB_STAT_DONE);
        }
    }
};

TEST(home, moveAbort) {
    TfbHomeMoveAbort asw;
    asw.count_to_exit = 60;
    asw.run();
}
