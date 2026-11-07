#ifndef _FB_ATTR_VISITOR_H_
#define _FB_ATTR_VISITOR_H_

// #include <cstddef>  
// #include <array>
#include "stdint.h"
#include "FbFactoryDef.h"

// #include "fbt_en_base.h"

namespace zmc{

class AxFbAttrBase{
public:
    virtual int setAttr(int name, double value) = 0;
    virtual int setPtrAttr(int name, const void* value) = 0;
    virtual int getAttr(int name, double&) const = 0;
    // virtual bool getBoolAttr(FbAttr name) const = 0;
    virtual int getPtrAttr(int name, void*&) const = 0;
    virtual void runCycle() = 0;
    virtual ~AxFbAttrBase() = default;
};

class FbWrapper :public AxFbAttrBase{
public:
    int setFbAttr(FbAttr name, double value){
        return setAttr(static_cast<int>(name), value);
    }
    virtual int getFbAttr(FbAttr name, double& value) const{
        return getAttr(static_cast<int>(name), value);
    }
};

} // namespace zmc


typedef double LREAL;
typedef bool BOOL;


// 1B, 2B, 4B, 8B
typedef struct FbAttrMoveAbsStruct {
    void* pImpl;
    // inout
    void* Axis;
    // input
    BOOL Execute;
    LREAL Position;
    LREAL Velocity;
    LREAL Acceleration;
    LREAL Deceleration;
    LREAL Jerk;
    int Direction;
    int BufferMode;
    // output
    BOOL Busy;
    BOOL Done;
    BOOL Error;
    BOOL CommandAborted;
    BOOL Active;
    int ErrorID;
	void runCycle(){};
}FbAttrMoveAbsStruct;


typedef struct FbAttrPowerStruct {
    void* pImpl;
    // inout
    void* Axis;
    // input
    BOOL Enable;
    // output
    BOOL Busy;
    BOOL Status;
    BOOL Error;
    int ErrorID;
	void runCycle(){};
}FbAttrPowerStruct;

#define OFFSET(T, M) ((size_t)&reinterpret_cast<char&>((((T*)0)->M)))
// static_assert(OFFSET(FbAttrMoveAbsStruct, Axis) == 8, "");


#if 0

// #define OFFSET(T, M) (reinterpret_cast<int>(&reinterpret_cast<const char&>(reinterpret_cast<const T*>(0)->M)))
// #define OFFSET(T, M) (static_cast<int>( \
//     reinterpret_cast<std::ptrdiff_t>(&(reinterpret_cast<const volatile char&>( \
//     reinterpret_cast<const T*>(0)->M))) \
// ))
// template<typename T, FbAttr ENUM> 
// struct FbAttrVisiter{ 
// 	static constexpr int offset = -1;
// };

// #define FbAttrVisiter_OFFSET(T, M, ENUM) \
// template<> \
// struct FbAttrVisiter<T, ENUM>{ \
// 	static constexpr int offset = OFFSET(T, M);\
// };

// FbAttrVisiter_OFFSET(FbAttrMoveAbsStruct, Axis, FbAttr::FB_ATTR_AXIS)
// FbAttrVisiter_OFFSET(FbAttrMoveAbsStruct, Execute, FbAttr::FB_ATTR_EXECUTE)
// FbAttrVisiter<FbAttrMoveAbsStruct, FbAttr::FB_ATTR_AXIS> fb;

template<typename T>
struct AttrTable;

template <typename T, typename M>
constexpr short int offset_of(M T::*member) {
    return reinterpret_cast<size_t>(&(reinterpret_cast<T*>(1)->*member)) - 1;
}


// auto offset_b = offset_of(&FbAttrMoveAbsStruct::Execute);


static const AttrPairt __pairs[] = {
    {FbAttr::FB_ATTR_POSITION,  OFFSET(FbAttrMoveAbsStruct, Position)},
    {FbAttr::FB_ATTR_POSITION,  OFFSET(FbAttrMoveAbsStruct, Position)}
};

struct AttrPairtStruct{
    static const AttrPairt _arr0[2];
    static const std::array<AttrPairt, 3> _arr;
};

const AttrPairt AttrPairtStruct::_arr0[2] = {
    {FbAttr::FB_ATTR_POSITION,  OFFSET(FbAttrMoveAbsStruct, Position)},
    {FbAttr::FB_ATTR_POSITION,  OFFSET(FbAttrMoveAbsStruct, Position)}
};

const std::array<AttrPairt, 3> AttrPairtStruct::_arr = {{
    {FbAttr::FB_ATTR_POSITION,  OFFSET(FbAttrMoveAbsStruct, Position)},
    {FbAttr::FB_ATTR_POSITION,  OFFSET(FbAttrMoveAbsStruct, Position)}
}};





template<>
struct AttrTable<FbAttrMoveAbsStruct> {
    static constexpr AttrPairt pairs[] = {
        // ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_ENABLE,   Enable),
        {FbAttr::FB_ATTR_EXECUTE,  1, FB_ATTR_SIZE_BOOL},
        // {FbAttr::FB_ATTR_POSITION,  OFFSET(FbAttrMoveAbsStruct, Position)},
        // {FbAttr::FB_ATTR_VELOCITY,  OFFSET(FbAttrMoveAbsStruct, Velocity)},
        // {FbAttr::FB_ATTR_BUSY,      OFFSET(FbAttrMoveAbsStruct, Busy)},
        // {FbAttr::FB_ATTR_DONE,      OFFSET(FbAttrMoveAbsStruct, Done)},
        // {FbAttr::FB_ATTR_ERROR_ID,  OFFSET(FbAttrMoveAbsStruct, ErrorID)}
    };
    static const AttrPairt pairs2[3];

    static const AttrPairt* begin() { return &pairs[0]; }
    static const AttrPairt* end() { return &pairs[LEN(pairs)]; }
    static int get_offset(FbAttr attr) {
        for (size_t i = 0; i < sizeof(pairs)/sizeof(pairs[0]); ++i) {
            if (pairs[i].attr == attr) {
                return pairs[i].offset;
            }
        }
        return -1;
    }
};


// struct AttrTable<FbAttrMoveAbsStruct>::pairs2[] = {
//     {FbAttr::FB_ATTR_EXECUTE,  1, FB_ATTR_SIZE_BOOL},
// };

template<>
struct AttrTable<FbAttrPowerStruct> {
    static constexpr AttrPairt pairs[9] = {
        // ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_ENABLE,   Enable),
        // ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_AXIS,     Axis  ),
        // ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_BUSY,     Busy  ),
        // ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_ERROR_ID, ErrorID),
        // ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_DONE,     Status)
    };
};




template<typename R, typename T>
int FbAttrVisitorWrite2(T& self, FbAttr attr, double val) {
    for (const auto& p: R::_arr) {
        if (p.attr == attr) {
            char* ptr = reinterpret_cast<char*>(&self) + p.offset;
            switch(p.size) {
                case FB_ATTR_SIZE_BOOL:
                    *reinterpret_cast<BOOL*>(ptr) = val != 0.0;
                    break;
                case FB_ATTR_SIZE_LREAL:
                    *reinterpret_cast<LREAL*>(ptr) = static_cast<LREAL>(val);
                    break;
                case FB_ATTR_SIZE_DWORD:
                    *reinterpret_cast<int*>(ptr) = static_cast<int>(val);
                    break;
                default:
                    break;
            }
            return 0;
        }
    }
    return -1;
}

template<typename T>
int FbAttrVisitorWrite(T& self, FbAttr attr, double val) {
    for (const auto* p = AttrTable<T>::begin(); p != AttrTable<T>::end(); ++p) {
        if (p->attr == attr) {
            char* ptr = reinterpret_cast<char*>(&self) + p->offset;
            switch(p->size) {
                case FB_ATTR_SIZE_BOOL:
                    *reinterpret_cast<BOOL*>(ptr) = val != 0.0;
                    break;
                case FB_ATTR_SIZE_LREAL:
                    *reinterpret_cast<LREAL*>(ptr) = static_cast<LREAL>(val);
                    break;
                case FB_ATTR_SIZE_DWORD:
                    *reinterpret_cast<int*>(ptr) = static_cast<int>(val);
                    break;
                default:
                    break;
            }
            return 0;
        }
    }
    return -1;
}

template<typename T>
int FbAttrVisitorRead(const T& self, FbAttr attr, double& val) {
    for (const auto* p = AttrTable<T>::begin(); p != AttrTable<T>::end(); ++p) {
        if (p->attr == attr) {
            const char* ptr = reinterpret_cast<const char*>(&self) + p->offset;
            switch(p->size) {
                case FB_ATTR_SIZE_BOOL:
                    val = *reinterpret_cast<const BOOL*>(ptr) ? 1.0 : 0.0;
                    break;
                case FB_ATTR_SIZE_LREAL:
                    val = static_cast<double>(*reinterpret_cast<const LREAL*>(ptr));
                    break;
                case FB_ATTR_SIZE_DWORD:
                    val = static_cast<double>(*reinterpret_cast<const int*>(ptr));
                    break;
                default:
                    val = 0.0;
                    break;
            }
        return 0;
        }
    }
    return -1;
}
#endif

#endif