#include <cstddef>
#include <cstdint>
#include <new>
#include <vector>
#include <cassert>
#include <iostream>

// 辅助工具：获取位图中的第一个零位 (找空闲槽)
static inline int find_first_free(uint64_t bitmap, int limit) {
    if (bitmap == ~0ULL) return -1;
    // 使用内置指令计算末尾零的个数 (__builtin_ctzll)
    int bit = __builtin_ctzll(~bitmap);
    return (bit < limit) ? bit : -1;
}

/**
 * @brief Slab 结构定义
 * 
 * 管理一块连续内存，内部划分为多个 Slots
 */
struct Slab {
    uint8_t* memory;      // 实际对象存储区域
    uint64_t usage_mask;  // 位图 (支持最多 64 个对象/Slab)
    uint8_t  active_count; // 已分配对象数
    uint8_t  total_slots;  // 总槽位数
    Slab*    next;
    Slab*    prev;

    Slab(void* mem, size_t slot_size, int count) 
        : memory(static_cast<uint8_t*>(mem)), usage_mask(0), 
          active_count(0), total_slots(count), next(nullptr), prev(nullptr) {}

    bool is_full() const { return active_count == total_slots; }
    bool is_empty() const { return active_count == 0; }

    void* allocate_slot(size_t slot_size) {
        int idx = find_first_free(usage_mask, total_slots);
        if (idx == -1) return nullptr;
        
        usage_mask |= (1ULL << idx);
        active_count++;
        return memory + (idx * slot_size);
    }

    bool deallocate_slot(void* ptr, size_t slot_size) {
        size_t offset = static_cast<uint8_t*>(ptr) - memory;
        int idx = offset / slot_size;
        if (!(usage_mask & (1ULL << idx))) return false; // Double free check

        usage_mask &= ~(1ULL << idx);
        active_count--;
        return true;
    }
};

/**
 * @brief kmem_cache
 * 
 * 管理特定对象类型的缓存。
 */
template <typename T>
class KmemCache {
public:
    KmemCache(size_t slab_capacity = 64) 
        : obj_size(sizeof(T)), 
          slots_per_slab(slab_capacity > 64 ? 64 : slab_capacity),
          full_slabs(nullptr), partial_slabs(nullptr), empty_slabs(nullptr) {
        // 确保对象大小满足指针或对齐要求
        if (obj_size < sizeof(void*)) obj_size = sizeof(void*);
    }

    T* alloc() {
        Slab* target = nullptr;

        // 1. 优先从部分空闲的 Slab 分配 (最优路径)
        if (partial_slabs) {
            target = partial_slabs;
        } 
        // 2. 其次从全空的 Slab 分配
        else if (empty_slabs) {
            target = empty_slabs;
            move_slab(&empty_slabs, &partial_slabs, target);
        }
        // 3. 最后创建新的 Slab
        else {
            target = create_new_slab();
            add_to_list(&partial_slabs, target);
        }

        void* ptr = target->allocate_slot(obj_size);
        
        // 如果 Slab 满了，移动到 full 链表
        if (target->is_full()) {
            move_slab(&partial_slabs, &full_slabs, target);
        }

        return new (ptr) T(); // 执行构造函数 (Object Caching)
    }

    void free(T* ptr) {
        ptr->~T(); // 执行析构函数
        
        // 寻找该指针属于哪个 Slab (实际应用中通常通过 Page Header 查找，这里简化模拟)
        Slab* s = find_slab_by_ptr(ptr);
        assert(s != nullptr);

        bool was_full = s->is_full();
        s->deallocate_slot(ptr, obj_size);

        if (s->is_empty()) {
            // 从当前链表移动到 empty
            Slab** current_list = was_full ? &full_slabs : &partial_slabs;
            move_slab(current_list, &empty_slabs, s);
            
            // 策略：如果 empty 过多，可以释放内存。这里保留以复用。
        } else if (was_full) {
            move_slab(&full_slabs, &partial_slabs, s);
        }
    }
    /**
     * @brief 预分配指定数量的 Slab
     * @param slab_count 需要预分配的 Slab 个数
     */
    void reserve_slabs(size_t slab_count) {
        for (size_t i = 0; i < slab_count; ++i) {
            Slab* s = create_new_slab();
            add_to_list(&empty_slabs, s);
        }
    }

    /**
     * @brief 确保 Cache 中至少能容纳指定数量的对象
     * @param object_count 总对象容量目标
     */
    void reserve_objects(size_t object_count) {
        // 计算当前总容量（这里仅计算空闲和部分空闲的槽位比较复杂，通常简单处理为总槽位）
        // 实际工程中，我们通常直接根据目标数计算需要补足多少个 Slab
        size_t current_slabs = count_all_slabs(); // 需辅助函数
        size_t target_slabs = (object_count + slots_per_slab - 1) / slots_per_slab;
        
        if (target_slabs > current_slabs) {
            reserve_slabs(target_slabs - current_slabs);
        }
    }

    private:
    // 辅助工具：统计当前已有的 Slab 总数
    size_t count_all_slabs() const {
        auto count_list = [](Slab* head) {
            size_t c = 0;
            while (head) { c++; head = head->next; }
            return c;
        };
        return count_list(full_slabs) + count_list(partial_slabs) + count_list(empty_slabs);
    }
private:
    size_t obj_size;
    int slots_per_slab;

    Slab* full_slabs;
    Slab* partial_slabs;
    Slab* empty_slabs;

    // 模拟操作系统底层的内存页分配
    Slab* create_new_slab() {
        size_t total_mem = obj_size * slots_per_slab;
        void* raw_mem = ::operator new(total_mem); 
        return new Slab(raw_mem, obj_size, slots_per_slab);
    }

    // 链表管理辅助函数
    void add_to_list(Slab** list, Slab* s) {
        s->next = *list;
        if (*list) (*list)->prev = s;
        *list = s;
        s->prev = nullptr;
    }

    void remove_from_list(Slab** list, Slab* s) {
        if (s->prev) s->prev->next = s->next;
        else *list = s->next;
        if (s->next) s->next->prev = s->prev;
    }

    void move_slab(Slab** from, Slab** to, Slab* s) {
        remove_from_list(from, s);
        add_to_list(to, s);
    }

    // 实际实现中，这里通过指针高位地址计算 Page Index 极快 (O(1))
    // 演示版本使用简单的全局记录（仅作示意）
    Slab* find_slab_by_ptr(void* ptr) {
        // 这里仅为逻辑完整性，实际 OS 中这步是不需要遍历的
        auto check = [&](Slab* head) -> Slab* {
            for (Slab* c = head; c; c = c->next) {
                if (ptr >= c->memory && ptr < (c->memory + obj_size * slots_per_slab))
                    return c;
            }
            return nullptr;
        };
        Slab* res = check(partial_slabs);
        if (!res) res = check(full_slabs);
        return res;
    }
};