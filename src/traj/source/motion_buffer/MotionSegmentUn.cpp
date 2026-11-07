/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-25
 * @ArtifactStatusAuthority: user
 */
#include "MotionSegmentUn.h"

#include <cmath>

#include "GroupBuffer.h"
#include "common/zmc/zmc_predef.h"

namespace zmc {

using std::isfinite;

namespace {

const double kProfileModeTolerance = 1e-6;
const double kTangentDotTolerance = 1e-9;

double dot(const double *lhs, const double *rhs, uint8_t count) {
    double result = 0.0;
    for (uint8_t i = 0; i < count; ++i) {
        result += lhs[i] * rhs[i];
    }
    return result;
}

bool valid_limits(const MotionLimits &limits) {
    return isfinite(limits.max_speed) && limits.max_speed > 0.0 &&
           isfinite(limits.max_acceleration) && limits.max_acceleration > 0.0 &&
           isfinite(limits.max_deceleration) && limits.max_deceleration > 0.0 &&
           isfinite(limits.max_jerk) && limits.max_jerk >= 0.0;
}

bool valid_coordinate_mode(MotionCoordinateMode mode) {
    return mode == MOTION_COORDINATE_ABSOLUTE || mode == MOTION_COORDINATE_RELATIVE;
}

int check_coordinates(const double *source, uint8_t axis_count) {
    if (!source) {
        return MOTION_SEGMENT_ERR_INVALID_ARGUMENT;
    }
    for (uint8_t i = 0; i < axis_count; ++i) {
        if (!isfinite(source[i])) {
            return MOTION_SEGMENT_ERR_INVALID_ARGUMENT;
        }
    }
    return MOTION_SEGMENT_OK;
}

}  // namespace

MotionSegmentUn::MotionSegmentUn()
    : next_(NULL),
      group_(NULL),
      traj(1),
      axis_count_(0),
      active_bank(0),
      type_(MOTION_SEGMENT_NONE),
      coordinate_mode(MOTION_COORDINATE_ABSOLUTE),
      direction(MOTION_ARC_COUNTERCLOCKWISE) {
    limits_.max_speed = 0.0;
    limits_.max_acceleration = 0.0;
    limits_.max_deceleration = 0.0;
    limits_.max_jerk = 0.0;
}

MotionSegmentUn::MotionSegmentUn(MotionSegmentUn &&other) noexcept
    : next_(other.next_),
      group_(other.group_),
      traj(other.traj),
      axis_count_(other.axis_count_),
      active_bank(other.active_bank),
      type_(other.type_),
      coordinate_mode(other.coordinate_mode),
      direction(other.direction),
      limits_(other.limits_) {
    profiles[0] = other.profiles[0];
    profiles[1] = other.profiles[1];
    other.next_ = NULL;
    other.group_ = NULL;
    other.reset_command_state();
    other.axis_count_ = 0;
}

MotionSegmentUn &MotionSegmentUn::operator=(MotionSegmentUn &&other) noexcept {
    if (this != &other) {
        next_ = other.next_;
        group_ = other.group_;
        traj = other.traj;
        profiles[0] = other.profiles[0];
        profiles[1] = other.profiles[1];
        axis_count_ = other.axis_count_;
        active_bank = other.active_bank;
        type_ = other.type_;
        coordinate_mode = other.coordinate_mode;
        direction = other.direction;
        limits_ = other.limits_;
        other.next_ = NULL;
        other.group_ = NULL;
        other.reset_command_state();
        other.axis_count_ = 0;
    }
    return *this;
}

MotionSegment *MotionSegmentUn::get_next() const { return next_; }

void MotionSegmentUn::set_next(MotionSegment *node) { next_ = node; }

int MotionSegmentUn::bind(GroupBuffer *group) {
    group_ = group;
    set_allocator(group_->get_allocator());
    return init_axis_num(group_->base_count_);
}

void MotionSegmentUn::rebind_group(GroupBuffer *group) { group_ = group; }

void MotionSegmentUn::runCycleBottom() {
    const double profile_end_time = end_time();
    if (!isfinite(profile_end_time) || !isfinite(group_->segment_time_) ||
        !isfinite(group_->remaining_time_) || profile_end_time < 0.0 ||
        group_->segment_time_ < 0.0 || group_->remaining_time_ < 0.0 ||
        group_->segment_time_ > profile_end_time) {
        // Invalid time ownership must stop before this node writes any output.
        group_->signal_error(ZMC_ERR_MOTBUF_INVALID_STATE);
        return;
    }

    const double time_to_end = profile_end_time - group_->segment_time_;
    if (group_->remaining_time_ < time_to_end) {
        group_->segment_time_ += group_->remaining_time_;
        group_->remaining_time_ = 0.0;
        // runCycleBottom executes on the queue front, which is the segment being sampled.
        group_->get_slice_segment(group_->segments_.front());
        group_->axis_group_state_ = MOTION_AXIS_GROUP_MOVING;
        return;
    }

    group_->segment_time_ = profile_end_time;
    group_->get_slice_segment(group_->segments_.front());
    group_->remaining_time_ -= time_to_end;

    // pop_front_segment() recycles this node, so no member access is allowed afterward.
    group_->pop_front_segment();
    return;
}

void MotionSegmentUn::reset_command_state() {
    active_bank = 0;
    type_ = MOTION_SEGMENT_NONE;
    coordinate_mode = MOTION_COORDINATE_ABSOLUTE;
    direction = MOTION_ARC_COUNTERCLOCKWISE;
    limits_.max_speed = 0.0;
    limits_.max_acceleration = 0.0;
    limits_.max_deceleration = 0.0;
    limits_.max_jerk = 0.0;
}

void MotionSegmentUn::set_allocator(void *pAlloc) {
    // Bind the caller-owned allocator before trajectory init can request cache.
    traj.set_allocator(pAlloc);
}

int MotionSegmentUn::init_axis_num(uint8_t axis_count) {
    if (axis_count == 0) {
        return MOTION_SEGMENT_ERR_INVALID_ARGUMENT;
    }

    const int ret = traj.init_axis_num(axis_count);
    if (ret != 0) {
        return ret;
    }
    // Reset command state only when the stored axis count changes.
    if (axis_count_ != axis_count) {
        axis_count_ = axis_count;
        reset_command_state();
    }
    return MOTION_SEGMENT_OK;
}

int MotionSegmentUn::set_line(const double *target, MotionCoordinateMode coordinate_mode,
                              const MotionLimits &limits) {
    if (axis_count_ == 0) {
        return MOTION_SEGMENT_ERR_NOT_INITIALIZED;
    }
    if (!valid_coordinate_mode(coordinate_mode) || !valid_limits(limits)) {
        return MOTION_SEGMENT_ERR_INVALID_ARGUMENT;
    }
    const int ret = check_coordinates(target, axis_count_);
    if (ret != 0) {
        return ret;
    }
    traj.set_line_param(target, coordinate_mode == MOTION_COORDINATE_RELATIVE);
    type_ = MOTION_SEGMENT_LINE;
    this->coordinate_mode = coordinate_mode;
    limits_ = limits;
    return MOTION_SEGMENT_OK;
}

int MotionSegmentUn::set_arc_through(const double *through_point, const double *end_point,
                                     MotionCoordinateMode coordinate_mode,
                                     const MotionLimits &limits) {
    if (axis_count_ == 0) {
        return MOTION_SEGMENT_ERR_NOT_INITIALIZED;
    }
    if (axis_count_ != 2 && axis_count_ != 3) {
        return MOTION_SEGMENT_ERR_NOT_SUPPORTED;
    }
    if (!valid_coordinate_mode(coordinate_mode) || !valid_limits(limits)) {
        return MOTION_SEGMENT_ERR_INVALID_ARGUMENT;
    }
    int ret = check_coordinates(through_point, axis_count_);
    if (ret == 0) {
        ret = check_coordinates(end_point, axis_count_);
    }
    if (ret != 0) {
        return ret;
    }
    traj.set_circ_param(through_point, end_point, false,
                        coordinate_mode == MOTION_COORDINATE_RELATIVE, CIRC_MODE_BORDER);
    type_ = MOTION_SEGMENT_ARC_THROUGH;
    this->coordinate_mode = coordinate_mode;
    limits_ = limits;
    return MOTION_SEGMENT_OK;
}

int MotionSegmentUn::set_arc_center(const double *center, const double *end_point,
                                    MotionArcDirection direction,
                                    MotionCoordinateMode coordinate_mode,
                                    const MotionLimits &limits) {
    if (axis_count_ == 0) {
        return MOTION_SEGMENT_ERR_NOT_INITIALIZED;
    }
    if (axis_count_ != 2 && axis_count_ != 3) {
        return MOTION_SEGMENT_ERR_NOT_SUPPORTED;
    }
    if ((direction != MOTION_ARC_CLOCKWISE && direction != MOTION_ARC_COUNTERCLOCKWISE) ||
        !valid_coordinate_mode(coordinate_mode) || !valid_limits(limits)) {
        return MOTION_SEGMENT_ERR_INVALID_ARGUMENT;
    }
    int ret = check_coordinates(center, axis_count_);
    if (ret == 0) {
        ret = check_coordinates(end_point, axis_count_);
    }
    if (ret != 0) {
        return ret;
    }
    traj.set_circ_param(center, end_point, direction == MOTION_ARC_COUNTERCLOCKWISE,
                        coordinate_mode == MOTION_COORDINATE_RELATIVE, CIRC_MODE_CENTER);
    type_ = MOTION_SEGMENT_ARC_CENTER;
    this->coordinate_mode = coordinate_mode;
    this->direction = direction;
    limits_ = limits;
    return MOTION_SEGMENT_OK;
}

int MotionSegmentUn::build_trajectory(const double *begin_position) {
    if (axis_count_ == 0) {
        return MOTION_SEGMENT_ERR_NOT_INITIALIZED;
    }
    if (!begin_position || !is_configured()) {
        return MOTION_SEGMENT_ERR_INVALID_ARGUMENT;
    }

    traj.set_begin_pos(begin_position);
    return traj.build();
}

int MotionSegmentUn::build_candidate_profile(double begin_position, double begin_velocity,
                                             double begin_acceleration, double end_velocity) {
    if (axis_count_ == 0) {
        return MOTION_SEGMENT_ERR_NOT_INITIALIZED;
    }
    if (!isfinite(begin_position) || !isfinite(begin_velocity) || !isfinite(begin_acceleration) ||
        !isfinite(end_velocity)) {
        return MOTION_SEGMENT_ERR_INVALID_ARGUMENT;
    }

    // Replanning writes only the inactive bank until the complete queue succeeds.
    VelocityWrap &profile = profiles[1 - active_bank];
    profile.set_mode(limits_.max_jerk < kProfileModeTolerance ? VEL_MODE_TRAPEZOID
                                                              : VEL_MODE_QUADRATIC);
    profile.set_config(limits_.max_speed, limits_.max_acceleration, limits_.max_deceleration,
                       limits_.max_jerk);
    profile.set_begin_pva(begin_position, begin_velocity, begin_acceleration);
    profile.set_end_pva(path_length(), end_velocity, 0.0);
    return profile.build();
}

void MotionSegmentUn::commit_candidate_profile() {
    if (axis_count_ == 0) {
        return;
    }
    // Bank ownership is local to the segment; committing is an allocation-free index flip.
    active_bank = 1 - active_bank;
}

int MotionSegmentUn::get_slice(double time, MotionPathState *path_state, double *position,
                               double *velocity, double *acceleration) const {
    // First sample scalar motion, then map it through geometry to every axis PVA.
    const int ret = profiles[active_bank].get_slice(
        time, &path_state->position, &path_state->velocity, &path_state->acceleration);
    traj.get_slice3(path_state->position, path_state->velocity, path_state->acceleration, position,
                    velocity, acceleration);
    return ret;
}

void MotionSegmentUn::get_end_position(double *position) const {
    if (axis_count_ != 0) {
        traj.get_end_pos(position);
    }
}

void MotionSegmentUn::get_begin_tangent(double *tangent) const {
    if (axis_count_ != 0) {
        traj.get_slice(0.0, NULL, tangent);
    }
}

void MotionSegmentUn::get_end_tangent(double *tangent) const {
    if (axis_count_ != 0) {
        traj.get_slice(path_length(), NULL, tangent);
    }
}

bool MotionSegmentUn::is_tangent_to(const MotionSegmentUn &next) const {
    if (axis_count_ == 0 || next.axis_count_ == 0) {
        return false;
    }
    const uint8_t n = axis_count();
    // Both unit tangents must remain available until their dot product is evaluated.
    double a[MC_MOTION_MAX_AXES];
    double b[MC_MOTION_MAX_AXES];
    get_end_tangent(a);
    next.get_begin_tangent(b);
    return dot(a, b, n) >= 1.0 - kTangentDotTolerance;
}

uint8_t MotionSegmentUn::axis_count() const { return axis_count_; }

const MotionLimits &MotionSegmentUn::limits() const { return limits_; }

double MotionSegmentUn::path_length() const { return axis_count_ != 0 ? traj.get_plan() : 0.0; }

double MotionSegmentUn::end_time() const {
    return axis_count_ != 0 ? profiles[active_bank].get_end_time() : 0.0;
}

double MotionSegmentUn::candidate_end_time() const {
    return axis_count_ != 0 ? profiles[1 - active_bank].get_end_time() : 0.0;
}

bool MotionSegmentUn::is_configured() const {
    return axis_count_ != 0 && type_ != MOTION_SEGMENT_NONE;
}

}  // namespace zmc
