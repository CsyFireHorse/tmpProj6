#include <gtest/gtest.h>
#include "fb_cpp_def.h"
#include "AxisRunTmpl.hpp"
#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "common/misc/CmdArgsToy.hpp"
#include "fbt_axis_morph.h"
#include "mc_enum_def.h"
#include "math.h"
#include "stdio.h"
#include <string>

using namespace zmc;

/**
 * @brief PouTaskBase subclass for testing blending motion functionality
 * This class demonstrates how to create a task that can be used with AxPou system
 * for testing motion blending between absolute and relative movements
 */
class BlendingTestTask : public PouTaskBase {
public:
    BlendingTestTask() {
        auto& args = CmdArgsToySingleton::get();
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        
        // Configure first movement (absolute)
        move1.Axis = axis;
        move1.Position = args["move1.Position"].get<double>(10.0);
        move1.Velocity = args["move1.Velocity"].get<double>(5.0);
        move1.Acceleration = args["move1.Acceleration"].get<double>(10.0);
        move1.Deceleration = args["move1.Deceleration"].get<double>(10.0);
        move1.Jerk = args["move1.Jerk"].get<double>(0.0);
        move1.Direction = MC_DIRECTION::mcPositiveDirection;
        move1.BufferMode = static_cast<MC_BUFFER_MODE>(args["move1.BufferMode"].get<int>(mcBlendingLow));
        
        // Configure second movement (relative)
        move2.Axis = axis;
        move2.Distance = args["move2.Distance"].get<double>(5.0);
        move2.Velocity = args["move2.Velocity"].get<double>(3.0);
        move2.Acceleration = args["move2.Acceleration"].get<double>(8.0);
        move2.Deceleration = args["move2.Deceleration"].get<double>(8.0);
        move2.Jerk = args["move2.Jerk"].get<double>(0.0);
        // move2.Direction = MC_DIRECTION::mcPositiveDirection;
        move2.BufferMode = static_cast<MC_BUFFER_MODE>(args["move2.BufferMode"].get<int>(mcBlendingLow));
        
        // Configure power control
        power.Axis = axis;
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        
        // Test parameters
        move1_delay = args["move1_delay"].get<int>(5);
        move2_delay = args["move2_delay"].get<int>(15);
        
        // Bind extra data for monitoring
        axis->bind_extra_data(&extraData);
    }
    
    int getIndex() const {
        return _index;
    }
    
    void runCycle() override {
        auto ax = (AxisImpl*)axis;
        // Power control
        power.runCycle();
        
        // First movement execution
        move1.Execute = power.Status && (getIndex() >= move1_delay);
        move1.runCycle();
        
        // Second movement execution
        move2.Execute = power.Status && (getIndex() >= move2_delay);
        move2.runCycle();
        
        // Test verification with EXPECT_ macros
        auto i = getIndex();
        if (move1.Done && doneIndex == 0){
            doneIndex = i;
            EXPECT_LE(ax->fSavePosition, move1.Position);
            EXPECT_GE(ax->fSetPosition, move1.Position);
            printf("ax->fSetVelocity = %f\n", ax->fSetVelocity);
        }
        if (move2.Done && doneIndex2 == 0){
            doneIndex2 = i;
            EXPECT_EQ(ax->fSetPosition, move1.Position + move2.Distance);
            EXPECT_LT(ax->fSavePosition, move1.Position + move2.Distance);
            EXPECT_EQ(ax->axisStat, mcStandstill);
        }        
        _index++;
    }
    void postRun()override{
        // EXPECT_EQ(axis->axisStat, mcStandstill);
        EXPECT_NE(doneIndex2, 0);
        EXPECT_NE(doneIndex, 0);
    }
private:
    AXIS_REF axis;
    FbtPower power;
    FbtMoveAbsolute move1;
    FbtMoveRelative move2;
    
    // Configuration parameters
    int move1_delay = 5;
    int move2_delay = 15;
    int _index = 0;
    int extraData = 0;
    int doneIndex = 0;
    int doneIndex2 = 0;    
};

#define LEN(x) sizeof(x)/sizeof(x[0])  

PouTmplDefaults defaults = {
    {"base.axis_num", "1"},
    {"axis_0.cycle_time", "10000"},
    {"axis_0.is_axis_virtual", "1"}
};


template<typename T>
void test_blending(const std::unordered_map<std::string, std::string>& dict) {
    auto argv = dict_to_argv(dict);
    pou_auto_default_tmpl<T>(argv.size(), argv.data(), defaults);
}



// Test cases using PouTaskBase approach
TEST(BlendingPouTest, abs_blend_rel_low_speed) {
    std::unordered_map<std::string, std::string> dict = {
        {"move1.Position", "5.0"},
        {"move1.Velocity", "2.0"},
        {"move1.BufferMode", "2"},
        {"move2.Distance", "5.0"},
        {"move2.Velocity", "3.0"},
        {"move2.BufferMode", "2"},
        {"move1_delay", "5"},
        {"move2_delay", "10"},
        {"cycles", "1500"},
    };
    test_blending<BlendingTestTask>(dict);
}


TEST(BlendingPouTest, abs_blend_rel_high_speed) {
    std::unordered_map<std::string, std::string> dict = {
        {"move1.Position", "10.0"},
        {"move1.Velocity", "8.0"},
        {"move1.BufferMode", "5"},
        {"move2.Distance", "5.0"},
        {"move2.Velocity", "6.0"},
        {"move2.BufferMode", "5"},
        {"move1_delay", "3"},
        {"move2_delay", "8"},
        {"cycles", "1200"},
    };
    test_blending<BlendingTestTask>(dict);
}

#if 1
struct BlendingParam{
    int idx;
    double midVelocity; // BlendingParam
    // TODO: add middle Velocity
};

TEST(BlendingPouTest, blending_modes_comparison) {
    // Test different blending modes
    // std::vector<int> modes = {2, 5, 3, 4}; // mcBlendingLow, mcBlendingHigh, mcBlendingPrevious, mcBlendingNext
    // std::vector<std::string> modeNames = {"Low", "High", "Previous", "Next"};
    const char* bufferModeArr[] = {"1", "2", "3", "4", "5"};
    BlendingParam arr[] = {{0, 0}, {1, 2},  {2, 4},  {3, 2},  {4, 4} };
    for (size_t i = 0; i < LEN(bufferModeArr); ++i) {
        std::unordered_map<std::string, std::string> dict = {
            {"move1.Position", "5.0"},
            {"move1.Velocity", "4.0"},
            // {"move1.BufferMode", std::to_string(arr[i].idx).c_str()},
            {"move2.Distance", "3.0"},
            {"move2.Velocity", "2.0"},
            {"move2.BufferMode", (char*) bufferModeArr[i]},
            {"move1_delay", "5"},
            {"move2_delay", "12"},
            {"cycles", "1000"},
        };
        test_blending<BlendingTestTask>(dict);
    }
}


#endif

