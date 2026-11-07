#ifndef _ZMC_FB_DECLARE_H_
#define _ZMC_FB_DECLARE_H_

namespace zmc {

class FbtPower;
class FbtReset;
class FbtSetPosition;
class FbtReadStatus;
class FbtReadActualPosition;
class FbtReadActualTorque;
class FbtReadActualVelocity;
class FbtReadAxisError;
class FbtReadSetPosition;
class FbtReadBoolParameter;
class FbtReadParameter;
class FbtReadCoeParameter;
class FbtWriteBoolParameter;
class FbtWriteParameter;
class FbtWriteCoeParameter;
class FbtStop;
class FbtHalt;
class FbtMoveAbsolute;
class FbtMoveAdditive;
class FbtMoveRelative;
class FbtMoveVelocity;
class FbtMoveContinuousAbsolute;
class FbtMoveContinuousRelative;
class FbtMoveSuperImposed;
class FbtPositionProfile;
class FbtAccelerationProfile;
class FbtVelocityProfile;
class FbtHome;
class FbtHoming;
class FbtJog;
class FbtInch;
class FbtGearIn;
class FbtGearInPos;
class FbtGearOut;
class FbtPhasing;
class FbtSetTorque;
class FbtFollowPosition;
class FbtFollowPositionVelocity;
class FbtFollowVelocity;
class FbtFollowSetValues;
class FbtTrackAxis;
class FbtTrackSetValues;
class FbtCamIn;
class FbtCamOut;
class FbtCamTableSelect;
class FbtGetTappetValue;
class FbtCamRegister;
class FbtCAMBounds;
class FbtCAMBounds_Pos;
class FbtGetCamSlaveSetPosition;
class FbtAbortTrigger;
class FbtTouchProbe;
class FbtDigitalCamSwitch;
class FbtReadCAM;
class FbtWriteCAM;
class FbtReadSetValues;
class FbtReadActualValues;
class FbtReadSetDriveValues;
class FbtReadActualDriveValues;
class FbtGetTrackingError;
class FbtGetTravelledDistance;
class FbtGetMaxSetAccDec;
class FbtGetMaxSetVelocity;
class FbtGetOverride;
class FbtSetOverride;

class FbtCheckLimits;
class FbtInPosition;

class FbtMoveDiscrete;
class FbtMoveDisOrVel;
class FbtMoveDisOrCont;

class FbtSetMovementType;
class FbtSetRampType;
class FbtChangeDynamicLimits;
class FbtChangeGearingRatio;
class FbtSetSoftwareLimits;
class FbtSetControllerMode;

// class FbCombineAxes;
// class FbHaltSuperimposed;
// class FbPhasingAbsolute;
// class FbPhasingRelative;
// class FbReadAxisInfo;
// class FbReadCommandVelocity;
// class FbReadMotionState;
}
#endif
