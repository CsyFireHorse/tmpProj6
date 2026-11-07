#include "mc_fb_api.h"
#include "fb_cpp_def.h"
#include "mc_fb_extra_api.h"
#include <new>

using namespace zmc;

int MC_Power_create(McPower* pFb){ 
    static_assert(sizeof(FbtPower) == sizeof(McPower), "McPower size not equal");
    new (pFb) FbtPower();
    return 0;
}

int MC_Power_exec(McPower* pFb){
    ((FbtPower*) pFb)->runCycle();
    return 0;
}

int MC_Power_destroy(McPower* pFb){
    ((FbtPower*)pFb)->~FbtPower();
    return 0;
}

int MC_Reset_create(McReset* pFb){
    new (pFb) FbtReset();
    return 0;
}

int MC_Reset_exec(McReset* pFb){
    ((FbtReset*) pFb)->runCycle();
    return 0;
}

int MC_Reset_destroy(McReset* pFb){
    ((FbtReset*)pFb)->~FbtReset();
    return 0;
}

int MC_SetPosition_create(McSetPosition* pFb){
    new (pFb) FbtSetPosition();
    return 0;
}

int MC_SetPosition_exec(McSetPosition* pFb){
    ((FbtSetPosition*) pFb)->runCycle();
    return 0;
}

int MC_SetPosition_destroy(McSetPosition* pFb){
    ((FbtSetPosition*)pFb)->~FbtSetPosition();
    return 0;
}

int MC_ReadStatus_create(McReadStatus* pFb){
    new (pFb) FbtReadStatus();
    return 0;
}

int MC_ReadStatus_exec(McReadStatus* pFb){
    ((FbtReadStatus*) pFb)->runCycle();
    return 0;
}

int MC_ReadStatus_destroy(McReadStatus* pFb){
    ((FbtReadStatus*)pFb)->~FbtReadStatus();
    return 0;
}

int MC_ReadActualPosition_create(McReadActualPosition* pFb){
    new (pFb) FbtReadActualPosition();
    return 0;
}

int MC_ReadActualPosition_exec(McReadActualPosition* pFb){
    ((FbtReadActualPosition*) pFb)->runCycle();
    return 0;
}

int MC_ReadActualPosition_destroy(McReadActualPosition* pFb){
    ((FbtReadActualPosition*)pFb)->~FbtReadActualPosition();
    return 0;
}

int MC_ReadActualTorque_create(McReadActualTorque* pFb){
    new (pFb) FbtReadActualTorque();
    return 0;
}

int MC_ReadActualTorque_exec(McReadActualTorque* pFb){
    ((FbtReadActualTorque*) pFb)->runCycle();
    return 0;
}

int MC_ReadActualTorque_destroy(McReadActualTorque* pFb){
    ((FbtReadActualTorque*)pFb)->~FbtReadActualTorque();
    return 0;
}

int MC_ReadActualVelocity_create(McReadActualVelocity* pFb){
    new (pFb) FbtReadActualVelocity();
    return 0;
}

int MC_ReadActualVelocity_exec(McReadActualVelocity* pFb){
    ((FbtReadActualVelocity*) pFb)->runCycle();
    return 0;
}

int MC_ReadActualVelocity_destroy(McReadActualVelocity* pFb){
    ((FbtReadActualVelocity*)pFb)->~FbtReadActualVelocity();
    return 0;
}

int MC_ReadAxisError_create(McReadAxisError* pFb){
    new (pFb) FbtReadAxisError();
    return 0;
}

int MC_ReadAxisError_exec(McReadAxisError* pFb){
    ((FbtReadAxisError*) pFb)->runCycle();
    return 0;
}

int MC_ReadAxisError_destroy(McReadAxisError* pFb){
    ((FbtReadAxisError*)pFb)->~FbtReadAxisError();
    return 0;
}

int MC_ReadCommandPosition_create(McReadCommandPosition* pFb){
    new (pFb) FbtReadCommandPosition();
    return 0;
}

int MC_ReadCommandPosition_exec(McReadCommandPosition* pFb){
    ((FbtReadCommandPosition*) pFb)->runCycle();
    return 0;
}

int MC_ReadCommandPosition_destroy(McReadCommandPosition* pFb){
    ((FbtReadCommandPosition*)pFb)->~FbtReadCommandPosition();
    return 0;
}

int MC_ReadSetPosition_create(McReadSetPosition* pFb){
    new (pFb) FbtReadSetPosition();
    return 0;
}

int MC_ReadSetPosition_exec(McReadSetPosition* pFb){
    ((FbtReadSetPosition*) pFb)->runCycle();
    return 0;
}

int MC_ReadSetPosition_destroy(McReadSetPosition* pFb){
    ((FbtReadSetPosition*)pFb)->~FbtReadSetPosition();
    return 0;
}

int MC_ReadBoolParameter_create(McReadBoolParameter* pFb){
    new (pFb) FbtReadBoolParameter();
    return 0;
}

int MC_ReadBoolParameter_exec(McReadBoolParameter* pFb){
    ((FbtReadBoolParameter*) pFb)->runCycle();
    return 0;
}

int MC_ReadBoolParameter_destroy(McReadBoolParameter* pFb){
    ((FbtReadBoolParameter*)pFb)->~FbtReadBoolParameter();
    return 0;
}

int MC_ReadParameter_create(McReadParameter* pFb){
    new (pFb) FbtReadParameter();
    return 0;
}

int MC_ReadParameter_exec(McReadParameter* pFb){
    ((FbtReadParameter*) pFb)->runCycle();
    return 0;
}

int MC_ReadParameter_destroy(McReadParameter* pFb){
    ((FbtReadParameter*)pFb)->~FbtReadParameter();
    return 0;
}

int MC_WriteBoolParameter_create(McWriteBoolParameter* pFb){
    new (pFb) FbtWriteBoolParameter();
    return 0;
}

int MC_WriteBoolParameter_exec(McWriteBoolParameter* pFb){
    ((FbtWriteBoolParameter*) pFb)->runCycle();
    return 0;
}

int MC_WriteBoolParameter_destroy(McWriteBoolParameter* pFb){
    ((FbtWriteBoolParameter*)pFb)->~FbtWriteBoolParameter();
    return 0;
}

int MC_WriteParameter_create(McWriteParameter* pFb){
    new (pFb) FbtWriteParameter();
    return 0;
}

int MC_WriteParameter_exec(McWriteParameter* pFb){
    ((FbtWriteParameter*) pFb)->runCycle();
    return 0;
}

int MC_WriteParameter_destroy(McWriteParameter* pFb){
    ((FbtWriteParameter*)pFb)->~FbtWriteParameter();
    return 0;
}

int MC_Stop_create(McStop* pFb){
    new (pFb) FbtStop();
    return 0;
}

int MC_Stop_exec(McStop* pFb){
    ((FbtStop*) pFb)->runCycle();
    return 0;
}

int MC_Stop_destroy(McStop* pFb){
    ((FbtStop*)pFb)->~FbtStop();
    return 0;
}

int MC_Halt_create(McHalt* pFb){
    new (pFb) FbtHalt();
    return 0;
}

int MC_Halt_exec(McHalt* pFb){
    ((FbtHalt*) pFb)->runCycle();
    return 0;
}

int MC_Halt_destroy(McHalt* pFb){
    ((FbtHalt*)pFb)->~FbtHalt();
    return 0;
}

int MC_MoveAbsolute_create(McMoveAbsolute* pFb){
    static_assert(sizeof(FbtMoveAbsolute) == sizeof(McMoveAbsolute), "MoveAbsolute size not equal");
    new (pFb) FbtMoveAbsolute();
    return 0;
}

int MC_MoveAbsolute_exec(McMoveAbsolute* pFb){
    ((FbtMoveAbsolute*) pFb)->runCycle();
    return 0;
}

int MC_MoveAbsolute_destroy(McMoveAbsolute* pFb){
    ((FbtMoveAbsolute*)pFb)->~FbtMoveAbsolute();
    return 0;
}

int MC_MoveAdditive_create(McMoveAdditive* pFb){
    new (pFb) FbtMoveAdditive();
    return 0;
}

int MC_MoveAdditive_exec(McMoveAdditive* pFb){
    ((FbtMoveAdditive*) pFb)->runCycle();
    return 0;
}

int MC_MoveAdditive_destroy(McMoveAdditive* pFb){
    ((FbtMoveAdditive*)pFb)->~FbtMoveAdditive();
    return 0;
}

int MC_MoveRelative_create(McMoveRelative* pFb){
    new (pFb) FbtMoveRelative();
    return 0;
}

int MC_MoveRelative_exec(McMoveRelative* pFb){
    ((FbtMoveRelative*) pFb)->runCycle();
    return 0;
}

int MC_MoveRelative_destroy(McMoveRelative* pFb){
    ((FbtMoveRelative*)pFb)->~FbtMoveRelative();
    return 0;
}

int MC_MoveVelocity_create(McMoveVelocity* pFb){
    new (pFb) FbtMoveVelocity();
    return 0;
}

int MC_MoveVelocity_exec(McMoveVelocity* pFb){
    ((FbtMoveVelocity*) pFb)->runCycle();
    return 0;
}

int MC_MoveVelocity_destroy(McMoveVelocity* pFb){
    ((FbtMoveVelocity*)pFb)->~FbtMoveVelocity();
    return 0;
}

int MC_MoveContinuousAbsolute_create(McMoveContinuousAbsolute* pFb){
    new (pFb) FbtMoveContinuousAbsolute();
    return 0;
}

int MC_MoveContinuousAbsolute_exec(McMoveContinuousAbsolute* pFb){
    ((FbtMoveContinuousAbsolute*) pFb)->runCycle();
    return 0;
}

int MC_MoveContinuousAbsolute_destroy(McMoveContinuousAbsolute* pFb){
    ((FbtMoveContinuousAbsolute*)pFb)->~FbtMoveContinuousAbsolute();
    return 0;
}

int MC_MoveContinuousRelative_create(McMoveContinuousRelative* pFb){
    new (pFb) FbtMoveContinuousRelative();
    return 0;
}

int MC_MoveContinuousRelative_exec(McMoveContinuousRelative* pFb){
    ((FbtMoveContinuousRelative*) pFb)->runCycle();
    return 0;
}

int MC_MoveContinuousRelative_destroy(McMoveContinuousRelative* pFb){
    ((FbtMoveContinuousRelative*)pFb)->~FbtMoveContinuousRelative();
    return 0;
}

int MC_MoveSuperImposed_create(McMoveSuperImposed* pFb){
    new (pFb) FbtMoveSuperImposed();
    return 0;
}

int MC_MoveSuperImposed_exec(McMoveSuperImposed* pFb){
    ((FbtMoveSuperImposed*) pFb)->runCycle();
    return 0;
}

int MC_MoveSuperImposed_destroy(McMoveSuperImposed* pFb){
    ((FbtMoveSuperImposed*)pFb)->~FbtMoveSuperImposed();
    return 0;
}

int MC_HaltSuperImposed_create(McHaltSuperImposed* pFb){
    new (pFb) FbtHaltSuperImposed();
    return 0;
}

int MC_HaltSuperImposed_exec(McHaltSuperImposed* pFb){
    ((FbtHaltSuperImposed*) pFb)->runCycle();
    return 0;
}

int MC_HaltSuperImposed_destroy(McHaltSuperImposed* pFb){
    ((FbtHaltSuperImposed*)pFb)->~FbtHaltSuperImposed();
    return 0;
}

int MC_PositionProfile_create(McPositionProfile* pFb){
    static_assert(sizeof(FbtPositionProfile) == sizeof(McPositionProfile), "PositionProfile size not equal");
    new (pFb) FbtPositionProfile();
    return 0;
}

int MC_PositionProfile_exec(McPositionProfile* pFb){
    ((FbtPositionProfile*) pFb)->runCycle();
    return 0;
}

int MC_PositionProfile_destroy(McPositionProfile* pFb){
    ((FbtPositionProfile*)pFb)->~FbtPositionProfile();
    return 0;
}

int MC_AccelerationProfile_create(McAccelerationProfile* pFb){
    new (pFb) FbtAccelerationProfile();
    return 0;
}

int MC_AccelerationProfile_exec(McAccelerationProfile* pFb){
    ((FbtAccelerationProfile*) pFb)->runCycle();
    return 0;
}

int MC_AccelerationProfile_destroy(McAccelerationProfile* pFb){
    ((FbtAccelerationProfile*)pFb)->~FbtAccelerationProfile();
    return 0;
}

int MC_VelocityProfile_create(McVelocityProfile* pFb){
    new (pFb) FbtVelocityProfile();
    return 0;
}

int MC_VelocityProfile_exec(McVelocityProfile* pFb){
    ((FbtVelocityProfile*) pFb)->runCycle();
    return 0;
}

int MC_VelocityProfile_destroy(McVelocityProfile* pFb){
    ((FbtVelocityProfile*)pFb)->~FbtVelocityProfile();
    return 0;
}

int MC_Home_create(McHome* pFb){
    new (pFb) FbtHome();
    return 0;
}

int MC_Home_exec(McHome* pFb){
    ((FbtHome*) pFb)->runCycle();
    return 0;
}

int MC_Home_destroy(McHome* pFb){
    ((FbtHome*)pFb)->~FbtHome();
    return 0;
}

int MC_Homing_create(McHoming* pFb){
    new (pFb) FbtHoming();
    return 0;
}

int MC_Homing_exec(McHoming* pFb){
    ((FbtHoming*) pFb)->runCycle();
    return 0;
}

int MC_Homing_destroy(McHoming* pFb){
    ((FbtHoming*)pFb)->~FbtHoming();
    return 0;
}

int MC_Jog_create(McJog* pFb){
    new (pFb) FbtJog();
    return 0;
}

int MC_Jog_exec(McJog* pFb){
    ((FbtJog*) pFb)->runCycle();
    return 0;
}

int MC_Jog_destroy(McJog* pFb){
    ((FbtJog*)pFb)->~FbtJog();
    return 0;
}

int MC_Inch_create(McInch* pFb){
    new (pFb) FbtInch();
    return 0;
}

int MC_Inch_exec(McInch* pFb){
    ((FbtInch*) pFb)->runCycle();
    return 0;
}

int MC_Inch_destroy(McInch* pFb){
    ((FbtInch*)pFb)->~FbtInch();
    return 0;
}

int MC_GearIn_create(McGearIn* pFb){
    new (pFb) FbtGearIn();
    return 0;
}

int MC_GearIn_exec(McGearIn* pFb){
    ((FbtGearIn*) pFb)->runCycle();
    return 0;
}

int MC_GearIn_destroy(McGearIn* pFb){
    ((FbtGearIn*)pFb)->~FbtGearIn();
    return 0;
}

int MC_GearInPos_create(McGearInPos* pFb){
    new (pFb) FbtGearInPos();
    return 0;
}

int MC_GearInPos_exec(McGearInPos* pFb){
    ((FbtGearInPos*) pFb)->runCycle();
    return 0;
}

int MC_GearInPos_destroy(McGearInPos* pFb){
    ((FbtGearInPos*)pFb)->~FbtGearInPos();
    return 0;
}

int MC_GearOut_create(McGearOut* pFb){
    new (pFb) FbtGearOut();
    return 0;
}

int MC_GearOut_exec(McGearOut* pFb){
    ((FbtGearOut*) pFb)->runCycle();
    return 0;
}

int MC_GearOut_destroy(McGearOut* pFb){
    ((FbtGearOut*)pFb)->~FbtGearOut();
    return 0;
}

int MC_Phasing_create(McPhasing* pFb){
    new (pFb) FbtPhasing();
    return 0;
}

int MC_Phasing_exec(McPhasing* pFb){
    ((FbtPhasing*) pFb)->runCycle();
    return 0;
}

int MC_Phasing_destroy(McPhasing* pFb){
    ((FbtPhasing*)pFb)->~FbtPhasing();
    return 0;
}

int MC_SetTorque_create(McSetTorque* pFb){
    new (pFb) FbtSetTorque();
    return 0;
}

int MC_SetTorque_exec(McSetTorque* pFb){
    ((FbtSetTorque*) pFb)->runCycle();
    return 0;
}

int MC_SetTorque_destroy(McSetTorque* pFb){
    ((FbtSetTorque*)pFb)->~FbtSetTorque();
    return 0;
}

int MC_FollowPosition_create(McFollowPosition* pFb){
    new (pFb) FbtFollowPosition();
    return 0;
}

int MC_FollowPosition_exec(McFollowPosition* pFb){
    ((FbtFollowPosition*) pFb)->runCycle();
    return 0;
}

int MC_FollowPosition_destroy(McFollowPosition* pFb){
    ((FbtFollowPosition*)pFb)->~FbtFollowPosition();
    return 0;
}

int MC_FollowPositionVelocity_create(McFollowPositionVelocity* pFb){
    new (pFb) FbtFollowPositionVelocity();
    return 0;
}

int MC_FollowPositionVelocity_exec(McFollowPositionVelocity* pFb){
    ((FbtFollowPositionVelocity*) pFb)->runCycle();
    return 0;
}

int MC_FollowPositionVelocity_destroy(McFollowPositionVelocity* pFb){
    ((FbtFollowPositionVelocity*)pFb)->~FbtFollowPositionVelocity();
    return 0;
}

int MC_FollowVelocity_create(McFollowVelocity* pFb){
    new (pFb) FbtFollowVelocity();
    return 0;
}

int MC_FollowVelocity_exec(McFollowVelocity* pFb){
    ((FbtFollowVelocity*) pFb)->runCycle();
    return 0;
}

int MC_FollowVelocity_destroy(McFollowVelocity* pFb){
    ((FbtFollowVelocity*)pFb)->~FbtFollowVelocity();
    return 0;
}

int MC_FollowSetValues_create(McFollowSetValues* pFb){
    static_assert(sizeof(FbtFollowSetValues) == sizeof(McFollowSetValues), "FollowSetValues size not equal");
    new (pFb) FbtFollowSetValues();
    return 0;
}

int MC_FollowSetValues_exec(McFollowSetValues* pFb){
    ((FbtFollowSetValues*) pFb)->runCycle();
    return 0;
}

int MC_FollowSetValues_destroy(McFollowSetValues* pFb){
    ((FbtFollowSetValues*)pFb)->~FbtFollowSetValues();
    return 0;
}

int MC_TrackAxis_create(McTrackAxis* pFb){
    new (pFb) FbtTrackAxis();
    return 0;
}

int MC_TrackAxis_exec(McTrackAxis* pFb){
    ((FbtTrackAxis*) pFb)->runCycle();
    return 0;
}

int MC_TrackAxis_destroy(McTrackAxis* pFb){
    ((FbtTrackAxis*)pFb)->~FbtTrackAxis();
    return 0;
}

int MC_TrackSetValues_create(McTrackSetValues* pFb){
    new (pFb) FbtTrackSetValues();
    return 0;
}

int MC_TrackSetValues_exec(McTrackSetValues* pFb){
    ((FbtTrackSetValues*) pFb)->runCycle();
    return 0;
}

int MC_TrackSetValues_destroy(McTrackSetValues* pFb){
    ((FbtTrackSetValues*)pFb)->~FbtTrackSetValues();
    return 0;
}

int MC_CamIn_create(McCamIn* pFb){
    static_assert(sizeof(FbtCamIn) == sizeof(McCamIn), "CamIn size not equal");
    new (pFb) FbtCamIn();
    return 0;
}

int MC_CamIn_exec(McCamIn* pFb){
    ((FbtCamIn*) pFb)->runCycle();
    return 0;
}

int MC_CamIn_destroy(McCamIn* pFb){
    ((FbtCamIn*)pFb)->~FbtCamIn();
    return 0;
}

int MC_CamOut_create(McCamOut* pFb){
    new (pFb) FbtCamOut();
    return 0;
}

int MC_CamOut_exec(McCamOut* pFb){
    ((FbtCamOut*) pFb)->runCycle();
    return 0;
}

int MC_CamOut_destroy(McCamOut* pFb){
    ((FbtCamOut*)pFb)->~FbtCamOut();
    return 0;
}

int MC_CamTableSelect_create(McCamTableSelect* pFb){
    new (pFb) FbtCamTableSelect();
    return 0;
}

int MC_CamTableSelect_exec(McCamTableSelect* pFb){
    ((FbtCamTableSelect*) pFb)->runCycle();
    return 0;
}

int MC_CamTableSelect_destroy(McCamTableSelect* pFb){
    ((FbtCamTableSelect*)pFb)->~FbtCamTableSelect();
    return 0;
}

int MC_GetTappetValue_create(McGetTappetValue* pFb){
    new (pFb) FbtGetTappetValue();
    return 0;
}

int MC_GetTappetValue_exec(McGetTappetValue* pFb){
    ((FbtGetTappetValue*) pFb)->runCycle();
    return 0;
}

int MC_GetTappetValue_destroy(McGetTappetValue* pFb){
    ((FbtGetTappetValue*)pFb)->~FbtGetTappetValue();
    return 0;
}

int MC_CamRegister_create(McCamRegister* pFb){
    new (pFb) FbtCamRegister();
    return 0;
}

int MC_CamRegister_exec(McCamRegister* pFb){
    ((FbtCamRegister*) pFb)->runCycle();
    return 0;
}

int MC_CamRegister_destroy(McCamRegister* pFb){
    ((FbtCamRegister*)pFb)->~FbtCamRegister();
    return 0;
}

int MC_CAMBounds_create(McCAMBounds* pFb){
    new (pFb) FbtCAMBounds();
    return 0;
}

int MC_CAMBounds_exec(McCAMBounds* pFb){
    ((FbtCAMBounds*) pFb)->runCycle();
    return 0;
}

int MC_CAMBounds_destroy(McCAMBounds* pFb){
    ((FbtCAMBounds*)pFb)->~FbtCAMBounds();
    return 0;
}

int MC_CAMBounds_Pos_create(McCAMBounds_Pos* pFb){
    new (pFb) FbtCAMBounds_Pos();
    return 0;
}

int MC_CAMBounds_Pos_exec(McCAMBounds_Pos* pFb){
    ((FbtCAMBounds_Pos*) pFb)->runCycle();
    return 0;
}

int MC_CAMBounds_Pos_destroy(McCAMBounds_Pos* pFb){
    ((FbtCAMBounds_Pos*)pFb)->~FbtCAMBounds_Pos();
    return 0;
}

int MC_GetCamSlaveSetPosition_create(McGetCamSlaveSetPosition* pFb){
    new (pFb) FbtGetCamSlaveSetPosition();
    return 0;
}

int MC_GetCamSlaveSetPosition_exec(McGetCamSlaveSetPosition* pFb){
    ((FbtGetCamSlaveSetPosition*) pFb)->runCycle();
    return 0;
}

int MC_GetCamSlaveSetPosition_destroy(McGetCamSlaveSetPosition* pFb){
    ((FbtGetCamSlaveSetPosition*)pFb)->~FbtGetCamSlaveSetPosition();
    return 0;
}


int MC_AbortTrigger_create(McAbortTrigger* pFb){
    new (pFb) FbtAbortTrigger();
    return 0;
}

int MC_AbortTrigger_exec(McAbortTrigger* pFb){
    ((FbtAbortTrigger*) pFb)->runCycle();
    return 0;
}

int MC_AbortTrigger_destroy(McAbortTrigger* pFb){
    ((FbtAbortTrigger*)pFb)->~FbtAbortTrigger();
    return 0;
}

int MC_TouchProbe_create(McTouchProbe* pFb){
    new (pFb) FbtTouchProbe();
    return 0;
}

int MC_TouchProbe_exec(McTouchProbe* pFb){
    ((FbtTouchProbe*) pFb)->runCycle();
    return 0;
}

int MC_TouchProbe_destroy(McTouchProbe* pFb){
    ((FbtTouchProbe*)pFb)->~FbtTouchProbe();
    return 0;
}

int MC_DigitalCamSwitch_create(McDigitalCamSwitch* pFb){
    new (pFb) FbtDigitalCamSwitch();
    return 0;
}

int MC_DigitalCamSwitch_exec(McDigitalCamSwitch* pFb){
    ((FbtDigitalCamSwitch*) pFb)->runCycle();
    return 0;
}

int MC_DigitalCamSwitch_destroy(McDigitalCamSwitch* pFb){
    ((FbtDigitalCamSwitch*)pFb)->~FbtDigitalCamSwitch();
    return 0;
}

int MC_ReadCAM_create(McReadCAM* pFb){
    new (pFb) FbtReadCAM();
    return 0;
}

int MC_ReadCAM_exec(McReadCAM* pFb){
    ((FbtReadCAM*) pFb)->runCycle();
    return 0;
}

int MC_ReadCAM_destroy(McReadCAM* pFb){
    ((FbtReadCAM*)pFb)->~FbtReadCAM();
    return 0;
}

int MC_WriteCAM_create(McWriteCAM* pFb){
    new (pFb) FbtWriteCAM();
    return 0;
}

int MC_WriteCAM_exec(McWriteCAM* pFb){
    ((FbtWriteCAM*) pFb)->runCycle();
    return 0;
}

int MC_WriteCAM_destroy(McWriteCAM* pFb){
    ((FbtWriteCAM*)pFb)->~FbtWriteCAM();
    return 0;
}

int MC_ReadSetValues_create(McReadSetValues* pFb){
    new (pFb) FbtReadSetValues();
    return 0;
}

int MC_ReadSetValues_exec(McReadSetValues* pFb){
    ((FbtReadSetValues*) pFb)->runCycle();
    return 0;
}

int MC_ReadSetValues_destroy(McReadSetValues* pFb){
    ((FbtReadSetValues*)pFb)->~FbtReadSetValues();
    return 0;
}

int MC_ReadActualValues_create(McReadActualValues* pFb){
    new (pFb) FbtReadActualValues();
    return 0;
}

int MC_ReadActualValues_exec(McReadActualValues* pFb){
    ((FbtReadActualValues*) pFb)->runCycle();
    return 0;
}

int MC_ReadActualValues_destroy(McReadActualValues* pFb){
    ((FbtReadActualValues*)pFb)->~FbtReadActualValues();
    return 0;
}

int MC_ReadSetDriveValues_create(McReadSetDriveValues* pFb){
    new (pFb) FbtReadSetDriveValues();
    return 0;
}

int MC_ReadSetDriveValues_exec(McReadSetDriveValues* pFb){
    ((FbtReadSetDriveValues*) pFb)->runCycle();
    return 0;
}

int MC_ReadSetDriveValues_destroy(McReadSetDriveValues* pFb){
    ((FbtReadSetDriveValues*)pFb)->~FbtReadSetDriveValues();
    return 0;
}

int MC_ReadActualDriveValues_create(McReadActualDriveValues* pFb){
    new (pFb) FbtReadActualDriveValues();
    return 0;
}

int MC_ReadActualDriveValues_exec(McReadActualDriveValues* pFb){
    ((FbtReadActualDriveValues*) pFb)->runCycle();
    return 0;
}

int MC_ReadActualDriveValues_destroy(McReadActualDriveValues* pFb){
    ((FbtReadActualDriveValues*)pFb)->~FbtReadActualDriveValues();
    return 0;
}

int MC_GetTrackingError_create(McGetTrackingError* pFb){
    static_assert(sizeof(FbtGetTrackingError) == sizeof(McGetTrackingError), "GetTrackingError size not equal");
    new (pFb) FbtGetTrackingError();
    return 0;
}

int MC_GetTrackingError_exec(McGetTrackingError* pFb){
    ((FbtGetTrackingError*) pFb)->runCycle();
    return 0;
}

int MC_GetTrackingError_destroy(McGetTrackingError* pFb){
    ((FbtGetTrackingError*)pFb)->~FbtGetTrackingError();
    return 0;
}

int MC_GetTravelledDistance_create(McGetTravelledDistance* pFb){
    static_assert(sizeof(FbtGetTravelledDistance) == sizeof(McGetTravelledDistance), "GetTravelledDistance size not equal");
    new (pFb) FbtGetTravelledDistance();
    return 0;
}

int MC_GetTravelledDistance_exec(McGetTravelledDistance* pFb){
    ((FbtGetTravelledDistance*) pFb)->runCycle();
    return 0;
}

int MC_GetTravelledDistance_destroy(McGetTravelledDistance* pFb){
    ((FbtGetTravelledDistance*)pFb)->~FbtGetTravelledDistance();
    return 0;
}

int MC_GetMaxSetAccDec_create(McGetMaxSetAccDec* pFb){
    static_assert(sizeof(FbtGetMaxSetAccDec) == sizeof(McGetMaxSetAccDec), "GetMaxSetAccDec size not equal");
    new (pFb) FbtGetMaxSetAccDec();
    return 0;
}

int MC_GetMaxSetAccDec_exec(McGetMaxSetAccDec* pFb){
    ((FbtGetMaxSetAccDec*) pFb)->runCycle();
    return 0;
}

int MC_GetMaxSetAccDec_destroy(McGetMaxSetAccDec* pFb){
    ((FbtGetMaxSetAccDec*)pFb)->~FbtGetMaxSetAccDec();
    return 0;
}

int MC_GetMaxSetVelocity_create(McGetMaxSetVelocity* pFb){
    static_assert(sizeof(FbtGetMaxSetVelocity) == sizeof(McGetMaxSetVelocity), "GetMaxSetVelocity size not equal");
    new (pFb) FbtGetMaxSetVelocity();
    return 0;
}

int MC_GetMaxSetVelocity_exec(McGetMaxSetVelocity* pFb){
    ((FbtGetMaxSetVelocity*) pFb)->runCycle();
    return 0;
}

int MC_GetMaxSetVelocity_destroy(McGetMaxSetVelocity* pFb){
    ((FbtGetMaxSetVelocity*)pFb)->~FbtGetMaxSetVelocity();
    return 0;
}

int MC_GetOverride_create(McGetOverride* pFb){
    static_assert(sizeof(FbtGetOverride) == sizeof(McGetOverride), "GetOverride size not equal");
    new (pFb) FbtGetOverride();
    return 0;
}

int MC_GetOverride_exec(McGetOverride* pFb){
    ((FbtGetOverride*) pFb)->runCycle();
    return 0;
}

int MC_GetOverride_destroy(McGetOverride* pFb){
    ((FbtGetOverride*)pFb)->~FbtGetOverride();
    return 0;
}

int MC_SetOverride_create(McSetOverride* pFb){
    static_assert(sizeof(FbtSetOverride) == sizeof(McSetOverride), "SetOverride size not equal");
    new (pFb) FbtSetOverride();
    return 0;
}

int MC_SetOverride_exec(McSetOverride* pFb){
    ((FbtSetOverride*) pFb)->runCycle();
    return 0;
}

int MC_SetOverride_destroy(McSetOverride* pFb){
    ((FbtSetOverride*)pFb)->~FbtSetOverride();
    return 0;
}

int MC_CheckLimits_create(McCheckLimits* pFb){
    static_assert(sizeof(FbtCheckLimits) == sizeof(McCheckLimits), "CheckLimits size not equal");
    new (pFb) FbtCheckLimits();
    return 0;
}

int MC_CheckLimits_exec(McCheckLimits* pFb){
    ((FbtCheckLimits*) pFb)->runCycle();
    return 0;
}

int MC_CheckLimits_destroy(McCheckLimits* pFb){
    ((FbtCheckLimits*)pFb)->~FbtCheckLimits();
    return 0;
}

int MC_InPosition_create(McInPosition* pFb){
    static_assert(sizeof(FbtInPosition) == sizeof(McInPosition), "InPosition size not equal");
    new (pFb) FbtInPosition();
    return 0;
}

int MC_InPosition_exec(McInPosition* pFb){
    ((FbtInPosition*) pFb)->runCycle();
    return 0;
}

int MC_InPosition_destroy(McInPosition* pFb){
    ((FbtInPosition*)pFb)->~FbtInPosition();
    return 0;
}

int MC_SetMovementType_create(McSetMovementType* pFb){
    static_assert(sizeof(FbtSetMovementType) == sizeof(McSetMovementType), "SetMovementType size not equal");
    new (pFb) FbtSetMovementType();
    return 0;
}

int MC_SetMovementType_exec(McSetMovementType* pFb){
    ((FbtSetMovementType*) pFb)->runCycle();
    return 0;
}

int MC_SetMovementType_destroy(McSetMovementType* pFb){
    ((FbtSetMovementType*)pFb)->~FbtSetMovementType();
    return 0;
}

int MC_SetRampType_create(McSetRampType* pFb){
    static_assert(sizeof(FbtSetRampType) == sizeof(McSetRampType), "SetRampType size not equal");
    new (pFb) FbtSetRampType();
    return 0;
}

int MC_SetRampType_exec(McSetRampType* pFb){
    ((FbtSetRampType*) pFb)->runCycle();
    return 0;
}

int MC_SetRampType_destroy(McSetRampType* pFb){
    ((FbtSetRampType*)pFb)->~FbtSetRampType();
    return 0;
}

int MC_ChangeDynamicLimits_create(McChangeDynamicLimits* pFb){
    static_assert(sizeof(FbtChangeDynamicLimits) == sizeof(McChangeDynamicLimits), "ChangeDynamicLimits size not equal");
    new (pFb) FbtChangeDynamicLimits();
    return 0;
}

int MC_ChangeDynamicLimits_exec(McChangeDynamicLimits* pFb){
    ((FbtChangeDynamicLimits*) pFb)->runCycle();
    return 0;
}

int MC_ChangeDynamicLimits_destroy(McChangeDynamicLimits* pFb){
    ((FbtChangeDynamicLimits*)pFb)->~FbtChangeDynamicLimits();
    return 0;
}

int MC_ChangeGearingRatio_create(McChangeGearingRatio* pFb){
    static_assert(sizeof(FbtChangeGearingRatio) == sizeof(McChangeGearingRatio), "ChangeGearingRatio size not equal");
    new (pFb) FbtChangeGearingRatio();
    return 0;
}

int MC_ChangeGearingRatio_exec(McChangeGearingRatio* pFb){
    ((FbtChangeGearingRatio*) pFb)->runCycle();
    return 0;
}

int MC_ChangeGearingRatio_destroy(McChangeGearingRatio* pFb){
    ((FbtChangeGearingRatio*)pFb)->~FbtChangeGearingRatio();
    return 0;
}

int MC_SetSoftwareLimits_create(McSetSoftwareLimits* pFb){
    static_assert(sizeof(FbtSetSoftwareLimits) == sizeof(McSetSoftwareLimits), "SetSoftwareLimits size not equal");
    new (pFb) FbtSetSoftwareLimits();
    return 0;
}

int MC_SetSoftwareLimits_exec(McSetSoftwareLimits* pFb){
    ((FbtSetSoftwareLimits*) pFb)->runCycle();
    return 0;
}

int MC_SetSoftwareLimits_destroy(McSetSoftwareLimits* pFb){
    ((FbtSetSoftwareLimits*)pFb)->~FbtSetSoftwareLimits();
    return 0;
}

int MC_SetControllerMode_create(McSetControllerMode* pFb){
    static_assert(sizeof(FbtSetControllerMode) == sizeof(McSetControllerMode), "SetControllerMode size not equal");
    new (pFb) FbtSetControllerMode();
    return 0;
}

int MC_SetControllerMode_exec(McSetControllerMode* pFb){
    ((FbtSetControllerMode*) pFb)->runCycle();
    return 0;
}

int MC_SetControllerMode_destroy(McSetControllerMode* pFb){
    ((FbtSetControllerMode*)pFb)->~FbtSetControllerMode();
    return 0;
}

int setTappetFromCamIn(McGetTappetValue* pTappet, McCamIn* pCamIn){
    if (pTappet && pCamIn){
        pTappet->Tappets = &pCamIn->Tappets;
        return 0;
    }
    return -1;
}

int MC_CamCalc_create(McCamCalc* pFb){
    new (pFb) FbtCamCalc();
    return 0;
}

int MC_CamCalc_exec(McCamCalc* pFb){
    ((FbtCamCalc*) pFb)->runCycle();
    return 0;
}

int MC_CamCalc_destroy(McCamCalc* pFb){
    ((FbtCamCalc*)pFb)->~FbtCamCalc();
    return 0;
}

int MC_EstimateDeadTime_create(McEstimateDeadTime* pFb){
    new (pFb) FbtEstimateDeadTime();
    return 0;
}

int MC_EstimateDeadTime_exec(McEstimateDeadTime* pFb){
    ((FbtEstimateDeadTime*) pFb)->runCycle();
    return 0;
}

int MC_EstimateDeadTime_destroy(McEstimateDeadTime* pFb){
    ((FbtEstimateDeadTime*)pFb)->~FbtEstimateDeadTime();
    return 0;
}

int MC_VirtualAxis_create(McVirtualAxis* pFb){
    new (pFb) FbtVirtualAxis();
    return 0;
}

int MC_VirtualAxis_exec(McVirtualAxis* pFb){
    ((FbtVirtualAxis*) pFb)->runCycle();
    return 0;
}

int MC_VirtualAxis_destroy(McVirtualAxis* pFb){
    ((FbtVirtualAxis*)pFb)->~FbtVirtualAxis();
    return 0;
}

int MC_MeasureDistance_create(McMeasureDistance* pFb){
    new (pFb) FbtMeasureDistance();
    return 0;
}

int MC_MeasureDistance_exec(McMeasureDistance* pFb){
    ((FbtMeasureDistance*) pFb)->runCycle();
    return 0;
}

int MC_MeasureDistance_destroy(McMeasureDistance* pFb){
    ((FbtMeasureDistance*)pFb)->~FbtMeasureDistance();
    return 0;
}

int MC_SetAdditionalConversionFactors_create(McSetAdditionalConversionFactors* pFb){
    new (pFb) FbtSetAdditionalConversionFactors();
    return 0;
}

int MC_SetAdditionalConversionFactors_exec(McSetAdditionalConversionFactors* pFb){
    ((FbtSetAdditionalConversionFactors*) pFb)->runCycle();
    return 0;
}

int MC_SetAdditionalConversionFactors_destroy(McSetAdditionalConversionFactors* pFb){
    ((FbtSetAdditionalConversionFactors*)pFb)->~FbtSetAdditionalConversionFactors();
    return 0;
}

int MC_ReinitDrive_create(McReinitDrive* pFb){
    new (pFb) FbtReinitDrive();
    return 0;
}

int MC_ReinitDrive_exec(McReinitDrive* pFb){
    ((FbtReinitDrive*) pFb)->runCycle();
    return 0;
}

int MC_ReinitDrive_destroy(McReinitDrive* pFb){
    ((FbtReinitDrive*)pFb)->~FbtReinitDrive();
    return 0;
}

int MC_ReadValues_create(McReadValues* pFb){
    new (pFb) FbtReadValues();
    return 0;
}

int MC_ReadValues_exec(McReadValues* pFb){
    ((FbtReadValues*) pFb)->runCycle();
    return 0;
}

int MC_ReadValues_destroy(McReadValues* pFb){
    ((FbtReadValues*)pFb)->~FbtReadValues();
    return 0;
}

int MC_ReadPowerParam_create(McReadPowerParam* pFb){
    new (pFb) FbtReadPowerParam();
    return 0;
}

int MC_ReadPowerParam_exec(McReadPowerParam* pFb){
    ((FbtReadPowerParam*) pFb)->runCycle();
    return 0;
}

int MC_ReadPowerParam_destroy(McReadPowerParam* pFb){
    ((FbtReadPowerParam*)pFb)->~FbtReadPowerParam();
    return 0;
}