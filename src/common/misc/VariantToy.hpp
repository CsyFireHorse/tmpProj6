#ifndef _ZMC_VARIANT_TOY_HPP_
#define _ZMC_VARIANT_TOY_HPP_

#include <cstdint>

namespace zmc{

struct VariantToy{
    enum Type{
        None,
        Double,
        Int,
        Boolean,
        Char,
        Int64
    };
    uint8_t fType = 0;
    union{
        double fDouble;
        int fInt;
        void* pChar; // no memory management, just use it as a pointer
        // make it as easy as possible, no need to copy, no need to assign, no need to free
        int64_t dInt64;
    };
    
    template<typename T>
    void set(const T& value);
    
    template<typename T>
    T get() const;

    template<typename T>
    T get(T default_value) const;

    double getDouble() const {
        if (fType == Double) return fDouble;
        else if (fType == Int) return fInt;
        else if (fType == Int64) return dInt64;
        else if (fType == Boolean) return (bool) fInt;
        return 0.0;
    }
    
    int getInt() const {
        if (fType == Double) return (int)fDouble;
        else if (fType == Int) return fInt;
        else if (fType == Int64) return dInt64;
        else if (fType == Boolean) return (int) (bool) fInt;
        return 0;
    }
    int64_t getInt64() const {
        if (fType == Double) return (int64_t)fDouble;
        else if (fType == Int) return fInt;
        else if (fType == Int64) return dInt64;
        else if (fType == Boolean) return (bool) fInt;
        return 0;
    }
    bool getBoolean() const {
        if (fType == Double) return (bool)fDouble;
        else if (fType == Int) return (bool) fInt;
        else if (fType == Int64) return (bool) dInt64;
        else if (fType == Boolean) return (bool) fInt;
        return 0;
    }
    const char* getChar() const {
        if (fType == Char) return (const char*)pChar;
        return nullptr;
    }
};
inline VariantToy makeVariantToy(){
    VariantToy var;
    var.dInt64 = 0;
    var.fType = 0;
    return var;
}
struct VariantToyStruct{      
    uint8_t fType = 0;
    double fDouble;
};
template<>
inline void VariantToy::set<double>(const double& value) {
    fType = Double;
    fDouble = value;
}

template<>
inline void VariantToy::set<int>(const int& value) {
    fType = Int;
    fInt = value;
}
template<>
inline void VariantToy::set<int64_t>(const int64_t& value) {
    fType = Int64;
    dInt64 = value;
}
template<>
inline void VariantToy::set<bool>(const bool& value) {
    fType = Boolean;
    fInt = (int) value;
}

template<>
inline void VariantToy::set<const char*>(const char* const& value) {
    fType = Char;
    pChar = (void*)value;
}

template<>
inline double VariantToy::get<double>() const {
    return getDouble();
}

template<>
inline int VariantToy::get<int>() const {
    return getInt();
}
template<>
inline bool VariantToy::get<bool>() const {
    return getBoolean();
}

template<>
inline const char* VariantToy::get<const char*>() const {
    return getChar();
}

template<>
inline double VariantToy::get<double>(double default_value) const {
    if (fType == Double) return fDouble;
    if (fType == Int) return (double)fInt;
    else if (fType == Int64) return (double) dInt64;
    else if (fType == Boolean) return (double) (bool) fInt;
    return default_value;
}

template<>
inline int VariantToy::get<int>(int default_value) const {
    if (fType == Int) return fInt;
    else if (fType == Double) return (int)fDouble;
    else if (fType == Boolean) return (int) (bool) fInt;
    return default_value;
}
template<>
inline bool VariantToy::get<bool>(bool default_value) const {
    if (fType == Int) return (bool) fInt;
    else if (fType == Double) return (bool) (int)fDouble;
    else if (fType == Int64) return (bool) dInt64;
    else if (fType == Boolean) return (bool) fInt;
    return default_value;
}

template<>
inline const char* VariantToy::get<const char*>(const char* default_value) const {
    if (fType == Char) return (const char*)pChar;
    return default_value;
}

}
#endif
