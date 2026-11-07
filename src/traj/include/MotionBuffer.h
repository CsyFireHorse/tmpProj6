/*
 * @ArtifactStatus: draft
 * @ArtifactStatusDate: 2026-08-25
 * @ArtifactStatusAuthority: GPT-5 Codex
 */
#ifndef ZMC_MOTION_BUFFER_H_
#define ZMC_MOTION_BUFFER_H_

#include <stdint.h>

#include "MotionBufferTypes.h"
#include "MotionSegment.h"

namespace zmc {

// Fixed command/segment node count (former init capacity / slots argument).
#define MOTION_BUFFER_SLOT_CAPACITY 32
// Fixed double pool for pending-command param_a and param_b (former slots * axes * 2).
#define MOTION_BUFFER_COMMAND_COORD_CAPACITY 4096

class MotionBuffer {
   public:
    // Construct a disabled buffer; init() prepares bounded node and workspace storage.
    MotionBuffer();
    ~MotionBuffer();

    // Initialize axis-scaled workspace and the fixed-size command/segment pools.
    // A pooled segment initializes its trajectory cache when it binds to an executing group.
    // Re-entry is permitted: calling init() again tears down the existing state
    // and rebuilds for the new axis count (queued work is discarded, cycle_time
    // resets to its 0.001 default, command PVA is zeroed).
    int init(uint8_t axis_count);

    // Configure the duration advanced by each runCycle() call.
    void set_cycle_time(double cycle_time);

    // Copy the externally supplied all-axis PVA frame while no trajectory is queued.
    int set_current_frame(const MotionFrame &current_frame);

    // Select the participating axis list for later push_* calls. Does not enqueue motion.
    int set_base(const uint8_t *axes, uint8_t count);

    // Select the junction policy while the buffer is at standstill.
    int set_junction_mode(MotionJunctionMode mode);

    // Reserve the override interface; this version performs no override action.
    int set_override(MotionOverrideType type, double factor);

    // Copy external axis feedback without changing the retained command PVA.
    void setInput(MotionAxInput *input, int len);

    // Configure and append a linear command.
    int push_line(const double *target, MotionCoordinateMode coordinate_mode,
                  const MotionLimits &limits);

    // Configure and append an arc passing through an intermediate point.
    int push_arc_through(const double *through_point, const double *end_point,
                         MotionCoordinateMode coordinate_mode, const MotionLimits &limits);

    // Configure and append a center-defined directed arc.
    int push_arc_center(const double *center, const double *end_point,
                        MotionArcDirection direction, MotionCoordinateMode coordinate_mode,
                        const MotionLimits &limits);

    // Advance exactly one configured cycle and retain the all-axis command PVA.
    void runCycle();

    // Copy retained PVA, bound group state, and group error code to caller-owned output.
    void getOutput(MotionAxOutput *output, int len) const;

    // Recycle all pending work and restore the state established by init().
    void reset();

    // queued_count() sums occupied command and segment nodes; free_count() sums both pools.
    // Immediately after successful init, these values are 0 and 2 * MOTION_BUFFER_SLOT_CAPACITY.
    MotionAxisGroupState state(uint8_t axis_index) const;
    uint8_t axis_count() const;
    uint8_t base_count() const;
    int32_t queued_count() const;
    int32_t free_count() const;

   private:
    MotionBuffer(const MotionBuffer &);
    MotionBuffer &operator=(const MotionBuffer &);

    struct Impl;
    Impl *impl_;
};

}  // namespace zmc

#endif  // ZMC_MOTION_BUFFER_H_
