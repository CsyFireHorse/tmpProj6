#include "FbFactory.h"

#include <vector>

#include "FbAttrVisitor.h"
#include "FbAttrVisitorSpec.h"
#include "fbt_axis.h"
#include "fb_cpp_def.h"

#include "axisAttrVisitor.h"
#include "FbAttrVisitorSpec2.hpp"
#include "AxisAttrWrapTmpl.h"

class AxisCiaConfig;
namespace zmc {

// using AxisAttrWrapNop = AxisAttrWrapTmpl<AxisAttrNop>;

class AxFbFactory {
    std::vector<FbWrapper*> fbs;
    std::vector<AxisAttrWrap*> axes;

    friend CommandProcessor;
   public:
    AxFbFactory();
    ~AxFbFactory();
    template <typename T>
    void createFb() {
        fbs.push_back(new FbWrapperImpl < T >());
    }
    void pushFb(FbWrapper* fb) { fbs.push_back(fb); }
    template <typename T>
    void createFbWrap() {
        fbs.push_back(new T());
    }

    template <typename T>
    void createAx() {
        auto ptr = new AxisAttrWrapTmpl<T>();
        axes.push_back(ptr);
    }
    template <typename T>
    void createAxArr(int num) {
        for (int i = 0; i < num; i++) createAx<T>();
    }

    FbWrapper* getFb(int fbId) { return fbs[fbId]; }
    AxisAttrWrap* getAxisWrp(int axisId) { return axes[axisId]; }
};

AxFbFactory::AxFbFactory() {
    axes.reserve(10);
    fbs.push_back(new FbWrapperNop());
    axes.push_back(new AxisAttrWrapTmpl<AxisAttrNop>());
}
AxFbFactory::~AxFbFactory() {
    for (auto fb : fbs) delete fb;
    for (auto axis : axes) delete axis;
}

CommandProcessor::CommandProcessor() : factory(*(new AxFbFactory())) {}

CommandProcessor::~CommandProcessor() {}

void CommandProcessor::createFb(FB_TYPE fbTyp) {
    factory.pushFb(create_fb_wrap(fbTyp));
}

void CommandProcessor::createAxis(AX_FACTORY_TYPE axType) {
    switch (axType) {
        case AX_FACTORY_CIA402:
            factory.createAx<AxisCia402>();
            break;
        case AX_FACTORY_VIRTUAL:
            factory.createAx<AxisVirtual>();
            break;
        case AX_FACTORY_ENCODER:
            factory.createAx<AxisEncoder>();
            break;
        case AX_FACTORY_CIA402_SIMULATED:
            factory.createAx<AxisCia402Fak>();
            break;
        case AX_FACTORY_NOP:
        default:
            factory.createAx<AxisAttrNop>();
            break;
    }
}
void CommandProcessor::axisRunCycle(int idx){
    auto p = getAxisWr(idx);
    if (p)
        p->runCycle();
}
void CommandProcessor::fbRunCycle(int idx){
    auto p = getFb(idx);
    if (p)
        p->runCycle();
}


void CommandProcessor::createAxisArr(int num, AX_FACTORY_TYPE axType) { 
    switch (axType) {
        case AX_FACTORY_CIA402:
            factory.createAxArr<AxisCia402>(num);
            break;
        case AX_FACTORY_VIRTUAL:
            factory.createAxArr<AxisVirtual>(num);
            break;
        case AX_FACTORY_ENCODER:
            factory.createAxArr<AxisEncoder>(num);
            break;
        case AX_FACTORY_CIA402_SIMULATED:
            factory.createAxArr<AxisCia402Fak>(num);
            break;
        case AX_FACTORY_NOP:
            factory.createAxArr<AxisAttrNop>(num); 
        default:
            break;
    }
}

AXIS_REF CommandProcessor::getAxis(int id){
    return factory.getAxisWrp(id)->getAxis();
}

 AxisAttrWrap* CommandProcessor::getAxisWr(int id){
    return factory.getAxisWrp(id);
}

FbWrapper* CommandProcessor::getFb(int id){
    return factory.getFb(id);
}
int CommandProcessor::getFbAttr(int fbId, int attrId, double& val){
    FbWrapper* fb = factory.getFb(fbId);
    if (fb == nullptr) return -1;
    return fb->getAttr((FbAttr)attrId, val);
}
int CommandProcessor::getAxAttr(int fbId, int attrId, double& val){
    AxisAttrWrap* ax = factory.getAxisWrp(fbId);
    if (ax == nullptr) return -1;
    return ax->getAttr(attrId, val);
}
int CommandProcessor::getFbAttr(int fbId, int attrId, void* val){
    FbWrapper* fb = factory.getFb(fbId);
    if (fb == nullptr) return -1;
    return fb->getPtrAttr((FbAttr)attrId, val);
}
int CommandProcessor::setFbAttr(int fbId, int attrId, const void* val){
    FbWrapper* fb = factory.getFb(fbId);
    if (fb == nullptr) return -1;
    return fb->setPtrAttr(attrId, val);
}

void CommandProcessor::process(const CommandStruct4& cmd) {
    process((const CommandStruct&)cmd);
}

void CommandProcessor::process(const CommandStruct& cmd) {
    FbWrapper* fb = factory.getFb(cmd.fbId);
    
    switch (cmd.attr) {
        case FB_ATTR_RUN:
                fb->runCycle();
                break;
        case FB_ATTR_AXRUN:{
                AxisAttrWrap* axis = factory.getAxisWrp(cmd.fbId);
                axis->runCycle();
                break;
            }
        case FB_ATTR_AXIS:
        case FB_ATTR_MASTER:
        case FB_ATTR_SLAVE:{
                AxisAttrWrap* axWr = factory.getAxisWrp((int) cmd.val);
                void* ptr = axWr->getAxis();
                fb->setPtrAttr((FbAttr)cmd.attr, ptr);
                break;
            }
        default:
            if ((cmd.action & 1) == 0) {// cmd.attr < FB_ATTR_LENGTH)
                fb->setAttr(cmd.attr, cmd.val);
            }else{
                AxisAttrWrap* axWr = factory.getAxisWrp((int) cmd.fbId);
                axWr->setAttr(cmd.attr + AX_ATTR_NOP, cmd.val);
            }
            break;
    }
}

unsigned int CommandProcessor::getFbNum() const{
    return factory.fbs.size();
}
unsigned int CommandProcessor::getAxisNum() const{
    return factory.axes.size();
}

void* create_fb(FB_TYPE fbTyp) {
    switch (fbTyp) {
        case FB_TYPE_POWER:
            return new FbtPower();
        case FB_TYPE_RESET:
            return new FbtReset();
        case FB_TYPE_SETPOSITION:
            return new FbtSetPosition();
        case FB_TYPE_READSTATUS:
            return new FbtReadStatus();
        case FB_TYPE_READACTUALPOSITION:
            return new FbtReadActualPosition();
        case FB_TYPE_READACTUALTORQUE:
            return new FbtReadActualTorque();
        case FB_TYPE_READACTUALVELOCITY:
            return new FbtReadActualVelocity();
        case FB_TYPE_READAXISERROR:
            return new FbtReadAxisError();
        case FB_TYPE_READSETPOSITION:
            return new FbtReadSetPosition();
        case FB_TYPE_READBOOLPARAMETER:
            return new FbtReadBoolParameter();
        case FB_TYPE_READPARAMETER:
            return new FbtReadParameter();
        case FB_TYPE_WRITEBOOLPARAMETER:
            return new FbtWriteBoolParameter();
        case FB_TYPE_WRITEPARAMETER:
            return new FbtWriteParameter();
        case FB_TYPE_WRITECOEPARAMETER:
            return new FbtWriteCoeParameter();
        case FB_TYPE_STOP:
            return new FbtStop();
        case FB_TYPE_HALT:
            return new FbtHalt();
        case FB_TYPE_MOVEABSOLUTE:
            return new FbtMoveAbsolute();
        case FB_TYPE_MOVEADDITIVE:
            return new FbtMoveAdditive();
        case FB_TYPE_MOVERELATIVE:
            return new FbtMoveRelative();
        case FB_TYPE_MOVEDISCRETE:
            return new FbtMoveDiscrete();
        case FB_TYPE_MOVEDISORVEL:
            return new FbtMoveDisOrVel();
        case FB_TYPE_MOVEDISORCONT:
            return new FbtMoveDisOrCont();
        case FB_TYPE_MOVEVELOCITY:
            return new FbtMoveVelocity();
        case FB_TYPE_MOVECONTINUOUSABSOLUTE:
            return new FbtMoveContinuousAbsolute();
        case FB_TYPE_MOVECONTINUOUSRELATIVE:
            return new FbtMoveContinuousRelative();
        case FB_TYPE_MOVESUPERIMPOSED:
            return new FbtMoveSuperImposed();
        case FB_TYPE_POSITIONPROFILE:
            return new FbtPositionProfile();
        case FB_TYPE_ACCELERATIONPROFILE:
            return new FbtAccelerationProfile();
        case FB_TYPE_VELOCITYPROFILE:
            return new FbtVelocityProfile();
        case FB_TYPE_HOME:
            return new FbtHome();
        case FB_TYPE_HOMING:
            return new FbtHoming();
        case FB_TYPE_JOG:
            return new FbtJog();
        case FB_TYPE_INCH:
            return new FbtInch();
        case FB_TYPE_GEARIN:
            return new FbtGearIn();
        case FB_TYPE_GEARINPOS:
            return new FbtGearInPos();
        case FB_TYPE_GEAROUT:
            return new FbtGearOut();
        case FB_TYPE_PHASING:
            return new FbtPhasing();
        case FB_TYPE_SETTORQUE:
            return new FbtSetTorque();
        case FB_TYPE_FOLLOWPOSITION:
            return new FbtFollowPosition();
        case FB_TYPE_FOLLOWPOSITIONVELOCITY:
            return new FbtFollowPositionVelocity();
        case FB_TYPE_FOLLOWVELOCITY:
            return new FbtFollowVelocity();
        case FB_TYPE_FOLLOWSETVALUES:
            return new FbtFollowSetValues();
        case FB_TYPE_TRACKAXIS:
            return new FbtTrackAxis();
        case FB_TYPE_TRACKSETVALUES:
            return new FbtTrackSetValues();
        case FB_TYPE_CAMIN:
            return new FbtCamIn();
        case FB_TYPE_CAMOUT:
            return new FbtCamOut();
        case FB_TYPE_CAMTABLESELECT:
            return new FbtCamTableSelect();
        case FB_TYPE_GETTAPPETVALUE:
            return new FbtGetTappetValue();
        case FB_TYPE_CAMREGISTER:
            return new FbtCamRegister();
        case FB_TYPE_CAMBOUNDS:
            return new FbtCAMBounds();
        case FB_TYPE_CAMBOUNDS_POS:
            return new FbtCAMBounds_Pos();
        case FB_TYPE_GETCAMSLAVESETPOSITION:
            return new FbtGetCamSlaveSetPosition();
        case FB_TYPE_ABORTTRIGGER:
            return new FbtAbortTrigger();
        case FB_TYPE_TOUCHPROBE:
            return new FbtTouchProbe();
        case FB_TYPE_DIGITALCAMSWITCH:
            return new FbtDigitalCamSwitch();
        case FB_TYPE_READCAM:
            return new FbtReadCAM();
        case FB_TYPE_WRITECAM:
            return new FbtWriteCAM();
        case FB_TYPE_READSETVALUES:
            return new FbtReadSetValues();
        case FB_TYPE_READACTUALVALUES:
            return new FbtReadActualValues();
        case FB_TYPE_READSETDRIVEVALUES:
            return new FbtReadSetDriveValues();
        case FB_TYPE_READACTUALDRIVEVALUES:
            return new FbtReadActualDriveValues();
        case FB_TYPE_GETTRACKINGERROR:
            return new FbtGetTrackingError();
        case FB_TYPE_GETTRAVELLEDDISTANCE:
            return new FbtGetTravelledDistance();
        case FB_TYPE_GETMAXSETACCDEC:
            return new FbtGetMaxSetAccDec();
        case FB_TYPE_GETMAXSETVELOCITY:
            return new FbtGetMaxSetVelocity();
        case FB_TYPE_GETOVERRIDE:
            return new FbtGetOverride();
        case FB_TYPE_SETOVERRIDE:
            return new FbtSetOverride();
        case FB_TYPE_CHECKLIMITS:
            return new FbtCheckLimits();
        case FB_TYPE_INPOSITION:
            return new FbtInPosition();
        default:
            return nullptr;
    }
}


FbWrapper* create_fb_wrap(FB_TYPE fbTyp) {
    switch (fbTyp) {
        case FB_TYPE_POWER:
            return new FbWrapperImpl<FbtPower>();
        case FB_TYPE_RESET:
            return new FbWrapperImpl<FbtReset>();
        case FB_TYPE_SETPOSITION:
            return new FbWrapperImpl<FbtSetPosition>();
        case FB_TYPE_READSTATUS:
            return new FbWrapperImpl<FbtReadStatus>();
        case FB_TYPE_READACTUALPOSITION:
            return new FbWrapperImpl<FbtReadActualPosition>();
        case FB_TYPE_READACTUALTORQUE:
            return new FbWrapperImpl<FbtReadActualTorque>();
        case FB_TYPE_READACTUALVELOCITY:
            return new FbWrapperImpl<FbtReadActualVelocity>();
        case FB_TYPE_READAXISERROR:
            return new FbWrapperImpl<FbtReadAxisError>();
        case FB_TYPE_READSETPOSITION:
            return new FbWrapperImpl<FbtReadSetPosition>();
        case FB_TYPE_READBOOLPARAMETER:
            return new FbWrapperImpl<FbtReadBoolParameter>();
        case FB_TYPE_READPARAMETER:
            return new FbWrapperImpl<FbtReadParameter>();
        case FB_TYPE_WRITEBOOLPARAMETER:
            return new FbWrapperImpl<FbtWriteBoolParameter>();
        case FB_TYPE_WRITEPARAMETER:
            return new FbWrapperImpl<FbtWriteParameter>();
        case FB_TYPE_WRITECOEPARAMETER:
            return new FbWrapperImpl<FbtWriteCoeParameter>();
        case FB_TYPE_STOP:
            return new FbWrapperImpl<FbtStop>();
        case FB_TYPE_HALT:
            return new FbWrapperImpl<FbtHalt>();
        case FB_TYPE_MOVEABSOLUTE:
            return new FbWrapperImpl<FbtMoveAbsolute>();
        case FB_TYPE_MOVEADDITIVE:
            return new FbWrapperImpl<FbtMoveAdditive>();
        case FB_TYPE_MOVERELATIVE:
            return new FbWrapperImpl<FbtMoveRelative>();
        case FB_TYPE_MOVEDISCRETE:
            return new FbWrapperImpl<FbtMoveDiscrete>();
        case FB_TYPE_MOVEDISORVEL:
            return new FbWrapperImpl<FbtMoveDisOrVel>();
        case FB_TYPE_MOVEDISORCONT:
            return new FbWrapperImpl<FbtMoveDisOrCont>();
        case FB_TYPE_MOVEVELOCITY:
            return new FbWrapperImpl<FbtMoveVelocity>();
        case FB_TYPE_MOVECONTINUOUSABSOLUTE:
            return new FbWrapperImpl<FbtMoveContinuousAbsolute>();
        case FB_TYPE_MOVECONTINUOUSRELATIVE:
            return new FbWrapperImpl<FbtMoveContinuousRelative>();
        case FB_TYPE_MOVESUPERIMPOSED:
            return new FbWrapperImpl<FbtMoveSuperImposed>();
        case FB_TYPE_POSITIONPROFILE:
            return new FbWrapperImpl<FbtPositionProfile>();
        case FB_TYPE_ACCELERATIONPROFILE:
            return new FbWrapperImpl<FbtAccelerationProfile>();
        case FB_TYPE_VELOCITYPROFILE:
            return new FbWrapperImpl<FbtVelocityProfile>();
        case FB_TYPE_HOME:
            return new FbWrapperImpl<FbtHome>();
        case FB_TYPE_HOMING:
            return new FbWrapperImpl<FbtHoming>();
        case FB_TYPE_JOG:
            return new FbWrapperImpl<FbtJog>();
        case FB_TYPE_INCH:
            return new FbWrapperImpl<FbtInch>();
        case FB_TYPE_GEARIN:
            return new FbWrapperImpl<FbtGearIn>();
        case FB_TYPE_GEARINPOS:
            return new FbWrapperImpl<FbtGearInPos>();
        case FB_TYPE_GEAROUT:
            return new FbWrapperImpl<FbtGearOut>();
        case FB_TYPE_PHASING:
            return new FbWrapperImpl<FbtPhasing>();
        case FB_TYPE_SETTORQUE:
            return new FbWrapperImpl<FbtSetTorque>();
        case FB_TYPE_FOLLOWPOSITION:
            return new FbWrapperImpl<FbtFollowPosition>();
        case FB_TYPE_FOLLOWPOSITIONVELOCITY:
            return new FbWrapperImpl<FbtFollowPositionVelocity>();
        case FB_TYPE_FOLLOWVELOCITY:
            return new FbWrapperImpl<FbtFollowVelocity>();
        case FB_TYPE_FOLLOWSETVALUES:
            return new FbWrapperImpl<FbtFollowSetValues>();
        case FB_TYPE_TRACKAXIS:
            return new FbWrapperImpl<FbtTrackAxis>();
        case FB_TYPE_TRACKSETVALUES:
            return new FbWrapperImpl<FbtTrackSetValues>();
        case FB_TYPE_CAMIN:
            return new FbWrapperImpl<FbtCamIn>();
        case FB_TYPE_CAMOUT:
            return new FbWrapperImpl<FbtCamOut>();
        case FB_TYPE_CAMTABLESELECT:
            return new FbWrapperImpl<FbtCamTableSelect>();
        case FB_TYPE_GETTAPPETVALUE:
            return new FbWrapperImpl<FbtGetTappetValue>();
        case FB_TYPE_CAMREGISTER:
            return new FbWrapperImpl<FbtCamRegister>();
        case FB_TYPE_CAMBOUNDS:
            return new FbWrapperImpl<FbtCAMBounds>();
        case FB_TYPE_CAMBOUNDS_POS:
            return new FbWrapperImpl<FbtCAMBounds_Pos>();
        case FB_TYPE_GETCAMSLAVESETPOSITION:
            return new FbWrapperImpl<FbtGetCamSlaveSetPosition>();
        case FB_TYPE_ABORTTRIGGER:
            return new FbWrapperImpl<FbtAbortTrigger>();
        case FB_TYPE_TOUCHPROBE:
            return new FbWrapperImpl<FbtTouchProbe>();
        case FB_TYPE_DIGITALCAMSWITCH:
            return new FbWrapperImpl<FbtDigitalCamSwitch>();
        case FB_TYPE_READCAM:
            return new FbWrapperImpl<FbtReadCAM>();
        case FB_TYPE_WRITECAM:
            return new FbWrapperImpl<FbtWriteCAM>();
        case FB_TYPE_READSETVALUES:
            return new FbWrapperImpl<FbtReadSetValues>();
        case FB_TYPE_READACTUALVALUES:
            return new FbWrapperImpl<FbtReadActualValues>();
        case FB_TYPE_READSETDRIVEVALUES:
            return new FbWrapperImpl<FbtReadSetDriveValues>();
        case FB_TYPE_READACTUALDRIVEVALUES:
            return new FbWrapperImpl<FbtReadActualDriveValues>();
        case FB_TYPE_GETTRACKINGERROR:
            return new FbWrapperImpl<FbtGetTrackingError>();
        case FB_TYPE_GETTRAVELLEDDISTANCE:
            return new FbWrapperImpl<FbtGetTravelledDistance>();
        case FB_TYPE_GETMAXSETACCDEC:
            return new FbWrapperImpl<FbtGetMaxSetAccDec>();
        case FB_TYPE_GETMAXSETVELOCITY:
            return new FbWrapperImpl<FbtGetMaxSetVelocity>();
        case FB_TYPE_GETOVERRIDE:
            return new FbWrapperImpl<FbtGetOverride>();
        case FB_TYPE_SETOVERRIDE:
            return new FbWrapperImpl<FbtSetOverride>();
        case FB_TYPE_CHECKLIMITS:
            return new FbWrapperImpl<FbtCheckLimits>();
        case FB_TYPE_INPOSITION:
            return new FbWrapperImpl<FbtInPosition>();
        default:
            return nullptr;
    }
}

}  // namespace zmc