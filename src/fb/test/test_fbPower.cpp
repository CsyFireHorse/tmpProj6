#include <gtest/gtest.h>
#include "math.h"
#include "stdio.h"
#include "AxisRunTmpl.hpp"
#include "fb_cpp_def.h"


using namespace zmc;

class AxPowerTestCaseBase : public AxisRunTmpl<> {
   public:
    int get_power_status() const {
        return fbPow.Busy + fbPow.bRegulatorRealState * 2 + fbPow.bDriveStartRealState * 4 + fbPow.Status * 8 + fbPow.Error * 16;
    }
    void preRunCycle() override {
        processCmd();
        fbPow.runCycle();
    }
    virtual void processCmd(){}
    FbtPower fbPow;
};



class AxPowerTestCase : public AxPowerTestCaseBase{
public:
    void processCmd()override{
        if (idx == 3){
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
        }else if (idx == 5){ 
            fbPow.Axis = (AXIS_REF) &axArr[0];
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
        }else if (idx == 11){
            EXPECT_TRUE(fbPow.Status);
            EXPECT_TRUE(fbPow.Busy);
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
            fbPow.Enable = false;
        }else if (idx == 13){
            EXPECT_TRUE(fbPow.Status);
            EXPECT_FALSE(fbPow.Busy);
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
        }else if (idx == 30) {
            fbPow.Enable = true;
            fbPow.bRegulatorOn = false;
            fbPow.bDriveStart = false;
        }else if (idx == 35){
            EXPECT_FALSE(fbPow.Status);
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
        }
    }
};
TEST(fbPower, base){
    AxPowerTestCase axp;
    axp.count_to_exit = 50;
    axp.run();    
}


class AxPowerTestCaseLatch : public AxPowerTestCaseBase{
public:
    void processCmd()override{
        if (idx == 5){
            fbPow.Axis = (AXIS_REF) &axArr[0];
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
        }else if (idx == 15){
            EXPECT_TRUE(fbPow.Status);
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
        }else if (idx >= 16 && idx <= 20){
            fbPow.Enable = false;
            EXPECT_TRUE(fbPow.Status);
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
        }else if (idx >20 && idx <= 25) {
            fbPow.bRegulatorOn = false;
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
        }else if (idx >25 && idx <= 30) {
            fbPow.Enable = true;
            fbPow.bRegulatorOn = false;
            fbPow.bDriveStart = false;
        }else if (idx == 35){
            EXPECT_FALSE(fbPow.Status);
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
        }
    }
};

TEST(fbPower, latch){
    AxPowerTestCaseLatch axp;
    axp.count_to_exit = 50;
    axp.run();    
}



class AxPowerTestCaseTimeout0 : public AxPowerTestCaseBase{
    public:
    int cnt_poweron = 1025;
    void processCmd() override {
        if (idx < cnt_poweron){
            axRef(0).regulatorRealState = false; 
            axRef(0).driveStartRealState = false;
        }
        if (idx == 0){
            fbPow.Axis = (AXIS_REF) &axArr[0];
        }else if(idx == 3){
            EXPECT_EQ(get_power_status(), 0);
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
        }else if (idx == 5){ 
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
        }else if (idx == 11){
            EXPECT_EQ(get_power_status(), 1);
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
        }else if (idx == 1010){
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
            EXPECT_EQ(get_power_status(), 16);
            EXPECT_EQ(fbPow.ErrorID, ZMC_ERR_P_TIMEOUT);
        }else if (idx == cnt_poweron + 20){
            EXPECT_EQ(get_power_status(), 16);
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
            EXPECT_EQ(fbPow.ErrorID, ZMC_ERR_P_TIMEOUT);
        }
    }
};

TEST(fbPower, timeout0){
    AxPowerTestCaseTimeout0 axp;
    axp.count_to_exit = 1050;
    axp.run();    
}


// poweron after timeout error
class AxPowerTestCaseTimeout : public AxPowerTestCaseBase{
    public:
    int cnt_poweron = 1025;
    void processCmd() override {
        if (idx < cnt_poweron){
            axRef(0).regulatorRealState = false; 
            axRef(0).driveStartRealState = false;
        }
        if (idx == 0){
            fbPow.Axis = (AXIS_REF) &axArr[0];
        }else if(idx == 3){
            EXPECT_EQ(get_power_status(), 0);
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
        }else if (idx == 5){ 
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
        }else if (idx == 11){
            EXPECT_EQ(get_power_status(), 1);
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
        }else if (idx == 1010){
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
            EXPECT_EQ(get_power_status(), 16);
            EXPECT_EQ(fbPow.ErrorID, ZMC_ERR_P_TIMEOUT);
        }else if (idx == cnt_poweron-5) {
            fbPow.Enable = true;
            fbPow.bRegulatorOn = false;
            fbPow.bDriveStart = false;
        }else if (idx == cnt_poweron-4) {
            EXPECT_EQ(get_power_status(), 1);
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
        }else if (idx == cnt_poweron+5){
            EXPECT_EQ(get_power_status(), 15);
            EXPECT_EQ(axRef(0).axisStat, mcStandstill);
        }
    }
};

TEST(fbPower, timeout){
    AxPowerTestCaseTimeout axp;
    axp.count_to_exit = 1050;
    axp.run();    
}

// test invalid axis cases
class AxPowerTestCaseInvalid : public AxPowerTestCaseBase {
public:
    void processCmd() override {
        int magicBak = 0;
        if (idx == 0) {
            // test nullptr axis
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
            EXPECT_EQ(get_power_status(), 0);
        }else if (idx == 1){
            EXPECT_EQ(fbPow.ErrorID, ZMC_AXIS_IS_NO_AXIS_REF);
        } else if (idx == 5) {
            // test invalid axis
            magicBak = axRef(0).wMagicNumber;
            axRef(0).wMagicNumber = 0;
            fbPow.Axis = (AXIS_REF)&axArr[0];
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
        } else if (idx == 6) {
            EXPECT_EQ(get_power_status(), 16); // Error bit set
            EXPECT_EQ(fbPow.ErrorID, ZMC_AXIS_IS_NO_AXIS_REF);
            EXPECT_FALSE(fbPow.bRegulatorRealState);
            EXPECT_FALSE(fbPow.bDriveStartRealState);
        } else if (idx == 10) {
            // test valid axis
            axRef(0).wMagicNumber = magicBak;
            fbPow.Axis = (AXIS_REF)&axArr[0];
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
        } else if (idx == 11) {
            EXPECT_EQ(get_power_status(), 1);
            EXPECT_EQ(axRef(0).axisStat, mcDisabled);
        }
    }
};

TEST(fbPower, invalid_axis) {
    AxPowerTestCaseInvalid axp;
    axp.count_to_exit = 10;
    axp.run();
}