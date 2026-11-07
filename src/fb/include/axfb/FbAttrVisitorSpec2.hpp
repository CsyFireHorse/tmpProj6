/**
 * History:
 * ================================================================
 * 2025-04-15 ChenSy <ChenSy@zlg.cn> created.
 *
 */
#ifndef _FB_ATTR_VISITOR_SPEC_2_HPP_
#define _FB_ATTR_VISITOR_SPEC_2_HPP_

#include "FbAttrVisitorSpec.h"
#include "fb_cpp_def.h"

namespace zmc {

// #ifndef _FB_RDWR_HPP_
// #define _FB_RDWR_HPP_



template<>
int FbAttrVisitorRead<FbtPower>(const FbtPower& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_BREGULATORON:
            val = self.bRegulatorOn ? 1.0 : 0.0;
            break;
        case FB_ATTR_BDRIVESTART:
            val = self.bDriveStart ? 1.0 : 0.0;
            break;
        case FB_ATTR_STATUS:
            val = self.Status ? 1.0 : 0.0;
            break;
        case FB_ATTR_BREGULATORREALSTATE:
            val = self.bRegulatorRealState ? 1.0 : 0.0;
            break;
        case FB_ATTR_BDRIVESTARTREALSTATE:
            val = self.bDriveStartRealState ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtPower>(FbtPower& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BREGULATORON:
            self.bRegulatorOn = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BDRIVESTART:
            self.bDriveStart = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_STATUS:
            self.Status = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BREGULATORREALSTATE:
            self.bRegulatorRealState = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BDRIVESTARTREALSTATE:
            self.bDriveStartRealState = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReset>(const FbtReset& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtReset>(FbtReset& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtSetPosition>(const FbtSetPosition& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        case FB_ATTR_MODE:
            val = self.Mode ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtSetPosition>(FbtSetPosition& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_MODE:
            self.Mode = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadStatus>(const FbtReadStatus& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_DISABLED:
            val = self.Disabled ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORSTOP:
            val = self.ErrorStop ? 1.0 : 0.0;
            break;
        case FB_ATTR_STOPPING:
            val = self.Stopping ? 1.0 : 0.0;
            break;
        case FB_ATTR_STANDSTILL:
            val = self.Standstill ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISCRETEMOTION:
            val = self.DiscreteMotion ? 1.0 : 0.0;
            break;
        case FB_ATTR_CONTINUOUSMOTION:
            val = self.ContinuousMotion ? 1.0 : 0.0;
            break;
        case FB_ATTR_SYNCHRONIZEDMOTION:
            val = self.SynchronizedMotion ? 1.0 : 0.0;
            break;
        case FB_ATTR_HOMING:
            val = self.Homing ? 1.0 : 0.0;
            break;
        case FB_ATTR_CONSTANTVELOCITY:
            val = self.ConstantVelocity ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACCELERATING:
            val = self.Accelerating ? 1.0 : 0.0;
            break;
        case FB_ATTR_DECELERATING:
            val = self.Decelerating ? 1.0 : 0.0;
            break;
        case FB_ATTR_FBERROROCCURRED:
            val = self.FBErrorOccurred ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadStatus>(FbtReadStatus& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_DISABLED:
            self.Disabled = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORSTOP:
            self.ErrorStop = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_STOPPING:
            self.Stopping = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_STANDSTILL:
            self.Standstill = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISCRETEMOTION:
            self.DiscreteMotion = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_CONTINUOUSMOTION:
            self.ContinuousMotion = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_SYNCHRONIZEDMOTION:
            self.SynchronizedMotion = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_HOMING:
            self.Homing = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_CONSTANTVELOCITY:
            self.ConstantVelocity = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACCELERATING:
            self.Accelerating = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DECELERATING:
            self.Decelerating = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FBERROROCCURRED:
            self.FBErrorOccurred = val ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadActualPosition>(const FbtReadActualPosition& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadActualPosition>(FbtReadActualPosition& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadActualTorque>(const FbtReadActualTorque& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_TORQUE:
            val = self.Torque;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadActualTorque>(FbtReadActualTorque& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_TORQUE:
            self.Torque = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadActualVelocity>(const FbtReadActualVelocity& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadActualVelocity>(FbtReadActualVelocity& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadAxisError>(const FbtReadAxisError& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_AXISERROR:
            val = self.AxisError ? 1.0 : 0.0;
            break;
        case FB_ATTR_AXISERRORID:
            val = self.AxisErrorID;
            break;
        case FB_ATTR_SWENDSWITCHACTIVE:
            val = self.SWEndSwitchActive ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadAxisError>(FbtReadAxisError& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_AXISERROR:
            self.AxisError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_AXISERRORID:
            self.AxisErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_SWENDSWITCHACTIVE:
            self.SWEndSwitchActive = val ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadSetPosition>(const FbtReadSetPosition& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            val = self.Position;
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
int FbAttrVisitorWrite<FbtReadSetPosition>(FbtReadSetPosition& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadBoolParameter>(const FbtReadBoolParameter& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            val = self.ParameterNumber;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_VALUE:
            val = self.Value ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadBoolParameter>(FbtReadBoolParameter& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            self.ParameterNumber = val;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_VALUE:
            self.Value = val ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadParameter>(const FbtReadParameter& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            val = self.ParameterNumber;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_VALUE:
            val = self.Value;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadParameter>(FbtReadParameter& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            self.ParameterNumber = val;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_VALUE:
            self.Value = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtWriteBoolParameter>(const FbtWriteBoolParameter& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            val = self.ParameterNumber;
            break;
        case FB_ATTR_VALUE:
            val = self.Value ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtWriteBoolParameter>(FbtWriteBoolParameter& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            self.ParameterNumber = val;
            break;
        case FB_ATTR_VALUE:
            self.Value = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtWriteParameter>(const FbtWriteParameter& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            val = self.ParameterNumber;
            break;
        case FB_ATTR_VALUE:
            val = self.Value;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtWriteParameter>(FbtWriteParameter& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            self.ParameterNumber = val;
            break;
        case FB_ATTR_VALUE:
            self.Value = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtWriteCoeParameter>(const FbtWriteCoeParameter& self, FbAttr attr,
                                             double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            val = self.ParameterNumber;
            break;
        case FB_ATTR_VALUE:
            val = self.Value;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtWriteCoeParameter>(FbtWriteCoeParameter& self, FbAttr attr,
                                              double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_PARAMETERNUMBER:
            self.ParameterNumber = val;
            break;
        case FB_ATTR_VALUE:
            self.Value = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtStop>(const FbtStop& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtStop>(FbtStop& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtHalt>(const FbtHalt& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtHalt>(FbtHalt& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtMoveAbsolute>(const FbtMoveAbsolute& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_DIRECTION:
            val = self.Direction;
            break;
        case FB_ATTR_BUFFERMODE:
            val = self.BufferMode;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            val = self.Active ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtMoveAbsolute>(FbtMoveAbsolute& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_DIRECTION:
            self.Direction = (MC_DIRECTION) (int) val;
            break;
        case FB_ATTR_BUFFERMODE:
            self.BufferMode = (MC_BUFFER_MODE) (int) val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            self.Active = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtMoveAdditive>(const FbtMoveAdditive& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            val = self.Distance;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtMoveAdditive>(FbtMoveAdditive& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            self.Distance = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtMoveRelative>(const FbtMoveRelative& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            val = self.Distance;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_BUFFERMODE:
            val = self.BufferMode;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            val = self.Active ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtMoveRelative>(FbtMoveRelative& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            self.Distance = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_BUFFERMODE:
            self.BufferMode = (MC_BUFFER_MODE) (int) val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            self.Active = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtMoveVelocity>(const FbtMoveVelocity& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_DIRECTION:
            val = self.Direction;
            break;
        case FB_ATTR_BUFFERMODE:
            val = self.BufferMode;
            break;
        case FB_ATTR_INVELOCITY:
            val = self.InVelocity ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            val = self.Active ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtMoveVelocity>(FbtMoveVelocity& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_DIRECTION:
            self.Direction = (MC_DIRECTION) (int) val;
            break;
        case FB_ATTR_BUFFERMODE:
            self.BufferMode = (MC_BUFFER_MODE) (int) val;
            break;
        case FB_ATTR_INVELOCITY:
            self.InVelocity = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            self.Active = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtMoveContinuousAbsolute>(const FbtMoveContinuousAbsolute& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ENDVELOCITY:
            val = self.EndVelocity;
            break;
        case FB_ATTR_ENDVELOCITYDIRECTION:
            val = self.EndVelocityDirection;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_DIRECTION:
            val = self.Direction;
            break;
        case FB_ATTR_ADAPTENDVELTOAVOIDOVERSHOOT:
            val = self.AdaptEndVelToAvoidOvershoot ? 1.0 : 0.0;
            break;
        case FB_ATTR_INENDVELOCITY:
            val = self.InEndVelocity ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITIONREACHED:
            val = self.PositionReached ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtMoveContinuousAbsolute>(FbtMoveContinuousAbsolute& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ENDVELOCITY:
            self.EndVelocity = val;
            break;
        case FB_ATTR_ENDVELOCITYDIRECTION:
            self.EndVelocityDirection = (MC_DIRECTION) (int) val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_DIRECTION:
            self.Direction = (MC_DIRECTION) (int) val;
            break;
        case FB_ATTR_ADAPTENDVELTOAVOIDOVERSHOOT:
            self.AdaptEndVelToAvoidOvershoot = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_INENDVELOCITY:
            self.InEndVelocity = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITIONREACHED:
            self.PositionReached = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtMoveContinuousRelative>(const FbtMoveContinuousRelative& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            val = self.Distance;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ENDVELOCITY:
            val = self.EndVelocity;
            break;
        case FB_ATTR_ENDVELOCITYDIRECTION:
            val = self.EndVelocityDirection;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_ADAPTENDVELTOAVOIDOVERSHOOT:
            val = self.AdaptEndVelToAvoidOvershoot ? 1.0 : 0.0;
            break;
        case FB_ATTR_INENDVELOCITY:
            val = self.InEndVelocity ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCETRAVELLED:
            val = self.DistanceTravelled ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtMoveContinuousRelative>(FbtMoveContinuousRelative& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            self.Distance = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ENDVELOCITY:
            self.EndVelocity = val;
            break;
        case FB_ATTR_ENDVELOCITYDIRECTION:
            self.EndVelocityDirection = (MC_DIRECTION) (int) val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_ADAPTENDVELTOAVOIDOVERSHOOT:
            self.AdaptEndVelToAvoidOvershoot = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_INENDVELOCITY:
            self.InEndVelocity = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCETRAVELLED:
            self.DistanceTravelled = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtMoveSuperImposed>(const FbtMoveSuperImposed& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_ABORT:
            val = self.Abort ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            val = self.Distance;
            break;
        case FB_ATTR_VELOCITYDIFF:
            val = self.VelocityDiff;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            val = self.Error ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            val = self.ErrorID;
            break;
        case FB_ATTR_DISTANCETRAVELLED:
            val = self.DistanceTravelled;
            break;
        case FB_ATTR_SUPERIMPOSEDVELOCITY:
            val = self.SuperImposedVelocity;
            break;
        case FB_ATTR_SUPERIMPOSEDACCELERATION:
            val = self.SuperImposedAcceleration;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtMoveSuperImposed>(FbtMoveSuperImposed& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ABORT:
            self.Abort = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            self.Distance = val;
            break;
        case FB_ATTR_VELOCITYDIFF:
            self.VelocityDiff = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_DISTANCETRAVELLED:
            self.DistanceTravelled = val;
            break;
        case FB_ATTR_SUPERIMPOSEDVELOCITY:
            self.SuperImposedVelocity = val;
            break;
        case FB_ATTR_SUPERIMPOSEDACCELERATION:
            self.SuperImposedAcceleration = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtPositionProfile>(const FbtPositionProfile& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_TIMEPOSITION:
            val = reinterpret_cast<const double&>(self.TimePosition);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_ARRAYSIZE:
            val = self.ArraySize;
            break;
        case FB_ATTR_POSITIONSCALE:
            val = self.PositionScale;
            break;
        case FB_ATTR_OFFSET:
            val = self.Offset;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtPositionProfile>(FbtPositionProfile& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_TIMEPOSITION:
            reinterpret_cast<double&>(self.TimePosition) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ARRAYSIZE:
            self.ArraySize = val;
            break;
        case FB_ATTR_POSITIONSCALE:
            self.PositionScale = val;
            break;
        case FB_ATTR_OFFSET:
            self.Offset = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtAccelerationProfile>(const FbtAccelerationProfile& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_TIMEACCELERATION:
            val = reinterpret_cast<const double&>(self.TimeAcceleration);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_ARRAYSIZE:
            val = self.ArraySize;
            break;
        case FB_ATTR_ACCELERATIONSCALE:
            val = self.AccelerationScale;
            break;
        case FB_ATTR_OFFSET:
            val = self.Offset;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORT:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtAccelerationProfile>(FbtAccelerationProfile& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_TIMEACCELERATION:
            reinterpret_cast<double&>(self.TimeAcceleration) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ARRAYSIZE:
            self.ArraySize = val;
            break;
        case FB_ATTR_ACCELERATIONSCALE:
            self.AccelerationScale = val;
            break;
        case FB_ATTR_OFFSET:
            self.Offset = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORT:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtVelocityProfile>(const FbtVelocityProfile& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_TIMEVELOCITY:
            val = reinterpret_cast<const double&>(self.TimeVelocity);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_ARRAYSIZE:
            val = self.ArraySize;
            break;
        case FB_ATTR_VELOCITYSCALE:
            val = self.VelocityScale;
            break;
        case FB_ATTR_OFFSET:
            val = self.Offset;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtVelocityProfile>(FbtVelocityProfile& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_TIMEVELOCITY:
            reinterpret_cast<double&>(self.TimeVelocity) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ARRAYSIZE:
            self.ArraySize = val;
            break;
        case FB_ATTR_VELOCITYSCALE:
            self.VelocityScale = val;
            break;
        case FB_ATTR_OFFSET:
            self.Offset = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtHome>(const FbtHome& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtHome>(FbtHome& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtHoming>(const FbtHoming& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_FHOMEPOSITION:
            val = self.fHomePosition;
            break;
        case FB_ATTR_FVELOCITYSLOW:
            val = self.fVelocitySlow;
            break;
        case FB_ATTR_FVELOCITYFAST:
            val = self.fVelocityFast;
            break;
        case FB_ATTR_FACCELERATION:
            val = self.fAcceleration;
            break;
        case FB_ATTR_FDECELERATION:
            val = self.fDeceleration;
            break;
        case FB_ATTR_FJERK:
            val = self.fJerk;
            break;
        case FB_ATTR_NDIRECTION:
            val = self.nDirection;
            break;
        case FB_ATTR_BREFERENCESWITCH:
            val = self.bReferenceSwitch ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSIGNALDELAY:
            val = self.fSignalDelay;
            break;
        case FB_ATTR_NHOMINGMODE:
            val = self.nHomingMode;
            break;
        case FB_ATTR_BRETURNTOZERO:
            val = self.bReturnToZero ? 1.0 : 0.0;
            break;
        case FB_ATTR_BINDEXOCCURED:
            val = self.bIndexOccured ? 1.0 : 0.0;
            break;
        case FB_ATTR_FINDEXPOSITION:
            val = self.fIndexPosition;
            break;
        case FB_ATTR_BIGNOREHWLIMIT:
            val = self.bIgnoreHWLimit ? 1.0 : 0.0;
            break;
        case FB_ATTR_BDONE:
            val = self.bDone ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            val = self.bCommandAborted ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_NERRORID:
            val = self.nErrorID;
            break;
        case FB_ATTR_BSTARTLATCHINGINDEX:
            val = self.bStartLatchingIndex ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtHoming>(FbtHoming& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FHOMEPOSITION:
            self.fHomePosition = val;
            break;
        case FB_ATTR_FVELOCITYSLOW:
            self.fVelocitySlow = val;
            break;
        case FB_ATTR_FVELOCITYFAST:
            self.fVelocityFast = val;
            break;
        case FB_ATTR_FACCELERATION:
            self.fAcceleration = val;
            break;
        case FB_ATTR_FDECELERATION:
            self.fDeceleration = val;
            break;
        case FB_ATTR_FJERK:
            self.fJerk = val;
            break;
        case FB_ATTR_NDIRECTION:
            self.nDirection = (MC_DIRECTION) (int) val;
            break;
        case FB_ATTR_BREFERENCESWITCH:
            self.bReferenceSwitch = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSIGNALDELAY:
            self.fSignalDelay = val;
            break;
        case FB_ATTR_NHOMINGMODE:
            self.nHomingMode = (MC_HOMING_MODE) (int) val;
            break;
        case FB_ATTR_BRETURNTOZERO:
            self.bReturnToZero = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BINDEXOCCURED:
            self.bIndexOccured = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FINDEXPOSITION:
            self.fIndexPosition = val;
            break;
        case FB_ATTR_BIGNOREHWLIMIT:
            self.bIgnoreHWLimit = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BDONE:
            self.bDone = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            self.bCommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_NERRORID:
            self.nErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_BSTARTLATCHINGINDEX:
            self.bStartLatchingIndex = val ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtJog>(const FbtJog& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_JOGFORWARD:
            val = self.JogForward ? 1.0 : 0.0;
            break;
        case FB_ATTR_JOGBACKWARD:
            val = self.JogBackward ? 1.0 : 0.0;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtJog>(FbtJog& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_JOGFORWARD:
            self.JogForward = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_JOGBACKWARD:
            self.JogBackward = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtInch>(const FbtInch& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_INCHFORWARD:
            val = self.InchForward ? 1.0 : 0.0;
            break;
        case FB_ATTR_INCHBACKWARD:
            val = self.InchBackward ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            val = self.Distance;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtInch>(FbtInch& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_INCHFORWARD:
            self.InchForward = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_INCHBACKWARD:
            self.InchBackward = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DISTANCE:
            self.Distance = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtGearIn>(const FbtGearIn& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            val = reinterpret_cast<const double&>(self.Master);
            break;
        case FB_ATTR_SLAVE:
            val = reinterpret_cast<const double&>(self.Slave);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_RATIONUMERATOR:
            val = self.RatioNumerator;
            break;
        case FB_ATTR_RATIODENOMINATOR:
            val = self.RatioDenominator;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_BUFFERMODE:
            val = self.BufferMode;
            break;
        case FB_ATTR_INGEAR:
            val = self.InGear ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            val = self.Active ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtGearIn>(FbtGearIn& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            reinterpret_cast<double&>(self.Master) = val;
            break;
        case FB_ATTR_SLAVE:
            reinterpret_cast<double&>(self.Slave) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_RATIONUMERATOR:
            self.RatioNumerator = val;
            break;
        case FB_ATTR_RATIODENOMINATOR:
            self.RatioDenominator = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_BUFFERMODE:
            self.BufferMode = (MC_BUFFER_MODE) (int) val;
            break;
        case FB_ATTR_INGEAR:
            self.InGear = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            self.Active = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtGearInPos>(const FbtGearInPos& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            val = reinterpret_cast<const double&>(self.Master);
            break;
        case FB_ATTR_SLAVE:
            val = reinterpret_cast<const double&>(self.Slave);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_RATIONUMERATOR:
            val = self.RatioNumerator;
            break;
        case FB_ATTR_RATIODENOMINATOR:
            val = self.RatioDenominator;
            break;
        case FB_ATTR_MASTERSYNCPOSITION:
            val = self.MasterSyncPosition;
            break;
        case FB_ATTR_SLAVESYNCPOSITION:
            val = self.SlaveSyncPosition;
            break;
        case FB_ATTR_MASTERSTARTDISTANCE:
            val = self.MasterStartDistance;
            break;
        case FB_ATTR_BUFFERMODE:
            val = self.BufferMode;
            break;
        case FB_ATTR_AVOIDREVERSAL:
            val = self.AvoidReversal ? 1.0 : 0.0;
            break;
        case FB_ATTR_STARTSYNC:
            val = self.StartSync ? 1.0 : 0.0;
            break;
        case FB_ATTR_INSYNC:
            val = self.InSync ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            val = self.Active ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtGearInPos>(FbtGearInPos& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            reinterpret_cast<double&>(self.Master) = val;
            break;
        case FB_ATTR_SLAVE:
            reinterpret_cast<double&>(self.Slave) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_RATIONUMERATOR:
            self.RatioNumerator = val;
            break;
        case FB_ATTR_RATIODENOMINATOR:
            self.RatioDenominator = val;
            break;
        case FB_ATTR_MASTERSYNCPOSITION:
            self.MasterSyncPosition = val;
            break;
        case FB_ATTR_SLAVESYNCPOSITION:
            self.SlaveSyncPosition = val;
            break;
        case FB_ATTR_MASTERSTARTDISTANCE:
            self.MasterStartDistance = val;
            break;
        case FB_ATTR_BUFFERMODE:
            self.BufferMode = (MC_BUFFER_MODE) (int) val;
            break;
        case FB_ATTR_AVOIDREVERSAL:
            self.AvoidReversal = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_STARTSYNC:
            self.StartSync = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_INSYNC:
            self.InSync = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ACTIVE:
            self.Active = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtGearOut>(const FbtGearOut& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_SLAVE:
            val = reinterpret_cast<const double&>(self.Slave);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtGearOut>(FbtGearOut& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_SLAVE:
            reinterpret_cast<double&>(self.Slave) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtPhasing>(const FbtPhasing& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            val = reinterpret_cast<const double&>(self.Master);
            break;
        case FB_ATTR_SLAVE:
            val = reinterpret_cast<const double&>(self.Slave);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_PHASESHIFT:
            val = self.PhaseShift;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtPhasing>(FbtPhasing& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            reinterpret_cast<double&>(self.Master) = val;
            break;
        case FB_ATTR_SLAVE:
            reinterpret_cast<double&>(self.Slave) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_PHASESHIFT:
            self.PhaseShift = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtSetTorque>(const FbtSetTorque& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_BENABLE:
            val = self.bEnable ? 1.0 : 0.0;
            break;
        case FB_ATTR_FTORQUE:
            val = self.fTorque;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_NERRORID:
            val = self.nErrorID;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtSetTorque>(FbtSetTorque& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_BENABLE:
            self.bEnable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FTORQUE:
            self.fTorque = val;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_NERRORID:
            self.nErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtFollowPosition>(const FbtFollowPosition& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSETPOSITION:
            val = self.fSetPosition;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            val = self.bCommandAborted ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            val = self.iErrorID;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtFollowPosition>(FbtFollowPosition& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSETPOSITION:
            self.fSetPosition = val;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            self.bCommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            self.iErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtFollowPositionVelocity>(const FbtFollowPositionVelocity& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSETPOSITION:
            val = self.fSetPosition;
            break;
        case FB_ATTR_FSETVELOCITY:
            val = self.fSetVelocity;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            val = self.bCommandAborted ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            val = self.iErrorID;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtFollowPositionVelocity>(FbtFollowPositionVelocity& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSETPOSITION:
            self.fSetPosition = val;
            break;
        case FB_ATTR_FSETVELOCITY:
            self.fSetVelocity = val;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            self.bCommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            self.iErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtFollowVelocity>(const FbtFollowVelocity& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSETVELOCITY:
            val = self.fSetVelocity;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            val = self.bCommandAborted ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            val = self.iErrorID;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtFollowVelocity>(FbtFollowVelocity& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSETVELOCITY:
            self.fSetVelocity = val;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            self.bCommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            self.iErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtFollowSetValues>(const FbtFollowSetValues& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_BABORT:
            val = self.bAbort ? 1.0 : 0.0;
            break;
        case FB_ATTR_DWVALUEMASK:
            val = self.dwValueMask;
            break;
        case FB_ATTR_FSETPOSITION:
            val = self.fSetPosition;
            break;
        case FB_ATTR_FSETVELOCITY:
            val = self.fSetVelocity;
            break;
        case FB_ATTR_FSETACCELERATION:
            val = self.fSetAcceleration;
            break;
        case FB_ATTR_FSETJERK:
            val = self.fSetJerk;
            break;
        case FB_ATTR_FSETTORQUE:
            val = self.fSetTorque;
            break;
        case FB_ATTR_FSETCURRENT:
            val = self.fSetCurrent;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            val = self.bCommandAborted ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            val = self.iErrorID;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtFollowSetValues>(FbtFollowSetValues& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BABORT:
            self.bAbort = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DWVALUEMASK:
            self.dwValueMask = val;
            break;
        case FB_ATTR_FSETPOSITION:
            self.fSetPosition = val;
            break;
        case FB_ATTR_FSETVELOCITY:
            self.fSetVelocity = val;
            break;
        case FB_ATTR_FSETACCELERATION:
            self.fSetAcceleration = val;
            break;
        case FB_ATTR_FSETJERK:
            self.fSetJerk = val;
            break;
        case FB_ATTR_FSETTORQUE:
            self.fSetTorque = val;
            break;
        case FB_ATTR_FSETCURRENT:
            self.fSetCurrent = val;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            self.bCommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            self.iErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtTrackAxis>(const FbtTrackAxis& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            val = reinterpret_cast<const double&>(self.Master);
            break;
        case FB_ATTR_SLAVE:
            val = reinterpret_cast<const double&>(self.Slave);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            val = self.bCommandAborted ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            val = self.iErrorID;
            break;
        case FB_ATTR_BINSYNC:
            val = self.bInSync ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtTrackAxis>(FbtTrackAxis& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            reinterpret_cast<double&>(self.Master) = val;
            break;
        case FB_ATTR_SLAVE:
            reinterpret_cast<double&>(self.Slave) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            self.bCommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            self.iErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_BINSYNC:
            self.bInSync = val ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtTrackSetValues>(const FbtTrackSetValues& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSETPOSITION:
            val = self.fSetPosition;
            break;
        case FB_ATTR_FSETVELOCITY:
            val = self.fSetVelocity;
            break;
        case FB_ATTR_FSETACCELERATION:
            val = self.fSetAcceleration;
            break;
        case FB_ATTR_FSETJERK:
            val = self.fSetJerk;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_SIGNALMODULOPERIOD:
            val = self.SignalModuloPeriod;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            val = self.bCommandAborted ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            val = self.iErrorID;
            break;
        case FB_ATTR_BINSYNC:
            val = self.bInSync ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtTrackSetValues>(FbtTrackSetValues& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FSETPOSITION:
            self.fSetPosition = val;
            break;
        case FB_ATTR_FSETVELOCITY:
            self.fSetVelocity = val;
            break;
        case FB_ATTR_FSETACCELERATION:
            self.fSetAcceleration = val;
            break;
        case FB_ATTR_FSETJERK:
            self.fSetJerk = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_SIGNALMODULOPERIOD:
            self.SignalModuloPeriod = val;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BCOMMANDABORTED:
            self.bCommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_IERRORID:
            self.iErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_BINSYNC:
            self.bInSync = val ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtCamIn>(const FbtCamIn& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            val = reinterpret_cast<const double&>(self.Master);
            break;
        case FB_ATTR_SLAVE:
            val = reinterpret_cast<const double&>(self.Slave);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_MASTEROFFSET:
            val = self.MasterOffset;
            break;
        case FB_ATTR_SLAVEOFFSET:
            val = self.SlaveOffset;
            break;
        case FB_ATTR_MASTERSCALING:
            val = self.MasterScaling;
            break;
        case FB_ATTR_SLAVESCALING:
            val = self.SlaveScaling;
            break;
        case FB_ATTR_STARTMODE:
            val = self.StartMode;
            break;
        case FB_ATTR_CAMTABLEID:
            Obj2db(self.CamTableID, val);
            break;
        case FB_ATTR_VELOCITYDIFF:
            val = self.VelocityDiff;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_DECELERATION:
            val = self.Deceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        case FB_ATTR_TAPPETHYSTERESIS:
            val = self.TappetHysteresis;
            break;
        case FB_ATTR_INSYNC:
            val = self.InSync ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            val = self.Busy ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            val = self.Error ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            val = self.ErrorID;
            break;
        case FB_ATTR_ENDOFPROFILE:
            val = self.EndOfProfile ? 1.0 : 0.0;
            break;
        case FB_ATTR_TAPPETS:
            Obj2db(self.Tappets, val);
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtCamIn>(FbtCamIn& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            reinterpret_cast<double&>(self.Master) = val;
            break;
        case FB_ATTR_SLAVE:
            reinterpret_cast<double&>(self.Slave) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_MASTEROFFSET:
            self.MasterOffset = val;
            break;
        case FB_ATTR_SLAVEOFFSET:
            self.SlaveOffset = val;
            break;
        case FB_ATTR_MASTERSCALING:
            self.MasterScaling = val;
            break;
        case FB_ATTR_SLAVESCALING:
            self.SlaveScaling = val;
            break;
        case FB_ATTR_STARTMODE:
            self.StartMode = (MC_STARTMODE) (int) val;
            break;
        case FB_ATTR_CAMTABLEID:
            db2Obj(val, self.CamTableID);
            break;
        case FB_ATTR_VELOCITYDIFF:
            self.VelocityDiff = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_DECELERATION:
            self.Deceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        case FB_ATTR_TAPPETHYSTERESIS:
            self.TappetHysteresis = val;
            break;
        case FB_ATTR_INSYNC:
            self.InSync = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_ENDOFPROFILE:
            self.EndOfProfile = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_TAPPETS:
            db2Obj(val, self.Tappets);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtCamOut>(const FbtCamOut& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_SLAVE:
            val = reinterpret_cast<const double&>(self.Slave);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtCamOut>(FbtCamOut& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_SLAVE:
            reinterpret_cast<double&>(self.Slave) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtCamTableSelect>(const FbtCamTableSelect& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            val = reinterpret_cast<const double&>(self.Master);
            break;
        case FB_ATTR_SLAVE:
            val = reinterpret_cast<const double&>(self.Slave);
            break;
        case FB_ATTR_CAMTABLE:
            val = reinterpret_cast<const double&>(self.CamTable);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_PERIODIC:
            val = self.Periodic ? 1.0 : 0.0;
            break;
        case FB_ATTR_MASTERABSOLUTE:
            val = self.MasterAbsolute ? 1.0 : 0.0;
            break;
        case FB_ATTR_SLAVEABSOLUTE:
            val = self.SlaveAbsolute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
        case FB_ATTR_CAMTABLEID:
            Obj2db(self.CamTableID, val);
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtCamTableSelect>(FbtCamTableSelect& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            reinterpret_cast<double&>(self.Master) = val;
            break;
        case FB_ATTR_SLAVE:
            reinterpret_cast<double&>(self.Slave) = val;
            break;
        case FB_ATTR_CAMTABLE:
            reinterpret_cast<double&>(self.CamTable) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_PERIODIC:
            self.Periodic = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_MASTERABSOLUTE:
            self.MasterAbsolute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_SLAVEABSOLUTE:
            self.SlaveAbsolute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_CAMTABLEID:
            db2Obj(val, self.CamTableID);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtGetTappetValue>(const FbtGetTappetValue& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_TAPPETS:
            val = reinterpret_cast<const double&>(self.Tappets);
            break;
        case FB_ATTR_IID:
            val = self.iID;
            break;
        case FB_ATTR_BINITVALUE:
            val = self.bInitValue ? 1.0 : 0.0;
            break;
        case FB_ATTR_BSETINITVALUEATRESET:
            val = self.bSetInitValueAtReset ? 1.0 : 0.0;
            break;
        case FB_ATTR_BTAPPET:
            val = self.bTappet ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtGetTappetValue>(FbtGetTappetValue& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_TAPPETS:
            reinterpret_cast<double&>(self.Tappets) = val;
            break;
        case FB_ATTR_IID:
            self.iID = val;
            break;
        case FB_ATTR_BINITVALUE:
            self.bInitValue = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BSETINITVALUEATRESET:
            self.bSetInitValueAtReset = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BTAPPET:
            self.bTappet = val ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtCamRegister>(const FbtCamRegister& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            val = reinterpret_cast<const double&>(self.Master);
            break;
        case FB_ATTR_CAMTABLE:
            val = reinterpret_cast<const double&>(self.CamTable);
            break;
        case FB_ATTR_BTAPPET:
            val = reinterpret_cast<const double&>(self.bTappet);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_MASTEROFFSET:
            val = self.MasterOffset;
            break;
        case FB_ATTR_MASTERSCALING:
            val = self.MasterScaling;
            break;
        case FB_ATTR_TAPPETHYSTERESIS:
            val = self.TappetHysteresis;
            break;
        case FB_ATTR_DEADTIMECOMPENSATION:
            val = self.DeadTimeCompensation;
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
        case FB_ATTR_ENDOFPROFILE:
            val = self.EndOfProfile ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtCamRegister>(FbtCamRegister& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            reinterpret_cast<double&>(self.Master) = val;
            break;
        case FB_ATTR_CAMTABLE:
            reinterpret_cast<double&>(self.CamTable) = val;
            break;
        case FB_ATTR_BTAPPET:
            reinterpret_cast<double&>(self.bTappet) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_MASTEROFFSET:
            self.MasterOffset = val;
            break;
        case FB_ATTR_MASTERSCALING:
            self.MasterScaling = val;
            break;
        case FB_ATTR_TAPPETHYSTERESIS:
            self.TappetHysteresis = val;
            break;
        case FB_ATTR_DEADTIMECOMPENSATION:
            self.DeadTimeCompensation = val;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_ENDOFPROFILE:
            self.EndOfProfile = val ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtCAMBounds>(const FbtCAMBounds& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_CAM:
            val = reinterpret_cast<const double&>(self.CAM);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DMASTERVELMAX:
            val = self.dMasterVelMax;
            break;
        case FB_ATTR_DMASTERACCMAX:
            val = self.dMasterAccMax;
            break;
        case FB_ATTR_DMASTERSCALING:
            val = self.dMasterScaling;
            break;
        case FB_ATTR_DSLAVESCALING:
            val = self.dSlaveScaling;
            break;
        case FB_ATTR_BDONE:
            val = self.bDone ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_NERRORID:
            val = self.nErrorID;
            break;
        case FB_ATTR_DMAXPOS:
            val = self.dMaxPos;
            break;
        case FB_ATTR_DMINPOS:
            val = self.dMinPos;
            break;
        case FB_ATTR_DMAXVEL:
            val = self.dMaxVel;
            break;
        case FB_ATTR_DMINVEL:
            val = self.dMinVel;
            break;
        case FB_ATTR_DMAXACCDEC:
            val = self.dMaxAccDec;
            break;
        case FB_ATTR_DMINACCDEC:
            val = self.dMinAccDec;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtCAMBounds>(FbtCAMBounds& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_CAM:
            reinterpret_cast<double&>(self.CAM) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DMASTERVELMAX:
            self.dMasterVelMax = val;
            break;
        case FB_ATTR_DMASTERACCMAX:
            self.dMasterAccMax = val;
            break;
        case FB_ATTR_DMASTERSCALING:
            self.dMasterScaling = val;
            break;
        case FB_ATTR_DSLAVESCALING:
            self.dSlaveScaling = val;
            break;
        case FB_ATTR_BDONE:
            self.bDone = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_NERRORID:
            self.nErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_DMAXPOS:
            self.dMaxPos = val;
            break;
        case FB_ATTR_DMINPOS:
            self.dMinPos = val;
            break;
        case FB_ATTR_DMAXVEL:
            self.dMaxVel = val;
            break;
        case FB_ATTR_DMINVEL:
            self.dMinVel = val;
            break;
        case FB_ATTR_DMAXACCDEC:
            self.dMaxAccDec = val;
            break;
        case FB_ATTR_DMINACCDEC:
            self.dMinAccDec = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtCAMBounds_Pos>(const FbtCAMBounds_Pos& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_CAM:
            val = reinterpret_cast<const double&>(self.CAM);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DMASTERVELMAX:
            val = self.dMasterVelMax;
            break;
        case FB_ATTR_DMASTERACCMAX:
            val = self.dMasterAccMax;
            break;
        case FB_ATTR_DMASTERSCALING:
            val = self.dMasterScaling;
            break;
        case FB_ATTR_DSLAVESCALING:
            val = self.dSlaveScaling;
            break;
        case FB_ATTR_BDONE:
            val = self.bDone ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_NERRORID:
            val = self.nErrorID;
            break;
        case FB_ATTR_DMAXPOS:
            val = self.dMaxPos;
            break;
        case FB_ATTR_DMINPOS:
            val = self.dMinPos;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtCAMBounds_Pos>(FbtCAMBounds_Pos& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_CAM:
            reinterpret_cast<double&>(self.CAM) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DMASTERVELMAX:
            self.dMasterVelMax = val;
            break;
        case FB_ATTR_DMASTERACCMAX:
            self.dMasterAccMax = val;
            break;
        case FB_ATTR_DMASTERSCALING:
            self.dMasterScaling = val;
            break;
        case FB_ATTR_DSLAVESCALING:
            self.dSlaveScaling = val;
            break;
        case FB_ATTR_BDONE:
            self.bDone = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_NERRORID:
            self.nErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_DMAXPOS:
            self.dMaxPos = val;
            break;
        case FB_ATTR_DMINPOS:
            self.dMinPos = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtGetCamSlaveSetPosition>(const FbtGetCamSlaveSetPosition& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            val = reinterpret_cast<const double&>(self.Master);
            break;
        case FB_ATTR_SLAVE:
            val = reinterpret_cast<const double&>(self.Slave);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_MASTEROFFSET:
            val = self.MasterOffset;
            break;
        case FB_ATTR_SLAVEOFFSET:
            val = self.SlaveOffset;
            break;
        case FB_ATTR_MASTERSCALING:
            val = self.MasterScaling;
            break;
        case FB_ATTR_SLAVESCALING:
            val = self.SlaveScaling;
            break;
        case FB_ATTR_CAMTABLEID:
            Obj2db(self.CamTableID, val);
            break;
        case FB_ATTR_FSTARTPOSITION:
            val = self.fStartPosition;
            break;
        case FB_ATTR_FSTARTVELOCITY:
            val = self.fStartVelocity;
            break;
        case FB_ATTR_FSTARTACCELERATION:
            val = self.fStartAcceleration;
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
int FbAttrVisitorWrite<FbtGetCamSlaveSetPosition>(FbtGetCamSlaveSetPosition& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_MASTER:
            reinterpret_cast<double&>(self.Master) = val;
            break;
        case FB_ATTR_SLAVE:
            reinterpret_cast<double&>(self.Slave) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_MASTEROFFSET:
            self.MasterOffset = val;
            break;
        case FB_ATTR_SLAVEOFFSET:
            self.SlaveOffset = val;
            break;
        case FB_ATTR_MASTERSCALING:
            self.MasterScaling = val;
            break;
        case FB_ATTR_SLAVESCALING:
            self.SlaveScaling = val;
            break;
        case FB_ATTR_CAMTABLEID:
            db2Obj(val, self.CamTableID);
            break;
        case FB_ATTR_FSTARTPOSITION:
            self.fStartPosition = val;
            break;
        case FB_ATTR_FSTARTVELOCITY:
            self.fStartVelocity = val;
            break;
        case FB_ATTR_FSTARTACCELERATION:
            self.fStartAcceleration = val;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtAbortTrigger>(const FbtAbortTrigger& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_TRIGGERINPUT:
            val = reinterpret_cast<const double&>(self.TriggerInput);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtAbortTrigger>(FbtAbortTrigger& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_TRIGGERINPUT:
            reinterpret_cast<double&>(self.TriggerInput) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtTouchProbe>(const FbtTouchProbe& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_TRIGGERINPUT:
            val = reinterpret_cast<const double&>(self.TriggerInput);
            break;
        case FB_ATTR_EXECUTE:
            val = self.Execute ? 1.0 : 0.0;
            break;
        case FB_ATTR_WINDOWONLY:
            val = self.WindowOnly ? 1.0 : 0.0;
            break;
        case FB_ATTR_FIRSTPOSITION:
            val = self.FirstPosition;
            break;
        case FB_ATTR_LASTPOSITION:
            val = self.LastPosition;
            break;
        case FB_ATTR_DONE:
            val = self.Done ? 1.0 : 0.0;
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
        case FB_ATTR_RECORDEDPOSITION:
            val = self.RecordedPosition;
            break;
        case FB_ATTR_COMMANDABORTED:
            val = self.CommandAborted ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtTouchProbe>(FbtTouchProbe& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_TRIGGERINPUT:
            reinterpret_cast<double&>(self.TriggerInput) = val;
            break;
        case FB_ATTR_EXECUTE:
            self.Execute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_WINDOWONLY:
            self.WindowOnly = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_FIRSTPOSITION:
            self.FirstPosition = val;
            break;
        case FB_ATTR_LASTPOSITION:
            self.LastPosition = val;
            break;
        case FB_ATTR_DONE:
            self.Done = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_RECORDEDPOSITION:
            self.RecordedPosition = val;
            break;
        case FB_ATTR_COMMANDABORTED:
            self.CommandAborted = val ? 1.0 : 0.0;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtDigitalCamSwitch>(const FbtDigitalCamSwitch& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_SWITCHES:
            val = reinterpret_cast<const double&>(self.Switches);
            break;
        case FB_ATTR_OUTPUTS:
            val = reinterpret_cast<const double&>(self.Outputs);
            break;
        case FB_ATTR_TRACKOPTIONS:
            val = reinterpret_cast<const double&>(self.TrackOptions);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_ENABLEMASK:
            val = self.EnableMask;
            break;
        case FB_ATTR_TAPPETMODE:
            val = self.TappetMode;
            break;
        case FB_ATTR_INOPERATION:
            val = self.InOperation ? 1.0 : 0.0;
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
        case FB_ATTR_SWITCHCORRUPTED:
            val = self.SwitchCorrupted;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtDigitalCamSwitch>(FbtDigitalCamSwitch& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_SWITCHES:
            reinterpret_cast<double&>(self.Switches) = val;
            break;
        case FB_ATTR_OUTPUTS:
            reinterpret_cast<double&>(self.Outputs) = val;
            break;
        case FB_ATTR_TRACKOPTIONS:
            reinterpret_cast<double&>(self.TrackOptions) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ENABLEMASK:
            self.EnableMask = val;
            break;
        case FB_ATTR_TAPPETMODE:
            self.TappetMode = (MC_TAPPETMODE) (int) val;
            break;
        case FB_ATTR_INOPERATION:
            self.InOperation = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_SWITCHCORRUPTED:
            self.SwitchCorrupted = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadCAM>(const FbtReadCAM& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_SFILENAME:
            Obj2db(self.sFileName, val);
            break;
        case FB_ATTR_BDONE:
            val = self.bDone ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            val = self.ErrorID;
            break;
        case FB_ATTR_CAM:
            val = reinterpret_cast<const double&>(self.CAM);
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadCAM>(FbtReadCAM& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_SFILENAME:
            db2Obj(val, self.sFileName);
            break;
        case FB_ATTR_BDONE:
            self.bDone = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_CAM:
            reinterpret_cast<double&>(self.CAM) = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtWriteCAM>(const FbtWriteCAM& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_CAM:
            val = reinterpret_cast<const double&>(self.CAM);
            break;
        case FB_ATTR_BEXECUTE:
            val = self.bExecute ? 1.0 : 0.0;
            break;
        case FB_ATTR_SFILENAME:
            Obj2db(self.sFileName, val);
            break;
        case FB_ATTR_BDONE:
            val = self.bDone ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            val = self.bBusy ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            val = self.bError ? 1.0 : 0.0;
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
int FbAttrVisitorWrite<FbtWriteCAM>(FbtWriteCAM& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_CAM:
            reinterpret_cast<double&>(self.CAM) = val;
            break;
        case FB_ATTR_BEXECUTE:
            self.bExecute = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_SFILENAME:
            db2Obj(val, self.sFileName);
            break;
        case FB_ATTR_BDONE:
            self.bDone = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BBUSY:
            self.bBusy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BERROR:
            self.bError = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadSetValues>(const FbtReadSetValues& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadSetValues>(FbtReadSetValues& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadActualValues>(const FbtReadActualValues& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadActualValues>(FbtReadActualValues& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadSetDriveValues>(const FbtReadSetDriveValues& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadSetDriveValues>(FbtReadSetDriveValues& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        default:
            return -1;
    }
    return 0;
}

template<>
int FbAttrVisitorRead<FbtReadActualDriveValues>(const FbtReadActualDriveValues& self, FbAttr attr, double& val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            val = reinterpret_cast<const double&>(self.Axis);
            break;
        case FB_ATTR_ENABLE:
            val = self.Enable ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            val = self.Valid ? 1.0 : 0.0;
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
        case FB_ATTR_POSITION:
            val = self.Position;
            break;
        case FB_ATTR_VELOCITY:
            val = self.Velocity;
            break;
        case FB_ATTR_ACCELERATION:
            val = self.Acceleration;
            break;
        case FB_ATTR_JERK:
            val = self.Jerk;
            break;
        default:
            return -1;
    }
    return 0;
}


template<>
int FbAttrVisitorWrite<FbtReadActualDriveValues>(FbtReadActualDriveValues& self, FbAttr attr, double val) {
    switch(attr) {
        case FB_ATTR_AXIS:
            reinterpret_cast<double&>(self.Axis) = val;
            break;
        case FB_ATTR_ENABLE:
            self.Enable = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_VALID:
            self.Valid = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_BUSY:
            self.Busy = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERROR:
            self.Error = val ? 1.0 : 0.0;
            break;
        case FB_ATTR_ERRORID:
            self.ErrorID = (MC_ERROR) ((int) val);
            break;
        case FB_ATTR_POSITION:
            self.Position = val;
            break;
        case FB_ATTR_VELOCITY:
            self.Velocity = val;
            break;
        case FB_ATTR_ACCELERATION:
            self.Acceleration = val;
            break;
        case FB_ATTR_JERK:
            self.Jerk = val;
            break;
        default:
            return -1;
    }
    return 0;
}


}

#endif