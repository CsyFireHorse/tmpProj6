#include "ZmlClientTest.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "fb/mc_parameter_enum_def.h"
#include <memory>

// GPT-6 Codex, 2026-10-29. Shared client contract tests.

namespace {
// One runtime belongs to the parameter suite; each test owns its client sessions.
class ZmlParameterTest : public ZmlClientTest {
   protected:
    static std::unique_ptr<SrvTestRuntime>& parameterRuntime() {
        static std::unique_ptr<SrvTestRuntime> runtime;
        return runtime;
    }

    static void SetUpTestSuite() {
        auto& runtime = parameterRuntime();
        runtime.reset(new SrvTestRuntime());
        ASSERT_EQ(0, runtime->start({std::string("--") + ZML_TEST_MODE,
                                     "--port=" + std::to_string(ZML_TEST_PORT), "--ini",
                                     std::string(ZML_TEST_FIXTURE_DIR) + "/zml_motion.ini"}));
        zml_sleep_ms(100);
        ASSERT_EQ(0, zml_sleep_ms(100));
    }

    static void TearDownTestSuite() {
        auto& runtime = parameterRuntime();
        if (runtime) {
            EXPECT_EQ(0, runtime->finish());
            runtime.reset();
        }
    }

    void SetUp() override {
        ASSERT_TRUE(open(handle_));
        ZML_FlowCtrlConfig flow = {};
        flow.period = 10;
        flow.verbose_flag = 1;
        ASSERT_EQ(0, zml_set_flow_config(handle_, &flow));
    }

    void TearDown() override {
        if (second_) EXPECT_EQ(0, zml_close(&second_));
        if (handle_) EXPECT_EQ(0, zml_close(&handle_));
    }
};

}  // namespace

TEST_F(ZmlParameterTest, ExecutedParametersSharedAcrossSessions) {
    ASSERT_TRUE(open(second_));
    // Units carry the explicit num/den ratio: 2.5 = 5/2 and 3.5 = 7/2.
    // GLM-5.3 (opencode), 2026-10-23.
    const int32_t num[] = {5, 7};
    for (int axis = 0; axis < 2; ++axis) {
        ASSERT_EQ(0, zml_ax_set_units(handle_, axis, num[axis], 2));
        ASSERT_EQ(0, zml_ax_set_velocity(handle_, axis, 12.5 + axis));
        ASSERT_EQ(0, zml_ax_set_accel(handle_, axis, 23.5 + axis));
    }
    for (int axis = 0; axis < 2; ++axis) {
        ASSERT_TRUE(parameter(second_, axis, zml_ax_get_units, 2.5 + axis));
        ASSERT_TRUE(parameter(second_, axis, zml_ax_get_speed, 12.5 + axis));
        ASSERT_TRUE(parameter(second_, axis, zml_ax_get_accel, 23.5 + axis));
    }
}

// Typed single-value reads publish the confirmed cycle value once each.
// GLM-5.3 (opencode), 2026-10-23.
TEST_F(ZmlParameterTest, TypedParameterReadsMatchExecutedMotion) {
    ASSERT_NO_FATAL_FAILURE(prepareAxis(0));
    ASSERT_EQ(0, zml_ax_move_abs(handle_, 0, 5));
    ASSERT_TRUE(position(0, 5));
    double set_pos = 0, act_pos = 0, limit_neg = 0, limit_pos = 0, units = 0;
    int32_t axis_state = -1, units_num = -1, units_den = -1;
    ASSERT_EQ(0, zml_ax_read_set_position(handle_, 0, &set_pos));
    EXPECT_NEAR(5, set_pos, 0.01);
    ASSERT_EQ(0, zml_ax_read_act_position(handle_, 0, &act_pos));
    EXPECT_NEAR(5, act_pos, 0.01);
    ASSERT_EQ(0, zml_ax_read_sw_limit_negative(handle_, 0, &limit_neg));
    ASSERT_EQ(0, zml_ax_read_sw_limit_positive(handle_, 0, &limit_pos));
    EXPECT_TRUE(std::isfinite(limit_neg));
    EXPECT_TRUE(std::isfinite(limit_pos));
    ASSERT_EQ(0, zml_ax_get_units(handle_, 0, &units));
    ASSERT_EQ(0, zml_ax_read_units_num(handle_, 0, &units_num));
    ASSERT_EQ(0, zml_ax_read_units_den(handle_, 0, &units_den));
    EXPECT_NE(0, units_den);
    EXPECT_NEAR(units, static_cast<double>(units_num) / units_den, 1e-9);
    ZML_AxisState basic = {};
    ASSERT_EQ(0, zml_ax_get_state(handle_, 0, &basic));
    ASSERT_EQ(0, zml_ax_read_state(handle_, 0, &axis_state));
    EXPECT_EQ(basic.axis_state & kAxisStateMask,
              static_cast<uint32_t>(axis_state) & kAxisStateMask);
    // Compare typed reads with the generic cycle read on the same axis.
    const int parameters[] = {PARA_fActPosition,         PARA_fSetPosition,
                              PARA_fSWLimitNegative,     PARA_fSWLimitPositive,
                              PARA_nAxisState,           PARA_iRatioTechUnitsNum,
                              PARA_dwRatioTechUnitsDenom};
    const double expected[] = {act_pos,
                               set_pos,
                               limit_neg,
                               limit_pos,
                               static_cast<double>(axis_state),
                               static_cast<double>(units_num),
                               static_cast<double>(units_den)};
    double value = 0;
    for (size_t i = 0; i < sizeof(parameters) / sizeof(parameters[0]); ++i) {
        ASSERT_EQ(0, zml_ax_read_parameter(handle_, 0, parameters[i], &value));
        EXPECT_DOUBLE_EQ(expected[i], value);
    }
    ASSERT_EQ(0, zml_ax_read_parameter(handle_, 0, PARA_fTaskCycle, &value));
    EXPECT_DOUBLE_EQ(0.001, value);
    EXPECT_EQ(ZMC_ERR_MOTCMD_INVALID_ARG, zml_ax_read_parameter(handle_, 2, PARA_fTaskCycle, &value));
    EXPECT_DOUBLE_EQ(0.001, value);
    EXPECT_NE(0, zml_ax_read_parameter(handle_, 0, -1, &value));
    EXPECT_DOUBLE_EQ(0.001, value);
}

// Software-limit enable reads match the generic parameter API for both states.
// GPT-6 Codex, 2026-10-24.
TEST_F(ZmlParameterTest, SoftwareLimitEnableReadsMatchGenericParameter) {
    const int axes[] = {0, 1};
    const int32_t expected[] = {0, 1};
    for (int i = 0; i < 2; ++i) {
        int32_t enable = -1;
        double generic_value = -1;
        ASSERT_EQ(0, zml_ax_read_sw_limit_enable(handle_, axes[i], &enable));
        ASSERT_EQ(0, zml_ax_read_parameter(handle_, axes[i], PARA_bSWLimitEnable, &generic_value));
        EXPECT_EQ(expected[i], enable);
        EXPECT_DOUBLE_EQ(static_cast<double>(enable), generic_value);
    }

    int32_t unchanged = 7;
    EXPECT_NE(0, zml_ax_read_sw_limit_enable(nullptr, 0, &unchanged));
    EXPECT_EQ(7, unchanged);
}

// Both transports wait for executed writes before publishing success. GPT-6 Codex.
TEST_F(ZmlParameterTest, ParameterWritesAcknowledgeExecutedValues) {
    ASSERT_TRUE(open(second_));
    for (int32_t axis = 0; axis < 2; ++axis) {
        ASSERT_EQ(0,
                  zml_ax_write_parameter(handle_, axis, PARA_fSWLimitNegative, -12.3456789 - axis));
        ASSERT_EQ(0, zml_ax_write_parameter(handle_, axis, PARA_fSWLimitPositive, 23.4567891 + axis));
    }
    for (int32_t axis = 0; axis < 2; ++axis) {
        double value = 0;
        ASSERT_EQ(0, zml_ax_read_parameter(second_, axis, PARA_fSWLimitNegative, &value));
        EXPECT_DOUBLE_EQ(-12.3456789 - axis, value);
        ASSERT_EQ(0, zml_ax_read_parameter(second_, axis, PARA_fSWLimitPositive, &value));
        EXPECT_DOUBLE_EQ(23.4567891 + axis, value);
    }
    EXPECT_EQ(ZMC_WP_PARAM_INVALID, zml_ax_write_parameter(handle_, 0, -1, 7));
    for (double expected : {1.0, 0.0}) {
        ASSERT_EQ(0, zml_ax_write_parameter(handle_, 0, PARA_bSWLimitEnable, expected));
        double value = -1;
        ASSERT_EQ(0, zml_ax_read_parameter(second_, 0, PARA_bSWLimitEnable, &value));
        EXPECT_DOUBLE_EQ(expected, value);
    }
}
