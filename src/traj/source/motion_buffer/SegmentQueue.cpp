/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-25
 * @ArtifactStatusAuthority: user
 */
#include "SegmentQueue.h"

namespace zmc {

SegmentList::SegmentList() {}

SegmentList::SegmentList(SegmentList &&other) noexcept {
    head = other.head;
    tail = other.tail;
    count = other.count;
    other.head = NULL;
    other.tail = NULL;
    other.count = 0;
}

SegmentList &SegmentList::operator=(SegmentList &&other) noexcept {
    if (this != &other) {
        head = other.head;
        tail = other.tail;
        count = other.count;
        other.head = NULL;
        other.tail = NULL;
        other.count = 0;
    }
    return *this;
}

void SegmentList::swap(SegmentList &other) {
    if (this == &other) {
        return;
    }
    SegmentNode *head_temp = head;
    SegmentNode *tail_temp = tail;
    const int32_t count_temp = count;
    head = other.head;
    tail = other.tail;
    count = other.count;
    other.head = head_temp;
    other.tail = tail_temp;
    other.count = count_temp;
}

SegmentNode *SegmentList::pop_back() {
    if (!tail) {
        return NULL;
    }
    SegmentNode *node = tail;
    if (head == tail) {
        head = NULL;
        tail = NULL;
    } else {
        SegmentNode *prev = head;
        while (prev->get_next() != tail) {
            prev = prev->get_next();
        }
        prev->set_next(NULL);
        tail = prev;
    }
    node->set_next(NULL);
    --count;
    return node;
}

}  // namespace zmc
