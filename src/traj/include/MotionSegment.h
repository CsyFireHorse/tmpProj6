/*
 * @ArtifactStatus: draft
 * @ArtifactStatusDate: 2026-08-27
 * @ArtifactStatusAuthority: Composer
 */
#ifndef ZMC_MOTION_SEGMENT_H_
#define ZMC_MOTION_SEGMENT_H_

#include <stdint.h>

namespace zmc {

struct GroupBuffer;

enum MotionSegmentError {
    MOTION_SEGMENT_OK = 0,
    MOTION_SEGMENT_ERR_INVALID_ARGUMENT = -1,
    MOTION_SEGMENT_ERR_NOT_INITIALIZED = -2,
    MOTION_SEGMENT_ERR_NOT_SUPPORTED = -8
};

enum MotionSegmentType : uint8_t {
    MOTION_SEGMENT_NONE,
    MOTION_SEGMENT_LINE,
    MOTION_SEGMENT_ARC_THROUGH,
    MOTION_SEGMENT_ARC_CENTER
};

enum MotionCoordinateMode : uint8_t { MOTION_COORDINATE_ABSOLUTE, MOTION_COORDINATE_RELATIVE };
enum MotionArcDirection : uint8_t { MOTION_ARC_CLOCKWISE, MOTION_ARC_COUNTERCLOCKWISE };

struct MotionLimits {
    double max_speed;
    double max_acceleration;
    double max_deceleration;
    double max_jerk;
};

struct MotionPathState {
    double position;
    double velocity;
    double acceleration;
};

class MotionSegment {
   public:
    // Embedded traj/profile banks exist after construction; axis_count stays 0
    // until bind initializes the storage for an executing group.
    MotionSegment();
    // vector relocates the complete storage-backed implementation through these.
    MotionSegment(MotionSegment &&) noexcept;
    MotionSegment &operator=(MotionSegment &&) noexcept;
    ~MotionSegment();

    // Forward intrusive list and pool linkage to the storage-backed implementation.
    MotionSegment *get_next() const;
    void set_next(MotionSegment *node);

    // Bind this segment to its executing group and initialize its axis dimension.
    int bind(GroupBuffer *group);

    // Consume group remaining time and update group output, state, and queue.
    void runCycleBottom();

    // Store a linear command without binding its absolute start position.
    int set_line(const double *target, MotionCoordinateMode coordinate_mode,
                 const MotionLimits &limits);

    // Store an arc-through command without binding its absolute start position.
    int set_arc_through(const double *through_point, const double *end_point,
                        MotionCoordinateMode coordinate_mode, const MotionLimits &limits);

    // Store a center-defined arc without binding its absolute start position.
    int set_arc_center(const double *center, const double *end_point, MotionArcDirection direction,
                       MotionCoordinateMode coordinate_mode, const MotionLimits &limits);

    // Build absolute trajectory geometry from the supplied start position.
    int build_trajectory(const double *begin_position);

    // Build without replacing the executable profile, then commit after all segments succeed.
    int build_candidate_profile(double begin_position, double begin_velocity,
                                double begin_acceleration, double end_velocity);
    void commit_candidate_profile();

    // Copy endpoint from the built geometry.
    void get_end_position(double *position) const;

    // True when this segment's end tangent aligns with next's begin tangent.
    bool is_tangent_to(const MotionSegment &next) const;

    // Query limits and active/candidate plan results used by GroupBuffer.
    const MotionLimits &limits() const;
    double path_length() const;
    double end_time() const;
    double candidate_end_time() const;

   private:
    MotionSegment(const MotionSegment &);
    MotionSegment &operator=(const MotionSegment &);

    friend struct GroupBuffer;

    // Update only the borrowed group pointer after GroupBuffer storage moves.
    void rebind_group(GroupBuffer *group);
    // Sample this segment's active profile into caller-owned runtime state and PVA storage.
    int get_slice(double time, MotionPathState *path_state, double *position, double *velocity,
                  double *acceleration) const;

    // Holds MotionSegmentWrap; keep large enough for TrajWrap + VelocityWrap[2] + fields.
    static constexpr int STORAGE_SIZE = 1024;
    char storage_[STORAGE_SIZE];
};

}  // namespace zmc

#endif  // ZMC_MOTION_SEGMENT_H_
