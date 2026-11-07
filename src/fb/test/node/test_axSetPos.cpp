#include <gtest/gtest.h>

#include "AxisRunTmpl.hpp"
#include "fb_cpp_def.h"
#include "math.h"
#include "stdio.h"
#include "common/get_file_path.hpp"

using namespace zmc;

class FbSetPositionTest : public AxDbMotTmpl<FbtSetPosition, FbtMoveRelative> {
public:
    FbSetPositionTest() {
        auto& cfg = this->get_config_ref(0);
        cfg.linear.increments_units_in_application_num = scale;
        cfg.linear.increments_units_in_application_den = 1;
        cfg.debug.servo_fak_pos = fakPos;
        this->reload_config(0);
        
        // Setup FbtSetPosition (this->mov)
        this->mov.Axis = this->axisRef(0);
        
        // Setup FbtMoveRelative (this->mov2)
        this->mov2.Distance = -40;
        set_fb_vel_config(this->mov2, 450, 4000, 4000, 0);
        this->mov2.Axis = this->axisRef(0);
        
        this->count_to_exit = 80;
    }
    
    void preRunCycle() override {
        auto _idx = this->getIndex();
        auto& axRef = this->axRef(0);
        if (_idx == 3){
            EXPECT_NEAR(axRef.fSetPosition, (double)fakPos / scale, 1e-6);
            EXPECT_NEAR(axRef.fActPosition, (double)fakPos / scale, 1e-6);
            EXPECT_EQ(axRef.diActPosition, fakPos);
        }else if (_idx == 5) {
            this->mov.Execute = true;
            this->mov.Position = 90;
            this->mov.Mode = true;
        } else if (_idx == 6) {
            this->mov.Execute = false;
            EXPECT_EQ(this->mov.Busy, false);
            EXPECT_EQ(this->mov.Done, true);
            EXPECT_NEAR(axRef.fSetPosition, -10, 1e-6);
        } else if (_idx == 10) {
            this->mov.Execute = true;
            this->mov.Position = -70;
            this->mov.Mode = false;
        } else if (_idx == 11) {
            EXPECT_NEAR(axRef.fSetPosition, -70, 1e-6);
            EXPECT_EQ(this->mov.Busy, false);
            EXPECT_EQ(this->mov.Done, true);
        } else if (_idx == 15) {
            this->mov2.Execute = true;
        } else if (_idx > 15) {
            if (this->mov2.Done) {
                EXPECT_NEAR(axRef.fSetPosition, -110, 1e-6);
            }
        }
        this->fbRunCycle();
        // Check motion consistency if in motion
        if (_idx > 13) {
            EXPECT_EQ(this->check_set_pva(0), 0);
        }
    }

public:
    const int scale = 10;
    const int fakPos = -1000;
    const double Pos = 200;
};

TEST(axis, setPos) {
    FbSetPositionTest asw;
    asw.run();
}

