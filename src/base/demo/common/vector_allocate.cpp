// Author: zmc
// Date: 2026-01-14
// Description: vector 分配器 demo（共享内存池）
#include <iostream>
#include <vector>
#include <cstddef>
#include <memory>
#include <limits>
#include <new>
#include <string>
#include <utility>
#include "common/misc/StatelessAllocator.hpp"

// ==========================================================
// 1. 简单的共享内存池 (本例使用简单的 malloc 包装，演示共享逻辑)
// ==========================================================
class SimplePool {
public:
    explicit SimplePool(const std::string& name) : name_(name), allocated_(0) {}

    void* allocate(std::size_t n) {
        allocated_ += n;
        void* p = std::malloc(n);
        std::cout << "[" << name_ << "] Allocating " << n << " bytes at " << p 
                  << " (Total: " << allocated_ << ")\n";
        return p;
    }

    void deallocate(void* p, std::size_t n) {
        allocated_ -= n;
        std::cout << "[" << name_ << "] Deallocating " << n << " bytes at " << p << " (Total: " << allocated_ << ")\n";
        std::free(p);
    }

    const std::string& name() const { return name_; }

private:
    std::string name_;
    std::size_t allocated_;
};

// ==========================================================
// 2. 有状态分配器 (C++11 风格)
// ==========================================================
template <typename T>
class SharedAlloc {
public:
    using value_type = T;

    // 必须提供 rebind，容器内部（如树形结构或内部节点）分配时需要
    template <typename U>
    struct rebind {
        using other = SharedAlloc<U>;
    };

    // 持有指向池的指针
    SimplePool* pool;

    // 构造函数
    explicit SharedAlloc(SimplePool* p) noexcept : pool(p) {}

    // 拷贝构造函数 (允许从 SharedAlloc<U> 构造 SharedAlloc<T>)
    template <typename U>
    SharedAlloc(const SharedAlloc<U>& other) noexcept : pool(other.pool) {}

    T* allocate(std::size_t n) {
        if (!pool) throw std::bad_alloc();
        return static_cast<T*>(pool->allocate(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept {
        if (pool) pool->deallocate(p, n * sizeof(T));
    }
};

// 相等性比较：指向同一个池的分配器被认为是等价的
template <typename T, typename U>
bool operator==(const SharedAlloc<T>& a, const SharedAlloc<U>& b) {
    return a.pool == b.pool;
}

template <typename T, typename U>
bool operator!=(const SharedAlloc<T>& a, const SharedAlloc<U>& b) {
    return a.pool != b.pool;
}

// ==========================================================
// 3. 使用用例
// ==========================================================
int main2() {
    // 创建一个唯一的内存池
    SimplePool my_shared_pool("GlobalPool");

    // 创建分配器实例
    SharedAlloc<int> alloc_for_v1(&my_shared_pool);
    SharedAlloc<double> alloc_for_v2(&my_shared_pool);

    std::cout << "--- Initializing Vectors ---\n";

    // 关键点：在构造函数中传入分配器实例
    std::vector<int, SharedAlloc<int>> vec1(alloc_for_v1);
    // std::vector<double, SharedAlloc<double>> vec1(alloc_for_v2);
    std::vector<double, SharedAlloc<double>> vec2(alloc_for_v2);

    std::cout << "\n--- Vector 1 Operations ---\n";
    vec1.push_back(1);
    vec1.push_back(2);

    std::cout << "\n--- Vector 2 Operations ---\n";
    vec2.push_back(1.1);
    vec2.push_back(2.2);

    std::cout << "\n--- Vector 1 Reallocating (Capacity Growth) ---\n";
    for(int i=0; i<10; ++i) vec1.push_back(i);
    std::cout << vec2[0] <<"," << vec2[1] << std::endl;

    std::cout << "\n--- Summary ---\n";
    std::cout << "Vec1 size: " << vec1.size() << ", Pool used by: " << vec1.get_allocator().pool->name() << "\n";
    std::cout << "Vec2 size: " << vec2.size() << ", Pool used by: " << vec2.get_allocator().pool->name() << "\n";

    std::cout << "\n--- Cleanup (Vectors go out of scope) ---\n";
    return 0;
}


// ==========================================================
// 5. 使用 MemoryAllocator 的示例
// ==========================================================
int main3() {
    std::cout << "=== MemoryAllocator Stateless Allocator Demo ===\n\n";

    // Check sizes (allocator should not add to vector size)
    std::cout << "sizeof(zmc::StatelessAllocatorMA<int>): " 
              << sizeof(zmc::StatelessAllocatorMA<int>) << std::endl;
    
    // Create vector without passing allocator as parameter
    // Allocator is default-constructed (stateless, uses global pool)
    std::vector<int, zmc::StatelessAllocatorMA<int>> vec;

    std::cout << "sizeof(vector<int, zmc::StatelessAllocatorMA<int>>): " 
              << sizeof(vec) << std::endl;
    std::cout << "Note: Allocator uses empty base optimization, adds no size\n\n";

    std::cout << "--- Vector Operations ---\n";
    
    // Use vector normally - all allocations go through global MemoryAllocator pool
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 10);
        std::cout << "Pushed " << (i * 10) << ", size: " << vec.size() 
                  << ", capacity: " << vec.capacity() << std::endl;
    }

    std::cout << "\n--- Vector Contents ---\n";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "vec[" << i << "] = " << vec[i] << std::endl;
    }

    // Create another vector with different type (shares same global pool)
    std::cout << "\n--- Creating Another Vector (double) ---\n";
    std::vector<double, zmc::StatelessAllocatorMA<double>> vec2;
    vec2.push_back(1.5);
    vec2.push_back(2.5);
    vec2.push_back(3.5);
    
    std::cout << "vec2 size: " << vec2.size() << std::endl;
    for (size_t i = 0; i < vec2.size(); ++i) {
        std::cout << "vec2[" << i << "] = " << vec2[i] << std::endl;
    }

    // Test capacity growth (reallocation)
    std::cout << "\n--- Testing Capacity Growth ---\n";
    std::cout << "Initial capacity: " << vec.capacity() << std::endl;
    for (int i = 10; i < 20; ++i) {
        vec.push_back(i * 10);
    }
    std::cout << "After growth, capacity: " << vec.capacity() << std::endl;
    std::cout << "All reallocations use MemoryAllocator pool\n";

    std::cout << "\n--- Summary ---\n";
    std::cout << "All memory allocations managed by global MemoryAllocator pool\n";
    std::cout << "No allocator instance stored in vector (empty base optimization)\n";
    std::cout << "No need to pass allocator as constructor parameter\n";

    std::cout << "\n--- Cleanup (Vectors go out of scope) ---\n";
    return 0;
}