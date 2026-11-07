/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-27
 * @ArtifactStatusAuthority: Composer
 *
 * MotionSegment storage_ wrap forwarding (mirrors TrajCppWrap).
 */
#include "MotionSegment.h"

#include <stddef.h>
#include <new>

#include "MotionSegmentUn.h"

namespace zmc {

static inline MotionSegmentUn &toUn(char *storage) {
    return *reinterpret_cast<MotionSegmentUn *>(storage);
}

static inline const MotionSegmentUn &toUn(const char *storage) {
    return *reinterpret_cast<const MotionSegmentUn *>(storage);
}

MotionSegment::MotionSegment() {
    static_assert(sizeof(MotionSegmentUn) <= STORAGE_SIZE, "MotionSegmentUn is too large");
    new (storage_) MotionSegmentUn();
}

MotionSegment::MotionSegment(MotionSegment &&other) noexcept {
    new (storage_) MotionSegmentUn(static_cast<MotionSegmentUn &&>(toUn(other.storage_)));
}

MotionSegment &MotionSegment::operator=(MotionSegment &&other) noexcept {
    if (this != &other) {
        toUn(storage_) = static_cast<MotionSegmentUn &&>(toUn(other.storage_));
    }
    return *this;
}

MotionSegment::~MotionSegment() { toUn(storage_).~MotionSegmentUn(); }

MotionSegment *MotionSegment::get_next() const { return toUn(storage_).get_next(); }

void MotionSegment::set_next(MotionSegment *node) { toUn(storage_).set_next(node); }

int MotionSegment::bind(GroupBuffer *group) { return toUn(storage_).bind(group); }

void MotionSegment::rebind_group(GroupBuffer *group) { toUn(storage_).rebind_group(group); }

int MotionSegment::get_slice(double time, MotionPathState *path_state, double *position,
                             double *velocity, double *acceleration) const {
    return toUn(storage_).get_slice(time, path_state, position, velocity, acceleration);
}

void MotionSegment::runCycleBottom() { toUn(storage_).runCycleBottom(); }

int MotionSegment::set_line(const double *target, MotionCoordinateMode coordinate_mode,
                            const MotionLimits &limits) {
    return toUn(storage_).set_line(target, coordinate_mode, limits);
}

int MotionSegment::set_arc_through(const double *through_point, const double *end_point,
                                   MotionCoordinateMode coordinate_mode,
                                   const MotionLimits &limits) {
    return toUn(storage_).set_arc_through(through_point, end_point, coordinate_mode, limits);
}

int MotionSegment::set_arc_center(const double *center, const double *end_point,
                                  MotionArcDirection direction,
                                  MotionCoordinateMode coordinate_mode,
                                  const MotionLimits &limits) {
    return toUn(storage_).set_arc_center(center, end_point, direction, coordinate_mode, limits);
}

int MotionSegment::build_trajectory(const double *begin_position) {
    return toUn(storage_).build_trajectory(begin_position);
}

int MotionSegment::build_candidate_profile(double begin_position, double begin_velocity,
                                           double begin_acceleration, double end_velocity) {
    return toUn(storage_).build_candidate_profile(begin_position, begin_velocity,
                                                  begin_acceleration, end_velocity);
}

void MotionSegment::commit_candidate_profile() { toUn(storage_).commit_candidate_profile(); }

void MotionSegment::get_end_position(double *position) const {
    toUn(storage_).get_end_position(position);
}

bool MotionSegment::is_tangent_to(const MotionSegment &next) const {
    return toUn(storage_).is_tangent_to(toUn(next.storage_));
}

const MotionLimits &MotionSegment::limits() const { return toUn(storage_).limits(); }

double MotionSegment::path_length() const { return toUn(storage_).path_length(); }

double MotionSegment::end_time() const { return toUn(storage_).end_time(); }

double MotionSegment::candidate_end_time() const { return toUn(storage_).candidate_end_time(); }

}  // namespace zmc
