/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-25
 * @ArtifactStatusAuthority: user
 */
#ifndef ZMC_SEGMENT_QUEUE_H_
#define ZMC_SEGMENT_QUEUE_H_

#include <stddef.h>
#include <stdint.h>

#include "List.h"
#include "MotionSegment.h"

namespace zmc {

// Segment pool/list node is MotionSegment itself (next_ link lives on the wrap).
typedef MotionSegment SegmentNode;

// GroupBuffer trajectory FIFO. O(1) push/pop front; pop_back is O(n) for append rollback only.
struct SegmentList : List<SegmentNode> {
    SegmentList();
    // Relocate FIFO links and empty the source; required by vector<GroupBuffer>.
    SegmentList(SegmentList &&other) noexcept;
    SegmentList &operator=(SegmentList &&other) noexcept;

    // Exchange FIFO ownership without moving through a temporary list.
    void swap(SegmentList &other);

    SegmentNode *back() const { return tail; }
    SegmentNode *pop_back();

   private:
    SegmentList(const SegmentList &);
    SegmentList &operator=(const SegmentList &);
};

// LIFO free list that owns its SegmentNode storage. No heap work in allocate/deallocate.
struct SegmentPool : Pool<SegmentNode> {};

}  // namespace zmc

#endif  // ZMC_SEGMENT_QUEUE_H_
