#ifndef _ZMC_FB_MOVE_LIST_HPP_
#define _ZMC_FB_MOVE_LIST_HPP_

#include "MotCmd.hpp"

namespace zmc {


// Type-erased wrapper for move function blocks
// Base interface with virtual runCycle method
class FbMoveErasedVirtual {
    public:
        virtual ~FbMoveErasedVirtual() = default;
        virtual void runCycle() = 0;
};
class FbMoveErasedBase:public FbMoveErasedVirtual {
     public:
        virtual ~FbMoveErasedBase() = default;
        void runCycle() override{};
        virtual void reset_runtime() {};
        inline void* getPtr() { return fb_; }
        inline const void* getPtr() const { return fb_; }
        template<typename T>
        inline T* getPtr() { return (T*) fb_; }
        template<typename T>
        inline const T* get() const { return (const T*) fb_; }

        void setPool(void* pool) {
            pool_ptr_ = pool;
        };
        void* getPool() const {
            return pool_ptr_;
        }
        
        // Intrusive list methods (non-virtual)
        void setNext(FbMoveErasedBase* next) {
            next_ = next;
        }
        
        FbMoveErasedBase* getNext() const {
            return next_;
        }
        
        void setCmdMid(const CommandMidDefault& cmd) { cmdMid_ = cmd; }
        const CommandMidDefault& getCmdMid() const { return cmdMid_; }
        CommandMidDefault& getCmdMid() { return cmdMid_; }
    protected:
        void* fb_;
        void* pool_ptr_ = nullptr;
        FbMoveErasedBase* next_ = nullptr; // use for pool_ptr_
        CommandMidDefault cmdMid_;
    };
    
    // Template type-erased wrapper for any function block with runCycle method
    template<typename T>
    class FbMoveErasedTmpl : public FbMoveErasedBase {
    public:
        // explicit FbMoveErasedTmpl(T* fb) : fb_(fb) {}
        FbMoveErasedTmpl() { fb_ = new T(); }
        virtual ~FbMoveErasedTmpl() {
            delete fb_;
        }
        void runCycle() override {
            if (fb_) {
                ((T*)fb_)->runCycle();
            }
        }
        void reset_runtime() override {
            if (fb_) {
                ((T*)fb_)->Execute = false;
                if (((T*)fb_)->Axis) {
                    ((T*)fb_)->runCycle();
                    ((T*)fb_)->runCycle();
                }
            }
            cmdMid_ = CommandMidDefault();
        }
        inline T* get() { return getPtr<T>(); }
        inline const T* get() const { return getPtr<T>(); }

    };


    template<typename T>
    class FbMoveCmdTmpl{
    public:
        // explicit FbMoveErasedTmpl(T* fb) : fb_(fb) {}
        FbMoveCmdTmpl() { fb_ = new T(); }
        ~FbMoveCmdTmpl() {
            delete fb_;
            fb_ = nullptr;
        }
        void runCycle(){
            if (fb_) {
                ((T*)fb_)->runCycle();
            }
        }
        inline T* get() { return fb_; }
        inline const T* get() const { return fb_; }
        inline const CommandMidDefault& getCmdMid() const { return cmdMid_; }
        inline CommandMidDefault& getCmdMid() { return cmdMid_; }
       protected:
        T* fb_;
        // void* pool_ptr_ = nullptr;
        FbMoveCmdTmpl<T>* next_ = nullptr; // use for pool_ptr_ or queue
        CommandMidDefault cmdMid_;
    };

/**
 * @brief Simple sentinel node class for FbMoveList
 */
class FbMoveListSentinel : public FbMoveErasedBase {
public:
    FbMoveListSentinel() {
        // Initialize protected members safely
        fb_ = nullptr;
        next_ = nullptr;
        pool_ptr_ = nullptr;
    }
};

/**
 * @brief Intrusive linked list with sentinel node for managing FbMoveErasedBase objects
 * 
 * Uses the same getNext()/setNext() mechanism as ObjectPoolAllocator, but for "used" nodes.
 * The sentinel node simplifies boundary handling, and tail pointer enables O(1) append.
 */
class FbMoveList {
private:
    FbMoveListSentinel sentinel_;    // Sentinel node (dummy head)
    FbMoveErasedBase* tail_;         // Tail pointer for O(1) append
    size_t size_;

public:
    FbMoveList() : tail_(&sentinel_), size_(0) {
        sentinel_.setNext(nullptr);
    }

    ~FbMoveList() {
        clear();
    }

    // Disable copy
    // FbMoveList(const FbMoveList&) = delete;
    // FbMoveList& operator=(const FbMoveList&) = delete;

    /**
     * @brief Append a node to the tail of the list (O(1))
     * @param node Pointer to the node to append
     */
    void pushBack(FbMoveErasedBase* node) {
        if (!node) return;
        tail_->setNext(node);
        node->setNext(nullptr);
        tail_ = node;
        size_++;
    }

    /**
     * @brief Remove the next node after prev (simplified deletion)
     * @param prev Pointer to the previous node (can be &sentinel_)
     * @return Pointer to the removed node, or nullptr if no next node
     */
    FbMoveErasedBase* removeNext(FbMoveErasedBase* prev) {
        if (!prev) return nullptr;
        
        FbMoveErasedBase* node = prev->getNext();
        if (!node) return nullptr;
        
        prev->setNext(node->getNext());
        // Update tail if removing the last node
        if (node == tail_) {
            tail_ = prev;
        }
        node->setNext(nullptr);
        size_--;
        return node;
    }

    /**
     * @brief Clear all nodes from the list
     */
    void clear() {
        FbMoveErasedBase* current = sentinel_.getNext();
        while (current) {
            FbMoveErasedBase* next = current->getNext();
            current->setNext(nullptr);
            current = next;
        }
        sentinel_.setNext(nullptr);
        tail_ = &sentinel_;
        size_ = 0;
    }

    /**
     * @brief Get the number of nodes in the list
     */
    size_t size() const {
        return size_;
    }

    /**
     * @brief Check if the list is empty
     */
    bool empty() const {
        return size_ == 0;
    }

    /**
     * @brief Get iterator to first node (for range-based for loop)
     */
    FbMoveErasedBase* begin() {
        return sentinel_.getNext();
    }

    /**
     * @brief Get iterator end marker (for range-based for loop)
     */
    FbMoveErasedBase* end() {
        return nullptr;
    }

    /**
     * @brief Get const iterator to first node
     */
    const FbMoveErasedBase* begin() const {
        return sentinel_.getNext();
    }

    /**
     * @brief Get const iterator end marker
     */
    const FbMoveErasedBase* end() const {
        return nullptr;
    }

    /**
     * @brief Get sentinel node pointer (for iteration with prev pointer)
     */
    FbMoveErasedBase* getSentinel() {
        return &sentinel_;
    }
};
}
#endif
