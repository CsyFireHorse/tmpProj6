#include "ZmlClientTest.hpp"
#include <memory>

// GPT-6 Codex, 2026-10-22. Expectations describe the published result wire contract.

namespace {
// One runtime belongs to the motion suite; each test owns its client sessions.
class ZmlMotionTest : public ZmlClientTest {
   protected:
    static std::unique_ptr<SrvTestRuntime>& motionRuntime() {
        static std::unique_ptr<SrvTestRuntime> runtime;
        return runtime;
    }

    static void SetUpTestSuite() {
        auto& runtime = motionRuntime();
        runtime.reset(new SrvTestRuntime());
        ASSERT_EQ(0, runtime->start({std::string("--") + ZML_TEST_MODE,
                                     "--port=" + std::to_string(ZML_TEST_PORT), "--ini",
                                     std::string(ZML_TEST_FIXTURE_DIR) + "/zml_motion.ini"}));
        zml_sleep_ms(100);
        ASSERT_EQ(0, zml_sleep_ms(100));
    }

    static void TearDownTestSuite() {
        auto& runtime = motionRuntime();
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
        if (handle_) EXPECT_EQ(0, zml_close(&handle_));
    }
};

}  // namespace

TEST_F(ZmlMotionTest, ExplicitAxesAndWaitsReachFinalPositions) {
    ASSERT_NO_FATAL_FAILURE(prepareAxis(0));
    ASSERT_NO_FATAL_FAILURE(prepareAxis(1));
    ASSERT_EQ(0, zml_ax_move_abs(handle_, 0, 5));
    ASSERT_EQ(0, zml_ax_move_abs(handle_, 1, 6));
    ASSERT_EQ(0, zml_ax_wait_idle(handle_, 1, 10000));
    ASSERT_EQ(0, zml_ax_wait_ticks(handle_, 2, 1));
    ASSERT_EQ(0, zml_ax_moveadd(handle_, 1, 2));
    ASSERT_TRUE(position(0, 5));
    ASSERT_TRUE(position(1, 8));
}

TEST_F(ZmlMotionTest, AxisStatesAndTerminalResultsMatchExecutedMove) {
    int32_t status = -1, error = -1;
    ASSERT_EQ(0, zml_ax_home_get_status(handle_, 1, &status, &error));
    EXPECT_EQ(ZML_HOME_STATUS_IDLE, status);
    EXPECT_EQ(0, error);
    ASSERT_NO_FATAL_FAILURE(prepareAxis(1));
    ZML_AxisState before = {};
    ASSERT_EQ(0, zml_ax_get_state(handle_, 1, &before));
    ASSERT_EQ(0, zml_ax_move_abs(handle_, 1, -5));
    ASSERT_TRUE(position(1, -5));
    ZML_AxisState basic = {};
    ZML_AxisStateEx extended = {};
    ASSERT_EQ(0, zml_ax_get_state(handle_, 1, &basic));
    ASSERT_EQ(0, zml_ax_get_state_ex(handle_, 1, &extended));
    EXPECT_EQ(1u, basic.axis_id);
    EXPECT_EQ(basic.axis_id, extended.axis_id);
    EXPECT_EQ(kStandstill, basic.axis_state & kAxisStateMask);
    EXPECT_EQ(basic.axis_state & kAxisStateMask, extended.axis_state & kAxisStateMask);
    EXPECT_EQ(0u, basic.error_id);
    EXPECT_EQ(0u, extended.error_id);
    EXPECT_GT(extended.tick, 0u);
    EXPECT_NEAR(-5, extended.act_pos, 0.01);
    EXPECT_NEAR(-5, extended.set_pos, 0.01);
    EXPECT_NE(0, extended.act_position);
    EXPECT_NE(0, extended.set_position);
    EXPECT_TRUE(std::isfinite(extended.set_vel));
    EXPECT_TRUE(std::isfinite(extended.set_acc));
    ZML_CmdResultInfo result = {};
    ASSERT_TRUE(wait([&]() {
        ZML_CmdResultInfo results[100];
        uint32_t count = 100;
        if (zml_get_cmd_results(handle_, results, &count) != 0) return false;
        bool found = false;
        for (uint32_t i = 0; i < count; ++i) {
            if (results[i].command_type == kMoveAbsolute && results[i].target_index == 1 &&
                results[i].tick > before.tick && (!found || results[i].tick > result.tick)) {
                result = results[i];
                found = true;
            }
        }
        return found && result.status == kDone;
    }));
    EXPECT_EQ(kAxisTarget, result.target_kind);
    EXPECT_EQ(0u, result.error_code);
    EXPECT_EQ(0u, result.io_count);
    EXPECT_NE(0u, result.id);
    EXPECT_LE(static_cast<uint32_t>(result.data), result.tick);
}

TEST_F(ZmlMotionTest, GroupListsAffectOnlyRequestedAxes) {
    ASSERT_NO_FATAL_FAILURE(prepareAxis(0));
    ASSERT_NO_FATAL_FAILURE(prepareAxis(1));
    ASSERT_EQ(0, zml_group_select(handle_, 0));
    const int32_t axes[] = {1};
    const double target[] = {4};
    // Group administration preserves axis power; executed motion proves list membership.
    ASSERT_EQ(0, zml_group_enable(handle_, 1, axes));
    ASSERT_TRUE(axisState(1, kStandstill));
    ASSERT_TRUE(axisState(0, kStandstill));
    ASSERT_EQ(0, zml_moveabs(handle_, 1, axes, target));
    ASSERT_TRUE(position(1, 4));
    ASSERT_TRUE(position(0, 0));
    ASSERT_EQ(0, zml_group_disable(handle_, 1, axes));
    ASSERT_EQ(0, zml_ax_move_rel(handle_, 1, 1));
    ASSERT_TRUE(position(1, 5));
    ASSERT_TRUE(position(0, 0));
    ASSERT_TRUE(axisState(0, kStandstill));
}

TEST_F(ZmlMotionTest, GroupMovesPreserveAxisOrder) {
    ASSERT_NO_FATAL_FAILURE(prepareAxis(0));
    ASSERT_NO_FATAL_FAILURE(prepareAxis(1));
    ASSERT_EQ(0, zml_group_select(handle_, 0));
    const int32_t axes[] = {1, 0};
    const double absolute[] = {12, 5};
    const double relative[] = {4, 2};
    ASSERT_EQ(0, zml_group_enable(handle_, 2, axes));
    ASSERT_EQ(0, zml_moveabs(handle_, 2, axes, absolute));
    ASSERT_TRUE(position(1, 12));
    ASSERT_TRUE(position(0, 5));
    ASSERT_EQ(0, zml_moverel(handle_, 2, axes, relative));
    ASSERT_TRUE(position(1, 16));
    ASSERT_TRUE(position(0, 7));
}
