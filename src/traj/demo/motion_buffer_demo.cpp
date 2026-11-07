#include "MotionBuffer.h"
#include "stdio.h"

using namespace zmc;

namespace {

const int kMaxDemoSteps = 10000;

MotionLimits make_limits(double speed, double acceleration, double jerk = 0.0) {
    MotionLimits limits;
    limits.max_speed = speed;
    limits.max_acceleration = acceleration;
    limits.max_deceleration = acceleration;
    limits.max_jerk = jerk;
    return limits;
}

// Sample one control cycle at a time until the queue is empty.
void print_buffer_steps(MotionBuffer &buffer, double cycle_time) {
    MotionAxOutput output[2] = {};
    printf("idx,time,axId,queued,state,pos,vel,acc\n");
    for (int i = 0; i < kMaxDemoSteps; ++i) {
        buffer.runCycle();
        buffer.getOutput(output, 2);
        const uint8_t axis_count = buffer.axis_count();
        for (uint8_t j = 0; j < axis_count; ++j) {
            if (buffer.state(j) == MOTION_AXIS_GROUP_ERRORSTOP) {
                printf("MotionBuffer runCycle entered error stop\n");
                return;
            }
            printf("%d,%f,%d,%d,%d,%f,%f,%f\n", i, i * cycle_time, j, buffer.queued_count(),
                   buffer.state(j), output[j].pos, output[j].vel, output[j].acc);
        }
        if (buffer.queued_count() == 0) {
            return;
        }
    }
    printf("MotionBuffer demo reached step limit\n");
}

}  // namespace

void MotionBuffer_line_demo() {
    double cycle_time = 0.1;
    double begin_pos[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {begin_pos, zero, zero};
    double target[] = {1.0, -2.0};
    MotionBuffer buffer;

    int ret = buffer.init(2);
    if (ret == MOTION_BUFFER_OK) {
        buffer.set_cycle_time(cycle_time);
    }
    if (ret == MOTION_BUFFER_OK) {
        ret = buffer.set_current_frame(initial);
    }
    if (ret != MOTION_BUFFER_OK) {
        printf("MotionBuffer_line_demo init failed: ret=%d\n", ret);
        return;
    }
    ret = buffer.push_line(target, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 3.0));
    if (ret != MOTION_BUFFER_OK) {
        printf("MotionBuffer_line_demo push_line failed: ret=%d\n", ret);
        return;
    }
    print_buffer_steps(buffer, cycle_time);
}

void MotionBuffer_corner_demo() {
    double cycle_time = 0.1;
    double begin_pos[] = {0.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {begin_pos, zero, zero};
    double first_end[] = {1.0, 0.0};
    double second_end[] = {1.0, 2.0};
    MotionBuffer buffer;

    int ret = buffer.init(2);
    if (ret == MOTION_BUFFER_OK) {
        buffer.set_cycle_time(cycle_time);
    }
    if (ret == MOTION_BUFFER_OK) {
        ret = buffer.set_current_frame(initial);
    }
    if (ret != MOTION_BUFFER_OK) {
        printf("MotionBuffer_corner_demo init failed: ret=%d\n", ret);
        return;
    }
    ret = buffer.push_line(first_end, MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 5.0));
    if (ret != MOTION_BUFFER_OK) {
        printf("MotionBuffer_corner_demo push_line failed: ret=%d\n", ret);
        return;
    }
    ret = buffer.push_line(second_end, MOTION_COORDINATE_RELATIVE, make_limits(2.0, 4.0));
    if (ret != MOTION_BUFFER_OK) {
        printf("MotionBuffer_corner_demo push_line failed: ret=%d\n", ret);
        return;
    }
    print_buffer_steps(buffer, cycle_time);
}

void MotionBuffer_arc_demo() {
    double cycle_time = 0.1;
    double begin_pos[] = {1.0, 0.0};
    double zero[] = {0.0, 0.0};
    const MotionFrame initial = {begin_pos, zero, zero};
    double center[] = {0.0, 0.0};
    double end[] = {0.0, 1.0};
    MotionBuffer buffer;

    int ret = buffer.init(2);
    if (ret == MOTION_BUFFER_OK) {
        buffer.set_cycle_time(cycle_time);
    }
    if (ret == MOTION_BUFFER_OK) {
        ret = buffer.set_current_frame(initial);
    }
    if (ret != MOTION_BUFFER_OK) {
        printf("MotionBuffer_arc_demo init failed: ret=%d\n", ret);
        return;
    }
    ret = buffer.push_arc_center(center, end, MOTION_ARC_COUNTERCLOCKWISE,
                                 MOTION_COORDINATE_ABSOLUTE, make_limits(1.0, 1.0));
    if (ret != MOTION_BUFFER_OK) {
        printf("MotionBuffer_arc_demo push_arc_center failed: ret=%d\n", ret);
        return;
    }
    print_buffer_steps(buffer, cycle_time);
}
