/*
 * @ArtifactStatus: draft
 * @ArtifactStatusDate: 2026-09-01
 * @ArtifactStatusAuthority: GPT-5 Codex
 */
#include "MotionBuffer.h"

#include <cmath>

#include "common/zmc/fbt_errorcode_def.h"
#include "gtest/gtest.h"

using namespace zmc;

namespace {

const double kExactTolerance = 1e-12;
const double kLinearTolerance = 1e-9;
const int32_t kFullPoolCount = 2 * MOTION_BUFFER_SLOT_CAPACITY;

MotionLimits make_limits(double speed, double acceleration, double jerk = 0.0) {
    MotionLimits limits;
    limits.max_speed = speed;
    limits.max_acceleration = acceleration;
    limits.max_deceleration = acceleration;
    limits.max_jerk = jerk;
    return limits;
}

int init_buffer(MotionBuffer &buffer, uint8_t axes, double cycle_time, const MotionFrame &frame) {
    int ret = buffer.init(axes);
    if (ret == MOTION_BUFFER_OK) {
        buffer.set_cycle_time(cycle_time);
        ret = buffer.set_current_frame(frame);
    }
    return ret;
}

// Return only a real motion error; completion is represented by queued_count().
int run_cycle(MotionBuffer &buffer, MotionAxOutput *output) {
    buffer.runCycle();
    buffer.getOutput(output, buffer.axis_count());
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        if (output[axis].errorCode != 0) {
            return output[axis].errorCode;
        }
    }
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        if (output[axis].state == MOTION_AXIS_GROUP_ERRORSTOP) {
            // External ERRORSTOP has no public group error code.
            return ZMC_ERR_MOTBUF_PLAN_FAILED;
        }
    }
    return MOTION_BUFFER_OK;
}

void expect_axis(const MotionAxOutput &output, uint8_t axis, double position, double velocity,
                 double acceleration, MotionAxisGroupState state, uint16_t error_code,
                 double tolerance) {
    EXPECT_EQ(output.idx, axis);
    EXPECT_EQ(output.state, state);
    EXPECT_EQ(output.errorCode, error_code);
    EXPECT_NEAR(output.pos, position, tolerance);
    EXPECT_NEAR(output.vel, velocity, tolerance);
    EXPECT_NEAR(output.acc, acceleration, tolerance);
}

void expect_finite(const MotionBuffer &buffer, const MotionAxOutput *output) {
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        EXPECT_TRUE(std::isfinite(output[axis].pos)) << "axis=" << static_cast<int>(axis);
        EXPECT_TRUE(std::isfinite(output[axis].vel)) << "axis=" << static_cast<int>(axis);
        EXPECT_TRUE(std::isfinite(output[axis].acc)) << "axis=" << static_cast<int>(axis);
    }
}

void expect_healthy(const MotionBuffer &buffer, const MotionAxOutput *output) {
    expect_finite(buffer, output);
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        EXPECT_NE(output[axis].state, MOTION_AXIS_GROUP_ERRORSTOP);
        EXPECT_EQ(output[axis].errorCode, 0);
    }
}

void expect_group(const MotionBuffer &buffer, MotionAxisGroupState state, uint16_t error_code) {
    MotionAxOutput output[64] = {};
    buffer.getOutput(output, buffer.axis_count());
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        EXPECT_EQ(buffer.state(axis), state);
        EXPECT_EQ(output[axis].idx, axis);
        EXPECT_EQ(output[axis].state, state);
        EXPECT_EQ(output[axis].errorCode, error_code);
    }
}

// Verify every terminal field and prove that an additional empty cycle is stable.
void expect_normal_endpoint(MotionBuffer &buffer, const double *target, double tolerance) {
    MotionAxOutput before[64] = {};
    buffer.getOutput(before, buffer.axis_count());
    expect_finite(buffer, before);
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        expect_axis(before[axis], axis, target[axis], 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0,
                    tolerance);
        EXPECT_EQ(buffer.state(axis), MOTION_AXIS_GROUP_STANDSTILL);
    }
    EXPECT_EQ(buffer.queued_count(), 0);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount);

    MotionAxOutput after[64] = {};
    EXPECT_EQ(run_cycle(buffer, after), MOTION_BUFFER_OK);
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        expect_axis(after[axis], axis, before[axis].pos, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0,
                    tolerance);
    }
    EXPECT_EQ(buffer.queued_count(), 0);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount);
}

void expect_reset_state(MotionBuffer &buffer) {
    double zero[64] = {};
    buffer.reset();
    EXPECT_EQ(buffer.base_count(), buffer.axis_count());
    expect_normal_endpoint(buffer, zero, kExactTolerance);
}

bool run_until_empty(MotionBuffer &buffer, MotionAxOutput *output, int max_cycles) {
    for (int cycle = 0; cycle < max_cycles; ++cycle) {
        const int ret = run_cycle(buffer, output);
        if (ret != MOTION_BUFFER_OK) {
            ADD_FAILURE() << "motion error " << ret << " at cycle " << cycle;
            return false;
        }
        expect_healthy(buffer, output);
        if (buffer.queued_count() == 0) {
            return true;
        }
    }
    ADD_FAILURE() << "motion exceeded derived cycle limit " << max_cycles;
    return false;
}

double rest_duration(double distance, double speed, double acceleration) {
    const double full_speed_distance = speed * speed / acceleration;
    return distance <= full_speed_distance
               ? 2.0 * std::sqrt(distance / acceleration)
               : 2.0 * speed / acceleration + (distance - full_speed_distance) / speed;
}

int cycle_limit(double duration, double cycle_time) {
    return static_cast<int>(std::ceil(duration / cycle_time)) + 2;
}

void set_axis_states(MotionBuffer &buffer, const MotionAxisState *states) {
    MotionAxInput input[64] = {};
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        input[axis].idx = axis;
        input[axis].state = states[axis];
    }
    buffer.setInput(input, buffer.axis_count());
}

}  // namespace

// Initialization exposes a complete zero frame and rejects short output buffers atomically.
TEST(MotionBuffer, InitProvidesZeroOutputAndIgnoresInvalidOutputFrame) {
    MotionBuffer buffer;
    MotionAxOutput output[2] = {};
    ASSERT_EQ(buffer.init(2), MOTION_BUFFER_OK);
    buffer.getOutput(output, 2);
    for (uint8_t axis = 0; axis < 2; ++axis) {
        expect_axis(output[axis], axis, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0,
                    kExactTolerance);
    }

    MotionAxOutput sentinel[2] = {};
    for (uint8_t axis = 0; axis < 2; ++axis) {
        sentinel[axis].idx = 99;
        sentinel[axis].state = MOTION_AXIS_GROUP_ERRORSTOP;
        sentinel[axis].errorCode = 1234;
        sentinel[axis].pos = 11.0;
        sentinel[axis].vel = 12.0;
        sentinel[axis].acc = 13.0;
    }
    buffer.getOutput(NULL, 2);
    buffer.getOutput(sentinel, 1);
    for (uint8_t axis = 0; axis < 2; ++axis) {
        expect_axis(sentinel[axis], 99, 11.0, 12.0, 13.0, MOTION_AXIS_GROUP_ERRORSTOP, 1234,
                    kExactTolerance);
    }
    buffer.getOutput(output, 2);
    for (uint8_t axis = 0; axis < 2; ++axis) {
        expect_axis(output[axis], axis, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0,
                    kExactTolerance);
    }
    EXPECT_EQ(buffer.queued_count(), 0);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount);
}

// Actual feedback is accepted without replacing the retained command PVA.
TEST(MotionBuffer, InputActualPositionDoesNotReplaceCommandPosition) {
    MotionBuffer buffer;
    MotionAxInput input[2] = {};
    MotionAxOutput output[2] = {};
    ASSERT_EQ(buffer.init(2), MOTION_BUFFER_OK);
    input[0].idx = 0;
    input[0].state = MOTION_AXIS_STANDSTILL;
    input[0].actPos = 12.0;
    input[1].idx = 1;
    input[1].state = MOTION_AXIS_STANDSTILL;
    input[1].actPos = -3.0;
    buffer.setInput(input, 2);
    buffer.getOutput(output, 2);
    for (uint8_t axis = 0; axis < 2; ++axis) {
        expect_axis(output[axis], axis, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0,
                    kExactTolerance);
    }
    EXPECT_EQ(buffer.queued_count(), 0);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount);
}

// The line follows the complete analytical triangular velocity profile.
TEST(MotionBuffer, LineRunsDirectlyAfterPush) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    double target[] = {1.0, 0.0};
    MotionBuffer buffer;
    const double dt = 0.1;
    ASSERT_EQ(init_buffer(buffer, 2, dt, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 1.0)),
              MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.queued_count(), 1);
    expect_group(buffer, MOTION_AXIS_GROUP_STANDSTILL, 0);

    MotionAxOutput output[2] = {};
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    expect_axis(output[0], 0, 0.005, 0.1, 1.0, MOTION_AXIS_GROUP_MOVING, 0, kLinearTolerance);
    expect_axis(output[1], 1, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_MOVING, 0, kExactTolerance);
    double previous = output[0].pos;
    const int max_cycles = cycle_limit(rest_duration(1.0, 2.0, 1.0), dt);
    for (int cycle = 2; cycle <= max_cycles && buffer.queued_count() != 0; ++cycle) {
        ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
        expect_healthy(buffer, output);
        EXPECT_GE(output[0].pos + kExactTolerance, previous);
        EXPECT_NEAR(output[1].pos, 0.0, kExactTolerance);
        EXPECT_NEAR(output[1].vel, 0.0, kExactTolerance);
        EXPECT_NEAR(output[1].acc, 0.0, kExactTolerance);
        EXPECT_LE(std::fabs(output[0].acc), 1.0 + kLinearTolerance);
        if (cycle == 9) {
            expect_axis(output[0], 0, 0.405, 0.9, 1.0, MOTION_AXIS_GROUP_MOVING, 0,
                        kLinearTolerance);
        } else if (cycle == 10) {
            EXPECT_NEAR(output[0].pos, 0.5, kLinearTolerance);
            EXPECT_NEAR(output[0].vel, 1.0, kLinearTolerance);
        } else if (cycle == 11) {
            expect_axis(output[0], 0, 0.595, 0.9, -1.0, MOTION_AXIS_GROUP_MOVING, 0,
                        kLinearTolerance);
        }
        previous = output[0].pos;
    }
    ASSERT_EQ(buffer.queued_count(), 0);
    expect_normal_endpoint(buffer, target, kLinearTolerance);
}

// Circular output contains exact tangent velocity and inward-normal acceleration terms.
TEST(MotionBuffer, CircleOutputsAxisPva) {
    double position0[] = {1.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    double center[] = {0.0, 0.0};
    double target[] = {0.0, 1.0};
    MotionBuffer buffer;
    const double dt = 0.1;
    ASSERT_EQ(init_buffer(buffer, 2, dt, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_arc_center(center, target, MOTION_ARC_COUNTERCLOCKWISE,
                                     MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 1.0)),
              MOTION_BUFFER_OK);
    MotionAxOutput output[2] = {};
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    const double angle = 0.005;
    EXPECT_NEAR(output[0].pos, std::cos(angle), kLinearTolerance);
    EXPECT_NEAR(output[1].pos, std::sin(angle), kLinearTolerance);
    EXPECT_NEAR(output[0].vel, -std::sin(angle) * 0.1, kLinearTolerance);
    EXPECT_NEAR(output[1].vel, std::cos(angle) * 0.1, kLinearTolerance);
    EXPECT_NEAR(output[0].acc, -std::cos(angle) * 0.01 - std::sin(angle), kLinearTolerance);
    EXPECT_NEAR(output[1].acc, -std::sin(angle) * 0.01 + std::cos(angle), kLinearTolerance);
    expect_healthy(buffer, output);
    const double arc_length = 0.5 * std::acos(-1.0);
    ASSERT_TRUE(
        run_until_empty(buffer, output, cycle_limit(rest_duration(arc_length, 2.0, 1.0), dt)));
    expect_normal_endpoint(buffer, target, kLinearTolerance);
}

// Tangent segments preserve the exact path speed at the real 2-to-1 queue transition.
TEST(MotionBuffer, TangentSegmentsRunContinuously) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    double first_end[] = {1.0, 0.0};
    double target[] = {2.0, 0.0};
    MotionBuffer buffer;
    const double dt = 0.002;
    ASSERT_EQ(init_buffer(buffer, 2, dt, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(first_end, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 5.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 5.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.queued_count(), 2);

    MotionAxOutput output[2] = {};
    MotionAxOutput previous[2] = {};
    bool saw_junction = false;
    int32_t previous_queue = 2;
    const int max_cycles = cycle_limit(2.2, dt);
    for (int cycle = 0; cycle < max_cycles && buffer.queued_count() != 0; ++cycle) {
        previous[0] = output[0];
        previous[1] = output[1];
        ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
        expect_healthy(buffer, output);
        EXPECT_NEAR(output[1].pos, 0.0, kExactTolerance);
        EXPECT_NEAR(output[1].vel, 0.0, kExactTolerance);
        EXPECT_NEAR(output[1].acc, 0.0, kExactTolerance);
        EXPECT_LE(std::fabs(output[0].acc), 5.0 + kLinearTolerance);
        if (!saw_junction && previous_queue == 2 && buffer.queued_count() == 1) {
            saw_junction = true;
            EXPECT_LT(previous[0].pos, 1.0);
            EXPECT_NEAR(previous[0].vel, 1.0, kLinearTolerance);
            expect_axis(output[0], 0, 1.0, 1.0, 0.0, MOTION_AXIS_GROUP_MOVING, 0,
                        kLinearTolerance);
            expect_axis(output[1], 1, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_MOVING, 0, kExactTolerance);
            MotionAxOutput after[2] = {};
            ASSERT_EQ(run_cycle(buffer, after), MOTION_BUFFER_OK);
            expect_healthy(buffer, after);
            EXPECT_GT(after[0].pos, 1.0);
            EXPECT_NEAR(after[0].vel, 1.0, kLinearTolerance);
            EXPECT_NEAR(after[0].acc, 0.0, kLinearTolerance);
            output[0] = after[0];
            output[1] = after[1];
        }
        previous_queue = buffer.queued_count();
    }
    ASSERT_TRUE(saw_junction);
    ASSERT_EQ(buffer.queued_count(), 0);
    expect_normal_endpoint(buffer, target, kLinearTolerance);
}

// A corner has an exact zero-speed shared sample before the second segment accelerates.
TEST(MotionBuffer, CornerStopsAtJunction) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    double first_end[] = {1.0, 0.0};
    double target[] = {1.0, 1.0};
    MotionBuffer buffer;
    const double dt = 0.001;
    ASSERT_EQ(init_buffer(buffer, 2, dt, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(first_end, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 5.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 5.0)),
              MOTION_BUFFER_OK);

    MotionAxOutput output[2] = {};
    MotionAxOutput previous_output[2] = {};
    bool saw_junction = false;
    int32_t previous_queue = 2;
    double previous_x = 0.0;
    double previous_y = 0.0;
    const int max_cycles = cycle_limit(2.4, dt);
    for (int cycle = 0; cycle < max_cycles && buffer.queued_count() != 0; ++cycle) {
        previous_output[0] = output[0];
        previous_output[1] = output[1];
        ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
        expect_healthy(buffer, output);
        EXPECT_GE(output[0].pos + kExactTolerance, previous_x);
        EXPECT_GE(output[1].pos + kExactTolerance, previous_y);
        if (!saw_junction && previous_queue == 2 && buffer.queued_count() == 1) {
            saw_junction = true;
            // The exact shared point is the sample immediately before the queue pop.
            expect_axis(previous_output[0], 0, 1.0, 0.0, 0.0, MOTION_AXIS_GROUP_MOVING, 0,
                        kLinearTolerance);
            expect_axis(previous_output[1], 1, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_MOVING, 0,
                        kLinearTolerance);
            EXPECT_NEAR(output[0].pos, 1.0, kLinearTolerance);
            EXPECT_NEAR(output[0].vel, 0.0, kLinearTolerance);
            EXPECT_NEAR(output[0].acc, 0.0, kLinearTolerance);
            EXPECT_NEAR(output[1].pos, 0.5 * 5.0 * dt * dt, kLinearTolerance);
            EXPECT_NEAR(output[1].vel, 5.0 * dt, kLinearTolerance);
            EXPECT_NEAR(output[1].acc, 5.0, kLinearTolerance);
        }
        previous_queue = buffer.queued_count();
        previous_x = output[0].pos;
        previous_y = output[1].pos;
    }
    ASSERT_TRUE(saw_junction);
    ASSERT_EQ(buffer.queued_count(), 0);
    expect_normal_endpoint(buffer, target, kLinearTolerance);
}

// Runtime insertion continues the analytical PVA and executes the appended target completely.
TEST(MotionBuffer, RuntimeAppendReplansRemainingMotion) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    double first_end[] = {2.0, 0.0};
    double target[] = {4.0, 0.0};
    MotionBuffer buffer;
    const double dt = 0.01;
    ASSERT_EQ(init_buffer(buffer, 2, dt, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(first_end, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);
    MotionAxOutput output[2] = {};
    for (int cycle = 1; cycle <= 20; ++cycle) {
        ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
        const double time = cycle * dt;
        expect_healthy(buffer, output);
        EXPECT_NEAR(output[0].pos, time * time, kLinearTolerance);
        EXPECT_NEAR(output[0].vel, 2.0 * time, kLinearTolerance);
        EXPECT_NEAR(output[0].acc, 2.0, kLinearTolerance);
        EXPECT_NEAR(output[1].pos, 0.0, kExactTolerance);
        EXPECT_NEAR(output[1].vel, 0.0, kExactTolerance);
        EXPECT_NEAR(output[1].acc, 0.0, kExactTolerance);
    }
    const MotionAxOutput before = output[0];
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.queued_count(), 2);
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    EXPECT_NEAR(output[0].pos, before.pos + before.vel * dt + dt * dt, kLinearTolerance);
    EXPECT_NEAR(output[0].vel, before.vel + 2.0 * dt, kLinearTolerance);
    EXPECT_NEAR(output[0].acc, 2.0, kLinearTolerance);
    EXPECT_EQ(buffer.queued_count(), 2);
    ASSERT_TRUE(run_until_empty(buffer, output, cycle_limit(4.5, dt)));
    expect_normal_endpoint(buffer, target, kLinearTolerance);
}

// Capacity accounting distinguishes active segments from deferred command nodes.
TEST(MotionBuffer, ReportsBufferErrors) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    MotionBuffer buffer;
    const int32_t slots = MOTION_BUFFER_SLOT_CAPACITY;
    ASSERT_EQ(init_buffer(buffer, 2, 0.01, initial), MOTION_BUFFER_OK);
    MotionAxOutput output[2] = {};
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    for (uint8_t axis = 0; axis < 2; ++axis) {
        expect_axis(output[axis], axis, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0,
                    kExactTolerance);
    }
    for (int32_t i = 0; i < slots; ++i) {
        double target[] = {static_cast<double>(i + 1), 0.0};
        ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0)),
                  MOTION_BUFFER_OK);
        EXPECT_EQ(buffer.queued_count(), i + 1);
        EXPECT_EQ(buffer.free_count(), kFullPoolCount - i - 1);
    }
    EXPECT_EQ(buffer.queued_count(), slots);
    EXPECT_EQ(buffer.free_count(), slots);
    for (int32_t i = 0; i < slots; ++i) {
        double target[] = {static_cast<double>(slots + i + 1), 0.0};
        ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0)),
                  MOTION_BUFFER_OK);
        EXPECT_EQ(buffer.queued_count(), slots + i + 1);
        EXPECT_EQ(buffer.free_count(), slots - i - 1);
    }
    double overflow[] = {static_cast<double>(kFullPoolCount + 1), 0.0};
    EXPECT_EQ(buffer.push_line(overflow, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0)),
              ZMC_ERR_MOTBUF_FULL);
    EXPECT_EQ(buffer.queued_count(), kFullPoolCount);
    EXPECT_EQ(buffer.free_count(), 0);
    expect_reset_state(buffer);
}

// Axis feedback aggregates across both axes, and external ERRORSTOP remains latched.
TEST(MotionBuffer, AggregatesAxisStateAndLatchesErrorStop) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    double target[] = {1.0, 0.0};
    MotionAxisState states[] = {MOTION_AXIS_DISABLE, MOTION_AXIS_DISABLE};
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 2, 0.01, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0)),
              MOTION_BUFFER_OK);
    set_axis_states(buffer, states);
    expect_group(buffer, MOTION_AXIS_GROUP_DISABLE, 0);
    states[1] = MOTION_AXIS_STANDSTILL;
    set_axis_states(buffer, states);
    expect_group(buffer, MOTION_AXIS_GROUP_STANDSTILL, 0);
    states[0] = MOTION_AXIS_MOVING;
    set_axis_states(buffer, states);
    expect_group(buffer, MOTION_AXIS_GROUP_MOVING, 0);
    states[1] = MOTION_AXIS_STOPPING;
    set_axis_states(buffer, states);
    expect_group(buffer, MOTION_AXIS_GROUP_STOPPING, 0);
    states[0] = MOTION_AXIS_ERRORSTOP;
    set_axis_states(buffer, states);
    expect_group(buffer, MOTION_AXIS_GROUP_ERRORSTOP, 0);
    states[0] = MOTION_AXIS_STANDSTILL;
    states[1] = MOTION_AXIS_STANDSTILL;
    set_axis_states(buffer, states);
    expect_group(buffer, MOTION_AXIS_GROUP_ERRORSTOP, 0);

    MotionAxOutput output[2] = {};
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_MOTBUF_PLAN_FAILED);
    for (uint8_t axis = 0; axis < 2; ++axis) {
        expect_axis(output[axis], axis, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP, 0,
                    kExactTolerance);
    }
    EXPECT_EQ(buffer.queued_count(), 1);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount - 1);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_MOTBUF_PLAN_FAILED);
    for (uint8_t axis = 0; axis < 2; ++axis) {
        expect_axis(output[axis], axis, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP, 0,
                    kExactTolerance);
    }
    expect_reset_state(buffer);
}

// Reserved override inputs leave every sample of the baseline profile unchanged.
TEST(MotionBuffer, OverrideInterfaceIsNoOp) {
    double position0[] = {0.0};
    double zero[] = {0.0};
    const MotionFrame initial = {position0, zero, zero};
    double target[] = {1.0};
    MotionBuffer buffer;
    EXPECT_EQ(buffer.set_override(MOTION_OVERRIDE_VELOCITY, 1.0), ZMC_ERR_MOTBUF_NOT_INITIALIZED);
    const double dt = 0.1;
    ASSERT_EQ(init_buffer(buffer, 1, dt, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 1.0)),
              MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.set_override(MOTION_OVERRIDE_VELOCITY, -1.0), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.set_override(MOTION_OVERRIDE_ACCELERATION, 0.0), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.set_override(MOTION_OVERRIDE_JERK, 123.0), MOTION_BUFFER_OK);
    MotionAxOutput output[1] = {};
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    expect_axis(output[0], 0, 0.005, 0.1, 1.0, MOTION_AXIS_GROUP_MOVING, 0, kLinearTolerance);
    ASSERT_TRUE(run_until_empty(buffer, output, cycle_limit(rest_duration(1.0, 2.0, 1.0), dt)));
    expect_normal_endpoint(buffer, target, kLinearTolerance);
}

// Positive saved velocity integrates to the exact stable software-stop position.
TEST(MotionBuffer, SoftwareErrorStopUsesSavedPositivePva) {
    double position0[] = {5.0};
    double velocity0[] = {2.0};
    double acceleration0[] = {-1500.0};
    const MotionFrame initial = {position0, velocity0, acceleration0};
    double target[] = {10.0};
    MotionAxisState states[] = {MOTION_AXIS_ERRORSTOP};
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 1, 0.001, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(3.0, 2.0)),
              MOTION_BUFFER_OK);
    set_axis_states(buffer, states);
    MotionAxOutput output[1] = {};
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_MOTBUF_PLAN_FAILED);
    expect_axis(output[0], 0, 5.00125, 0.5, -1500.0, MOTION_AXIS_GROUP_ERRORSTOP, 0,
                kExactTolerance);
    EXPECT_EQ(buffer.queued_count(), 1);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_MOTBUF_PLAN_FAILED);
    expect_axis(output[0], 0, 5.001333333333333, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP, 0,
                kExactTolerance);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_MOTBUF_PLAN_FAILED);
    expect_axis(output[0], 0, 5.001333333333333, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP, 0,
                kExactTolerance);
    EXPECT_EQ(buffer.queued_count(), 1);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount - 1);
    expect_reset_state(buffer);
}

// Negative saved velocity brakes without crossing zero or reversing direction.
TEST(MotionBuffer, SoftwareErrorStopUsesSavedNegativePva) {
    double position0[] = {0.0};
    double velocity0[] = {-2.0};
    double acceleration0[] = {1500.0};
    const MotionFrame initial = {position0, velocity0, acceleration0};
    double target[] = {-10.0};
    MotionAxisState states[] = {MOTION_AXIS_ERRORSTOP};
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 1, 0.001, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(3.0, 2.0)),
              MOTION_BUFFER_OK);
    set_axis_states(buffer, states);
    MotionAxOutput output[1] = {};
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_MOTBUF_PLAN_FAILED);
    expect_axis(output[0], 0, -0.00125, -0.5, 1500.0, MOTION_AXIS_GROUP_ERRORSTOP, 0,
                kExactTolerance);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_MOTBUF_PLAN_FAILED);
    expect_axis(output[0], 0, -0.001333333333333, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP, 0,
                kExactTolerance);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_MOTBUF_PLAN_FAILED);
    expect_axis(output[0], 0, -0.001333333333333, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP, 0,
                kExactTolerance);
    EXPECT_EQ(buffer.queued_count(), 1);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount - 1);
    expect_reset_state(buffer);
}

// A failed initial geometry remains loaded until its saved PVA reaches a stable stop.
TEST(MotionBuffer, EmptyErrorStopGroupRemainsLoaded) {
    double position0[] = {1.0, 0.0};
    double velocity0[] = {2.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, velocity0, zero};
    double center[] = {0.0, 0.0};
    double opposite[] = {-1.0, 0.0};
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 2, 0.001, initial), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.push_arc_center(center, opposite, MOTION_ARC_COUNTERCLOCKWISE,
                                     MOTION_COORDINATE_ABSOLUTE, make_limits(3.0, 2.0)),
              ZMC_ERR_TRAJ_BUILD_FAILED);
    ASSERT_EQ(buffer.queued_count(), 0);
    ASSERT_EQ(buffer.free_count(), kFullPoolCount);
    expect_group(buffer, MOTION_AXIS_GROUP_ERRORSTOP,
                 static_cast<uint16_t>(ZMC_ERR_TRAJ_BUILD_FAILED));
    MotionAxOutput output[2] = {};
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_TRAJ_BUILD_FAILED);
    expect_axis(output[0], 0, 1.0015, 1.0, -1000.0, MOTION_AXIS_GROUP_ERRORSTOP,
                static_cast<uint16_t>(ZMC_ERR_TRAJ_BUILD_FAILED), kExactTolerance);
    expect_axis(output[1], 1, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP,
                static_cast<uint16_t>(ZMC_ERR_TRAJ_BUILD_FAILED), kExactTolerance);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_TRAJ_BUILD_FAILED);
    expect_axis(output[0], 0, 1.002, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP,
                static_cast<uint16_t>(ZMC_ERR_TRAJ_BUILD_FAILED), kExactTolerance);
    expect_axis(output[1], 1, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP,
                static_cast<uint16_t>(ZMC_ERR_TRAJ_BUILD_FAILED), kExactTolerance);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_TRAJ_BUILD_FAILED);
    EXPECT_NEAR(output[0].pos, 1.002, kExactTolerance);
    EXPECT_NEAR(output[0].vel, 0.0, kExactTolerance);
    EXPECT_NEAR(output[0].acc, 0.0, kExactTolerance);
    expect_reset_state(buffer);
}

// Deferred geometry failure starts software braking in the dispatch cycle itself.
TEST(MotionBuffer, PendingDispatchFailureStartsErrorStopSameCycle) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    MotionBuffer buffer;
    const int32_t slots = MOTION_BUFFER_SLOT_CAPACITY;
    ASSERT_EQ(init_buffer(buffer, 2, 1.0, initial), MOTION_BUFFER_OK);
    for (int32_t i = 0; i < slots; ++i) {
        double target[] = {static_cast<double>(i + 1), 0.0};
        ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(10.0, 100.0)),
                  MOTION_BUFFER_OK);
    }
    double center[] = {0.0, 0.0};
    double opposite[] = {-1.0, 0.0};
    ASSERT_EQ(buffer.push_arc_center(center, opposite, MOTION_ARC_COUNTERCLOCKWISE,
                                     MOTION_COORDINATE_ABSOLUTE, make_limits(3.0, 2.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.queued_count(), slots + 1);
    MotionAxOutput output[2] = {};
    bool pending_only = false;
    const int max_cycles = cycle_limit(slots * rest_duration(1.0, 10.0, 100.0), 1.0) + 2;
    for (int cycle = 0; cycle < max_cycles; ++cycle) {
        ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
        expect_healthy(buffer, output);
        if (buffer.queued_count() == 1 && buffer.state(0) == MOTION_AXIS_GROUP_STANDSTILL) {
            pending_only = true;
            break;
        }
    }
    ASSERT_TRUE(pending_only);
    expect_axis(output[0], 0, static_cast<double>(slots), 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL,
                0, kLinearTolerance);
    expect_axis(output[1], 1, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0, kExactTolerance);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount - 1);

    // The pending command binds against this restored frame on the next cycle.
    buffer.set_cycle_time(0.001);
    double restore_position[] = {1.0, 0.0};
    double restore_velocity[] = {2.0, 0.0};
    const MotionFrame restore = {restore_position, restore_velocity, zero};
    ASSERT_EQ(buffer.set_current_frame(restore), MOTION_BUFFER_OK);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_TRAJ_BUILD_FAILED);
    expect_axis(output[0], 0, 1.0015, 1.0, -1000.0, MOTION_AXIS_GROUP_ERRORSTOP,
                static_cast<uint16_t>(ZMC_ERR_TRAJ_BUILD_FAILED), kExactTolerance);
    expect_axis(output[1], 1, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP,
                static_cast<uint16_t>(ZMC_ERR_TRAJ_BUILD_FAILED), kExactTolerance);
    EXPECT_EQ(buffer.queued_count(), 0);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_TRAJ_BUILD_FAILED);
    expect_axis(output[0], 0, 1.002, 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP,
                static_cast<uint16_t>(ZMC_ERR_TRAJ_BUILD_FAILED), kExactTolerance);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_TRAJ_BUILD_FAILED);
    EXPECT_NEAR(output[0].pos, 1.002, kExactTolerance);
    EXPECT_NEAR(output[0].vel, 0.0, kExactTolerance);
    EXPECT_NEAR(output[0].acc, 0.0, kExactTolerance);
    expect_reset_state(buffer);
}

// A long cycle consumes both short segments and publishes the complete terminal frame.
TEST(MotionBuffer, OneCycleCanFinishMultipleSegments) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    double first_end[] = {0.1, 0.0};
    double target[] = {0.2, 0.0};
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 2, 1.0, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(first_end, MOTION_COORDINATE_ABSOLUTE, make_limits(10.0, 100.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(10.0, 100.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.queued_count(), 2);
    MotionAxOutput output[2] = {};
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.queued_count(), 0);
    expect_axis(output[0], 0, 0.2, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0, kExactTolerance);
    expect_axis(output[1], 1, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0, kExactTolerance);
    expect_normal_endpoint(buffer, target, kExactTolerance);
}

// The 0.2 s cycle carries exactly 0.05 s beyond the first 0.15 s segment.
TEST(MotionBuffer, OneCycleCarriesRemainingTimeIntoNextSegment) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    double first_end[] = {0.1, 0.0};
    double target[] = {1.1, 0.0};
    MotionBuffer buffer;
    const double dt = 0.2;
    ASSERT_EQ(init_buffer(buffer, 2, dt, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(first_end, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 10.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 10.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.queued_count(), 2);
    MotionAxOutput output[2] = {};
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.queued_count(), 1);
    expect_axis(output[0], 0, 0.15, 1.0, 0.0, MOTION_AXIS_GROUP_MOVING, 0, kLinearTolerance);
    expect_axis(output[1], 1, 0.0, 0.0, 0.0, MOTION_AXIS_GROUP_MOVING, 0, kExactTolerance);
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.queued_count(), 1);
    expect_axis(output[0], 0, 0.35, 1.0, 0.0, MOTION_AXIS_GROUP_MOVING, 0, kLinearTolerance);
    ASSERT_TRUE(run_until_empty(buffer, output, cycle_limit(0.85, dt) + 1));
    expect_normal_endpoint(buffer, target, kLinearTolerance);
}

// The shared trajectory storage is reused by a two-axis arc and then a four-axis line.
TEST(MotionBuffer, SharedTrajectoryHeapSupportsArcAndBaseDimensionReuse) {
    double position0[] = {1.0, 0.0, 0.0, 0.0};
    double zero[] = {0.0, 0.0, 0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    MotionBuffer buffer;
    const double dt = 0.01;
    ASSERT_EQ(init_buffer(buffer, 4, dt, initial), MOTION_BUFFER_OK);
    const uint8_t base01[] = {0, 1};
    double center[] = {0.0, 0.0};
    double arc_target[] = {0.0, 1.0, 0.0, 0.0};
    ASSERT_EQ(buffer.set_base(base01, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_arc_center(center, arc_target, MOTION_ARC_COUNTERCLOCKWISE,
                                     MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 4.0)),
              MOTION_BUFFER_OK);
    MotionAxOutput output[4] = {};
    const double arc_length = 0.5 * std::acos(-1.0);
    ASSERT_TRUE(
        run_until_empty(buffer, output, cycle_limit(rest_duration(arc_length, 2.0, 4.0), dt)));
    expect_normal_endpoint(buffer, arc_target, kLinearTolerance);

    const uint8_t base0123[] = {0, 1, 2, 3};
    double line_target[] = {0.0, 1.0, 1.0, 0.0};
    ASSERT_EQ(buffer.set_base(base0123, 4), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(line_target, MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 4.0)),
              MOTION_BUFFER_OK);
    ASSERT_TRUE(run_until_empty(buffer, output, cycle_limit(rest_duration(1.0, 2.0, 4.0), dt)));
    expect_normal_endpoint(buffer, line_target, kLinearTolerance);
}

// Reinitialization rejects an invalid frame without mutation and restores all defaults.
TEST(MotionBuffer, InitCanBeReenteredToResize) {
    double position0[] = {3.0, -4.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame invalid = {NULL, zero, zero};
    const MotionFrame valid = {position0, zero, zero};
    MotionBuffer buffer;
    buffer.reset();
    ASSERT_EQ(buffer.init(2), MOTION_BUFFER_OK);
    MotionAxOutput before[2] = {};
    buffer.getOutput(before, 2);
    EXPECT_EQ(buffer.set_current_frame(invalid), ZMC_ERR_MOTBUF_INVALID_ARGUMENT);
    MotionAxOutput after[2] = {};
    buffer.getOutput(after, 2);
    for (uint8_t axis = 0; axis < 2; ++axis) {
        expect_axis(after[axis], axis, before[axis].pos, before[axis].vel, before[axis].acc,
                    MOTION_AXIS_GROUP_STANDSTILL, 0, kExactTolerance);
    }
    buffer.set_cycle_time(0.01);
    ASSERT_EQ(buffer.set_current_frame(valid), MOTION_BUFFER_OK);
    buffer.getOutput(after, 2);
    expect_axis(after[0], 0, 3.0, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0, kExactTolerance);
    expect_axis(after[1], 1, -4.0, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0, kExactTolerance);
    ASSERT_EQ(buffer.init(2), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.base_count(), 2);
    EXPECT_EQ(buffer.queued_count(), 0);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount);
    double target[] = {0.0, 0.0};
    expect_normal_endpoint(buffer, target, kExactTolerance);
}

// Reset recycles active and pending nodes before a complete proportional reuse motion.
TEST(MotionBuffer, ResetRecyclesActiveAndPendingResources) {
    double position0[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {position0, zero, zero};
    const uint8_t base0[] = {0};
    const uint8_t base01[] = {0, 1};
    double first[] = {1.0};
    double pending[] = {2.0, 3.0};
    MotionAxisState states[] = {MOTION_AXIS_MOVING, MOTION_AXIS_STANDSTILL};
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 2, 0.01, initial), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_junction_mode(MOTION_JUNCTION_STOP_AT_CORNER), MOTION_BUFFER_OK);
    set_axis_states(buffer, states);
    ASSERT_EQ(buffer.set_base(base0, 1), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(first, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base01, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(pending, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.queued_count(), 2);
    ASSERT_EQ(buffer.free_count(), kFullPoolCount - 2);
    expect_reset_state(buffer);

    const double dt = 0.01;
    buffer.set_cycle_time(dt);
    double target[] = {7.0, 8.0};
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0)),
              MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.queued_count(), 1);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount - 1);
    MotionAxOutput output[2] = {};
    double previous_x = 0.0;
    double previous_y = 0.0;
    const double distance = std::sqrt(7.0 * 7.0 + 8.0 * 8.0);
    const int max_cycles = cycle_limit(rest_duration(distance, 1.0, 1.0), dt);
    for (int cycle = 0; cycle < max_cycles && buffer.queued_count() != 0; ++cycle) {
        ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
        expect_healthy(buffer, output);
        EXPECT_GE(output[0].pos + kExactTolerance, previous_x);
        EXPECT_GE(output[1].pos + kExactTolerance, previous_y);
        EXPECT_NEAR(output[0].pos * 8.0, output[1].pos * 7.0, kLinearTolerance);
        previous_x = output[0].pos;
        previous_y = output[1].pos;
    }
    ASSERT_EQ(buffer.queued_count(), 0);
    expect_normal_endpoint(buffer, target, kLinearTolerance);
    expect_reset_state(buffer);
}

// Seventeen tangent segments traverse the fixed sixteen-node lookahead window.
TEST(MotionBuffer, LookaheadWindowSlidesPastSixteenSegments) {
    const int32_t segment_count = 17;
    double position0[] = {0.0};
    double zero[] = {0.0};
    const MotionFrame initial = {position0, zero, zero};
    MotionBuffer buffer;
    const double dt = 0.01;
    ASSERT_EQ(init_buffer(buffer, 1, dt, initial), MOTION_BUFFER_OK);
    for (int32_t i = 0; i < segment_count; ++i) {
        double target[] = {static_cast<double>(i + 1)};
        ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(4.0, 16.0)),
                  MOTION_BUFFER_OK);
    }
    ASSERT_EQ(buffer.queued_count(), segment_count);
    MotionAxOutput output[1] = {};
    double previous_position = 0.0;
    int32_t previous_queue = segment_count;
    bool saw_last_window = false;
    const int max_cycles = cycle_limit(rest_duration(17.0, 4.0, 16.0), dt) + 2;
    for (int cycle = 0; cycle < max_cycles && buffer.queued_count() != 0; ++cycle) {
        ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
        expect_healthy(buffer, output);
        EXPECT_GE(output[0].pos + kExactTolerance, previous_position);
        if (!saw_last_window && previous_queue > 1 && buffer.queued_count() == 1) {
            saw_last_window = true;
            // The small cycle slides the window continuously and samples inside node 17.
            EXPECT_NEAR(output[0].pos, 16.02, kLinearTolerance);
            EXPECT_NEAR(output[0].vel, 4.0, kLinearTolerance);
            EXPECT_NEAR(output[0].acc, 0.0, kLinearTolerance);
            EXPECT_EQ(buffer.state(0), MOTION_AXIS_GROUP_MOVING);
        }
        previous_queue = buffer.queued_count();
        previous_position = output[0].pos;
    }
    ASSERT_TRUE(saw_last_window);
    ASSERT_EQ(buffer.queued_count(), 0);
    double target[] = {17.0};
    expect_normal_endpoint(buffer, target, kLinearTolerance);
}

// The seventeenth node waits for a fresh cycle after the planned horizon is exhausted.
TEST(MotionBuffer, PlannedHorizonExhaustionWaitsForNextCycle) {
    const int32_t segment_count = 17;
    double position0[] = {0.0};
    double zero[] = {0.0};
    const MotionFrame initial = {position0, zero, zero};
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 1, 1.0, initial), MOTION_BUFFER_OK);
    for (int32_t i = 0; i < segment_count; ++i) {
        double target[] = {static_cast<double>(i + 1) * 0.01};
        ASSERT_EQ(
            buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(100.0, 10000.0)),
            MOTION_BUFFER_OK);
    }
    MotionAxOutput output[1] = {};
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.queued_count(), 1);
    expect_axis(output[0], 0, 0.16, 0.0, 0.0, MOTION_AXIS_GROUP_MOVING, 0, kLinearTolerance);
    ASSERT_EQ(run_cycle(buffer, output), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.queued_count(), 0);
    expect_axis(output[0], 0, 0.17, 0.0, 0.0, MOTION_AXIS_GROUP_STANDSTILL, 0, kLinearTolerance);
    double target[] = {0.17};
    expect_normal_endpoint(buffer, target, kLinearTolerance);
}

// Deferred zero-length conversion reports the exact planner failure and stays stable.
TEST(MotionBuffer, ResetAcceptsErrorStopAndRecyclesPendingCommand) {
    double position0[] = {0.0};
    double zero[] = {0.0};
    const MotionFrame initial = {position0, zero, zero};
    MotionBuffer buffer;
    const int32_t slots = MOTION_BUFFER_SLOT_CAPACITY;
    ASSERT_EQ(init_buffer(buffer, 1, 1.0, initial), MOTION_BUFFER_OK);
    for (int32_t i = 0; i < slots; ++i) {
        double target[] = {static_cast<double>(i + 1)};
        ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(10.0, 100.0)),
                  MOTION_BUFFER_OK);
    }
    double parked[] = {static_cast<double>(slots)};
    ASSERT_EQ(buffer.push_line(parked, MOTION_COORDINATE_ABSOLUTE, make_limits(10.0, 100.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.queued_count(), slots + 1);
    MotionAxOutput output[1] = {};
    int ret = MOTION_BUFFER_OK;
    const int max_cycles = cycle_limit(slots * rest_duration(1.0, 10.0, 100.0), 1.0) + 3;
    for (int cycle = 0; cycle < max_cycles && ret == MOTION_BUFFER_OK; ++cycle) {
        ret = run_cycle(buffer, output);
        expect_finite(buffer, output);
    }
    ASSERT_EQ(ret, ZMC_ERR_MOTBUF_PLAN_FAILED);
    expect_axis(output[0], 0, static_cast<double>(slots), 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP,
                static_cast<uint16_t>(ZMC_ERR_MOTBUF_PLAN_FAILED), kLinearTolerance);
    EXPECT_EQ(buffer.queued_count(), 1);
    EXPECT_EQ(buffer.free_count(), kFullPoolCount - 1);
    EXPECT_EQ(run_cycle(buffer, output), ZMC_ERR_MOTBUF_PLAN_FAILED);
    expect_axis(output[0], 0, static_cast<double>(slots), 0.0, 0.0, MOTION_AXIS_GROUP_ERRORSTOP,
                static_cast<uint16_t>(ZMC_ERR_MOTBUF_PLAN_FAILED), kLinearTolerance);
    expect_reset_state(buffer);
}
