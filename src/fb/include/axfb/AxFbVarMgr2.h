#ifndef _ZMC_AXFB_VAR_MGR2_H_
#define _ZMC_AXFB_VAR_MGR2_H_

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "common/misc/ZmcAllocator.hpp"

struct ObjBase {
    uint32_t metaInfo;
    uint32_t size;
};

union VarValue {
    double dVal;
    int64_t iVal;
    void* ptr;
    ObjBase* obj;
};

enum VarTag { VAR_NONE, VAR_BOOL, VAR_INT, VAR_DOUBLE, VAR_PTR, VAR_OBJ };

struct VarDef_v2 {
    const char* name;
    uint8_t tag;
    VarValue value;
};

namespace zmc {



struct Obj : public ObjBase {
    // char data[0];      // FAM - Flexible Array Member
    void* getData() { return static_cast<void*>(this + sizeof(ObjBase)); }
    const void* getData() const { return static_cast<const void*>(this + sizeof(ObjBase)); }
};

class VariableVectManager_v2 {
   public:
    bool registerVar(const char* name, VarTag tag, int sz=0) {
        VarDef_v2 varDef;
        varDef.name = strdup(name);
        varDef.tag = tag;

        switch (tag) {
            case VarTag::VAR_BOOL:
                varDef.value.iVal = 0.0;
                break;
            case VarTag::VAR_DOUBLE:
                varDef.value.dVal = 0.0;
                break;
            case VarTag::VAR_INT:
                varDef.value.iVal = 0;
                break;
            case VarTag::VAR_OBJ:
                return registerObjVar(name, sz);
            case VarTag::VAR_PTR:
                varDef.value.ptr = nullptr;
                break;
            default:
                break;
        }

        varDefArr.push_back(varDef);
        return true;
    }

    bool registerObjVar(const char* name, uint32_t size) {
        VarDef_v2 varDef;
        varDef.name = strdup(name);
        varDef.tag = VarTag::VAR_OBJ;

        Obj* pObj = static_cast<Obj*>(arena.allocate(sizeof(ObjBase) + size));
        if (!pObj) return false;

        // pObj->tag = VarTag::VAR_OBJ;
        pObj->metaInfo = 0;
        pObj->size = size;
        memset(pObj->getData(), 0, size);

        varDef.value.obj = pObj;
        varDefArr.push_back(varDef);
        return true;
    }
    VarDef_v2& operator[](int idx) { return varDefArr[idx]; }
    const VarDef_v2& operator[](int idx) const { return varDefArr[idx]; }


    bool setDouble(int idx, double value) {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return false;
        if (varDefArr[idx].tag == VarTag::VAR_INT) {
            varDefArr[idx].value.iVal = value;
            return true;
        }else if (varDefArr[idx].tag == VarTag::VAR_BOOL) {
            varDefArr[idx].value.iVal = (bool) value;
            return true;
        }
        if (varDefArr[idx].tag != VarTag::VAR_DOUBLE) return false;

        varDefArr[idx].value.dVal = value;
        return true;
    }

    bool setInt64(int idx, int64_t value) {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return false;
        if (varDefArr[idx].tag != VarTag::VAR_INT) return false;

        varDefArr[idx].value.iVal = value;
        return true;
    }

    bool setPtr(int idx, void* ptr) {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return false;
        if (varDefArr[idx].tag != VarTag::VAR_PTR) return false;

        varDefArr[idx].value.ptr = ptr;
        return true;
    }

    double getDouble(int idx) const {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return 0.0;
        if (varDefArr[idx].tag == VarTag::VAR_INT) return varDefArr[idx].value.iVal;
        else if (varDefArr[idx].tag != VarTag::VAR_DOUBLE) return 0.0;

        return varDefArr[idx].value.dVal;
    }

    int64_t getInt64(int idx) const {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return 0;
        if (varDefArr[idx].tag == VarTag::VAR_DOUBLE) return varDefArr[idx].value.dVal;
        else if (varDefArr[idx].tag != VarTag::VAR_INT) return 0;

        return varDefArr[idx].value.iVal;
    }

    void* getPtr(int idx) const {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return nullptr;
        if (varDefArr[idx].tag != VarTag::VAR_PTR) return nullptr;

        return varDefArr[idx].value.ptr;
    }

    // Primary template: returns default value for unsupported types
    template <typename T>
    T getValue(int idx) const {
        static T defaultVal = T();
        return defaultVal;
    }

    template <typename T>
    bool setObjValue(int idx, const T& value) {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return false;
        if (varDefArr[idx].tag != VarTag::VAR_OBJ) return false;

        ObjBase* objBase = varDefArr[idx].value.obj;
        Obj* obj = reinterpret_cast<Obj*>(objBase);

        if (sizeof(T) > obj->size) return false;
        memcpy(obj->getData(), &value, sizeof(T));
        return true;
    }

    bool setObjByAddr(int idx, const void* pData, size_t size) {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return false;
        if (varDefArr[idx].tag != VarTag::VAR_OBJ) return false;

        ObjBase* objBase = varDefArr[idx].value.obj;
        Obj* pObj = reinterpret_cast<Obj*>(objBase);

        if (size > pObj->size) return false;
        memcpy(pObj->getData(), pData, size);
        return true;
    }


    template <typename T>
    T getObjValue(int idx) const {
        T defaultVal = T();
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return defaultVal;
        if (varDefArr[idx].tag != VarTag::VAR_OBJ) return defaultVal;

        ObjBase* objBase = varDefArr[idx].value.obj;
        Obj* obj = reinterpret_cast<Obj*>(objBase);

        if (sizeof(T) > obj->size) return defaultVal;

        T value;
        memcpy(&value, obj->getData(), sizeof(T));
        return value;
    }


    template <typename T>
    T& getObjRef(int idx) {
        static T defaultVal = T();
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return defaultVal;
        if (varDefArr[idx].tag != VarTag::VAR_OBJ) return defaultVal;

        ObjBase* objBase = varDefArr[idx].value.obj;
        Obj* obj = reinterpret_cast<Obj*>(objBase);

        if (sizeof(T) > obj->size) return defaultVal;

        return *reinterpret_cast<T*>(obj->getData());
    }

    void* getObjData(int idx) {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return nullptr;
        if (varDefArr[idx].tag != VarTag::VAR_OBJ) return nullptr;

        ObjBase* objBase = varDefArr[idx].value.obj;
        Obj* obj = reinterpret_cast<Obj*>(objBase);

        return obj->getData();
    }

    const void* getObjData(int idx) const {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) return nullptr;
        if (varDefArr[idx].tag != VarTag::VAR_OBJ) return nullptr;

        ObjBase* objBase = varDefArr[idx].value.obj;
        Obj* obj = reinterpret_cast<Obj*>(objBase);

        return obj->getData();
    }

    size_t size() const { return varDefArr.size(); }

    VarDef_v2 at(int idx) const {
        if (idx < 0 || static_cast<size_t>(idx) >= varDefArr.size()) {
            static VarDef_v2 emptyDef = {nullptr, VarTag::VAR_NONE, {0}};
            return emptyDef;
        }
        return varDefArr[idx];
    }

   private:
    AllocatorAreno<4096> arena;        
    std::vector<VarDef_v2> varDefArr;  
};

// Explicit specializations must be at namespace scope, not inside the class

// Specialization for double
template <>
inline double VariableVectManager_v2::getValue<double>(int idx) const {
    return getDouble(idx);
}
// Specialization for int64_t
template <>
inline int64_t VariableVectManager_v2::getValue<int64_t>(int idx) const {
    return getInt64(idx);
}
// Specialization for void*
template <>
inline void* VariableVectManager_v2::getValue<void*>(int idx) const {
    return getPtr(idx);
}
} // namespace zmc

#endif  // _ZMC_AXFB_VAR_MGR2_H_
