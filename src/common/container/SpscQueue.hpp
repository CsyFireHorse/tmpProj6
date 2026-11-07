#ifndef _ZMC_SPSC_QUEUE_HPP_
#define _ZMC_SPSC_QUEUE_HPP_

#include <atomic>
#include <array>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <memory>
 
 // ============================================================================
 // Configuration: Data Structure Size (64, 128, 256, 512)
 // ============================================================================


 
 // ============================================================================
 // 1. SPSCQueue Template Class - Lock-free Ring Buffer
 // ============================================================================
 
 template<typename T, size_t Capacity>
 class SPSCQueue {
     static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
     
 private:
     // Separate cache lines to avoid false sharing
     alignas(64) std::atomic<size_t> write_pos_{0};
     alignas(64) std::atomic<size_t> read_pos_{0};
    //  alignas(64) 
     std::array<T, Capacity> buffer_;
     
     static constexpr size_t MASK = Capacity - 1;
     
 public:
     // Producer side (called from consumer thread for feedback queue)
     bool try_push(const T& item) {
         const size_t write = write_pos_.load(std::memory_order_relaxed);
         const size_t next_write = (write + 1) & MASK;
         
         // Check if queue is full
         if (next_write == read_pos_.load(std::memory_order_acquire)) {
             return false; // queue full
         }
         
         buffer_[write] = item;
         write_pos_.store(next_write, std::memory_order_release);
         return true;
     }
     
     // Consumer side (called from producer thread for feedback queue)
     bool try_pop(T& item) {
         const size_t read = read_pos_.load(std::memory_order_relaxed);
         
         // Check if queue is empty
         if (read == write_pos_.load(std::memory_order_acquire)) {
             return false; // queue empty
         }
         
         item = buffer_[read];
         read_pos_.store((read + 1) & MASK, std::memory_order_release);
         return true;
     }
     
     // Utilities (non-blocking, can be called from any thread)
     size_t size() const {
         const size_t write = write_pos_.load(std::memory_order_acquire);
         const size_t read = read_pos_.load(std::memory_order_acquire);
         return (write - read) & MASK;
     }
     
     size_t capacity() const { return Capacity; }
     bool empty() const { return size() == 0; }
     bool full() const { return size() == Capacity - 1; }
 };
 
#endif
