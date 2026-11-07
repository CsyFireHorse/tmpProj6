#ifndef _ZMC_RINGBUFFER_EASY_HPP_
#define _ZMC_RINGBUFFER_EASY_HPP_
#include <array>

namespace zmc{

// Simple ring buffer for single-thread use (overwrites oldest on full)
template<typename T, unsigned int Capacity>
class RingBufferEasy {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
private:
    std::array<T, Capacity> buffer_;
    unsigned int head_ = 0;  // write position
    unsigned int tail_ = 0;  // read position
    static constexpr unsigned int MASK = Capacity - 1;
    
public:
    // Push item, overwrites oldest if full
    bool push(const T& item) {
        unsigned int next = (head_ + 1) & MASK;
        if (next == tail_) {
            // Full: overwrite oldest (move tail forward)
            tail_ = (tail_ + 1) & MASK;
        }
        buffer_[head_] = item;
        head_ = next;
        return true;
    }
    
    // Pop oldest item
    bool pop(T& item) {
        if (tail_ == head_) return false;  // empty
        item = buffer_[tail_];
        tail_ = (tail_ + 1) & MASK;
        return true;
    }
    // pop front without return item
    bool pop_front() {
        if (tail_ == head_) return false;  // empty
        tail_ = (tail_ + 1) & MASK;
        return true;
    }
    // Peek oldest item without removing
    bool front(T& item) const {
        if (tail_ == head_) return false;
        item = buffer_[tail_];
        return true;
    }
    T  front() const {
        // if (tail_ == head_) return buffer_[tail_];
        return buffer_[tail_];
    }
    // Read a queued item without removing it; caller keeps offset below size().
    const T& at(unsigned int offset) const { return buffer_[(tail_ + offset) & MASK]; }
    inline bool full() const { return ((head_ + 1) & MASK) == tail_; }
    inline bool empty() const { return tail_ == head_; }
    inline unsigned int size() const { return (head_ - tail_) & MASK; }
    inline unsigned int capacity() const { return Capacity - 1; }
    // Discard all queued items while retaining the fixed storage.
    void clear() { head_ = tail_ = 0; }
};
}
#endif
