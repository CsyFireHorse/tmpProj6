#include <gtest/gtest.h>
#include "fbt_axis.h"
#include "fbt_axis_impl.h"
#include "fbt_axis_encoder.h"
#include "fb_cpp_def.h"
#include "fbt_fb_base.hpp"
#include "axMoveTestTmpl.hpp"

using namespace zmc;

class EncoderTestCase : public PouTaskBase {
public:
    AXIS_REF axis;
    int cnt = 0;
    int32_t currentPos = 0;
    int32_t increment = 1000;

    EncoderTestCase() {
        const auto& config = AxMoveTestCaseConfig::get_config();
        increment = config.get_int("increment", 1000);
    }

    void runCycle() override {        
        // Set encoder position
        AxisImpl& ax = *((AxisImpl*)axis);
        ax.diEncoderPosition = currentPos;
        
        if (cnt >= 5) {
            EXPECT_NEAR(ax.fActVelocity, increment/0.05/11*3, 1e-6);
            EXPECT_NEAR(ax.fSetVelocity, ax.fActVelocity, 1e-6);
        }
        currentPos += increment; 
        // axis->print(6);
        cnt++;
    }
};

ConfigDict defaultDict = {
    {"axis_0.increments_units_in_application_num", "11"},
    {"axis_0.increments_units_in_application_den", "3"},
    {"axis_0.cycle_time", "50000"},
};

template<typename T, typename R>
void ax_move_test_tmpl_dict_default(const ConfigDict& config) {
    ConfigDict mergedConfig = defaultDict;
    mergedConfig.merge(config);
    return ax_move_test_tmpl_dict<T, R>(mergedConfig);
}

TEST(encoder, basic) {
    ConfigDict config = {
        {"base.count_to_exit", "50"},
        {"increment", "1000"},
        {"axis_0.bit_width", "16"}
    };
    ax_move_test_tmpl_dict_default<EncoderTestCase, AxisEncoder>(config);
}

TEST(encoder, overflow_12bit) {
    ConfigDict config = {
        {"base.count_to_exit", "50"},
        {"axis_0.bit_width", "12"},
        {"increment", "100"}
    };
    ax_move_test_tmpl_dict_default<EncoderTestCase, AxisEncoder>(config);
}

TEST(encoder, overflow_16bit) {
    ConfigDict config = {
        {"base.count_to_exit", "20"},
        {"increment", "10000"},
        {"axis_0.bit_width", "16"}
    };
    ax_move_test_tmpl_dict_default<EncoderTestCase, AxisEncoder>(config);
}

TEST(encoder, overflow_24bit) {
    ConfigDict config = {
        {"base.count_to_exit", "15"},
        {"axis_0.bit_width", "24"},
        {"increment", "1000000"},
    };
    ax_move_test_tmpl_dict_default<EncoderTestCase, AxisEncoder>(config);
}
