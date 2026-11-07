/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-26
 * @ArtifactStatusAuthority: GLM-5.2
 */
#include "MotionBufferImpl.h"

#include <cmath>
#include <new>
#include <utility>

#include "common/zmc/zmc_predef.h"

namespace zmc {

using std::isfinite;

namespace {

// Bitmask axis-set ops require axis index in [0, 64).
const uint8_t kMaxAxisMaskBits = 64;

uint64_t make_base_mask(const uint8_t *axes, uint8_t count) {
    uint64_t mask = 0;
    for (uint8_t i = 0; i < count; ++i) {
        mask |= (static_cast<uint64_t>(1) << axes[i]);
    }
    return mask;
}

bool valid_command_limits(const MotionLimits &limits) {
    return isfinite(limits.max_speed) && limits.max_speed > 0.0 &&
           isfinite(limits.max_acceleration) && limits.max_acceleration > 0.0 &&
           isfinite(limits.max_deceleration) && limits.max_deceleration > 0.0 &&
           isfinite(limits.max_jerk) && limits.max_jerk >= 0.0;
}

int check_params(const double *params, uint8_t count) {
    if (!params) {
        return ZMC_ERR_MOTBUF_INVALID_ARGUMENT;
    }
    for (uint8_t i = 0; i < count; ++i) {
        if (!isfinite(params[i])) {
            return ZMC_ERR_MOTBUF_INVALID_ARGUMENT;
        }
    }
    return MOTION_BUFFER_OK;
}

// Pending commands must be fully validated before they can enter the pending FIFO.
int validate_command(const double *param_a, const double *param_b, uint8_t count,
                     SegmentCommandKind kind, MotionCoordinateMode coordinate_mode,
                     MotionArcDirection direction, const MotionLimits &limits) {
    if ((coordinate_mode != MOTION_COORDINATE_ABSOLUTE &&
         coordinate_mode != MOTION_COORDINATE_RELATIVE) ||
        !valid_command_limits(limits)) {
        return ZMC_ERR_MOTBUF_INVALID_ARGUMENT;
    }
    if (kind == SEGMENT_COMMAND_ARC_CENTER && direction != MOTION_ARC_CLOCKWISE &&
        direction != MOTION_ARC_COUNTERCLOCKWISE) {
        return ZMC_ERR_MOTBUF_INVALID_ARGUMENT;
    }
    int ret = check_params(param_a, count);
    if (ret == 0 && kind != SEGMENT_COMMAND_LINE) {
        ret = check_params(param_b, count);
    }
    return ret;
}

}  // namespace

MotionBuffer::Impl::Impl()
    : max_groups(0),
      active_group_count(0),
      occupied_mask(0),
      current_position(NULL),
      current_velocity(NULL),
      current_acceleration(NULL),
      current_base(NULL),
      current_base_count(0),
      current_base_mask(0),
      axis_count(0),
      cycle_time(0.001) {}

int MotionBuffer::Impl::init_storage(uint8_t axes) {
    const int32_t slots = MOTION_BUFFER_SLOT_CAPACITY;
    // Command param pool must cover param_a and param_b for every slot at this axis count.
    if (static_cast<size_t>(slots) * 2u * static_cast<size_t>(axes) >
        static_cast<size_t>(MOTION_BUFFER_COMMAND_COORD_CAPACITY)) {
        return ZMC_ERR_MOTBUF_PLAN_FAILED;
    }

    axis_count = axes;
    max_groups = axes;
    active_group_count = 0;
    occupied_mask = 0;
    groups.resize(max_groups);
    command_pool.resize(slots);
    segment_pool.resize(slots);
    axis_input.resize(axes);

    // Workspace layout (doubles): global PVA only. Command params live in
    // command_param_pool. Group sample PVA and BASE storage come from the traj heap.
    // Axis indices: current BASE + per-command (group BASE is owned by GroupBuffer).
    const size_t double_count = static_cast<size_t>(axes) * 3u;
    workspace.resize(double_count);
    command_param_pool.resize(static_cast<size_t>(MOTION_BUFFER_COMMAND_COORD_CAPACITY));
    axis_index_workspace.resize(static_cast<size_t>(axes) +
                                static_cast<size_t>(slots) * static_cast<size_t>(axes));

    double *cursor = workspace.data();
    current_position = cursor;
    cursor += axes;
    current_velocity = cursor;
    cursor += axes;
    current_acceleration = cursor;

    double *command_param_a = command_param_pool.data();
    double *command_param_b =
        command_param_a + static_cast<size_t>(slots) * static_cast<size_t>(axes);

    uint8_t *axis_cursor = axis_index_workspace.data();
    current_base = axis_cursor;
    axis_cursor += axes;
    uint8_t *command_base_pool = axis_cursor;

    // Create the backing heap during init; segment caches and group sample PVA share it.
    trajectory_allocator.reset(new (std::nothrow) AllocatorMemoryAdapter());
    if (!trajectory_allocator) {
        return ZMC_ERR_MOTBUF_PLAN_FAILED;
    }

    for (uint8_t g = 0; g < max_groups; ++g) {
        GroupBuffer &group = groups[g];
        group.set_resources(trajectory_allocator.get(), &segment_pool);
        group.reset();
    }

    for (int32_t i = 0; i < slots; ++i) {
        PendingCommandNode &command = command_pool[i];
        command.base_axes = command_base_pool + static_cast<size_t>(i) * axes;
        command.param_a = command_param_a + static_cast<size_t>(i) * axes;
        command.param_b = command_param_b + static_cast<size_t>(i) * axes;
    }
    pending.reset();
    return MOTION_BUFFER_OK;
}

void MotionBuffer::Impl::unload_group(GroupBuffer &group) {
    const uint8_t group_index = static_cast<uint8_t>(&group - groups.data());
    if (group_index >= active_group_count) {
        return;
    }
    const uint64_t mask = group.base_mask();
    group.reset();
    // Clear occupied bits only if this group had already been marked occupied.
    if (mask != 0 && (occupied_mask & mask) != 0) {
        occupied_mask &= ~mask;
    }
    const uint8_t last_group_index = active_group_count - 1;
    if (group_index != last_group_index) {
        group.swap(groups[last_group_index]);
    }
    --active_group_count;
}

void MotionBuffer::Impl::zero_pva() {
    for (uint8_t i = 0; i < axis_count; ++i) {
        current_position[i] = 0.0;
        current_velocity[i] = 0.0;
        current_acceleration[i] = 0.0;
    }
}

bool MotionBuffer::Impl::has_moving_group() const {
    for (uint8_t g = 0; g < active_group_count; ++g) {
        if (groups[g].state() == MOTION_AXIS_GROUP_MOVING) {
            return true;
        }
    }
    return false;
}

GroupBuffer *MotionBuffer::Impl::find_bound_group(uint64_t base_mask) {
    for (uint8_t g = 0; g < active_group_count; ++g) {
        if (groups[g].base_mask() == base_mask) {
            return &groups[g];
        }
    }
    return NULL;
}

const GroupBuffer *MotionBuffer::Impl::find_group_for_axis(uint8_t axis_index) const {
    const uint64_t axis_mask = static_cast<uint64_t>(1) << axis_index;
    for (uint8_t g = 0; g < active_group_count; ++g) {
        if ((groups[g].base_mask() & axis_mask) != 0) {
            return &groups[g];
        }
    }
    return NULL;
}

GroupBuffer *MotionBuffer::Impl::bind_group(const uint8_t *axes, uint8_t count,
                                            uint64_t base_mask) {
    if (active_group_count >= max_groups) {
        return NULL;
    }
    GroupBuffer *group = &groups[active_group_count];

    // Free slots are already unbound: init() or unload_group() reset them.
    if (group->bind(axes, count, base_mask, cycle_time) != 0) {
        return NULL;
    }
    group->update_axis_group_state(axis_input.data());
    ++active_group_count;
    return group;
}

int MotionBuffer::Impl::dispatch_command(PendingCommandNode *command, GroupBuffer *group) {
    if (!group) {
        group = bind_group(command->base_axes, command->base_count, command->base_mask);
        if (!group) {
            return ZMC_ERR_MOTBUF_FULL;
        }
    }

    const bool was_empty = group->empty();
    const int ret = group->submit_command(*command, current_position, current_velocity,
                                          current_acceleration);
    if (was_empty && (ret == 0 || group->state() == MOTION_AXIS_GROUP_ERRORSTOP)) {
        // Occupied groups are pairwise disjoint; OR is exact.
        occupied_mask |= group->base_mask();
    }
    return ret;
}

// Drain pending after all occupied groups are empty. Same dispatch as push_command.
int MotionBuffer::Impl::dispatch_pending() {
    while (!pending.empty()) {
        PendingCommandNode *command = pending.front();
        GroupBuffer *group = find_bound_group(command->base_mask);
        // Axes free of every occupied group => can bind/parallel; else leave pending.
        if (!group && (command->base_mask & occupied_mask) != 0) {
            break;
        }
        if (segment_pool.free_count() == 0) {
            break;
        }

        pending.pop_front();
        const int ret = dispatch_command(command, group);
        command_pool.deallocate(command);
        if (ret != 0) {
            return ret;
        }
    }
    return MOTION_BUFFER_OK;
}

// Dispatch (after set_* validation), axis sets via bitmask:
// 1. pending nonempty => global block: park everyone.
// 2. bound group with same mask => forward the command to that group.
// 3. (cmd_mask & occupied_mask) == 0 => bind new / parallel lane.
// 4. else => park (intersection with an occupied group).
int MotionBuffer::Impl::push_command(const double *param_a, const double *param_b,
                                     SegmentCommandKind kind, MotionCoordinateMode coordinate_mode,
                                     MotionArcDirection direction, const MotionLimits &limits) {
    if (kind != SEGMENT_COMMAND_LINE && current_base_count != 2 && current_base_count != 3) {
        return ZMC_ERR_MOTBUF_NOT_SUPPORTED;
    }

    PendingCommandNode *command = command_pool.allocate();
    if (!command) {
        return ZMC_ERR_MOTBUF_FULL;
    }
    int ret = validate_command(param_a, param_b, current_base_count, kind, coordinate_mode,
                               direction, limits);
    if (ret != 0) {
        command_pool.deallocate(command);
        return ret;
    }
    command->store_command(current_base, current_base_count, param_a, param_b, kind,
                           coordinate_mode, direction, limits);

    const bool waiting = !pending.empty();
    GroupBuffer *group = waiting ? NULL : find_bound_group(current_base_mask);
    const bool can_bind = !waiting && group == NULL && (current_base_mask & occupied_mask) == 0;
    if (waiting || (group == NULL && !can_bind) || segment_pool.free_count() == 0) {
        pending.push_back(command);
        return MOTION_BUFFER_OK;
    }

    ret = dispatch_command(command, group);
    command_pool.deallocate(command);
    return ret;
}

int MotionBuffer::Impl::init(uint8_t axes) {
    if (axes == 0 || axes > kMaxAxisMaskBits) {
        return ZMC_ERR_MOTBUF_INVALID_ARGUMENT;
    }
    // Re-entry (resize): tear down existing state before re-allocating. Segment
    // caches borrow the trajectory allocator, so they must be destroyed (via
    // segment_pool.clear(), which preserves vector capacity) before that
    // allocator is destroyed. command_pool is left for init_storage() to resize
    // in place; the per-element reset loops in init_storage() reinitialize them.
    if (!groups.empty()) {
        while (active_group_count != 0) {
            unload_group(groups[active_group_count - 1]);
        }
        command_pool.deallocate_list(pending);
        occupied_mask = 0;
        segment_pool.clear();
        trajectory_allocator.reset();
    }
    int ret = init_storage(axes);
    if (ret != 0) {
        groups.clear();
        // Segments return their caches before the shared allocator is destroyed.
        segment_pool.clear();
        trajectory_allocator.reset();
        command_pool.clear();
        axis_input.clear();
        workspace.clear();
        command_param_pool.clear();
        axis_index_workspace.clear();
        return ret;
    }
    for (uint8_t i = 0; i < axes; ++i) {
        axis_input[i].idx = i;
        axis_input[i].state = MOTION_AXIS_STANDSTILL;
        axis_input[i].errorCode = 0;
        axis_input[i].actPos = 0.0;
        current_base[i] = i;
    }
    current_base_count = axes;
    current_base_mask = make_base_mask(current_base, axes);
    cycle_time = 0.001;
    zero_pva();
    return MOTION_BUFFER_OK;
}

void MotionBuffer::Impl::set_cycle_time(double cycle) {
    if (!isfinite(cycle) || cycle <= 0.0) {
        return;
    }
    cycle_time = cycle;
    for (uint8_t g = 0; g < active_group_count; ++g) {
        groups[g].set_cycle_time(cycle);
    }
}

int MotionBuffer::Impl::set_current_frame(const MotionFrame &current_frame) {
    if (!current_frame.position || !current_frame.velocity || !current_frame.acceleration) {
        return ZMC_ERR_MOTBUF_INVALID_ARGUMENT;
    }
    for (uint8_t i = 0; i < axis_count; ++i) {
        current_position[i] = current_frame.position[i];
        current_velocity[i] = current_frame.velocity[i];
        current_acceleration[i] = current_frame.acceleration[i];
    }
    return MOTION_BUFFER_OK;
}

int MotionBuffer::Impl::set_base(const uint8_t *axes, uint8_t count) {
    if (!axes || count == 0 || count > axis_count) {
        return ZMC_ERR_MOTBUF_INVALID_ARGUMENT;
    }
    uint64_t mask = 0;
    for (uint8_t i = 0; i < count; ++i) {
        if (axes[i] >= axis_count) {
            return ZMC_ERR_MOTBUF_INVALID_ARGUMENT;
        }
        const uint64_t bit = static_cast<uint64_t>(1) << axes[i];
        if ((mask & bit) != 0) {
            return ZMC_ERR_MOTBUF_INVALID_ARGUMENT;
        }
        mask |= bit;
    }
    current_base_count = count;
    current_base_mask = mask;
    for (uint8_t i = 0; i < count; ++i) {
        current_base[i] = axes[i];
    }
    return MOTION_BUFFER_OK;
}

int MotionBuffer::Impl::set_junction_mode(MotionJunctionMode mode) {
    // Only STOP_AT_CORNER is planned; persist nothing, but reject changes while moving.
    GroupBuffer *group = find_bound_group(current_base_mask);
    group->set_junction_mode(mode);
    return MOTION_BUFFER_OK;
}

int MotionBuffer::Impl::set_override(MotionOverrideType type, double factor) {
    // The public override shape is reserved; this version intentionally changes no state.
    (void)type;
    (void)factor;
    return MOTION_BUFFER_OK;
}

void MotionBuffer::Impl::set_input(MotionAxInput *input, int len) {
    if (!input || len != axis_count) {
        return;
    }
    for (uint8_t i = 0; i < axis_count; ++i) {
        if (input[i].idx >= axis_count || input[i].state < MOTION_AXIS_DISABLE ||
            input[i].state > MOTION_AXIS_ERRORSTOP || !isfinite(input[i].actPos)) {
            return;
        }
        for (uint8_t j = 0; j < i; ++j) {
            if (input[j].idx == input[i].idx) {
                return;
            }
        }
    }
    for (uint8_t i = 0; i < axis_count; ++i) {
        const uint8_t axis_index = input[i].idx;
        axis_input[axis_index] = input[i];
    }
    for (uint8_t g = 0; g < active_group_count; ++g) {
        if (groups[g].base_mask() != 0) {
            groups[g].update_axis_group_state(axis_input.data());
        }
    }
}

int MotionBuffer::Impl::push_line(const double *target, MotionCoordinateMode coordinate_mode,
                                  const MotionLimits &limits) {
    return push_command(target, NULL, SEGMENT_COMMAND_LINE, coordinate_mode,
                        MOTION_ARC_COUNTERCLOCKWISE, limits);
}

int MotionBuffer::Impl::push_arc_through(const double *through_point, const double *end_point,
                                         MotionCoordinateMode coordinate_mode,
                                         const MotionLimits &limits) {
    return push_command(through_point, end_point, SEGMENT_COMMAND_ARC_THROUGH, coordinate_mode,
                        MOTION_ARC_COUNTERCLOCKWISE, limits);
}

int MotionBuffer::Impl::push_arc_center(const double *center, const double *end_point,
                                        MotionArcDirection direction,
                                        MotionCoordinateMode coordinate_mode,
                                        const MotionLimits &limits) {
    return push_command(center, end_point, SEGMENT_COMMAND_ARC_CENTER, coordinate_mode, direction,
                        limits);
}

void MotionBuffer::Impl::runCycle() {
    // Flush pending only when every occupied group is empty (global unblock).
    // On failure the bound group is already ERRORSTOP; still run it this cycle.
    if (occupied_mask == 0 && !pending.empty()) {
        (void)dispatch_pending();
    }
    uint8_t g = 0;
    while (g < active_group_count) {
        GroupBuffer &group = groups[g];
        if (group.empty() && group.state() != MOTION_AXIS_GROUP_ERRORSTOP) {
            ++g;
            continue;
        }
        group.runCycle();
        group.expand_pva(current_position, current_velocity, current_acceleration);
        if (group.empty() && group.state() != MOTION_AXIS_GROUP_ERRORSTOP) {
            unload_group(group);
        } else {
            ++g;
        }
    }
}

void MotionBuffer::Impl::get_output(MotionAxOutput *output, int len) const {
    if (!initialized() || !output || len < axis_count) {
        return;
    }
    for (uint8_t i = 0; i < axis_count; ++i) {
        const GroupBuffer *group = find_group_for_axis(i);
        output[i].idx = i;
        // Publish bound group state and error code; single-axis input stays on setInput only.
        output[i].state = group ? group->state() : MOTION_AXIS_GROUP_STANDSTILL;
        output[i].errorCode =
            group ? static_cast<uint16_t>(group->error_code()) : static_cast<uint16_t>(0);
        output[i].pos = current_position[i];
        output[i].vel = current_velocity[i];
        output[i].acc = current_acceleration[i];
    }
}

void MotionBuffer::Impl::reset() {
    if (!initialized()) {
        return;
    }
    while (active_group_count != 0) {
        unload_group(groups[active_group_count - 1]);
    }
    command_pool.deallocate_list(pending);
    occupied_mask = 0;

    // Restore configuration and external axis input to their post-init defaults.
    for (uint8_t i = 0; i < axis_count; ++i) {
        axis_input[i].idx = i;
        axis_input[i].state = MOTION_AXIS_STANDSTILL;
        axis_input[i].errorCode = 0;
        axis_input[i].actPos = 0.0;
        current_base[i] = i;
    }
    current_base_count = axis_count;
    current_base_mask = make_base_mask(current_base, axis_count);
    cycle_time = 0.001;
    zero_pva();
}

MotionAxisGroupState MotionBuffer::Impl::state(uint8_t axis_index) const {
    if (!initialized() || axis_index >= axis_count) {
        return MOTION_AXIS_GROUP_DISABLE;
    }
    const GroupBuffer *group = find_group_for_axis(axis_index);
    return group ? group->state() : MOTION_AXIS_GROUP_STANDSTILL;
}

int32_t MotionBuffer::Impl::queued_count() const {
    if (!initialized()) {
        return 0;
    }
    const int32_t pending_commands = command_pool.capacity() - command_pool.free_count();
    const int32_t trajectory_segments = segment_pool.capacity() - segment_pool.free_count();
    return pending_commands + trajectory_segments;
}

int32_t MotionBuffer::Impl::free_count() const {
    return initialized() ? command_pool.free_count() + segment_pool.free_count() : 0;
}

uint8_t MotionBuffer::Impl::base_count() const { return initialized() ? current_base_count : 0; }

}  // namespace zmc
