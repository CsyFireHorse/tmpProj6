#ifndef _ZMC_AXFB_VAR_MGR_H_
#define _ZMC_AXFB_VAR_MGR_H_

#include "common/misc/ZmcAllocator.hpp"
#include <unordered_map>
#include <cstring>
#include <vector>
// typedef enum VarType{
//     VAR_TYPE_NONE,
//     VAR_TYPE_BOOL,
//     VAR_TYPE_INTEGER,
//     VAR_TYPE_FLOAT,
//     VAR_TYPE_DOUBLE,
//     VAR_TYPE_STRING,
// }VarType;

struct VarDefCfg{
    char* name;
    uint16_t size; 
    uint16_t type; 
};

namespace zmc {

struct VarDef {
    const char* name; 
    void* pAddr; // or use buffer_offset?
    uint16_t size; 
    uint16_t type;
};

class VariableManager {
public:
    // VariableManager(size_t poolSize = 4096) {}
    bool registerVar(const VarDefCfg& var) {
        void* mem = arena.allocate(var.size);
        if(!mem) return false;
        
        varMap[var.name] = VarDef{
            var.name,
            mem,
            var.size,
            var.type
        };
        return true;
    }

    template<typename T>
    bool setValue(const char* name, const T& value) {
        auto it = varMap.find(name);
        if(it == varMap.end()) return false;
        
        if(sizeof(T) > it->second.size) return false;
        memcpy(it->second.pAddr, &value, sizeof(T));
        return true;
    }

    template<typename T>
    T getValue(const char* name) {
        auto it = varMap.find(name);
        if(it == varMap.end()) return T();
        
        T value;
        memcpy(&value, it->second.pAddr, sizeof(T));
        return value;
    }

    template<typename T>
    const T& getValueCRef(const char* name) const{
        auto it = varMap.find(name);
        if(it == varMap.end()) return T();
        
        return *((T*)it->second.pAddr);
    }
private:
    AllocatorAreno<4096> arena;
    std::vector<void*> varPtrArr;
    std::unordered_map<std::string, VarDef> varMap;
};


class VariableVectManager {
public:
    bool registerVar(const VarDefCfg& var) {
        void* mem = arena.allocate(var.size);
        if(!mem) return false;
        
        varPtrArr.push_back(VarDef{
            var.name,
            mem,
            var.size,
            var.type
        });
        return true;
    }
    VarDef operator[](int idx){
        return varPtrArr[idx]; 
    }
    const VarDef operator[](int idx)const{
        return varPtrArr[idx]; 
    }
    VarDef at(int idx){
        return varPtrArr.at(idx); 
    }
    template<typename T>
    bool setValue(int idx, const T& value) {
        auto it = varPtrArr.at(idx);
        if(sizeof(T) > it.size) return false;
        memcpy(it.pAddr, &value, sizeof(T));
        return true;
    }

    template<typename T>
    T getValue(int idx) {
        auto it = varPtrArr.at(idx);
        // if(it == varMap.end()) return T();
        T value;
        memcpy(&value, it.pAddr, sizeof(T));
        return value;
    }

    template<typename T>
    const T& getValueCRef(int idx) const{
        auto it = varPtrArr.at(idx);
        // if(it == varMap.end()) return T();
        return *((T*)it.pAddr);
    }
private:
    AllocatorAreno<4096> arena;
    std::vector<VarDef> varPtrArr;
};

} // namespace zmc
#endif