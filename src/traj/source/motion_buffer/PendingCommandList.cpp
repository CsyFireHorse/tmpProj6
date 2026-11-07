/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-26
 * @ArtifactStatusAuthority: GPT-5 Codex
 */
#include "PendingCommandList.h"

namespace zmc {

PendingCommandNode::PendingCommandNode()
    : base_count(0),
      base_axes(NULL),
      base_mask(0),
      param_a(NULL),
      param_b(NULL),
      kind(SEGMENT_COMMAND_LINE),
      coordinate_mode(MOTION_COORDINATE_ABSOLUTE),
      direction(MOTION_ARC_COUNTERCLOCKWISE),
      next_(NULL) {
    limits.max_speed = 0.0;
    limits.max_acceleration = 0.0;
    limits.max_deceleration = 0.0;
    limits.max_jerk = 0.0;
}

void PendingCommandNode::store_command(const uint8_t *axes, uint8_t count, const double *param_a,
                                       const double *param_b, SegmentCommandKind kind,
                                       MotionCoordinateMode coordinate_mode,
                                       MotionArcDirection direction,
                                       const MotionLimits &limits) {
    base_count = count;
    base_mask = 0;
    for (uint8_t i = 0; i < count; ++i) {
        base_axes[i] = axes[i];
        base_mask |= (static_cast<uint64_t>(1) << axes[i]);
        this->param_a[i] = param_a[i];
        if (param_b) {
            this->param_b[i] = param_b[i];
        }
    }
    this->kind = kind;
    this->coordinate_mode = coordinate_mode;
    this->direction = direction;
    this->limits = limits;
}

}  // namespace zmc
