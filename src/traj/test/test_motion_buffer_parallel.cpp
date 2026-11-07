/*
 * @ArtifactStatus: draft
 * @ArtifactStatusDate: 2026-08-25
 * @ArtifactStatusAuthority: GPT-5 Codex
 */
#include "MotionBuffer.h"

#include <math.h>

#include "gtest/gtest.h"

using namespace zmc;

namespace {

MotionLimits make_limits(double speed, double acceleration, double jerk = 0.0) {
    MotionLimits limits;
    limits.max_speed = speed;
    limits.max_acceleration = acceleration;
    limits.max_deceleration = acceleration;
    limits.max_jerk = jerk;
    return limits;
}

enum MotionBufferTestCycleResult { TEST_CYCLE_RUNNING = 0, TEST_CYCLE_FINISHED = 1 };

int init_buffer(MotionBuffer &buffer, uint8_t axis_count, double cycle_time,
                const MotionFrame &frame) {
    int ret = buffer.init(axis_count);
    if (ret == MOTION_BUFFER_OK) {
        buffer.set_cycle_time(cycle_time);
    }
    if (ret == MOTION_BUFFER_OK) {
        ret = buffer.set_current_frame(frame);
    }
    return ret;
}

int run_cycle(MotionBuffer &buffer, double *position, double *velocity, double *acceleration) {
    buffer.runCycle();
    MotionAxOutput output[64] = {};
    buffer.getOutput(output, buffer.axis_count());
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        position[axis] = output[axis].pos;
        velocity[axis] = output[axis].vel;
        acceleration[axis] = output[axis].acc;
    }
    for (uint8_t axis = 0; axis < buffer.axis_count(); ++axis) {
        if (buffer.state(axis) == MOTION_AXIS_GROUP_ERRORSTOP) {
            return ZMC_ERR_MOTBUF_PLAN_FAILED;
        }
    }
    return buffer.queued_count() == 0 ? TEST_CYCLE_FINISHED : TEST_CYCLE_RUNNING;
}

MotionFrame zero_frame(double *position, uint8_t axes, double *velocity, double *acceleration) {
    for (uint8_t i = 0; i < axes; ++i) {
        velocity[i] = 0.0;
        acceleration[i] = 0.0;
    }
    MotionFrame frame;
    frame.position = position;
    frame.velocity = velocity;
    frame.acceleration = acceleration;
    return frame;
}

}  // namespace

// Disjoint BASE lists can advance in the same control cycle.
TEST(MotionBufferParallel, DisjointBasesMoveTogether) {
    double initial_position[] = {0.0, 0.0, 0.0, 0.0};
    double zero_v[4] = {};
    double zero_a[4] = {};
    const MotionFrame initial = zero_frame(initial_position, 4, zero_v, zero_a);
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 4, 0.1, initial), MOTION_BUFFER_OK);

    const uint8_t base01[] = {0, 1};
    const uint8_t base23[] = {2, 3};
    double target01[] = {1.0, 0.0};
    double target23[] = {1.0, 0.0};
    ASSERT_EQ(buffer.set_base(base01, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target01, MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 1.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base23, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target23, MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 1.0)),
              MOTION_BUFFER_OK);

    double position[4] = {};
    double velocity[4] = {};
    double acceleration[4] = {};
    ASSERT_EQ(run_cycle(buffer, position, velocity, acceleration), TEST_CYCLE_RUNNING);
    EXPECT_NEAR(position[0], 0.005, 1e-9);
    EXPECT_NEAR(position[2], 0.005, 1e-9);
    EXPECT_NEAR(position[1], 0.0, 1e-12);
    EXPECT_NEAR(position[3], 0.0, 1e-12);
}

// A partial BASE overlap waits for the previous live group before starting.
TEST(MotionBufferParallel, PartialOverlapWaitsThenStarts) {
    double initial_position[] = {0.0, 0.0, 0.0};
    double zero_v[3] = {};
    double zero_a[3] = {};
    const MotionFrame initial = zero_frame(initial_position, 3, zero_v, zero_a);
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 3, 0.01, initial), MOTION_BUFFER_OK);

    const uint8_t base01[] = {0, 1};
    const uint8_t base02[] = {0, 2};
    double first[] = {1.0, 0.0};
    double second[] = {2.0, 1.0};
    ASSERT_EQ(buffer.set_base(base01, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(first, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base02, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(second, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);

    double position[3] = {};
    double velocity[3] = {};
    double acceleration[3] = {};
    bool first_done = false;
    bool saw_axis2_hold = false;
    bool saw_axis2_move = false;
    for (int i = 0; i < 5000; ++i) {
        const int result = run_cycle(buffer, position, velocity, acceleration);
        ASSERT_GE(result, TEST_CYCLE_RUNNING);
        if (!first_done) {
            if (fabs(position[2]) > 1e-9) {
                FAIL() << "axis 2 moved before the first group finished";
            }
            if (fabs(position[0] - 1.0) < 1e-3 && fabs(velocity[0]) < 1e-6 &&
                fabs(velocity[1]) < 1e-6) {
                first_done = true;
                saw_axis2_hold = true;
            }
        } else if (position[2] > 1e-6) {
            saw_axis2_move = true;
            break;
        }
        if (result == TEST_CYCLE_FINISHED) {
            break;
        }
    }
    EXPECT_TRUE(saw_axis2_hold);
    EXPECT_TRUE(saw_axis2_move);
}

// Overlap against multiple live lanes waits until every prior group is idle.
TEST(MotionBufferParallel, GlobalWaitIdleWaitsAllGroups) {
    double initial_position[] = {0.0, 0.0, 0.0, 0.0};
    double zero_v[4] = {};
    double zero_a[4] = {};
    const MotionFrame initial = zero_frame(initial_position, 4, zero_v, zero_a);
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 4, 0.01, initial), MOTION_BUFFER_OK);

    const uint8_t base01[] = {0, 1};
    const uint8_t base23[] = {2, 3};
    const uint8_t base02[] = {0, 2};
    double target01[] = {1.0, 0.0};
    double target23[] = {2.0, 0.0};
    double target02[] = {1.0, 1.0};
    ASSERT_EQ(buffer.set_base(base01, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target01, MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 2.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base23, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target23, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base02, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target02, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0)),
              MOTION_BUFFER_OK);

    double position[4] = {};
    double velocity[4] = {};
    double acceleration[4] = {};
    bool axis0_stopped = false;
    bool axis0_held_while_23_moving = false;
    bool both_prior_idle = false;
    bool gated_started = false;
    for (int i = 0; i < 12000; ++i) {
        const int result = run_cycle(buffer, position, velocity, acceleration);
        ASSERT_GE(result, TEST_CYCLE_RUNNING);

        const bool axes01_idle =
            fabs(position[0] - 1.0) < 1e-3 && fabs(velocity[0]) < 1e-6 && fabs(velocity[1]) < 1e-6;
        const bool axes23_busy = fabs(velocity[2]) > 1e-6 || fabs(velocity[3]) > 1e-6 ||
                                 (position[2] > 1e-3 && fabs(position[2] - 2.0) > 1e-2);
        const bool axes23_idle =
            fabs(position[2] - 2.0) < 1e-2 && fabs(velocity[2]) < 1e-6 && fabs(velocity[3]) < 1e-6;

        if (!axis0_stopped && axes01_idle) {
            axis0_stopped = true;
        }
        if (axis0_stopped && axes23_busy) {
            EXPECT_NEAR(position[0], 1.0, 1e-3);
            axis0_held_while_23_moving = true;
        }
        if (axes01_idle && axes23_idle) {
            both_prior_idle = true;
        }
        if (both_prior_idle && position[2] < 1.95) {
            gated_started = true;
            break;
        }
        if (result == TEST_CYCLE_FINISHED) {
            break;
        }
    }
    EXPECT_TRUE(axis0_held_while_23_moving);
    EXPECT_TRUE(gated_started);
}

// Reordered BASE with the same axis set appends and remaps; a disjoint group keeps moving.
TEST(MotionBufferParallel, ReorderedBaseAppendsSameGroup) {
    double initial_position[] = {0.0, 0.0, 0.0, 0.0};
    double zero_v[4] = {};
    double zero_a[4] = {};
    const MotionFrame initial = zero_frame(initial_position, 4, zero_v, zero_a);
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 4, 0.01, initial), MOTION_BUFFER_OK);

    const uint8_t base01[] = {0, 1};
    const uint8_t base23[] = {2, 3};
    const uint8_t base10[] = {1, 0};
    double first[] = {1.0, 0.0};
    double slow23[] = {2.0, 0.0};
    double remapped[] = {2.0, 3.0};
    ASSERT_EQ(buffer.set_base(base01, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(first, MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 4.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base23, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(slow23, MOTION_COORDINATE_ABSOLUTE, make_limits(0.5, 0.5)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base10, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(remapped, MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 4.0)),
              MOTION_BUFFER_OK);

    double position[4] = {};
    double velocity[4] = {};
    double acceleration[4] = {};
    bool axis0_moved_while_23_busy = false;
    for (int i = 0; i < 12000; ++i) {
        const int result = run_cycle(buffer, position, velocity, acceleration);
        ASSERT_GE(result, TEST_CYCLE_RUNNING);
        const bool axes23_busy =
            fabs(velocity[2]) > 1e-6 || (position[2] > 1e-3 && position[2] < 1.9);
        if (axes23_busy && position[0] > 1.05) {
            axis0_moved_while_23_busy = true;
            break;
        }
        if (result == TEST_CYCLE_FINISHED) {
            break;
        }
    }
    EXPECT_TRUE(axis0_moved_while_23_busy);

    for (int i = 0; i < 20000; ++i) {
        const int result = run_cycle(buffer, position, velocity, acceleration);
        ASSERT_GE(result, TEST_CYCLE_RUNNING);
        if (result == TEST_CYCLE_FINISHED) {
            break;
        }
    }
    EXPECT_NEAR(position[0], 3.0, 1e-2);
    EXPECT_NEAR(position[1], 2.0, 1e-2);
}

// wait-idle blocks even a same-set append until every live group is idle.
TEST(MotionBufferParallel, WaitIdleBlocksSameSetAppend) {
    double initial_position[] = {0.0, 0.0, 0.0};
    double zero_v[3] = {};
    double zero_a[3] = {};
    const MotionFrame initial = zero_frame(initial_position, 3, zero_v, zero_a);
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 3, 0.01, initial), MOTION_BUFFER_OK);

    const uint8_t base01[] = {0, 1};
    const uint8_t base02[] = {0, 2};
    double first[] = {1.0, 0.0};
    double overlap[] = {2.0, 1.0};
    double later01[] = {3.0, 0.0};
    ASSERT_EQ(buffer.set_base(base01, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(first, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base02, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(overlap, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base01, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(later01, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);

    double position[3] = {};
    double velocity[3] = {};
    double acceleration[3] = {};
    bool first_done = false;
    bool overlap_started = false;
    bool later_held_during_first = true;
    bool later_held_during_overlap = true;
    for (int i = 0; i < 12000; ++i) {
        const int result = run_cycle(buffer, position, velocity, acceleration);
        ASSERT_GE(result, TEST_CYCLE_RUNNING);
        if (!first_done) {
            if (position[0] > 1.05) {
                later_held_during_first = false;
            }
            if (fabs(position[0] - 1.0) < 1e-3 && fabs(velocity[0]) < 1e-6 &&
                fabs(velocity[1]) < 1e-6) {
                first_done = true;
            }
        } else if (!overlap_started) {
            if (position[2] > 1e-6) {
                overlap_started = true;
            }
        } else if (position[2] < 0.95 && position[0] > 2.05) {
            later_held_during_overlap = false;
        }
        if (result == TEST_CYCLE_FINISHED) {
            break;
        }
    }
    EXPECT_TRUE(first_done);
    EXPECT_TRUE(overlap_started);
    EXPECT_TRUE(later_held_during_first);
    EXPECT_TRUE(later_held_during_overlap);
    EXPECT_NEAR(position[0], 3.0, 1e-2);
    EXPECT_NEAR(position[1], 0.0, 1e-2);
    EXPECT_NEAR(position[2], 1.0, 1e-2);
}

// wait-idle parks the command without consuming a group slot.
TEST(MotionBufferParallel, WaitIdleSucceedsWhenAllGroupsBusy) {
    double initial_position[] = {0.0, 0.0};
    double zero_v[2] = {};
    double zero_a[2] = {};
    const MotionFrame initial = zero_frame(initial_position, 2, zero_v, zero_a);
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 2, 0.01, initial), MOTION_BUFFER_OK);

    const uint8_t base0[] = {0};
    const uint8_t base1[] = {1};
    const uint8_t base01[] = {0, 1};
    double target0[] = {1.0};
    double target1[] = {1.0};
    double target01[] = {2.0, 2.0};
    ASSERT_EQ(buffer.set_base(base0, 1), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target0, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base1, 1), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target1, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.set_base(base01, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target01, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              MOTION_BUFFER_OK);

    double position[2] = {};
    double velocity[2] = {};
    double acceleration[2] = {};
    bool both_single_idle = false;
    bool pair_started = false;
    for (int i = 0; i < 8000; ++i) {
        const int result = run_cycle(buffer, position, velocity, acceleration);
        ASSERT_GE(result, TEST_CYCLE_RUNNING);
        const bool singles_idle = fabs(position[0] - 1.0) < 1e-3 &&
                                  fabs(position[1] - 1.0) < 1e-3 && fabs(velocity[0]) < 1e-6 &&
                                  fabs(velocity[1]) < 1e-6;
        if (singles_idle) {
            both_single_idle = true;
        }
        if (both_single_idle && position[0] > 1.05) {
            pair_started = true;
            break;
        }
        if (result == TEST_CYCLE_FINISHED) {
            break;
        }
    }
    EXPECT_TRUE(both_single_idle);
    EXPECT_TRUE(pair_started);
}

// A full segment pool does not consume the independent pending-command capacity.
TEST(MotionBufferParallel, SegmentPoolFullParksUntilCommandPoolIsFull) {
    double initial_position[] = {0.0};
    double zero_v[1] = {};
    double zero_a[1] = {};
    const MotionFrame initial = zero_frame(initial_position, 1, zero_v, zero_a);
    MotionBuffer buffer;
    const int32_t slots = MOTION_BUFFER_SLOT_CAPACITY;
    const int32_t pool_free = 2 * slots;

    ASSERT_EQ(init_buffer(buffer, 1, 0.01, initial), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.queued_count(), 0);
    EXPECT_EQ(buffer.free_count(), pool_free);

    for (int32_t i = 0; i < slots; ++i) {
        double target[] = {static_cast<double>(i + 1)};
        ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
                  MOTION_BUFFER_OK);
    }
    EXPECT_EQ(buffer.queued_count(), slots);
    EXPECT_EQ(buffer.free_count(), slots);

    EXPECT_EQ(
        buffer.push_line(initial_position, MOTION_COORDINATE_ABSOLUTE, make_limits(0.0, 2.0)),
        ZMC_ERR_MOTBUF_INVALID_ARGUMENT);
    EXPECT_EQ(buffer.queued_count(), slots);
    EXPECT_EQ(buffer.free_count(), slots);

    for (int32_t i = 0; i < slots; ++i) {
        double target[] = {static_cast<double>(slots + i + 1)};
        ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
                  MOTION_BUFFER_OK);
    }
    EXPECT_EQ(buffer.queued_count(), pool_free);
    EXPECT_EQ(buffer.free_count(), 0);
    double overflow[] = {static_cast<double>(pool_free + 1)};
    EXPECT_EQ(buffer.push_line(overflow, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 2.0)),
              ZMC_ERR_MOTBUF_FULL);

    double position[1] = {};
    double velocity[1] = {};
    double acceleration[1] = {};
    int result = TEST_CYCLE_RUNNING;
    for (int i = 0; i < 200000 && result == TEST_CYCLE_RUNNING; ++i) {
        result = run_cycle(buffer, position, velocity, acceleration);
        ASSERT_GE(result, TEST_CYCLE_RUNNING);
        EXPECT_EQ(buffer.queued_count() + buffer.free_count(), pool_free);
    }
    EXPECT_EQ(result, TEST_CYCLE_FINISHED);
    EXPECT_NEAR(position[0], static_cast<double>(pool_free), 1e-9);
    EXPECT_EQ(buffer.queued_count(), 0);
    EXPECT_EQ(buffer.free_count(), pool_free);
}

TEST(MotionBufferParallel, SetBaseRejectsDuplicateAndOutOfRange) {
    double initial_position[] = {0.0, 0.0};
    double zero_v[2] = {};
    double zero_a[2] = {};
    const MotionFrame initial = zero_frame(initial_position, 2, zero_v, zero_a);
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 2, 0.01, initial), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.base_count(), 2);

    const uint8_t duplicate[] = {0, 0};
    const uint8_t out_of_range[] = {2};
    EXPECT_EQ(buffer.set_base(duplicate, 2), ZMC_ERR_MOTBUF_INVALID_ARGUMENT);
    EXPECT_EQ(buffer.set_base(out_of_range, 1), ZMC_ERR_MOTBUF_INVALID_ARGUMENT);
    EXPECT_EQ(buffer.set_base(NULL, 1), ZMC_ERR_MOTBUF_INVALID_ARGUMENT);

    const uint8_t single[] = {1};
    ASSERT_EQ(buffer.set_base(single, 1), MOTION_BUFFER_OK);
    EXPECT_EQ(buffer.base_count(), 1);
}

TEST(MotionBufferParallel, SubsetBaseMovesOnlyListedAxes) {
    double initial_position[] = {0.0, 0.0, 0.0, 0.0};
    double zero_v[4] = {};
    double zero_a[4] = {};
    const MotionFrame initial = zero_frame(initial_position, 4, zero_v, zero_a);
    MotionBuffer buffer;
    ASSERT_EQ(init_buffer(buffer, 4, 0.1, initial), MOTION_BUFFER_OK);

    const uint8_t base23[] = {2, 3};
    double target[] = {1.0, 0.0};
    ASSERT_EQ(buffer.set_base(base23, 2), MOTION_BUFFER_OK);
    ASSERT_EQ(buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(2.0, 1.0)),
              MOTION_BUFFER_OK);

    double position[4] = {};
    double velocity[4] = {};
    double acceleration[4] = {};
    ASSERT_EQ(run_cycle(buffer, position, velocity, acceleration), TEST_CYCLE_RUNNING);
    EXPECT_NEAR(position[2], 0.005, 1e-9);
    EXPECT_NEAR(position[0], 0.0, 1e-12);
    EXPECT_NEAR(position[1], 0.0, 1e-12);
    EXPECT_NEAR(position[3], 0.0, 1e-12);
}
