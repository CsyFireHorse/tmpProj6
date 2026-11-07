/**
 * History:
 * ================================================================
 * 2025-04-18 ChenSy <ChenSy@zlg.cn> created.
 *
 */
	
#ifndef _AXIS_ATTR_VISITOR_H_
#define _AXIS_ATTR_VISITOR_H_

#include "fbt_axis_base.h"
#include "FbFactoryDef.h"

namespace zmc{

inline int AxAttrVisitorRead(const AxisImplBase& self, AxAttr attr, double& val) {
    switch(attr) {
        case AX_ATTR_NATIVE:
            // val = self.native;
            break;
        case AX_ATTR_DRIVEERROR:
            val = self.driveError;
            break;
        case AX_ATTR_AXISSWERROR:
            val = self.axisSwError;
            break;
        case AX_ATTR_FBERROR:
            val = self.fbError;
            break;
        case AX_ATTR_BERROR:
            val = self.bError;
            break;
        case AX_ATTR_BREGULATORON:
            val = self.bRegulatorOn;
            break;
        case AX_ATTR_BDRIVESTART:
            val = self.bDriveStart;
            break;
        case AX_ATTR_REGULATORREALSTATE:
            val = self.regulatorRealState;
            break;
        case AX_ATTR_DRIVESTARTREALSTATE:
            val = self.driveStartRealState;
            break;
        case AX_ATTR_BSTARTREFERENCE:
            val = self.bStartReference;
            break;
        case AX_ATTR_BSTARTREFERENCEREALSTATE:
            val = self.bStartReferenceRealState;
            break;
        case AX_ATTR_BLOGICAL:
            val = self.bLogical;
            break;
        case AX_ATTR_BSIMULATION:
            val = self.bSimulation;
            break;
        case AX_ATTR_BMOTORROTARYTYPE:
            val = self.bMotorRotaryType;
            break;
        case AX_ATTR_IMOVEMENTTYPE:
            val = self.iMovementType;
            break;
        case AX_ATTR_CONSTANTVELOCITY:
            val = self.ConstantVelocity;
            break;
        case AX_ATTR_ACCELERATING:
            val = self.Accelerating;
            break;
        case AX_ATTR_DECELERATING:
            val = self.Decelerating;
            break;
        case AX_ATTR_NDIRECTION:
            val = self.nDirection;
            break;
        case AX_ATTR_EAXISMTYPE:
            val = self.eAxisMtype;
            break;
        case AX_ATTR_ERAMPTYPE:
            val = self.eRampType;
            break;
        case AX_ATTR_BYCONTROLLERMODE:
            val = self.byControllerMode;
            break;
        case AX_ATTR_BYREALCONTROLLERMODE:
            val = self.byRealControllerMode;
            break;
        case AX_ATTR_AXISSTAT:
            val = self.axisStat;
            break;
        case AX_ATTR_AXISSTATBAK:
            val = self.axisStatBak;
            break;
        case AX_ATTR_WAXISSTRUCTID:
            val = self.wAxisStructID;
            break;
        case AX_ATTR_DWERRORID:
            val = self.dwErrorID;
            break;
        case AX_ATTR_FBEFBERROR:
            val = self.fbeFBError;
            break;
        case AX_ATTR_CYCLETIME:
            val = self.cycleTime;
            break;
        case AX_ATTR_FSETPOSITION:
            val = self.fSetPosition;
            break;
        case AX_ATTR_FSETVELOCITY:
            val = self.fSetVelocity;
            break;
        case AX_ATTR_FSETACCELERATION:
            val = self.fSetAcceleration;
            break;
        case AX_ATTR_FACTPOSITION:
            val = self.fActPosition;
            break;
        case AX_ATTR_FACTVELOCITY:
            val = self.fActVelocity;
            break;
        case AX_ATTR_FACTACCELERATION:
            val = self.fActAcceleration;
            break;
        case AX_ATTR_FSAVEPOSITION:
            val = self.fSavePosition;
            break;
        case AX_ATTR_FSAVEVELOCITY:
            val = self.fSaveVelocity;
            break;
        case AX_ATTR_FSAVEACCELERATION:
            val = self.fSaveAcceleration;
            break;
        case AX_ATTR_FMAXVELOCITY:
            val = self.fMaxVelocity;
            break;
        case AX_ATTR_FMAXACCELERATION:
            val = self.fMaxAcceleration;
            break;
        case AX_ATTR_FMAXDECELERATION:
            val = self.fMaxDeceleration;
            break;
        case AX_ATTR_FSWMAXVELOCITY:
            val = self.fSWMaxVelocity;
            break;
        case AX_ATTR_FSWMAXACCELERATION:
            val = self.fSWMaxAcceleration;
            break;
        case AX_ATTR_FSWMAXDECELERATION:
            val = self.fSWMaxDeceleration;
            break;
        case AX_ATTR_FSWMAXJERK:
            val = self.fSWMaxJerk;
            break;
        case AX_ATTR_FAIMPOSITION:
            val = self.fAimPosition;
            break;
        case AX_ATTR_FMARKPOSITION:
            val = self.fMarkPosition;
            break;
        case AX_ATTR_FMARKVELOCITY:
            val = self.fMarkVelocity;
            break;
        case AX_ATTR_FMARKACCELERATION:
            val = self.fMarkAcceleration;
            break;
        case AX_ATTR_FSETTORQUE:
            val = self.fSetTorque;
            break;
        case AX_ATTR_FACTTORQUE:
            val = self.fActTorque;
            break;
        case AX_ATTR_FMAXTORQUE:
            val = self.fMaxTorque;
            break;
        case AX_ATTR_FSWMAXTORQUE:
            val = self.fSWMaxTorque;
            break;
        case AX_ATTR_FSCALEFACTOR:
            val = self.fScalefactor;
            break;
        case AX_ATTR_FFACTORVEL:
            val = self.fFactorVel;
            break;
        case AX_ATTR_FFACTORACC:
            val = self.fFactorAcc;
            break;
        case AX_ATTR_FFACTORTOR:
            val = self.fFactorTor;
            break;
        case AX_ATTR_FFACTORJERK:
            val = self.fFactorJerk;
            break;
        case AX_ATTR_FPOSITIONPERIOD:
            val = self.fPositionPeriod;
            break;
        case AX_ATTR_IRATIOTECHUNITSNUM:
            val = self.iRatioTechUnitsNum;
            break;
        case AX_ATTR_DWRATIOTECHUNITSDENOM:
            val = self.dwRatioTechUnitsDenom;
            break;
        case AX_ATTR_FOFFSETPOSITION:
            val = self.fOffsetPosition;
            break;
        case AX_ATTR_DWPOSOFFSETFORRESIDUALS:
            val = self.dwPosOffsetForResiduals;
            break;
        case AX_ATTR_DWLASTPOSITION:
            val = self.dwLastPosition;
            break;
        case AX_ATTR_ITURN:
            val = self.iTurn;
            break;
        case AX_ATTR_DISETPOSITION:
            val = self.diSetPosition;
            break;
        case AX_ATTR_DIACTPOSITION:
            val = self.diActPosition;
            break;
        case AX_ATTR_DISETVELOCITY:
            val = self.diSetVelocity;
            break;
        case AX_ATTR_DIACTVELOCITY:
            val = self.diActVelocity;
            break;
        case AX_ATTR_DISETACCELERATION:
            val = self.diSetAcceleration;
            break;
        case AX_ATTR_DIACTACCELERATION:
            val = self.diActAcceleration;
            break;
        case AX_ATTR_DISETCURRENT:
            val = self.diSetCurrent;
            break;
        case AX_ATTR_DIACTCURRENT:
            val = self.diActCurrent;
            break;
        case AX_ATTR_DISETTORQUE:
            val = self.diSetTorque;
            break;
        case AX_ATTR_DIACTTORQUE:
            val = self.diActTorque;
            break;
        case AX_ATTR_DIENCODERPOSITION:
            val = self.diEncoderPosition;
            break;
        case AX_ATTR_BSWLIMITENABLE:
            val = self.bSWLimitEnable;
            break;
        case AX_ATTR_BHWLIMITENABLE:
            val = self.bHWLimitEnable;
            break;
        case AX_ATTR_BPOSITIONLAGACTIVE:
            val = self.bPositionLagActive;
            break;
        case AX_ATTR_ECHECKPOSITIONLAG:
            val = self.eCheckPositionLag;
            break;
        case AX_ATTR_FSWLIMITPOSITIVE:
            val = self.fSWLimitPositive;
            break;
        case AX_ATTR_FSWLIMITNEGATIVE:
            val = self.fSWLimitNegative;
            break;
        case AX_ATTR_FMAXPOSITIONLAG:
            val = self.fMaxPositionLag;
            break;
        case AX_ATTR_FSWLIMITDECELERATION:
            val = self.fSWLimitDeceleration;
            break;
        case AX_ATTR_FSWERRORMAXDISTANCE:
            val = self.fSWErrorMaxDistance;
            break;
        case AX_ATTR_FSETACTTIMELAGCYCLES:
            val = self.fSetActTimeLagCycles;
            break;
        case AX_ATTR_USEENCODERBITWIDTH:
            val = self.usiEncoderBitWidth;
            break;
        case AX_ATTR_SWSMEX:
            val = self.swSmEx;
            break;
        case AX_ATTR_CMD:
            val = self.cmd;
            break;
        case AX_ATTR_WSTATUSWORD:
            val = self.wStatusWord;
            break;
        case AX_ATTR_WCONTROLWORD:
            val = self.wControlWord;
            break;
        case AX_ATTR_M_CYCLECOUNTER:
            val = self.m_cycleCounter;
            break;
        case AX_ATTR_FBBAK:
            // val = self.fbBak;
            break;
        case AX_ATTR_PNODE:
            // val = self.pNode;
            break;
        default:
            return -1;
    }
    return 0;
}

inline int AxAttrVisitorWrite(AxisImplBase& self, AxAttr attr, double val) {
    switch(attr) {
        case AX_ATTR_NATIVE:
            // self.native = val;
            break;
        case AX_ATTR_DRIVEERROR:
            self.driveError = ((int) val);
            break;
        case AX_ATTR_AXISSWERROR:
            self.axisSwError = ((int) val);
            break;
        case AX_ATTR_FBERROR:
            self.fbError = ((int) val);
            break;
        case AX_ATTR_BERROR:
            self.bError = ((int) val);
            break;
        case AX_ATTR_BREGULATORON:
            self.bRegulatorOn = val;
            break;
        case AX_ATTR_BDRIVESTART:
            self.bDriveStart = val;
            break;
        case AX_ATTR_REGULATORREALSTATE:
            self.regulatorRealState = val;
            break;
        case AX_ATTR_DRIVESTARTREALSTATE:
            self.driveStartRealState = val;
            break;
        case AX_ATTR_BSTARTREFERENCE:
            self.bStartReference = val;
            break;
        case AX_ATTR_BSTARTREFERENCEREALSTATE:
            self.bStartReferenceRealState = val;
            break;
        case AX_ATTR_BLOGICAL:
            self.bLogical = val;
            break;
        case AX_ATTR_BSIMULATION:
            self.bSimulation = val;
            break;
        case AX_ATTR_BMOTORROTARYTYPE:
            self.bMotorRotaryType = val;
            break;
        case AX_ATTR_IMOVEMENTTYPE:
            self.iMovementType = val;
            break;
        case AX_ATTR_CONSTANTVELOCITY:
            self.ConstantVelocity = val;
            break;
        case AX_ATTR_ACCELERATING:
            self.Accelerating = val;
            break;
        case AX_ATTR_DECELERATING:
            self.Decelerating = val;
            break;
        case AX_ATTR_NDIRECTION:
            self.nDirection = val;
            break;
        case AX_ATTR_EAXISMTYPE:
            self.eAxisMtype = val;
            break;
        case AX_ATTR_ERAMPTYPE:
            self.eRampType = val;
            break;
        case AX_ATTR_BYCONTROLLERMODE:
            self.byControllerMode = val;
            break;
        case AX_ATTR_BYREALCONTROLLERMODE:
            self.byRealControllerMode = val;
            break;
        case AX_ATTR_AXISSTAT:
            self.axisStat = (MC_AXIS_STATES) (int) val;
            break;
        case AX_ATTR_AXISSTATBAK:
            self.axisStatBak = (MC_AXIS_STATES) (int) val;
            break;
        case AX_ATTR_WAXISSTRUCTID:
            self.wAxisStructID = val;
            break;
        case AX_ATTR_DWERRORID:
            self.dwErrorID = (MC_ERROR) ((int) val);
            break;
        case AX_ATTR_FBEFBERROR:
            self.fbeFBError = (MC_ERROR) ((int) val);
            break;
        case AX_ATTR_CYCLETIME:
            self.cycleTime = val;
            break;
        case AX_ATTR_FSETPOSITION:
            self.fSetPosition = val;
            break;
        case AX_ATTR_FSETVELOCITY:
            self.fSetVelocity = val;
            break;
        case AX_ATTR_FSETACCELERATION:
            self.fSetAcceleration = val;
            break;
        case AX_ATTR_FACTPOSITION:
            self.fActPosition = val;
            break;
        case AX_ATTR_FACTVELOCITY:
            self.fActVelocity = val;
            break;
        case AX_ATTR_FACTACCELERATION:
            self.fActAcceleration = val;
            break;
        case AX_ATTR_FSAVEPOSITION:
            self.fSavePosition = val;
            break;
        case AX_ATTR_FSAVEVELOCITY:
            self.fSaveVelocity = val;
            break;
        case AX_ATTR_FSAVEACCELERATION:
            self.fSaveAcceleration = val;
            break;
        case AX_ATTR_FMAXVELOCITY:
            self.fMaxVelocity = val;
            break;
        case AX_ATTR_FMAXACCELERATION:
            self.fMaxAcceleration = val;
            break;
        case AX_ATTR_FMAXDECELERATION:
            self.fMaxDeceleration = val;
            break;
        case AX_ATTR_FSWMAXVELOCITY:
            self.fSWMaxVelocity = val;
            break;
        case AX_ATTR_FSWMAXACCELERATION:
            self.fSWMaxAcceleration = val;
            break;
        case AX_ATTR_FSWMAXDECELERATION:
            self.fSWMaxDeceleration = val;
            break;
        case AX_ATTR_FSWMAXJERK:
            self.fSWMaxJerk = val;
            break;
        case AX_ATTR_FAIMPOSITION:
            self.fAimPosition = val;
            break;
        case AX_ATTR_FMARKPOSITION:
            self.fMarkPosition = val;
            break;
        case AX_ATTR_FMARKVELOCITY:
            self.fMarkVelocity = val;
            break;
        case AX_ATTR_FMARKACCELERATION:
            self.fMarkAcceleration = val;
            break;
        case AX_ATTR_FSETTORQUE:
            self.fSetTorque = val;
            break;
        case AX_ATTR_FACTTORQUE:
            self.fActTorque = val;
            break;
        case AX_ATTR_FMAXTORQUE:
            self.fMaxTorque = val;
            break;
        case AX_ATTR_FSWMAXTORQUE:
            self.fSWMaxTorque = val;
            break;
        case AX_ATTR_FSCALEFACTOR:
            self.fScalefactor = val;
            break;
        case AX_ATTR_FFACTORVEL:
            self.fFactorVel = val;
            break;
        case AX_ATTR_FFACTORACC:
            self.fFactorAcc = val;
            break;
        case AX_ATTR_FFACTORTOR:
            self.fFactorTor = val;
            break;
        case AX_ATTR_FFACTORJERK:
            self.fFactorJerk = val;
            break;
        case AX_ATTR_FPOSITIONPERIOD:
            self.fPositionPeriod = val;
            break;
        case AX_ATTR_IRATIOTECHUNITSNUM:
            self.iRatioTechUnitsNum = val;
            break;
        case AX_ATTR_DWRATIOTECHUNITSDENOM:
            self.dwRatioTechUnitsDenom = val;
            break;
        case AX_ATTR_FOFFSETPOSITION:
            self.fOffsetPosition = val;
            break;
        case AX_ATTR_DWPOSOFFSETFORRESIDUALS:
            self.dwPosOffsetForResiduals = val;
            break;
        case AX_ATTR_DWLASTPOSITION:
            self.dwLastPosition = val;
            break;
        case AX_ATTR_ITURN:
            self.iTurn = val;
            break;
        case AX_ATTR_DISETPOSITION:
            self.diSetPosition = val;
            break;
        case AX_ATTR_DIACTPOSITION:
            self.diActPosition = val;
            break;
        case AX_ATTR_DISETVELOCITY:
            self.diSetVelocity = val;
            break;
        case AX_ATTR_DIACTVELOCITY:
            self.diActVelocity = val;
            break;
        case AX_ATTR_DISETACCELERATION:
            self.diSetAcceleration = val;
            break;
        case AX_ATTR_DIACTACCELERATION:
            self.diActAcceleration = val;
            break;
        case AX_ATTR_DISETCURRENT:
            self.diSetCurrent = val;
            break;
        case AX_ATTR_DIACTCURRENT:
            self.diActCurrent = val;
            break;
        case AX_ATTR_DISETTORQUE:
            self.diSetTorque = val;
            break;
        case AX_ATTR_DIACTTORQUE:
            self.diActTorque = val;
            break;
        case AX_ATTR_DIENCODERPOSITION:
            self.diEncoderPosition = val;
            break;
        case AX_ATTR_BSWLIMITENABLE:
            self.bSWLimitEnable = val;
            break;
        case AX_ATTR_BHWLIMITENABLE:
            self.bHWLimitEnable = val;
            break;
        case AX_ATTR_BPOSITIONLAGACTIVE:
            self.bPositionLagActive = val;
            break;
        case AX_ATTR_ECHECKPOSITIONLAG:
            self.eCheckPositionLag = val;
            break;
        case AX_ATTR_FSWLIMITPOSITIVE:
            self.fSWLimitPositive = val;
            break;
        case AX_ATTR_FSWLIMITNEGATIVE:
            self.fSWLimitNegative = val;
            break;
        case AX_ATTR_FMAXPOSITIONLAG:
            self.fMaxPositionLag = val;
            break;
        case AX_ATTR_FSWLIMITDECELERATION:
            self.fSWLimitDeceleration = val;
            break;
        case AX_ATTR_FSWERRORMAXDISTANCE:
            self.fSWErrorMaxDistance = (MC_ERROR) ((int) val);
            break;
        case AX_ATTR_FSETACTTIMELAGCYCLES:
            self.fSetActTimeLagCycles = val;
            break;
        case AX_ATTR_USEENCODERBITWIDTH:
            self.usiEncoderBitWidth = val;
            break;
        case AX_ATTR_SWSMEX:
            self.swSmEx = val;
            break;
        case AX_ATTR_CMD:
            self.cmd = (AxisCmdEnum) (int) val;
            break;
        case AX_ATTR_WSTATUSWORD:
            self.wStatusWord = val;
            break;
        case AX_ATTR_WCONTROLWORD:
            self.wControlWord = val;
            break;
        case AX_ATTR_M_CYCLECOUNTER:
            self.m_cycleCounter = val;
            break;
        case AX_ATTR_FBBAK:
            // self.fbBak = val;
            break;
        case AX_ATTR_PNODE:
            // self.pNode = val;
            break;
        default:
            return -1;
    }
    return 0;
}


inline
int AxAttrVisitorRead(const AxisImplBase& self, AxAttr attr, void*& ptr) {
    double& val = reinterpret_cast<double&>(ptr);
    return AxAttrVisitorRead(self, attr, val);
}

inline
int AxAttrVisitorWrite(AxisImplBase& self, AxAttr attr, const void* ref) {
    double val = reinterpret_cast<const double&>(ref);
    return AxAttrVisitorWrite(self, attr, val);
    // return -1;
}

} // namespace zmc
#endif