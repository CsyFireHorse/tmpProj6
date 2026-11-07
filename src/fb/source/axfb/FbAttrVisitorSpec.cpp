#include "FbAttrVisitorSpec.h"


#if 0
#include "FbAttrTable.h"
#else 

namespace zmc{

template<>
int FbAttrVisitorRead<FbAttrPowerStruct>(const FbAttrPowerStruct& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = (double&) self.Axis;
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable;
            break;
        case FB_ATTR_STATUS:  
            val = self.Status ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR: 
            val = self.Error ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            val = self.ErrorID;
            break;
        default:
            return -1;
    }
    return 0; 
}


template<>
int FbAttrVisitorWrite<FbAttrPowerStruct>(FbAttrPowerStruct& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            // self.Axis = db2voidPtr(val);
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = (BOOL) val;
            break;
        case FB_ATTR_STATUS:
            self.Status = (BOOL) val;
            break;
        case FB_ATTR_BUSY:
            self.Busy = (BOOL) val;
            break;
        case FB_ATTR_ERROR:   
            self.Error = (BOOL) val;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = val;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorRead<FbAttrMoveAbsStruct>(const FbAttrMoveAbsStruct& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = (double&) self.Axis;
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute;
            break;
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy;
            break;
        case FB_ATTR_ERROR:
            val = self.Error;
            break;
        case FB_ATTR_ERRORID:
            val = self.ErrorID;
            break;
        default:
            return -1;
    }
    return 0; 
}
template<>
int FbAttrVisitorWrite<FbAttrMoveAbsStruct>(FbAttrMoveAbsStruct& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            // self.Axis = db2voidPtr(val);
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = (BOOL) val;
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_BUSY:
            self.Busy = (BOOL) val;
            break;
        case FB_ATTR_ERROR:
            self.Error = (BOOL) val;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = val;
            break;
        default:
            return -1;
    }
    return 0;
}

// template<> int FbAttrVisitorRead<FbtPower>(const FbtPower& self, FbAttr attr, double& val);
// template<> int FbAttrVisitorWrite<FbtPower>(FbtPower& self, FbAttr attr, double val);
// template<> int FbAttrVisitorRead<FbtReadActualPosition>(const FbtReadActualPosition& self, FbAttr attr, double& val);
// template<> int FbAttrVisitorWrite<FbtReadActualPosition>(FbtReadActualPosition& self, FbAttr attr, double val);
// template<> int FbAttrVisitorRead<FbtReadActualVelocity>(const FbtReadActualVelocity& self, FbAttr attr, double& val);
// template<> int FbAttrVisitorWrite<FbtReadActualVelocity>(FbtReadActualVelocity& self, FbAttr attr, double val);

// template<> int FbAttrVisitorRead<FbtMoveAbsolute>(const FbtMoveAbsolute& self, FbAttr attr, double& val);
// template<> int FbAttrVisitorWrite<FbtMoveAbsolute>(FbtMoveAbsolute& self, FbAttr attr, double val);
// template<> int FbAttrVisitorRead<FbtMoveRelative>(const FbtMoveRelative& self, FbAttr attr, double& val);
// template<> int FbAttrVisitorWrite<FbtMoveRelative>(FbtMoveRelative& self, FbAttr attr, double val);
// template<> int FbAttrVisitorRead<FbtMoveAdditive>(const FbtMoveAdditive& self, FbAttr attr, double& val);
// template<> int FbAttrVisitorWrite<FbtMoveAdditive>(FbtMoveAdditive& self, FbAttr attr, double val);


}
#endif