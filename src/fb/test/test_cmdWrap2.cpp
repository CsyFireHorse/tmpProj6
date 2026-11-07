#include <gtest/gtest.h>
#include "motbuf/FbMoveWrap2.hpp"
#include "axPouTmplRunner.hpp"
#include "common/misc/CmdArgsToy.hpp"

using namespace zmc;

class MoveGrpTestTask : public MoveGrp {
public:
    MoveGrpTestTask():MoveGrp(){
        args = CmdArgsToySingleton::get();
        install(args);        
    }
    void onPreCmd() override {
        setPowerState(0, args["power.regulatorOn"].get<bool>(true));
        // if (getIndex() >= 10) setExecute(AxisMultFb::FBM_MOVE_ABSOLUTE, true);
        if (getIndex() >= 30) {
            // EXPECT_EQ(this->getAxRef(0)->axisStat, mcStandstill); 
            setExecute(AxisMultFb::FBM_HALT_2, true);
        }
    }
    void onPostCmd() override {
        if (getIndex() >= 31 && getIndex() < 35) {
            EXPECT_EQ(this->getAxRef(0)->axisStat, args["expect.axis.axisStat"].get<int>(0));
            EXPECT_EQ(this->getAxisMultFb(0).mHalt2.Error, args["expect.mHalt2.Error"].get<int>(0));
        }
        // this->getAxisRef(0)->print(1);
    }
    CmdArgsToy args;
};

#define LEN(x) sizeof(x)/sizeof(x[0]) 

TEST(MoveGrpTest, test_select_halt_normal) {
    char* argv[] = {"", "--select", "27", "--select2", "39", "--decelHalt", "20.0", "--decelHalt2", "20.0", "--cycles", "50", "--power.regulatorOn", "1", "--expect.axis.axisStat", "1", "--expect.mHalt2.Error", "0"};
    pou_auto_default_tmpl<MoveGrpTestTask>(LEN(argv), argv);
}

TEST(MoveGrpTest, test_select_halt_with_zero_decel) {
    char* argv[] = {"", "--select", "27", "--select2", "39", "--decelHalt", "0.0", "--decelHalt2", "0.0", "--cycles", "50", "--power.regulatorOn", "1", "--expect.axis.axisStat", "1", "--expect.mHalt2.Error", "1"};
    pou_auto_default_tmpl<MoveGrpTestTask>(LEN(argv), argv);
}

TEST(MoveGrpTest, test_select_halt_with_power_off) {
    char* argv[] = {"", "--select", "27", "--select2", "39", "--decelHalt", "0.0", "--decelHalt2", "0.0", "--power.regulatorOn", "0", "--cycles", "50", "--expect.axis.axisStat", "0", "--expect.mHalt2.Error", "1"};
    pou_auto_default_tmpl<MoveGrpTestTask>(LEN(argv), argv);
}
