/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-26
 * @ArtifactStatusAuthority: GPT-5 Codex
 */
#ifndef ZMC_PENDING_COMMAND_LIST_H_
#define ZMC_PENDING_COMMAND_LIST_H_

#include <stddef.h>
#include <stdint.h>

#include "List.h"
#include "MotionSegment.h"

namespace zmc {

enum SegmentCommandKind {
    SEGMENT_COMMAND_LINE = 0,
    SEGMENT_COMMAND_ARC_THROUGH,
    SEGMENT_COMMAND_ARC_CENTER
};

// One pending command. Storage for base_axes/param_* is borrowed from Impl workspace.
struct PendingCommandNode {
    PendingCommandNode();

    // Copy a validated command into this node's fixed buffers.
    void store_command(const uint8_t *axes, uint8_t count, const double *param_a,
                       const double *param_b, SegmentCommandKind kind,
                       MotionCoordinateMode coordinate_mode, MotionArcDirection direction,
                       const MotionLimits &limits);

    PendingCommandNode *get_next() const { return next_; }
    void set_next(PendingCommandNode *node) { next_ = node; }

    uint8_t base_count;
    uint8_t *base_axes;
    // Order-independent axis set; bit i means axis i participates (axis_count <= 64).
    uint64_t base_mask;
    double *param_a;
    double *param_b;
    SegmentCommandKind kind;
    MotionCoordinateMode coordinate_mode;
    MotionArcDirection direction;
    MotionLimits limits;

   private:
    PendingCommandNode *next_;
};

// FIFO used only by the global pending-command barrier.
struct PendingCommandList : List<PendingCommandNode> {};

// LIFO free list that owns its PendingCommandNode storage.
struct PendingCommandPool : Pool<PendingCommandNode> {};

}  // namespace zmc

#endif  // ZMC_PENDING_COMMAND_LIST_H_
