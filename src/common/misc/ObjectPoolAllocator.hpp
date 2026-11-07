#ifndef OBJECT_POOL_ALLOCATOR_HPP
#define OBJECT_POOL_ALLOCATOR_HPP

#include <cstddef>
#include <array>

/**
 * @brief Base class interface for object pool, used for type erasure
 */
// class ObjectPoolBase {
// public:
//     virtual void push(void* obj) = 0;
//     virtual ~ObjectPoolBase() = default;
// };
/**
 * @brief Intrusive Object Pool Allocator (Head pop/push only)
 * 
 * This allocator uses intrusive design: objects managed by the pool must implement
 * the methods setNext(T*) and getNext(), which allow intrusive singly-linked free-list management.
 * 
 * - All objects are pre-constructed and held internally.
 * - Allocation (pop) and deallocation (push) always operate at the head of the free list.
 * - The pool never expands or shrinks at runtime.
 * 
 * @note Object type T must implement:
 *   void setNext(T* next);
 *   T* getNext() const;
 * 
 * Usage scenario:
 *   - Extremely fast constant-time allocation and deallocation.
 *   - No dynamic allocation/deallocation; ideal for high-performance systems or embedded.
 *   - Not thread-safe by default.
 */

/**
 * @brief Fixed-size object pool allocator
 * 
 * Constructs all objects during initialization, uses pop/push operations at runtime
 * 
 * @tparam T Object type (must have setNext(T*) and getNext() methods)
 * @tparam BlockCount Total number of objects in the pool
 */
template <typename T, std::size_t BlockCount>
class ObjectPoolAllocator {
private:
    T* free_list_head_ = nullptr;
    std::array<T, BlockCount> storage_;


public:
    /**
     * @brief Constructor: constructs all objects and adds them to free list
     */
    ObjectPoolAllocator() {
        // Link into free list (objects are already constructed by std::array)
        free_list_head_ = storage_.data();
        for (std::size_t i = 0; i < BlockCount - 1; ++i) {
            storage_[i].setNext(&storage_[i + 1]);
        }
        storage_[BlockCount - 1].setNext(nullptr);
    }

    // Disable copy
    ObjectPoolAllocator(const ObjectPoolAllocator&) = delete;
    ObjectPoolAllocator& operator=(const ObjectPoolAllocator&) = delete;

    /**
     * @brief Pop an object from the head of free list
     * @return T* Pointer to the object, returns nullptr if pool is empty
     */
    T* pop() {
        if (!free_list_head_) return nullptr;

        T* obj = free_list_head_;
        free_list_head_ = static_cast<T*>(obj->getNext());
        return obj;
    }

    /**
     * @brief Push an object back to the head of free list
     * @param obj Pointer to the object to be recycled
     */
    void push(T* obj) {
        if (!obj) return;

        obj->setNext(free_list_head_);
        free_list_head_ = obj;
    }

    // Return every pre-constructed object to the free list.
    void reset() {
        free_list_head_ = storage_.data();
        for (std::size_t i = 0; i < BlockCount - 1; ++i) {
            storage_[i].setNext(&storage_[i + 1]);
        }
        storage_[BlockCount - 1].setNext(nullptr);
    }

    // Query interface
    static constexpr std::size_t object_size() { return sizeof(T); }
    static constexpr std::size_t capacity() { return BlockCount; }
};

#endif // OBJECT_POOL_ALLOCATOR_HPP
