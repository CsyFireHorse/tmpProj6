/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-26
 * @ArtifactStatusAuthority: GLM-5.2
 */
#ifndef ZMC_MOTION_BUFFER_IMPL_H_
#define ZMC_MOTION_BUFFER_IMPL_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "GroupBuffer.h"
#include "MotionBuffer.h"
#include "PendingCommandList.h"
#include "SegmentQueue.h"
#include "common/misc/ZmcMemAllocator.hpp"

namespace zmc {

// Private PIMPL: dispatch, pending parking, workspace. Planning lives in GroupBuffer.
// Axis-set tests use uint64_t bitmasks (axis_count <= 64); ordered base_axes stay for remap/PVA.
struct MotionBuffer::Impl {
    Impl();

    // Build axis-scaled workspace, node pools, and the traj heap.
    int init_storage(uint8_t axes);
    // Unload one lane for reuse: reset, drop occupied bits, compact active range.
    void unload_group(GroupBuffer &group);
    // Clear retained command PVA to zero (init/reset).
    void zero_pva();
    bool has_moving_group() const;
    bool initialized() const { return !groups.empty(); }

    GroupBuffer *find_bound_group(uint64_t base_mask);
    const GroupBuffer *find_group_for_axis(uint8_t axis_index) const;
    GroupBuffer *bind_group(const uint8_t *axes, uint8_t count, uint64_t base_mask);
    // Dispatch into group, or bind a new group when group is NULL.
    int dispatch_command(PendingCommandNode *command, GroupBuffer *group);
    // Drain pending after every occupied group is empty.
    int dispatch_pending();
    int push_command(const double *param_a, const double *param_b, SegmentCommandKind kind,
                     MotionCoordinateMode coordinate_mode, MotionArcDirection direction,
                     const MotionLimits &limits);
    int init(uint8_t axes);
    void set_cycle_time(double cycle);
    int set_current_frame(const MotionFrame &current_frame);
    int set_base(const uint8_t *axes, uint8_t count);
    int set_junction_mode(MotionJunctionMode mode);
    int set_override(MotionOverrideType type, double factor);
    void set_input(MotionAxInput *input, int len);
    int push_line(const double *target, MotionCoordinateMode coordinate_mode,
                  const MotionLimits &limits);
    int push_arc_through(const double *through_point, const double *end_point,
                         MotionCoordinateMode coordinate_mode, const MotionLimits &limits);
    int push_arc_center(const double *center, const double *end_point,
                        MotionArcDirection direction, MotionCoordinateMode coordinate_mode,
                        const MotionLimits &limits);
    void runCycle();
    void get_output(MotionAxOutput *output, int len) const;
    void reset();
    MotionAxisGroupState state(uint8_t axis_index) const;
    int32_t queued_count() const;
    int32_t free_count() const;
    uint8_t base_count() const;

    // Shared traj heap outlives group sample PVA and segment caches (declared first =>
    // destroyed last).
    std::unique_ptr<AllocatorMemoryAdapter> trajectory_allocator;

    uint8_t max_groups;
    // Bound groups occupy groups[0..active_group_count) without holes.
    uint8_t active_group_count;
    // OR of occupied group base_mask values; occupied groups are pairwise disjoint.
    uint64_t occupied_mask;
    std::vector<GroupBuffer> groups;
    // Owns SegmentNode storage; free list is rebuilt on resize.
    SegmentPool segment_pool;

    // Pending commands and built trajectory segments have independent fixed-capacity storage.
    // Owns PendingCommandNode storage; free list is rebuilt on resize.
    PendingCommandPool command_pool;
    PendingCommandList pending;

    // Preallocated scalar workspace and externally reported per-axis input.
    std::vector<double> workspace;
    // Fixed pool for pending-command param_a / param_b; not part of workspace.
    std::vector<double> command_param_pool;
    std::vector<uint8_t> axis_index_workspace;
    std::vector<MotionAxInput> axis_input;

    // Views into workspace; no view owns memory.
    double *current_position;
    double *current_velocity;
    double *current_acceleration;

    uint8_t *current_base;
    uint8_t current_base_count;
    uint64_t current_base_mask;

    uint8_t axis_count;
    double cycle_time;
};

}  // namespace zmc

#endif  // ZMC_MOTION_BUFFER_IMPL_H_
