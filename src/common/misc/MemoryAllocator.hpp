#ifndef _ZMC_MEMORY_ALLOCATOR_HPP_
#define _ZMC_MEMORY_ALLOCATOR_HPP_

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace zmc{
class MemoryAllocator {
public:
    // 构造函数初始化内存分配器
    explicit MemoryAllocator(size_t heap_size = 1024 * 1024) 
        : heap_size_(heap_size) {
        initialize();
    }

    // 析构函数释放整个堆
    ~MemoryAllocator() {
        std::free(heap_start_);
    }

    // 分配内存
    void* allocate(size_t size) {
        if (size == 0) return nullptr;

        // 计算需要分配的块大小（对齐处理）
        size_t total_size = align_size(size + HEADER_SIZE + FOOTER_SIZE);
        if (total_size < MIN_BLOCK_SIZE) {
            #if 0
            throw std::bad_alloc();
            #else
            total_size = MIN_BLOCK_SIZE;
            #endif
        }

        // 遍历空闲链表（首次适应算法）
        for (auto* current = free_list_.next; current != &free_list_; current = current->next) {
            BlockHeader* header = get_header_from_node(current);
            const size_t block_size = header->get_size();

            if (block_size >= total_size) {
                // 计算分割后剩余大小
                const size_t remaining_size = block_size - total_size;
                
                if (remaining_size >= MIN_BLOCK_SIZE) {
                    // 分割块：创建分配块和新的空闲块
                    header->set_size_and_allocated(total_size, true);
                    
                    // 创建新的空闲块
                    BlockHeader* new_free_header = get_next_block(header);
                    new_free_header->set_size_and_allocated(remaining_size, false);
                    
                    // 更新空闲链表
                    insert_free_block(new_free_header);
                } else {
                    // 整个块分配（不分割）
                    header->set_allocated(true);
                }
                
                // 从空闲链表移除当前块
                remove_free_block(current);
                
                // 返回用户数据区地址
                return header->user_ptr();
            }
        }
        return nullptr;
        // throw std::bad_alloc(); // 没有足够内存
    }

    // 释放内存
    void deallocate(void* ptr) {
        if (!ptr) return;
        
        // 检查指针是否属于本分配器
        // if (!is_pointer_valid(ptr)) {
            // throw std::runtime_error("Attempt to free invalid pointer");
        // }
        // 获取块头部
        BlockHeader* header = BlockHeader::from_user_ptr(ptr);
        header->set_allocated(false);
        
        // 尝试合并相邻空闲块
        BlockHeader* new_header = coalesce_block(header);
        
        // 将合并后的块加入空闲链表
        insert_free_block(new_header);
    }
bool is_pointer_valid(void* ptr) const {
    // 1. 空指针检查
    if (!ptr) return false;
    
    // 2. 堆范围检查
    char* char_ptr = static_cast<char*>(ptr);
    if (char_ptr < heap_start_ || char_ptr >= heapEnd()) {
        return false;
    }
    
    // 3. 获取头部指针
    BlockHeader* header = BlockHeader::from_user_ptr(ptr);
    
    // 4. 头部地址有效性检查
    if (reinterpret_cast<char*>(header) < heap_start_ || 
        HEADER_SIZE >= ((char*)heapEnd()) - reinterpret_cast<char*>(header) ) {
        return false;
    }
    
    // 5. 块大小有效性检查
    if (header->get_size() < MIN_BLOCK_SIZE) {
        return false;
    }
    
    // 6. 头部/脚部一致性检查
    if (!header->is_footer_valid()) {
        return false;
    }
    
    // 7. 分配状态检查
    return header->is_allocated();
}
private:
    // 内存块头部/脚部结构
    class BlockHeader {
    public:
        // 设置块大小和分配状态
        void set_size_and_allocated(size_t size, bool allocated) {
            size_ = size | (allocated ? 1 : 0);
            get_footer()->size_ = size_;
        }

        // 设置分配状态
        void set_allocated(bool allocated) {
            if (allocated) {
                size_ |= 1;
            } else {
                size_ &= ~1;
            }
            get_footer()->size_ = size_;
        }

        // 获取块大小（清除分配位）
        size_t get_size() const { 
            return size_ & ~1; 
        }
        // 检查是否已分配
        bool is_allocated() const { 
            return !is_free(); 
        }
        // 检查是否空闲
        bool is_free() const { 
            return (size_ & 1) == 0; 
        }

        // 获取用户指针
        void* user_ptr() { 
            return reinterpret_cast<char*>(this) + HEADER_SIZE; 
        }

        // 从用户指针获取头部指针
        static BlockHeader* from_user_ptr(void* ptr) {
            return reinterpret_cast<BlockHeader*>(static_cast<char*>(ptr) - HEADER_SIZE);
        }

        // 获取脚部指针
        BlockHeader* get_footer() {
            return reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(this) + get_size() - FOOTER_SIZE);
        }
        bool is_footer_valid() {
            return get_footer()->size_ == size_;
        }
    private:
        size_t size_;  // 块大小（最低位表示分配状态）
    };

    // 空闲链表节点结构（存储在空闲块中）
    struct FreeListNode {
        FreeListNode* prev;
        FreeListNode* next;
    };

    // 常量定义
    static constexpr size_t ALIGNMENT = 8;         // 对齐要求
    static constexpr size_t MIN_BLOCK_SIZE = 32;   // 最小块大小
    static constexpr size_t HEADER_SIZE = sizeof(BlockHeader);
    static constexpr size_t FOOTER_SIZE = sizeof(BlockHeader);
    static constexpr size_t MIN_BLOCK_DATA_SIZE = sizeof(FreeListNode);

    // 成员变量
    size_t heap_size_;
    void* heap_start_ = nullptr;
    FreeListNode free_list_;  // 空闲链表哨兵节点

    inline void* heapEnd()const{
        return static_cast<char*>(heap_start_) + heap_size_;
    }
    // 初始化内存分配器
    void initialize() {
        // 申请堆内存
        heap_start_ = std::malloc(heap_size_);
        if (!heap_start_) {
            // throw std::bad_alloc();
            return;
        }

        // 初始化空闲链表
        free_list_.prev = &free_list_;
        free_list_.next = &free_list_;

        // 设置序言块（已分配）
        BlockHeader* prologue = static_cast<BlockHeader*>(heap_start_);
        prologue->set_size_and_allocated(HEADER_SIZE, true);

        // 设置结尾块（在堆末尾，已分配）
        BlockHeader* epilogue = reinterpret_cast<BlockHeader*>(
            static_cast<char*>(heap_start_) + heap_size_ - HEADER_SIZE);
        epilogue->set_size_and_allocated(0, true);

        // 创建初始空闲块（占据剩余空间）
        const size_t free_size = heap_size_ - HEADER_SIZE - HEADER_SIZE;
        BlockHeader* free_header = reinterpret_cast<BlockHeader*>(
            static_cast<char*>(heap_start_) + HEADER_SIZE);
        free_header->set_size_and_allocated(free_size, false);
        
        // 将初始空闲块加入链表
        insert_free_block(free_header);
    }

    // 对齐处理
    static size_t align_size(size_t size) {
        return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    // 获取下一个块的头部
    BlockHeader* get_next_block(BlockHeader* header) {
        return reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(header) + header->get_size());
    }

    // 获取上一个块的头部
    BlockHeader* get_prev_block(BlockHeader* header) {
        // 当前块头部前是上一个块的脚部
        BlockHeader* prev_footer = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(header) - FOOTER_SIZE);
        return reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(header) - prev_footer->get_size());
    }

    // 从空闲链表节点获取头部
    BlockHeader* get_header_from_node(FreeListNode* node) {
        return reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(node) - HEADER_SIZE);
    }

    // 从头部获取空闲链表节点
    FreeListNode* get_node_from_header(BlockHeader* header) {
        return reinterpret_cast<FreeListNode*>(
            reinterpret_cast<char*>(header) + HEADER_SIZE);
    }

    // 将空闲块插入链表
    void insert_free_block(BlockHeader* header) {
        FreeListNode* node = get_node_from_header(header);
        
        node->prev = &free_list_;
        node->next = free_list_.next;
        free_list_.next->prev = node;
        free_list_.next = node;
    }

    // 从空闲链表移除块
    void remove_free_block(FreeListNode* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    // 合并相邻空闲块
    BlockHeader* coalesce_block(BlockHeader* header) {
        size_t total_size = header->get_size();
        BlockHeader* current = header;

        // 检查并合并下一个块
        BlockHeader* next = get_next_block(header);
        if (next->is_free() && next->get_size() > 0) {
            total_size += next->get_size();
            remove_free_block(get_node_from_header(next));
        }

        // 检查并合并上一个块
        if (reinterpret_cast<char*>(header) > static_cast<char*>(heap_start_) + HEADER_SIZE) {
            BlockHeader* prev = get_prev_block(header);
            if (prev->is_free()) {
                total_size += prev->get_size();
                current = prev;
                remove_free_block(get_node_from_header(prev));
            }
        }

        // 设置合并后块的头部和脚部
        current->set_size_and_allocated(total_size, false);
        return current;
    }
};

}
#endif
// // 测试代码
// int main() {
//     try {
//         MemoryAllocator allocator;
        
//         // 分配测试
//         void* p1 = allocator.allocate(100);
//         void* p2 = allocator.allocate(200);
//         void* p3 = allocator.allocate(300);
        
//         std::cout << "Allocated: p1=" << p1 << ", p2=" << p2 << ", p3=" << p3 << "\n";
        
//         // 测试指针验证
//         std::cout << "p1 valid: " << allocator.is_pointer_valid(p1) << "\n";
//         std::cout << "p2 valid: " << allocator.is_pointer_valid(p2) << "\n";
//         std::cout << "p3 valid: " << allocator.is_pointer_valid(p3) << "\n";
        
//         // 测试无效指针
//         int x;
//         std::cout << "Local variable valid: " << allocator.is_pointer_valid(&x) << "\n";
//         std::cout << "Null pointer valid: " << allocator.is_pointer_valid(nullptr) << "\n";
          
//         // 释放测试
//         allocator.deallocate(p2);
//         std::cout << "Freed p2\n";
        
//         allocator.deallocate(p1);
//         std::cout << "Freed p1\n";
        
//         // 重新分配
//         void* p4 = allocator.allocate(150);
//         std::cout << "Allocated p4=" << p4 << "\n";
        
//         allocator.deallocate(p3);
//         allocator.deallocate(p4);
        
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << "\n";
//         return 1;
//     }
    
//     return 0;
// }