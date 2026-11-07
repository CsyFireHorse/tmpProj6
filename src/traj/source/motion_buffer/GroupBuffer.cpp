/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-26
 * @ArtifactStatusAuthority: GLM-5.2
 */
#include "GroupBuffer.h"

#include <math.h>
#include <utility>

#include "PendingCommandList.h"
#include "common/misc/ZmcAllocator.hpp"
#include "vel/include/lookahead_relaxation.h"
#include "vel/include/vel_stop_online.hpp"

#define MOTION_BUFFER_LOOKAHEAD_CAPACITY 16

namespace zmc {
namespace {

const uint8_t kMaxAxisMaskBits = 64;
const double kGeometryEpsilon = 1e-12;
const double kSoftwareErrorStopDeceleration = 1000.0;

// Ordered equality only: decides whether command parameters require remapping.
bool bases_order_equal(const uint8_t *a, uint8_t na, const uint8_t *b, uint8_t nb) {
    if (na != nb) {
        return false;
    }
    for (uint8_t i = 0; i < na; ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

// Remap group-local coordinates from command BASE order to bound BASE order.
void remap_params(const uint8_t *src_base, uint8_t count, const uint8_t *dst_base,
                  const double *src, double *dst) {
    for (uint8_t j = 0; j < count; ++j) {
        for (uint8_t i = 0; i < count; ++i) {
            if (src_base[i] == dst_base[j]) {
                dst[j] = src[i];
                break;
            }
        }
    }
}

// Apply the validated pending command to one pooled motion segment.
int apply_command(const PendingCommandNode &command, MotionSegment &segment, const double *param_a,
                  const double *param_b) {
    if (command.kind == SEGMENT_COMMAND_LINE) {
        return segment.set_line(param_a, command.coordinate_mode, command.limits);
    }
    if (command.kind == SEGMENT_COMMAND_ARC_THROUGH) {
        return segment.set_arc_through(param_a, param_b, command.coordinate_mode, command.limits);
    }
    return segment.set_arc_center(param_a, param_b, command.direction, command.coordinate_mode,
                                  command.limits);
}

}  // namespace

GroupBuffer::GroupBuffer()
    : base_count_(0),
      base_mask_(0),
      planned_segment_count_(0),
      segment_time_(0.0),
      remaining_time_(0.0),
      has_path_state_(false),
      cycle_time_(0.0),
      segment_pool_(NULL),
      axis_group_state_(MOTION_AXIS_GROUP_STANDSTILL),
      error_code_(MOTION_BUFFER_OK),
      allocator_(NULL),
      sample_position_(NULL),
      sample_velocity_(NULL),
      sample_acceleration_(NULL),
      fSavePosition(NULL),
      fSaveVelocity(NULL),
      fSaveAcceleration(NULL),
      sample_capacity_(0),
      base_axes_(NULL),
      next_begin_pos_(NULL),
      base_capacity_(0) {
    event_.value = 0;
    path_state_.position = 0.0;
    path_state_.velocity = 0.0;
    path_state_.acceleration = 0.0;
}

GroupBuffer::~GroupBuffer() {
    deallocate_sample();
    deallocate_base_storage();
}

GroupBuffer::GroupBuffer(GroupBuffer &&other) noexcept
    : base_count_(other.base_count_),
      base_mask_(other.base_mask_),
      segments_(std::move(other.segments_)),
      planned_segment_count_(other.planned_segment_count_),
      segment_time_(other.segment_time_),
      remaining_time_(other.remaining_time_),
      path_state_(other.path_state_),
      has_path_state_(other.has_path_state_),
      cycle_time_(other.cycle_time_),
      segment_pool_(other.segment_pool_),
      axis_group_state_(other.axis_group_state_),
      error_code_(other.error_code_),
      allocator_(other.allocator_),
      sample_position_(other.sample_position_),
      sample_velocity_(other.sample_velocity_),
      sample_acceleration_(other.sample_acceleration_),
      fSavePosition(other.fSavePosition),
      fSaveVelocity(other.fSaveVelocity),
      fSaveAcceleration(other.fSaveAcceleration),
      sample_capacity_(other.sample_capacity_),
      base_axes_(other.base_axes_),
      next_begin_pos_(other.next_begin_pos_),
      base_capacity_(other.base_capacity_) {
    event_.value = other.event_.value;
    other.base_count_ = 0;
    other.base_mask_ = 0;
    other.event_.value = 0;
    other.planned_segment_count_ = 0;
    other.segment_time_ = 0.0;
    other.remaining_time_ = 0.0;
    other.has_path_state_ = false;
    other.cycle_time_ = 0.0;
    other.segment_pool_ = NULL;
    other.axis_group_state_ = MOTION_AXIS_GROUP_STANDSTILL;
    other.error_code_ = MOTION_BUFFER_OK;
    other.allocator_ = NULL;
    other.sample_position_ = NULL;
    other.sample_velocity_ = NULL;
    other.sample_acceleration_ = NULL;
    other.fSavePosition = NULL;
    other.fSaveVelocity = NULL;
    other.fSaveAcceleration = NULL;
    other.sample_capacity_ = 0;
    other.base_axes_ = NULL;
    other.next_begin_pos_ = NULL;
    other.base_capacity_ = 0;
    rebind_segments();
}

GroupBuffer &GroupBuffer::operator=(GroupBuffer &&other) noexcept {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

bool GroupBuffer::empty() const { return segments_.empty(); }

void GroupBuffer::set_resources(PmAllocator *allocator, SegmentPool *segment_pool) {
    allocator_ = allocator;
    segment_pool_ = segment_pool;
}

PmAllocator *GroupBuffer::get_allocator() const {
    if (allocator_) {
        return allocator_;
    }
    // Same default as TrajBase: malloc/free when no borrowed heap is set.
    static AllocatorDefault default_alloc;
    return &default_alloc;
}

void GroupBuffer::deallocate_sample() {
    if (!sample_position_) {
        sample_velocity_ = NULL;
        sample_acceleration_ = NULL;
        fSavePosition = NULL;
        fSaveVelocity = NULL;
        fSaveAcceleration = NULL;
        sample_capacity_ = 0;
        return;
    }
    // One block holds current PVA followed by the last published PVA.
    PmAllocator *p = get_allocator();
    const int bytes =
        static_cast<int>(static_cast<size_t>(sample_capacity_) * 6u * sizeof(double));
    p->deallocate(sample_position_, bytes);
    sample_position_ = NULL;
    sample_velocity_ = NULL;
    sample_acceleration_ = NULL;
    fSavePosition = NULL;
    fSaveVelocity = NULL;
    fSaveAcceleration = NULL;
    sample_capacity_ = 0;
}

int GroupBuffer::ensure_sample(uint8_t axes) {
    if (sample_capacity_ >= axes && sample_position_ && sample_velocity_ && sample_acceleration_ &&
        fSavePosition && fSaveVelocity && fSaveAcceleration) {
        return MOTION_BUFFER_OK;
    }
    PmAllocator *p = get_allocator();
    deallocate_sample();
    // One block: [sample PVA | saved PVA], with one axis-sized channel per pointer.
    const int bytes = static_cast<int>(static_cast<size_t>(axes) * 6u * sizeof(double));
    double *block = static_cast<double *>(p->allocate(bytes));
    if (!block) {
        return ZMC_ERR_MOTBUF_PLAN_FAILED;
    }
    sample_position_ = block;
    sample_velocity_ = block + axes;
    sample_acceleration_ = block + static_cast<size_t>(axes) * 2u;
    fSavePosition = block + static_cast<size_t>(axes) * 3u;
    fSaveVelocity = block + static_cast<size_t>(axes) * 4u;
    fSaveAcceleration = block + static_cast<size_t>(axes) * 5u;
    sample_capacity_ = axes;
    for (uint8_t i = 0; i < axes; ++i) {
        sample_position_[i] = 0.0;
        sample_velocity_[i] = 0.0;
        sample_acceleration_[i] = 0.0;
        fSavePosition[i] = 0.0;
        fSaveVelocity[i] = 0.0;
        fSaveAcceleration[i] = 0.0;
    }
    return MOTION_BUFFER_OK;
}

void GroupBuffer::deallocate_base_storage() {
    if (!next_begin_pos_) {
        base_axes_ = NULL;
        base_capacity_ = 0;
        return;
    }
    // One block: [next_begin_pos doubles | base_axes bytes], owned at next_begin_pos_.
    PmAllocator *p = get_allocator();
    const int bytes = static_cast<int>(static_cast<size_t>(base_capacity_) * sizeof(double) +
                                       static_cast<size_t>(base_capacity_) * sizeof(uint8_t));
    p->deallocate(next_begin_pos_, bytes);
    next_begin_pos_ = NULL;
    base_axes_ = NULL;
    base_capacity_ = 0;
}

int GroupBuffer::ensure_base_storage(uint8_t axes) {
    if (base_capacity_ >= axes && base_axes_ && next_begin_pos_) {
        return MOTION_BUFFER_OK;
    }
    PmAllocator *p = get_allocator();
    deallocate_base_storage();
    // Doubles first so the block stays naturally aligned; axes packed after.
    const int bytes = static_cast<int>(static_cast<size_t>(axes) * sizeof(double) +
                                       static_cast<size_t>(axes) * sizeof(uint8_t));
    double *block = static_cast<double *>(p->allocate(bytes));
    if (!block) {
        return ZMC_ERR_MOTBUF_PLAN_FAILED;
    }
    next_begin_pos_ = block;
    base_axes_ = reinterpret_cast<uint8_t *>(block + axes);
    base_capacity_ = axes;
    for (uint8_t i = 0; i < axes; ++i) {
        base_axes_[i] = 0;
        next_begin_pos_[i] = 0.0;
    }
    return MOTION_BUFFER_OK;
}

void GroupBuffer::swap(GroupBuffer &other) {
    if (this == &other) {
        return;
    }
    using std::swap;
    swap(base_count_, other.base_count_);
    swap(base_mask_, other.base_mask_);
    segments_.swap(other.segments_);
    swap(event_.value, other.event_.value);
    swap(planned_segment_count_, other.planned_segment_count_);
    swap(segment_time_, other.segment_time_);
    swap(remaining_time_, other.remaining_time_);
    swap(path_state_, other.path_state_);
    swap(has_path_state_, other.has_path_state_);
    swap(cycle_time_, other.cycle_time_);
    swap(segment_pool_, other.segment_pool_);
    swap(axis_group_state_, other.axis_group_state_);
    swap(error_code_, other.error_code_);
    swap(allocator_, other.allocator_);
    swap(sample_position_, other.sample_position_);
    swap(sample_velocity_, other.sample_velocity_);
    swap(sample_acceleration_, other.sample_acceleration_);
    swap(fSavePosition, other.fSavePosition);
    swap(fSaveVelocity, other.fSaveVelocity);
    swap(fSaveAcceleration, other.fSaveAcceleration);
    swap(sample_capacity_, other.sample_capacity_);
    swap(base_axes_, other.base_axes_);
    swap(next_begin_pos_, other.next_begin_pos_);
    swap(base_capacity_, other.base_capacity_);
    rebind_segments();
    other.rebind_segments();
}

void GroupBuffer::reset() {
    if (segment_pool_) {
        segment_pool_->deallocate_list(segments_);
    } else {
        segments_.reset();
    }
    deallocate_sample();
    deallocate_base_storage();
    base_count_ = 0;
    base_mask_ = 0;
    event_.value = 0;
    planned_segment_count_ = 0;
    segment_time_ = 0.0;
    remaining_time_ = 0.0;
    path_state_.position = 0.0;
    path_state_.velocity = 0.0;
    path_state_.acceleration = 0.0;
    has_path_state_ = false;
    cycle_time_ = 0.0;
    axis_group_state_ = MOTION_AXIS_GROUP_STANDSTILL;
    error_code_ = MOTION_BUFFER_OK;
}

int GroupBuffer::bind(const uint8_t *axes, uint8_t count, uint64_t mask, double cycle) {
    int ret = ensure_sample(count);
    if (ret != 0) {
        return ret;
    }
    ret = ensure_base_storage(count);
    if (ret != 0) {
        return ret;
    }
    base_count_ = count;
    base_mask_ = mask;
    for (uint8_t i = 0; i < count; ++i) {
        base_axes_[i] = axes[i];
    }
    cycle_time_ = cycle;
    return MOTION_BUFFER_OK;
}

int GroupBuffer::submit_command(const PendingCommandNode &command, const double *current_position,
                                const double *current_velocity,
                                const double *current_acceleration) {
    if (segments_.empty()) {
        // An empty group starts and, on error, stops from the retained all-axis PVA.
        for (uint8_t i = 0; i < base_count_; ++i) {
            const uint8_t axis = base_axes_[i];
            next_begin_pos_[i] = current_position[axis];
            sample_position_[i] = current_position[axis];
            sample_velocity_[i] = current_velocity[axis];
            sample_acceleration_[i] = current_acceleration[axis];
            fSavePosition[i] = current_position[axis];
            fSaveVelocity[i] = current_velocity[axis];
            fSaveAcceleration[i] = current_acceleration[axis];
        }
    }

    SegmentNode *node = segment_pool_->allocate();
    if (!node) {
        signal_error(ZMC_ERR_MOTBUF_FULL);
        return ZMC_ERR_MOTBUF_FULL;
    }

    // Stack remap buffers remain valid until the command is applied below.
    double remap_a[kMaxAxisMaskBits];
    double remap_b[kMaxAxisMaskBits];
    const double *param_a = command.param_a;
    const double *param_b = command.kind == SEGMENT_COMMAND_LINE ? NULL : command.param_b;
    if (!bases_order_equal(command.base_axes, command.base_count, base_axes_, base_count_)) {
        remap_params(command.base_axes, command.base_count, base_axes_, command.param_a, remap_a);
        param_a = remap_a;
        if (param_b) {
            remap_params(command.base_axes, command.base_count, base_axes_, command.param_b,
                         remap_b);
            param_b = remap_b;
        }
    }

    int ret = node->bind(this);
    if (ret == 0) {
        ret = apply_command(command, *node, param_a, param_b);
    }
    if (ret == 0) {
        ret = build_and_append(node);
    }
    if (ret != 0) {
        signal_error(ret);
        segment_pool_->deallocate(node);
    }
    return ret;
}

void GroupBuffer::set_cycle_time(double cycle) { cycle_time_ = cycle; }

uint64_t GroupBuffer::base_mask() const { return base_mask_; }

MotionAxisGroupState GroupBuffer::state() const { return axis_group_state_; }

int GroupBuffer::error_code() const { return error_code_; }

void GroupBuffer::signal_error(int error_code) {
    // Latch both ERRORSTOP and the concrete code; reset() is the only clearing path.
    error_code_ = error_code;
    axis_group_state_ = MOTION_AXIS_GROUP_ERRORSTOP;
}

void GroupBuffer::update_axis_group_state(const MotionAxInput *axis_input) {
    // ERRORSTOP is latched at group scope and reset() is its only clearing path.
    if (axis_group_state_ == MOTION_AXIS_GROUP_ERRORSTOP) {
        return;
    }
    MotionAxisGroupState aggregate = MOTION_AXIS_GROUP_DISABLE;
    for (uint8_t i = 0; i < base_count_; ++i) {
        const MotionAxisState axis = axis_input[base_axes_[i]].state;
        if (axis == MOTION_AXIS_ERRORSTOP) {
            aggregate = MOTION_AXIS_GROUP_ERRORSTOP;
        } else if (axis == MOTION_AXIS_STOPPING && aggregate < MOTION_AXIS_GROUP_STOPPING) {
            aggregate = MOTION_AXIS_GROUP_STOPPING;
        } else if (axis == MOTION_AXIS_MOVING && aggregate < MOTION_AXIS_GROUP_MOVING) {
            aggregate = MOTION_AXIS_GROUP_MOVING;
        } else if (axis == MOTION_AXIS_STANDSTILL && aggregate < MOTION_AXIS_GROUP_STANDSTILL) {
            aggregate = MOTION_AXIS_GROUP_STANDSTILL;
        }
    }
    axis_group_state_ = aggregate;
}

int GroupBuffer::rebuild_profiles() {
    // Lookahead consumes fixed-capacity arrays only while this group is being replanned.
    double distance[MOTION_BUFFER_LOOKAHEAD_CAPACITY];
    double max_acceleration[MOTION_BUFFER_LOOKAHEAD_CAPACITY];
    double max_deceleration[MOTION_BUFFER_LOOKAHEAD_CAPACITY];
    double max_jerk[MOTION_BUFFER_LOOKAHEAD_CAPACITY];
    double junction_velocity[MOTION_BUFFER_LOOKAHEAD_CAPACITY + 1];
    // Subgroup always starts from rest; path_state_ then tracks in-group motion.
    if (!has_path_state_) {
        path_state_.position = 0.0;
        path_state_.velocity = 0.0;
        path_state_.acceleration = 0.0;
        has_path_state_ = true;
    }

    const int32_t queued_segment_count = segments_.size();
    const int32_t lookahead_count = queued_segment_count < MOTION_BUFFER_LOOKAHEAD_CAPACITY
                                        ? queued_segment_count
                                        : MOTION_BUFFER_LOOKAHEAD_CAPACITY;
    int32_t i = 0;
    for (SegmentNode *node = segments_.front(); node != NULL && i < lookahead_count;
         node = node->get_next(), ++i) {
        const double begin = i == 0 ? path_state_.position : 0.0;
        distance[i] = node->path_length() - begin;
        if (distance[i] <= kGeometryEpsilon) {
            return ZMC_ERR_MOTBUF_PLAN_FAILED;
        }
        max_acceleration[i] = node->limits().max_acceleration;
        max_deceleration[i] = node->limits().max_deceleration;
        max_jerk[i] = node->limits().max_jerk;
    }

    junction_velocity[0] = path_state_.velocity;
    SegmentNode *node = segments_.front();
    for (i = 0; i + 1 < lookahead_count; ++i) {
        const MotionSegment &current = *node;
        const MotionSegment &next = *node->get_next();
        junction_velocity[i + 1] = current.is_tangent_to(next)
                                       ? (current.limits().max_speed < next.limits().max_speed
                                              ? current.limits().max_speed
                                              : next.limits().max_speed)
                                       : 0.0;
        node = node->get_next();
    }
    // Keep the bounded horizon stoppable; popping the front segment advances this boundary.
    junction_velocity[lookahead_count] = 0.0;

    if (lookahead_relaxation(junction_velocity, distance, lookahead_count, max_acceleration,
                             max_deceleration, max_jerk) != 0) {
        return ZMC_ERR_MOTBUF_PLAN_FAILED;
    }

    i = 0;
    for (node = segments_.front(); node != NULL && i < lookahead_count;
         node = node->get_next(), ++i) {
        const int ret = node->build_candidate_profile(
            i == 0 ? path_state_.position : 0.0, junction_velocity[i],
            i == 0 ? path_state_.acceleration : 0.0, junction_velocity[i + 1]);
        if (ret != 0 || node->candidate_end_time() < 0.0) {
            return ret != 0 ? ret : ZMC_ERR_MOTBUF_PLAN_FAILED;
        }
    }
    i = 0;
    for (node = segments_.front(); node != NULL && i < lookahead_count;
         node = node->get_next(), ++i) {
        node->commit_candidate_profile();
    }
    planned_segment_count_ = static_cast<uint8_t>(lookahead_count);
    segment_time_ = 0.0;
    return MOTION_BUFFER_OK;
}

int GroupBuffer::consume_cycle_entry_events() {
    const uint8_t entry_events = event_.value;
    if (entry_events == 0) {
        return MOTION_BUFFER_OK;
    }

    // Queue changes are facts; only a confirmed window slide or override triggers planning.
    const bool needs_lookahead = is_lookahead_window_sliding() || event_.isOverride;
    if (!needs_lookahead) {
        event_.value = static_cast<uint8_t>(event_.value & ~entry_events);
        return MOTION_BUFFER_OK;
    }

    const int ret = rebuild_profiles();
    if (ret == MOTION_BUFFER_OK) {
        event_.value = static_cast<uint8_t>(event_.value & ~entry_events);
    }
    return ret;
}

bool GroupBuffer::is_lookahead_window_sliding() const {
    return (event_.isEnqueue || event_.isDequeue) && segments_.size() > planned_segment_count_;
}

void GroupBuffer::pop_front_segment() {
    MotionSegment *completed = segments_.pop_front();
    if (planned_segment_count_ > 0) {
        --planned_segment_count_;
    }
    event_.isDequeue = true;
    segment_time_ = 0.0;
    if (segments_.empty()) {
        for (uint8_t i = 0; i < base_count_; ++i) {
            sample_velocity_[i] = 0.0;
            sample_acceleration_[i] = 0.0;
        }
        path_state_.velocity = 0.0;
        path_state_.acceleration = 0.0;
        remaining_time_ = 0.0;
        axis_group_state_ = MOTION_AXIS_GROUP_STANDSTILL;
    } else {
        path_state_.position = 0.0;
        has_path_state_ = true;
        axis_group_state_ = MOTION_AXIS_GROUP_MOVING;
    }
    // Returning the current node to the pool must be the final operation on this segment.
    segment_pool_->deallocate(completed);
}

void GroupBuffer::get_slice_segment(MotionSegment *segment) {
    // The group owns the sample buffers while the specified segment owns the active profile.
    segment->get_slice(segment_time_, &path_state_, sample_position_, sample_velocity_,
                       sample_acceleration_);
}

void GroupBuffer::rebind_segments() {
    for (SegmentNode *node = segments_.front(); node != NULL; node = node->get_next()) {
        node->rebind_group(this);
    }
}

int GroupBuffer::append_node(SegmentNode *node) {
    segments_.push_back(node);
    event_.isEnqueue = true;
    node->get_end_position(next_begin_pos_);
    return MOTION_BUFFER_OK;
}

int GroupBuffer::build_and_append(SegmentNode *node) {
    // Geometry starts at the cached tail; next_begin_pos_ is already in group-local order.
    const int ret = node->build_trajectory(next_begin_pos_);
    if (ret != 0) {
        return ret;
    }
    return append_node(node);
}

void GroupBuffer::expand_pva(double *position, double *velocity, double *acceleration) const {
    for (uint8_t i = 0; i < base_count_; ++i) {
        const uint8_t axis = base_axes_[i];
        position[axis] = sample_position_[i];
        velocity[axis] = sample_velocity_[i];
        acceleration[axis] = sample_acceleration_[i];
    }
}

void GroupBuffer::save_pva() {
    for (uint8_t i = 0; i < base_count_; ++i) {
        fSavePosition[i] = sample_position_[i];
        fSaveVelocity[i] = sample_velocity_[i];
        fSaveAcceleration[i] = sample_acceleration_[i];
    }
}

void GroupBuffer::run_errorstop_cycle() {
    remaining_time_ = 0.0;
    _onSwErrorstop();
    save_pva();
}

void GroupBuffer::_onSwErrorstop() {
    for (uint8_t i = 0; i < base_count_; ++i) {
        const PosVelAcc pva = {fSavePosition[i], fSaveVelocity[i], fSaveAcceleration[i]};
        double deceleration = kSoftwareErrorStopDeceleration;
        if (fSaveAcceleration[i] > 0.0 && fSaveVelocity[i] < 0.0) {
            deceleration =
                deceleration >= fSaveAcceleration[i] ? deceleration : fSaveAcceleration[i];
        } else if (fSaveAcceleration[i] < 0.0 && fSaveVelocity[i] > 0.0) {
            deceleration =
                deceleration >= -fSaveAcceleration[i] ? deceleration : -fSaveAcceleration[i];
        }

        // The group uses the existing zero maximum-distance rule from AxisImpl.
        const PosVelAcc next = calc_next_decel_pva(pva, deceleration, cycle_time_);
        sample_position_[i] = next.pos;
        sample_velocity_[i] = next.vel;
        sample_acceleration_[i] = next.acc;
    }
}

void GroupBuffer::runCycle() {
    if (axis_group_state_ == MOTION_AXIS_GROUP_ERRORSTOP) {
        run_errorstop_cycle();
        return;
    }

    if (empty()) {
        remaining_time_ = 0.0;
        axis_group_state_ = MOTION_AXIS_GROUP_STANDSTILL;
        save_pva();
        return;
    }

    remaining_time_ = cycle_time_;
    const int ret = consume_cycle_entry_events();
    if (ret != MOTION_BUFFER_OK) {
        signal_error(ret);
        run_errorstop_cycle();
        return;
    }

    while (!empty() && planned_segment_count_ > 0 && remaining_time_ > kGeometryEpsilon) {
        segments_.front()->runCycleBottom();
        if (axis_group_state_ == MOTION_AXIS_GROUP_ERRORSTOP) {
            // Discard any temporary node sample and stop from the last published PVA.
            run_errorstop_cycle();
            return;
        }
    }

    if (!empty() && planned_segment_count_ == 0 && remaining_time_ > kGeometryEpsilon) {
        // A cycle never consumes beyond its planned horizon and never performs a second lookahead.
        remaining_time_ = 0.0;
    }
    save_pva();
}

void GroupBuffer::set_junction_mode(MotionJunctionMode mode) {
    // junction_mode_ = mode;
}
}  // namespace zmc
