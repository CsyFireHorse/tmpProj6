/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-27
 * @ArtifactStatusAuthority: Composer
 *
 * Storage-backed MotionSegment implementation. Not part of the public include surface.
 */
#ifndef ZMC_MOTION_SEGMENT_UN_H_
#define ZMC_MOTION_SEGMENT_UN_H_

#include <stdint.h>

#include "MotionSegment.h"
#include "TrajCppWrap.h"
#include "vel/include/VelUnCppWrap.h"

namespace zmc {

// Impl body placed into MotionSegment::storage_ via placement-new.
class MotionSegmentUn {
   public:
    MotionSegmentUn();
    MotionSegmentUn(MotionSegmentUn &&) noexcept;
    MotionSegmentUn &operator=(MotionSegmentUn &&) noexcept;
    ~MotionSegmentUn() {}

    MotionSegment *get_next() const;
    void set_next(MotionSegment *node);

    int bind(GroupBuffer *group);
    void rebind_group(GroupBuffer *group);
    void runCycleBottom();

    void set_allocator(void *pAlloc);
    int init_axis_num(uint8_t axis_count);
    uint8_t axis_count() const;

    int set_line(const double *target, MotionCoordinateMode coordinate_mode,
                 const MotionLimits &limits);
    int set_arc_through(const double *through_point, const double *end_point,
                        MotionCoordinateMode coordinate_mode, const MotionLimits &limits);
    int set_arc_center(const double *center, const double *end_point, MotionArcDirection direction,
                       MotionCoordinateMode coordinate_mode, const MotionLimits &limits);

    int build_trajectory(const double *begin_position);
    int build_candidate_profile(double begin_position, double begin_velocity,
                                double begin_acceleration, double end_velocity);
    void commit_candidate_profile();

    // Sample an executable profile into group-owned state and PVA storage.
    int get_slice(double time, MotionPathState *path_state, double *position, double *velocity,
                  double *acceleration) const;

    void get_end_position(double *position) const;
    bool is_tangent_to(const MotionSegmentUn &next) const;

    const MotionLimits &limits() const;
    double path_length() const;
    double end_time() const;
    double candidate_end_time() const;

   private:
    MotionSegmentUn(const MotionSegmentUn &);
    MotionSegmentUn &operator=(const MotionSegmentUn &);

    void reset_command_state();
    bool is_configured() const;
    void get_begin_tangent(double *tangent) const;
    void get_end_tangent(double *tangent) const;

    // Intrusive linkage and runtime ownership belong to the storage-backed implementation.
    MotionSegment *next_;
    GroupBuffer *group_;

    TrajWrap traj;
    VelocityWrap profiles[2];

    uint8_t axis_count_;
    uint8_t active_bank;
    MotionSegmentType type_;
    MotionCoordinateMode coordinate_mode;
    MotionArcDirection direction;
    MotionLimits limits_;
};

}  // namespace zmc

#endif  // ZMC_MOTION_SEGMENT_UN_H_
