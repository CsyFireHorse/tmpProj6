#ifndef INTRUSIVE_DEQUE_HPP
#define INTRUSIVE_DEQUE_HPP

#include <cstddef>
#include <array>

/**
 * @brief Intrusive double-ended queue with sentinel nodes
 * 
 * This deque uses intrusive design: objects managed must implement
 * setNext(T*), getNext(), setPrev(T*), and getPrev() methods.
 * 
 * - Push operations occur at the tail (back)
 * - Pop operations occur at the head (front)
 * - Head and tail sentinel nodes ensure push/pop isolation
 * 
 * @note Object type T must implement:
 *   void setNext(T* next);
 *   T* getNext() const;
 *   void setPrev(T* prev);
 *   T* getPrev() const;
 * 
 * @tparam T Object type
 * @tparam BlockCount Total number of objects in the pool
 */
template <typename T, std::size_t BlockCount>
class IntrusiveDeque {
private:
    std::array<T, BlockCount + 2> storage_;
    T* head_sentinel_ = nullptr;
    T* tail_sentinel_ = nullptr;

public:
    /**
     * @brief Constructor: constructs all objects and initializes sentinels
     */
    IntrusiveDeque() {
        // Initialize sentinels (first and last elements in storage_)
        head_sentinel_ = &storage_[0];
        tail_sentinel_ = &storage_[BlockCount + 1];
        
        // Link sentinels together
        head_sentinel_->setNext(&storage_[1]);
        head_sentinel_->setPrev(nullptr);
        for (std::size_t i = 1; i <= BlockCount; ++i) {
            T* curr = &storage_[i];
            T* prev = &storage_[i - 1];
            T* next = &storage_[i + 1];
            curr->setPrev(prev);
            curr->setNext(next);
        }
        tail_sentinel_->setNext(nullptr);
        tail_sentinel_->setPrev(&storage_[BlockCount]);
    }

    // Disable copy
    IntrusiveDeque(const IntrusiveDeque&) = delete;
    IntrusiveDeque& operator=(const IntrusiveDeque&) = delete;

    /**
     * @brief Pop an object from the head (front) of the deque
     * @return T* Pointer to the object, returns nullptr if deque is empty
     */
    T* pop() {
        T* first = head_sentinel_->getNext();
        if (first == tail_sentinel_) return nullptr;  // Empty

        // Remove from deque
        T* second = first->getNext();
        head_sentinel_->setNext(second);
        second->setPrev(head_sentinel_);
        
        // Clear links
        first->setNext(nullptr);
        first->setPrev(nullptr);
        
        return first;
    }

    /**
     * @brief Push an object to the tail (back) of the deque
     * @param obj Pointer to the object to be added
     */
    void push(T* obj) {
        if (!obj) return;

        // Insert before tail sentinel
        T* last = tail_sentinel_->getPrev();
        last->setNext(obj);
        obj->setPrev(last);
        obj->setNext(tail_sentinel_);
        tail_sentinel_->setPrev(obj);
    }

    /**
     * @brief Check if deque is empty
     * @return true if empty, false otherwise
     */
    inline bool empty() const {
        return head_sentinel_->getNext() == tail_sentinel_;
    }

    // Query interface
    static constexpr std::size_t capacity() { return BlockCount; }
};

#endif // INTRUSIVE_DEQUE_HPP
