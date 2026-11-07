#include <gtest/gtest.h>
#include "AxisRunTmpl.hpp"
// #include "AxisFbCompose.hpp"
// #include "common/misc/AxisChecker.hpp"
#include "fbt_axis.h"
#include "fbt_axis_impl.h"
#include "fb_cpp_def.h"
#include "fbt_fb_base.hpp"
#include "math.h"
#include "stdio.h"
// #include "velocity_utils.hpp"
// #include "common/misc/pou_base.hpp"
#include "AxisArrRunner.hpp"
#include "axMoveTestTmpl.hpp"

using namespace zmc;

// 基础测试模板
template<typename T>
struct AxTestTmpl : public AxSgMotTmpl<T, 1> {
    AxTestTmpl() {
        this->get_config_ref(0).debug.servo_fak_pos = 3;
        this->get_config_ref(0).cycle_time = 200000;
        this->reload_config(0);
        this->mov.Axis = this->axisRef(0);
        set_fb_vel_config(this->mov, 5, 3, 3, 0);
        this->count_to_exit = 100;
    }
};

// struct FooParam3{
//     int count_to_exit;
//     double posOrDist;
//     double endPos;
//     double Position2;
// };

template<typename T>
class PowerAndMove2:public PouTaskBase{
    public:
    AxisMotEmu* axis=nullptr;
    FbtPower power;
    T move;
    double targetPos = 0;

    PowerAndMove2(){
        auto& config = AxMoveTestCaseConfig::get_config();
        set_fb_vel_config(this->move, 5, 3, 3, 0);
        double posOrDist = config.get_double("posOrDist", 0.0);
        targetPos = config.get_double("endPos", posOrDist);
        setFbDistance(&move, posOrDist);
    }
    void runCycle()override{
        EXPECT_EQ(0, axis->check_set_pva());
        move.Axis = axis;
        power.Axis = axis;
        power.Enable = true;
        power.bRegulatorOn = true;
        power.bDriveStart = true;
        power.runCycle();
        move.Execute = power.Status;
        move.runCycle();
    }
    void postRun()override{
        auto& ax = *((AxisImpl*)axis);
        EXPECT_NEAR(ax.fSetPosition, this->targetPos, 1e-6);
    }
};


// class AxRunner3:public SgAxisRunner<AxisMotEmu>{
// public:
// AxRunner3 (const FooParam3& param){
//         this->count_to_exit = param.count_to_exit;
//         this->axArr[0].get_config_ref().cycle_time = 100000;
//         this->axArr[0].get_config_ref().debug.servo_fak_pos = 3;
//         this->axArr[0].reload_config();
//     }
// };

TEST(motNode, mov_abs) {
    ConfigDict config = {
        {"base.count_to_exit", "100"},
        {"posOrDist", "8"},
        {"endPos", "8"},
        {"axis_0.cycle_time", "100000"},
        {"axis_0.debug.servo_fak_pos", "3"}
    };
    ax_move_test_tmpl_dict<PowerAndMove2<FbtMoveAbsolute>, AxisMotEmu>(config);
}
TEST(motNode, mov_rel) {
    ConfigDict config = {
        {"base.count_to_exit", "100"},
        {"posOrDist", "-5"},
        {"endPos", "-2"},
        {"axis_0.cycle_time", "100000"},
        {"axis_0.debug.servo_fak_pos", "3"}
    };
    ax_move_test_tmpl_dict<PowerAndMove2<FbtMoveRelative>, AxisMotEmu>(config);
}
TEST(motNode, mov_add) {
    ConfigDict config = {
        {"base.count_to_exit", "100"},
        {"posOrDist", "-5"},
        {"endPos", "-2"},
        {"axis_0.cycle_time", "100000"},
        {"axis_0.debug.servo_fak_pos", "3"}
    };
    ax_move_test_tmpl_dict<PowerAndMove2<FbtMoveAdditive>, AxisMotEmu>(config);
}



class PowerAndInvertVel: public PouTaskBase {
    public:
    AxisMotEmu* axis = nullptr;
    FbtPower power;
    FbtMoveAbsolute move;
    double targetPos = 0;
    double alternatePos = 0;
    int cycleCount = 0;

    PowerAndInvertVel() {
        auto& config = AxMoveTestCaseConfig::get_config();
        set_fb_vel_config(this->move, 3, 2, 2, 0);
        double posOrDist = config.get_double("posOrDist", 0.0);
        this->move.Position = posOrDist;
        targetPos = config.get_double("endPos", posOrDist);
        alternatePos = config.get_double("Position2", posOrDist);
    }
    
    void runCycle() override {
        EXPECT_EQ(0, axis->check_set_pva());
        move.Axis = axis;
        power.Axis = axis;
        
        power.Enable = true;
        power.bRegulatorOn = true;
        power.bDriveStart = true;
        power.runCycle();
        
        if (cycleCount == 7) {
            move.Execute = true;
        } else if (cycleCount == 19) {
            move.Execute = false;
        } else if (cycleCount == 20) {
            move.Velocity = 2;
            move.Position = alternatePos;
            move.Execute = true;
        }
        
        move.runCycle();
        
        cycleCount++;
    }
    
    void postRun() override {
        auto& ax = *((AxisImpl*)axis);
        EXPECT_NEAR(ax.fSetPosition, targetPos, 1e-6);
    }
};

class PowerAndContUpdate: public PouTaskBase {
public:
    AxisMotEmu* axis = nullptr;
    FbtPower power;
    FbtMoveAbsolute move;
    double targetPos = 0;
    int cycleCount = 0;

    PowerAndContUpdate() {
        auto& config = AxMoveTestCaseConfig::get_config();
        set_fb_vel_config(this->move, 3, 2, 2, 0);
        double posOrDist = config.get_double("posOrDist", 0.0);
        this->move.Position = posOrDist;
        this->targetPos = config.get_double("endPos", posOrDist);
    }
    
    void runCycle() override {
        EXPECT_EQ(0, axis->check_set_pva());
        move.Axis = axis;
        power.Axis = axis;
        
        power.Enable = true;
        power.bRegulatorOn = true;
        power.bDriveStart = true;
        power.runCycle();
        
        if (cycleCount == 6) {
            move.Execute = true;
        } else if (cycleCount == 20) { 
            setFbContinousUpdate(move, true);
            move.Velocity = 2;
        } else if (cycleCount == 30) { 
            set_fb_vel_config(move, 4, 5, 5, 0);
        }

        move.runCycle();
        auto& ax = *((AxisImpl*)axis);
        if (cycleCount == 5) { 
            EXPECT_NEAR(ax.fSetPosition, 3, 1e-3);
        } else if (cycleCount == 21) { 
            EXPECT_NEAR(ax.fSetAcceleration, -2, 1e-3); 
        } else if (cycleCount == 27) { 
            EXPECT_NEAR(ax.fSetVelocity, 2, 1e-3);
        } else if (cycleCount == 37) {  
            EXPECT_NEAR(ax.fSetVelocity, 4, 1e-3);
        }        
        cycleCount++;
    }
    
    void postRun() override {
        auto& ax = *((AxisImpl*)axis);
        EXPECT_NEAR(ax.fSetPosition, targetPos, 1e-5);
        EXPECT_NEAR(ax.fSetVelocity, 0, 1e-5);
    }
};
TEST(motNode, abs_cont_update) {
    ConfigDict config = {
        {"base.count_to_exit", "100"},
        {"posOrDist", "15"},
        {"endPos", "15"},
        {"axis_0.cycle_time", "100000"},
        {"axis_0.debug.servo_fak_pos", "3"}
    };
    ax_move_test_tmpl_dict<PowerAndContUpdate, AxisMotEmu>(config);
}
TEST(moveabs, invert_end) {
    ConfigDict config = {
        {"base.count_to_exit", "100"},
        {"posOrDist", "15"},
        {"endPos", "5"},
        {"Position2", "5"},
        {"axis_0.cycle_time", "100000"},
        {"axis_0.debug.servo_fak_pos", "3"}
    };
    ax_move_test_tmpl_dict<PowerAndInvertVel, AxisMotEmu>(config);
}

TEST(moveabs, invert_vel) {
    ConfigDict config = {
        {"base.count_to_exit", "100"},
        {"posOrDist", "15"},
        {"endPos", "-3"},
        {"Position2", "-3"},
        {"axis_0.cycle_time", "100000"},
        {"axis_0.debug.servo_fak_pos", "3"}
    };
    ax_move_test_tmpl_dict<PowerAndInvertVel, AxisMotEmu>(config);
}


/*
idx,id,axStat,mePos,pos,vel,acc
4,0,1,-1000,-100,0,0
5,0,3,-1000,-100,0,400 tick=5, mtype=7, total_time=0, pos=0, vel=200, acc=400, dec=400
6,0,3,-1000,-100,0,0
7,0,1,-1000,-100,0,0 tick=7, mtype=7,Fb=2080
8,0,1,-1000,-100,0,0
9,0,1,-1000,-100,0,0
10,0,1,-1000,-100,0,0
*/
class FbSgMoveRelativeZero : public PouTaskBase {
public:
    FbSgMoveRelativeZero() {
        auto& config = AxMoveTestCaseConfig::get_config();
        scale = config.get_int("axis_0.increments_units_in_application_num", 10);
        fakPos = config.get_int("axis_0.debug.servo_fak_pos", -1000);
        set_fb_vel_config(mov, 200, 400, 400, 0);
        mov.Distance = 0;
    }
    
    void runCycle() override {
        auto _idx = this->cycleCount;
        auto& ax = *((AxisImpl*)axis);
        if (_idx == 0)
            this->mov.Axis = axis;

        if (_idx == 5) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            this->mov.Execute = true;
            EXPECT_NEAR(ax.fActPosition, (double)fakPos / scale, 1e-6);
            EXPECT_NEAR(ax.fSetPosition, (double)fakPos / scale, 1e-6);
            EXPECT_EQ(ax.diActPosition, fakPos);
            EXPECT_NEAR(ax.fSetVelocity, 0, 1e-6);
            EXPECT_NEAR(ax.fActVelocity, 0, 1e-6);
        } else if (_idx == 6) {
            this->mov.Execute = false;
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
            EXPECT_EQ(this->mov.Busy, true);
        } else if (_idx == 7) {
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
            EXPECT_EQ(this->mov.Busy, true);
            EXPECT_NEAR(ax.fSetVelocity, 0, 1e-6);
            EXPECT_NEAR(ax.fActVelocity, 0, 1e-6);
        } else if (_idx == 8) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_EQ(this->mov.Done, true);
        } else if (_idx == 9) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_EQ(this->mov.Done, true);
        } else if (_idx == 10) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_EQ(this->mov.Busy, false);
            EXPECT_EQ(this->mov.Done, false);
        }
        this->mov.runCycle();

        // Additional checks after runCycle
        if (_idx == 7) {
            EXPECT_EQ(this->mov.Busy, true);
        } else if (_idx == 9) {
            EXPECT_EQ(this->mov.Busy, false);
            EXPECT_EQ(this->mov.Done, false);
        }
        
        // Check motion consistency if in motion
        if (_idx > 5 && _idx < 10) {
            // EXPECT_EQ(this->check_set_pva(0), 0);
        }
    }

public:
    AXIS_REF axis;
    FbtMoveRelative mov;

    int scale = 10;
    int fakPos = -1000;
    int cycleCount = 0;
};

TEST(axis, moveZero) {
    ConfigDict config = {
        {"base.count_to_exit", "12"},
        {"axis_0.increments_units_in_application_num", "10"},
        {"axis_0.cycle_time", "100000"},
        {"axis_0.debug.servo_fak_pos", "-1000"}
    };
    ax_move_test_tmpl_dict<FbSgMoveRelativeZero, AxisMotEmu>(config);
}


class PowerAndOverride: public PouTaskBase {
public:
    AxisMotEmu* axis = nullptr;
    FbtPower power;
    FbtMoveAbsolute move;
    FbtSetOverride override;
    double targetPos = 0;
    int cycleCount = 0;

    PowerAndOverride() {
        auto& config = AxMoveTestCaseConfig::get_config();
        set_fb_vel_config(this->move, 5, 3, 3, 0);
        double posOrDist = config.get_double("posOrDist", 0.0);
        this->move.Position = posOrDist;
        targetPos = config.get_double("endPos", posOrDist);
    }
    
    void runCycle() override {
        EXPECT_EQ(0, axis->check_set_pva());
        move.Axis = axis;
        power.Axis = axis;
        override.Axis = axis;
        
        power.Enable = true;
        power.bRegulatorOn = true;
        power.bDriveStart = true;
        power.runCycle();
        
        if (cycleCount == 6) {
            move.Execute = true;
        } else if (cycleCount == 25) {
            override.VelFactor = 0.6;
        } else if (cycleCount == 35) {
            override.VelFactor = 0.8;
            override.AccFactor = 0.5;
        }
        
        move.runCycle();
        
        override.Enable = true;
        override.runCycle();
        
        cycleCount++;
    }
    
    void postRun() override {
        auto& ax = *((AxisImpl*)axis);
        EXPECT_NEAR(ax.fSetPosition, targetPos, 1e-6);
    }
};


TEST(moveabs, move_override){
    ConfigDict config = {
        {"base.count_to_exit", "100"},
        {"posOrDist", "20"},
        {"endPos", "20"},
        {"axis_0.cycle_time", "100000"},
        {"axis_0.debug.servo_fak_pos", "3"}
    };
    ax_move_test_tmpl_dict<PowerAndOverride, AxisMotEmu>(config);
}


// TEST(motNode, get_max_vel) {
//     AxTestTmpl<FbtMoveRelative> axFb;
//     FbtGetMaxSetVelocity fbMaxVel;
//     FbtGetMaxSetAccDec fbMaxAccDec;
//     axFb.mov.Distance = -10;

//     fbMaxVel.Axis = axFb.axisRef(0);
//     fbMaxVel.Enable = true;
//     fbMaxAccDec.Axis = axFb.axisRef(0);
//     fbMaxAccDec.Enable = true;
//     axFb.run([&](){
//         auto i = axFb.getIndex();
//         if (i == 1)
//             axFb.mov.Execute = true;
//         fbMaxVel.runCycle();
//         fbMaxAccDec.runCycle();
//         axFb.check_set_pva(0);
//     });
//     EXPECT_TRUE(fbMaxVel.Valid);
//     EXPECT_NEAR(fbMaxVel.MaxVelocity, 5, 1e-6);
//     EXPECT_TRUE(fbMaxAccDec.Valid);
//     EXPECT_NEAR(fbMaxAccDec.MaxAcceleration, 3, 1e-6);
//     EXPECT_NEAR(axFb.axRef(0).fSetPosition, 3+axFb.mov.Distance, 1e-6);
// }

class PowerAndMaxVel: public PouTaskBase {
public:
    AxisMotEmu* axis = nullptr;
    FbtPower power;
    FbtMoveRelative move;
    FbtGetMaxSetVelocity fbMaxVel;
    FbtGetMaxSetAccDec fbMaxAccDec;
    double distance = 0;

    PowerAndMaxVel() {
        auto& config = AxMoveTestCaseConfig::get_config();
        set_fb_vel_config(this->move, 5, 3, 3, 0);
        distance = config.get_double("posOrDist", 0.0);
        setFbDistance(&move, distance);
    }
    
    void runCycle() override {
        EXPECT_EQ(0, axis->check_set_pva());
        move.Axis = axis;
        power.Axis = axis;
        fbMaxVel.Axis = axis;
        fbMaxAccDec.Axis = axis;
        
        power.Enable = true;
        power.bRegulatorOn = true;
        power.bDriveStart = true;
        power.runCycle();
        
        move.Execute = power.Status;
        move.runCycle();
        
        fbMaxVel.Enable = true;
        fbMaxVel.runCycle();
        fbMaxAccDec.Enable = true;
        fbMaxAccDec.runCycle();
    }
    
    void postRun() override {
        auto& ax = *((AxisImpl*)axis);
        EXPECT_TRUE(fbMaxVel.Valid);
        EXPECT_NEAR(fbMaxVel.MaxVelocity, 5, 1e-6);
        EXPECT_TRUE(fbMaxAccDec.Valid);
        EXPECT_NEAR(fbMaxAccDec.MaxAcceleration, 3, 1e-6);
        EXPECT_NEAR(ax.fSetPosition, 3 + distance, 1e-6);
    }
};

TEST(motNode, get_max_vel2) {
    ConfigDict config = {
        {"base.count_to_exit", "100"},
        {"posOrDist", "-10"},
        {"endPos", "-7"},
        {"axis_0.cycle_time", "100000"},
        {"axis_0.debug.servo_fak_pos", "3"}
    };
    ax_move_test_tmpl_dict<PowerAndMaxVel, AxisMotEmu>(config);
}