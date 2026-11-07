#include <gtest/gtest.h>
#include "fb_cpp_def.h"
#include "AxisRunTmpl.hpp"
#include "math.h"
#include "stdio.h"

using namespace zmc;



template<typename T, typename T2>
struct SgAxFbDbTmpl2:public AxDbMotTmpl<T, T2, 1>{
    SgAxFbDbTmpl2(){
        this->get_config_ref(0).cycle_time = 50000;
        this->reload_config(0);
        this->bindAxis(0);
        set_fb_vel_config(this->mov, 5, 3, 3, 0);
        set_fb_vel_config(this->mov2, 3, 4, 2, 0);
        this->count_to_exit = 140;
    }
    // void postRunCycle()override{
        // return this->printAxisFb();
    // }
};

TEST(motNode, abs_blend_rel){
    SgAxFbDbTmpl2<FbtMoveAbsolute, FbtMoveRelative> axFb;
    axFb.mov.Position = 5;
    axFb.mov.BufferMode = mcBlendingLow;
    setFbVelEndLimit(axFb.mov, 2.0);
    axFb.mov2.Distance = 5;
    axFb.mov2.BufferMode = mcBlendingLow;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1){
            axFb.mov.Execute = true;
            axFb.mov2.Execute = true;
        }else if(i == 10){
            EXPECT_TRUE(axFb.mov.Active);
            EXPECT_TRUE(axFb.mov2.Busy);
            EXPECT_FALSE(axFb.mov2.Active);
            EXPECT_EQ(axFb.axRef(0).axisStat, mcDiscreteMotion);
        }else if(i == 50){
            EXPECT_TRUE(axFb.mov.Done);
            EXPECT_TRUE(axFb.mov2.Active);
            EXPECT_EQ(axFb.axRef(0).axisStat, mcDiscreteMotion);
        }else if (i==95){
            EXPECT_TRUE(axFb.mov2.Done);
            EXPECT_EQ(axFb.axRef(0).axisStat, mcStandstill);
        }
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
    EXPECT_NEAR(axFb.axRef(0).fSetPosition, 10, 1e-3);
}

TEST(motNode, abs_blend_rel2){
    SgAxFbDbTmpl2<FbtMoveAbsolute, FbtMoveRelative> axFb;
    axFb.mov.Position = 5;
    axFb.mov.BufferMode = mcBlendingLow;
    axFb.mov2.Distance = 5;
    axFb.mov2.BufferMode = mcBlendingLow;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 30){
            axFb.mov2.Execute = true;
        }else if(i == 35){
            EXPECT_TRUE(axFb.mov.Active);
            EXPECT_TRUE(axFb.mov2.Busy);
            EXPECT_FALSE(axFb.mov2.Active);
            EXPECT_EQ(axFb.axRef(0).axisStat, mcDiscreteMotion);
        }else if(i == 50){
            EXPECT_TRUE(axFb.mov.Done);
            EXPECT_TRUE(axFb.mov2.Active);
            EXPECT_EQ(axFb.axRef(0).axisStat, mcDiscreteMotion);
        }else if (i==95){
            EXPECT_TRUE(axFb.mov2.Done);
            EXPECT_EQ(axFb.axRef(0).axisStat, mcStandstill);
        }
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
    EXPECT_NEAR(axFb.axRef(0).fSetPosition, 10, 1e-3);
    // endPos = 15
}