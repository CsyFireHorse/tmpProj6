#ifndef _ZMC_MOT_CMD_HPP_
#define _ZMC_MOT_CMD_HPP_

#include "common/misc/MemoryAllocator.hpp"
#include "common/misc/StatelessAllocator.hpp"
#include "MotionCmdTypes.hpp"
#include <cstring>
#include <cstdint>

namespace zmc{

// Use unified SharedAllocator template
// SharedCommandAllocator is now an alias for backward compatibility
template<typename Alloc>
using SharedCommandAllocator = SharedAllocator<Alloc>;

template<typename Alloc>
struct CommandStruct {
    int cmdType;  // see EnumFbm
    uint32_t id = 0;  // Internal packet-item identity.
    
    // Parameter length (total bytes)
    // paramLen <= 48: inline storage
    // paramLen > 48: heap allocation
    uint16_t paramLen;
    
    union {
        // Inline storage options (48 bytes each)
        double inlineDoubleFull[6];  // 48 bytes: 6 * sizeof(double)
        int inlineInt[12];           // 48 bytes: 12 * sizeof(int)
        uint16_t inlineU16[24];     // 48 bytes: 24 * sizeof(uint16_t)
        uint8_t inlineU8[48];       // 48 bytes: 48 * sizeof(uint8_t)
        
        // Large params: heap pointer + size
        struct {
            void* heapPtr;      // 指向分配器分配的内存
            size_t heapSize;    // 实际分配的大小
        } heap;
    } param;
    
    // Backward compatibility: val shares memory with param.inlineDoubleFull[0]
    // Note: val and param.inlineDoubleFull[0] are the same storage
    double& val() { return param.inlineDoubleFull[0]; }
    const double& val() const { return param.inlineDoubleFull[0]; }
    
    // Get allocator instance (shared with CommandMid<Alloc>)
    static Alloc& getAllocator() {
        return SharedCommandAllocator<Alloc>::getAllocator();
    }
    
    // Default constructor
    CommandStruct() 
        : cmdType(0), paramLen(0) {
        std::memset(&param, 0, sizeof(param));
    }
    
    // Copy constructor (deep copy)
    CommandStruct(const CommandStruct& other) 
        : cmdType(other.cmdType), id(other.id), paramLen(other.paramLen) {
        if (isHeapAllocated()) {
            // Deep copy: allocate new memory and copy data
            void* newPtr = getAllocator().allocate(other.paramLen);
            if (newPtr) {
                std::memcpy(newPtr, other.param.heap.heapPtr, other.paramLen);
                param.heap.heapPtr = newPtr;
                param.heap.heapSize = other.param.heap.heapSize;
            } else {
                // Allocation failed, clear
                paramLen = 0;
                std::memset(&param, 0, sizeof(param));
            }
        } else {
            // Inline data copy
            std::memcpy(param.inlineU8, other.param.inlineU8, sizeof(param.inlineU8));
        }
    }
    // Move constructor
    CommandStruct(CommandStruct&& other) noexcept
        : cmdType(other.cmdType), id(other.id), paramLen(other.paramLen)
    {
        if (other.isHeapAllocated()) {
            // Take ownership of heap pointer
            param.heap.heapPtr = other.param.heap.heapPtr;
            param.heap.heapSize = other.param.heap.heapSize;
            other.param.heap.heapPtr = nullptr;
            other.param.heap.heapSize = 0;
            other.paramLen = 0;
        } else {
            // Move inline data
            std::memcpy(param.inlineU8, other.param.inlineU8, sizeof(param.inlineU8));
        }
    }

    // Move assignment operator
    CommandStruct& operator=(CommandStruct&& other) noexcept
    {
        if (this != &other) {
            release();

            cmdType = other.cmdType;
            id = other.id;
            paramLen = other.paramLen;

            if (other.isHeapAllocated()) {
                param.heap.heapPtr = other.param.heap.heapPtr;
                param.heap.heapSize = other.param.heap.heapSize;
                other.param.heap.heapPtr = nullptr;
                other.param.heap.heapSize = 0;
                other.paramLen = 0;
            } else {
                std::memcpy(param.inlineU8, other.param.inlineU8, sizeof(param.inlineU8));
            }
        }
        return *this;
    }
    // Copy assignment operator (deep copy)
    CommandStruct& operator=(const CommandStruct& other) {
        if (this != &other) {
            // Release current memory
            release();
            
            cmdType = other.cmdType;
            id = other.id;
            paramLen = other.paramLen;
            
            if (isHeapAllocated()) {
                void* newPtr = getAllocator().allocate(other.paramLen);
                if (newPtr) {
                    std::memcpy(newPtr, other.param.heap.heapPtr, other.paramLen);
                    param.heap.heapPtr = newPtr;
                    param.heap.heapSize = other.param.heap.heapSize;
                } else {
                    paramLen = 0;
                    std::memset(&param, 0, sizeof(param));
                }
            } else {
                std::memcpy(param.inlineU8, other.param.inlineU8, sizeof(param.inlineU8));
            }
        }
        return *this;
    }
    
    // Destructor: release heap memory
    ~CommandStruct() {
        release();
    }
    
    // Determine if heap allocation is used (by paramLen)
    bool isHeapAllocated() const {
        return paramLen > 48;
    }
    
    // Release resources
    void release() {
        if (isHeapAllocated() && param.heap.heapPtr) {
            getAllocator().deallocate(param.heap.heapPtr);
            param.heap.heapPtr = nullptr;
            param.heap.heapSize = 0;
        }
        paramLen = 0;
    }
    
    // Get parameter data pointer (uniform access)
    const uint8_t* getParamData() const {
        if (isHeapAllocated()) {
            return static_cast<const uint8_t*>(param.heap.heapPtr);
        } else {
            return param.inlineU8;
        }
    }
    
    uint8_t* getParamData() {
        if (isHeapAllocated()) {
            return static_cast<uint8_t*>(param.heap.heapPtr);
        } else {
            return param.inlineU8;
        }
    }
    
    // Type-safe access helpers
    const double* getDoubleArray(size_t& count) const {
        if (isHeapAllocated() || paramLen % sizeof(double) != 0) {
            count = 0;
            return nullptr;
        }
        count = paramLen / sizeof(double);
        if (isHeapAllocated()) {
            return reinterpret_cast<const double*>(param.heap.heapPtr);
        } else {
            return param.inlineDoubleFull;
        }
    }
    
    const int* getIntArray(size_t& count) const {
        if (isHeapAllocated() || paramLen % sizeof(int) != 0) {
            count = 0;
            return nullptr;
        }
        count = paramLen / sizeof(int);
        if (isHeapAllocated()) {
            return reinterpret_cast<const int*>(param.heap.heapPtr);
        } else {
            return param.inlineInt;
        }
    }
    
    const uint16_t* getU16Array(size_t& count) const {
        if (isHeapAllocated() || paramLen % sizeof(uint16_t) != 0) {
            count = 0;
            return nullptr;
        }
        count = paramLen / sizeof(uint16_t);
        if (isHeapAllocated()) {
            return reinterpret_cast<const uint16_t*>(param.heap.heapPtr);
        } else {
            return param.inlineU16;
        }
    }
    
    const uint8_t* getU8Array(size_t& count) const {
        count = paramLen;
        return getParamData();
    }
    
    int getInt() const {
        if (paramLen >= sizeof(int)) {
            if (isHeapAllocated()) {
                int val;
                std::memcpy(&val, getParamData(), sizeof(int));
                return val;
            }else {
                return param.inlineInt[0];
            }
        }
        return 0;
    }
    int getIntByIndex(int index) const {
        if (index < 0 || index >= 12) return 0;
        return param.inlineInt[index];
    }
    void setInt(int v) {
        release();
        paramLen = sizeof(int);
        param.inlineInt[0] = v;
    }
    void setIntByIndex(int idx, int v) {
        release();
        paramLen = sizeof(int);
        param.inlineInt[idx] = v;
    }
    // Backward compatibility: get first double value
    double getDouble() const {
        if (paramLen >= sizeof(double)) {
            if (isHeapAllocated()) {
                double val;
                std::memcpy(&val, getParamData(), sizeof(double));
                return val;
            } else {
                double val = param.inlineDoubleFull[0];
                return val;
            }
        }
        return 0.0;
    }
    
    // Backward compatibility: set double value
    void setDouble(double v) {
        release();
        paramLen = sizeof(double);
        param.inlineDoubleFull[0] = v;
    }
    
    // Generic template: read parameter as type T
    template<typename T>
    bool readParamAs(T& outVal) const {
        if (paramLen < sizeof(T)) return false;
        std::memcpy(&outVal, getParamData(), sizeof(T));
        return true;
    }
};

// Default MemoryAllocator
using CommandStructDefault = CommandStruct<MemoryAllocator>;

// Build CommandStruct from CmdPacket param_ptr/param_len
template<typename Alloc>
inline CommandStruct<Alloc> createCommandFromPacket(int cmdType, const uint8_t* param_ptr, uint16_t param_len) {
    CommandStruct<Alloc> cmd;
    cmd.cmdType = cmdType;
    cmd.paramLen = param_len;
    
    constexpr size_t INLINE_SIZE = 48;  // 48 bytes
    
    if (param_len == 0) {
        return cmd;
    }
    
    if (param_len <= INLINE_SIZE) {
        // Short params: inline storage
        std::memcpy(cmd.param.inlineU8, param_ptr, param_len);
        // Zero remaining bytes
        if (param_len < INLINE_SIZE) {
            std::memset(cmd.param.inlineU8 + param_len, 0, INLINE_SIZE - param_len);
        }
    } else {
        // Long params: use allocator
        void* heapPtr = cmd.getAllocator().allocate(param_len);
        if (heapPtr) {
            std::memcpy(heapPtr, param_ptr, param_len);
            cmd.param.heap.heapPtr = heapPtr;
            cmd.param.heap.heapSize = param_len;
        } else {
            // Allocation failed, clear command
            cmd.paramLen = 0;
            std::memset(&cmd.param, 0, sizeof(cmd.param));
        }
    }
    
    return cmd;
}

template<typename Alloc>
struct CommandMid{
    uint16_t cmdType; // see EnumFbm
    uint32_t id = 0;
    uint16_t axisId; // optional
    uint32_t tickStarted;
    // uint16_t paramLen;
    double val;
    
    // Get allocator instance (shared with CommandStruct<Alloc>)
    static Alloc& getAllocator() {
        return SharedCommandAllocator<Alloc>::getAllocator();
    }
};

// Default MemoryAllocator
using CommandMidDefault = CommandMid<MemoryAllocator>;

/* CmdResult wire layout lives in MotionCmdTypes.hpp. */
using CmdResult = ::CmdResult;
using CmdResultInfo = CmdResult;

}
#endif
