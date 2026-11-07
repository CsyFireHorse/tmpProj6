#ifdef _FB_ATTR_TABLE_H_
#define _FB_ATTR_TABLE_H_

#include "FbAttrVisitor.h"

// depreciated
enum FbAttrSize: int8_t{
	FB_ATTR_SIZE_BOOL = 1,
	FB_ATTR_SIZE_WORD = 2,
	FB_ATTR_SIZE_DWORD = 4,
	FB_ATTR_SIZE_LREAL = 8, // Pointers are 8 bytes in 64-bit mode.
};

struct AttrPairt {
    FbAttr attr;
    short int offset;
    FbAttrSize size;
};

#define ATTR_ENTRY(T, ATTR, M) {FbAttr::ATTR, OFFSET(T, M), FbAttrSize(sizeof(T::M))}

#define LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))


template <typename T>
struct AttrTableTraits;

template <typename T>
struct AttrTableTraits {
    static constexpr size_t N = 10;
};

template <typename T>
struct AttrPairtTable {
    static const std::array<AttrPairt, AttrTableTraits<T>::N> arr;
};

template <typename T>
const std::array<AttrPairt, AttrTableTraits<T>::N> AttrPairtTable<T>::arr = {
    {{FB_ATTR_NONE, -1, FB_ATTR_SIZE_BOOL}}};

template <typename T>
int FbAttrVisitorWrite(T& self, FbAttr attr, double val) {
    for (const auto& p : AttrPairtTable<T>::arr) {
        if (p.attr == attr) {
            char* ptr = reinterpret_cast<char*>(&self) + p.offset;
            switch (p.size) {
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

template <typename T>
int FbAttrVisitorRead(const T& self, FbAttr attr, double& val) {
    for (const auto& p : AttrPairtTable<T>::arr) {
        if (p.attr == attr) {
            const char* ptr = reinterpret_cast<const char*>(&self) + p.offset;
            switch (p.size) {
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

// sepcialize

#define ATTR_TRAITS_LEN(T, SZ)          \
    template <>                         \
    struct AttrTableTraits<T> {         \
        static constexpr size_t N = SZ; \
    };

ATTR_TRAITS_LEN(FbAttrMoveAbsStruct, 15);
template <>
const std::array<AttrPairt, AttrTableTraits<FbAttrMoveAbsStruct>::N>
    AttrPairtTable<FbAttrMoveAbsStruct>::arr = {{
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_AXIS, Axis),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_EXECUTE, Execute),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_POSITION, Position),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_VELOCITY, Velocity),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_ACCELERATION, Acceleration),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_DECELERATION, Deceleration),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_JERK, Jerk),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_DIRECTION, Direction),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_BUFFER_MODE, BufferMode),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_BUSY, Busy),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_ACTIVE, Active),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_DONE, Done),
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_ERROR_ID, ErrorID),
    }};

ATTR_TRAITS_LEN(FbAttrPowerStruct, 6);
// template<> struct AttrTableTraits<FbAttrPowerStruct> { static constexpr size_t N = 6;};
template <>
const std::array<AttrPairt, AttrTableTraits<FbAttrPowerStruct>::N>
    AttrPairtTable<FbAttrPowerStruct>::arr = {{
        ATTR_ENTRY(FbAttrMoveAbsStruct, FB_ATTR_AXIS, Axis),
        ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_ENABLE, Enable),
        ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_STATUS, Status),
        ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_BUSY, Busy),
        ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_ERROR, Error),
        ATTR_ENTRY(FbAttrPowerStruct, FB_ATTR_ERROR_ID, ErrorID),
    }};

template <typename T>
class FbWrapperImpl : public FbWrapper {
    T fb;

   public:
    void setAttr(FbAttr name, double val) override { FbAttrVisitorWrite(*this, name, val); }

    double getAttr(FbAttr name) const override {
        double val;
        FbAttrVisitorRead(*this, name, val);
        return val;
    }

    void runCycle() override { fb.runCycle(); }
};

#endif