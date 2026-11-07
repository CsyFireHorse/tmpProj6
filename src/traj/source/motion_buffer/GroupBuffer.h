/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-26
 * @ArtifactStatusAuthority: GLM-5.2
 */
#ifndef ZMC_GROUP_BUFFER_H_
#define ZMC_GROUP_BUFFER_H_

#include <stdint.h>

#include "MotionBufferTypes.h"
#include "SegmentQueue.h"

namespace zmc {

class MotionSegmentUn;
class PmAllocator;
struct PendingCommandNode;

// Record queue changes for the next GroupBuffer cycle.
union GroupBufferEvent {
    uint8_t value;
    struct {
        uint8_t isEnqueue : 1;
        uint8_t isDequeue : 1;
        uint8_t isOverride : 1;
    };
};

// One BASE-bound command lane that owns its runtime state.
struct GroupBuffer {
    GroupBuffer();
    ~GroupBuffer();
    // Move relocates owned storage, sample ownership, and SegmentList; copy stays disabled.
    GroupBuffer(GroupBuffer &&other) noexcept;
    GroupBuffer &operator=(GroupBuffer &&other) noexcept;

    // Exchange all group-owned state without constructing a temporary GroupBuffer.
    void swap(GroupBuffer &other);

    // Borrow the shared allocator and segment pool used by this group.
    void set_resources(PmAllocator *allocator, SegmentPool *segment_pool);

    // Release queued segments, free owned buffers, and restore the unbound state.
    void reset();
    // True when this group has no queued trajectory segments.
    bool empty() const;

    // Bind the ordered BASE, BASE mask, and cycle time.
    int bind(const uint8_t *axes, uint8_t count, uint64_t mask, double cycle);

    // Submit one validated command and perform all group-owned state changes.
    int submit_command(const PendingCommandNode &command, const double *current_position,
                       const double *current_velocity, const double *current_acceleration);

    // Advance one cycle and sample into the group-owned PVA buffers.
    void runCycle();
    // Expand group-local sample PVA into caller all-axis buffers.
    void expand_pva(double *position, double *velocity, double *acceleration) const;

    // Update the cycle time through the group boundary.
    void set_cycle_time(double cycle);

    // Return the order-independent BASE mask used by global dispatch.
    uint64_t base_mask() const;

    // Return the single group-owned runtime state without exposing writable storage.
    MotionAxisGroupState state() const;

    // Return the latched group error code; MOTION_BUFFER_OK when no error is stored.
    int error_code() const;

    // Aggregate external single-axis states into the group-owned state.
    void update_axis_group_state(const MotionAxInput *axis_input);
    void set_junction_mode(MotionJunctionMode mode);

   private:
    GroupBuffer(const GroupBuffer &);
    GroupBuffer &operator=(const GroupBuffer &);

    friend class MotionSegment;
    friend class MotionSegmentUn;

    // Latch ERRORSTOP and retain the concrete group error code for output.
    void signal_error(int error_code);

    // Borrowed allocator, or static AllocatorDefault (malloc) when unset — matches TrajBase.
    PmAllocator *get_allocator() const;
    // Grow or allocate current and saved PVA for at least axes doubles per channel.
    int ensure_sample(uint8_t axes);
    // Return the single six-channel PVA block to get_allocator().
    void deallocate_sample();
    // Grow or allocate BASE index and next-begin pose storage for at least axes.
    int ensure_base_storage(uint8_t axes);
    // Return BASE storage blocks to get_allocator().
    void deallocate_base_storage();

    // Rebuild the bounded lookahead profiles owned by this group.
    int rebuild_profiles();
    // Consume entry events, check window sliding, and execute at most one lookahead.
    int consume_cycle_entry_events();
    // Return whether a queue change exposed an unplanned segment inside the bounded horizon.
    bool is_lookahead_window_sliding() const;
    // Remove and recycle the completed front segment.
    void pop_front_segment();
    // Sample the specified segment into this group's runtime state and PVA storage.
    void get_slice_segment(MotionSegment *segment);
    // Save the last group-local PVA published by this cycle.
    void save_pva();
    // Execute the group process around one software-error-stop algorithm step.
    void run_errorstop_cycle();
    // Generate one software-error-stop sample from the last published group PVA.
    void _onSwErrorstop();
    // Restore borrowed group pointers after a GroupBuffer move or swap.
    void rebind_segments();
    // Append one built node and record an enqueue event.
    int append_node(SegmentNode *node);
    // Build trajectory geometry and append one node.
    int build_and_append(SegmentNode *node);

    uint8_t base_count_;
    uint64_t base_mask_;
    SegmentList segments_;
    GroupBufferEvent event_;
    // Number of front segments with an executable profile in the bounded horizon.
    uint8_t planned_segment_count_;
    double segment_time_;
    // Time not yet consumed from the current external runCycle call.
    double remaining_time_;
    MotionPathState path_state_;
    bool has_path_state_;
    double cycle_time_;
    SegmentPool *segment_pool_;

    MotionAxisGroupState axis_group_state_;
    // Latched until reset(); published through MotionAxOutput::errorCode.
    int error_code_;

    // Optional borrowed traj heap; NULL falls back to AllocatorDefault.
    PmAllocator *allocator_;
    double *sample_position_;
    double *sample_velocity_;
    double *sample_acceleration_;
    double *fSavePosition;
    double *fSaveVelocity;
    double *fSaveAcceleration;
    uint8_t sample_capacity_;

    // Owned ordered BASE and next-begin pose; capacity tracks allocated length.
    uint8_t *base_axes_;
    double *next_begin_pos_;
    uint8_t base_capacity_;
};

}  // namespace zmc

#endif  // ZMC_GROUP_BUFFER_H_
