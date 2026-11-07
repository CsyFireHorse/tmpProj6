#ifndef _FB_ATTR_VISITOR_SPEC_H_
#define _FB_ATTR_VISITOR_SPEC_H_

#include "FbAttrVisitor.h"
#include "fb_cpp_def.h"
// #include "FbAttrTable.h"

namespace zmc{
template<typename T>
int FbAttrVisitorRead(const T& self, FbAttr attr, double& val) {
    return -1;
}

template<typename T>
int FbAttrVisitorWrite(T& self, FbAttr attr, double val) {
    return -1;
}



union UnDoubleVoidPtr{
    double d;
    void* p;
};
union UnVoidPtrDouble{
    void* p;
    double d;
};
inline void* db2voidPtr(double v){
    UnDoubleVoidPtr u = {v};
    return u.p;
}
inline double voidPtr2db(void* p){
    UnVoidPtrDouble u = {p};
    return u.d;
}

template<typename T>
inline void Obj2db(const T& obj, double v){
    UnDoubleVoidPtr u = {v};
    *((T*)u.p) = obj; 
}

template<typename T>
inline void db2Obj(const double v, T& obj){
    UnDoubleVoidPtr u = {v};
    obj = *((T*)u.p); 
}


inline void db2Str(const double v, char* pStr){
    UnDoubleVoidPtr u = {v};
    ZMC_STRING_255& str = *((ZMC_STRING_255*)u.p);
    ZMC_STRING_255& strDest = *((ZMC_STRING_255*)pStr);
    strDest = str; 
}
inline void Str2db(const char* pStr, double& v){
    UnDoubleVoidPtr u = {v};
    ZMC_STRING_255& strDest = *((ZMC_STRING_255*)u.p);
    ZMC_STRING_255& str = *((ZMC_STRING_255*)pStr);
    strDest = str; 
}
template<> int FbAttrVisitorRead<FbAttrPowerStruct>(const FbAttrPowerStruct& self, FbAttr attr, double& val);
template<> int FbAttrVisitorWrite<FbAttrPowerStruct>(FbAttrPowerStruct& self, FbAttr attr, double val);
template<> int FbAttrVisitorRead<FbAttrMoveAbsStruct>(const FbAttrMoveAbsStruct& self, FbAttr attr, double& val);
template<> int FbAttrVisitorWrite<FbAttrMoveAbsStruct>(FbAttrMoveAbsStruct& self, FbAttr attr, double val);



template<> int FbAttrVisitorRead<FbtPower>(const FbtPower& self, FbAttr attr, double& val);
template<> int FbAttrVisitorWrite<FbtPower>(FbtPower& self, FbAttr attr, double val);

template<> int FbAttrVisitorRead<FbtReadActualPosition>(const FbtReadActualPosition& self, FbAttr attr, double& val);
template<> int FbAttrVisitorWrite<FbtReadActualPosition>(FbtReadActualPosition& self, FbAttr attr, double val);
template<> int FbAttrVisitorRead<FbtReadActualVelocity>(const FbtReadActualVelocity& self, FbAttr attr, double& val);
template<> int FbAttrVisitorWrite<FbtReadActualVelocity>(FbtReadActualVelocity& self, FbAttr attr, double val);

template<> int FbAttrVisitorRead<FbtMoveAbsolute>(const FbtMoveAbsolute& self, FbAttr attr, double& val);
template<> int FbAttrVisitorWrite<FbtMoveAbsolute>(FbtMoveAbsolute& self, FbAttr attr, double val);
template<> int FbAttrVisitorRead<FbtMoveRelative>(const FbtMoveRelative& self, FbAttr attr, double& val);
template<> int FbAttrVisitorWrite<FbtMoveRelative>(FbtMoveRelative& self, FbAttr attr, double val);
template<> int FbAttrVisitorRead<FbtMoveAdditive>(const FbtMoveAdditive& self, FbAttr attr, double& val);
template<> int FbAttrVisitorWrite<FbtMoveAdditive>(FbtMoveAdditive& self, FbAttr attr, double val);

template<typename T>
int FbAttrVisitorRead(const T& self, FbAttr attr, void*& ptr) {
    double& val = reinterpret_cast<double&>(ptr);
    return FbAttrVisitorRead<T>(self, attr, val);
}

template<typename T>
int FbAttrVisitorWrite(T& self, FbAttr attr, const void* ref) {
    double val = reinterpret_cast<const double&>(ref);
    return FbAttrVisitorWrite<T>(self, attr, val);
    // return -1;
}

template <typename T>
class FbWrapperImpl : public FbWrapper {
    T fb;
public:
    ~FbWrapperImpl() override = default;
    int setAttr(int name, double val) override {
        return FbAttrVisitorWrite<T>(fb, (FbAttr)name, val);
    }

    int getAttr(int name, double& val) const override {
        return FbAttrVisitorRead<T>(fb, (FbAttr)name, val);
    }
    int setPtrAttr(int name, const void* val) override {
        return FbAttrVisitorWrite<T>(fb, (FbAttr)name, val);
    }

    int getPtrAttr(int name, void* &val) const override {
        return FbAttrVisitorRead<T>(fb, (FbAttr)name, val);
    }
    void runCycle() override { fb.runCycle(); }
};

class FbAttrNop {
    char _;
    public:
    inline void runCycle(){};
};

using FbWrapperNop = FbWrapperImpl<FbAttrNop>;

} // namespace zmc
#endif