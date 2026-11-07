
#include <stdio.h>
#include "fbt_fb_base.hpp"

#include "fbt_axis_base.h"
#include "common/misc/TrapzVelToyForwarder.hpp"
#include "fbt_move_impl.hpp"
#include "SplineCppWrap.h"
#include "mc_cam_table.h"
#include "mc_cam_table_impl.h"
#include "spline_def.h"
#include "cam_tappet_serialize.h"
#include "cam_interactive.h"
#include "zmc_cam_table.h"
#include "timerMgr.hpp"
#include "vel/include/Vel4Wrap.h"


namespace zmc {

struct TimerTappet:public Timer{ 
    enum Action { delay, duration, duration_nodelay } action;
    void* context;
    
    void runCycle() {
        if (action == delay) {
            if (context) {
                *(bool*)context = true;
            }
        } else if (action == duration || action == duration_nodelay) {
            if (context) {
                *(bool*)context = false;
            }
        }
    }
};


class TimerTappetMgr:public TimerMgr<TimerTappet>{
public:
    uint32_t dwCycleTime = 1000; // us
    // Clear all timers for a specific group ID
    void tryClearGroup(int groupId) {
        for (auto it = timers.begin(); it != timers.end(); ++it) {
            if (it->id == groupId * 2 || it->id == groupId * 2 + 1) {
                it = timers.erase(it);
            }
        }
    }
    // Push tappet timer for TAPPETACTION_time
    void pushTappetTimer(const SMC_CAMTappet* pTaps, bool& bTappet) {
        if (!pTaps) return;
        
        int groupId = pTaps->iGroupID;
        
        // Convert delay and duration from milliseconds to ticks
        // Assuming 1 tick = 1 cycle, and we want to convert ms to cycles
        uint32_t delayTicks = pTaps->dwDelay / dwCycleTime;      // Delay in ticks
        uint32_t durationTicks = pTaps->dwDuration / dwCycleTime; // Duration in ticks
        
        // Ensure minimum 1 tick
        if (durationTicks == 0) durationTicks = 1;
        
        // Calculate target ticks for delay and duration
        uint32_t delayTarget = this->tick + delayTicks;
        uint32_t durationTarget = delayTarget + durationTicks;
        
        if (delayTicks > 0) {
            // Create delay timer (even ID)
            int delayId = groupId * 2;
            TimerTappet delayTimer;
            delayTimer.id = delayId;
            delayTimer.tick = delayTarget;
            delayTimer.action = TimerTappet::delay;
            delayTimer.context = (void*)&bTappet;   
            
            // Create duration timer (odd ID)
            int durationId = groupId * 2 + 1;
            TimerTappet durationTimer;
            durationTimer.id = durationId;
            durationTimer.tick = durationTarget;
            durationTimer.action = TimerTappet::duration;
            durationTimer.context = (void*)&bTappet;   
            
            // Push both timers to the manager
            this->push(delayTimer);
            this->push(durationTimer);
        } else {
            // No delay, only duration (duration_nodelay)
            int durationId = groupId * 2 + 1;
            TimerTappet durationTimer;
            durationTimer.id = durationId;
            durationTimer.tick = durationTarget;
            durationTimer.action = TimerTappet::duration_nodelay;
            durationTimer.context = (void*)&bTappet;   
            
            bTappet = true; // !!!
            // Push duration timer to the manager
            this->push(durationTimer);
        }
    }
};

class FbtCamTableSelectImpl:public FbtExeTmpl<FbtCamTableSelectImpl, FbtImplParamTmpl<FbtCamTableSelectStruct>>  {
public:
    // PoliFitterWrap camFitter;
    void onStarted();
    void onExecuting();
    void onCapture(){
        this->execute = this->pParam->Execute;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};

void FbtCamTableSelectImpl::onStarted() {
    auto& param = *this->pParam;
    if (this->fbStat != FB_STAT_IDLE) return;
    if (!param.Slave) {
        this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        return;
    }
    // if (!checkAxisStatBeforeActivate()) return;
    this->fbStat = FB_STAT_BUSY_ACTIVE;
}

void FbtCamTableSelectImpl::onExecuting(){
    if (!pParam || !pParam->CamTable) {
        onError(ZMC_NO_CAM_REF_TYPE);
        return;
    }

    MC_CAM_TABLE* pCam = pParam->CamTable;
    auto errCode = static_cast<CamTable*>(pCam)->validate();

    if (errCode){
        return onError((MC_ERROR_CODE) errCode);
    }
    if (pCam->byType == mcCamTypeXYVA && pCam->nElements > 0) {
        SMC_CAMXYVA* pXYVA = reinterpret_cast<SMC_CAMXYVA*>(pCam->pce);
        // camFitter.set_ptype(SPLINE_FIFTH_XYVA);
        // camFitter.set_xyva(pCam->nElements*4, (const double*) pXYVA);
        // camFitter.set_default_allocator();

        // if (camFitter.build() != 0) {
            // onError(ZMC_CAM_TABLE_INVALID_DATA);
        // }
        pParam->CamTableID.pCT = (uint8_t*) pCam;
        pParam->CamTableID.MasterAbsolute = pParam->MasterAbsolute;
        pParam->CamTableID.SlaveAbsolute = pParam->SlaveAbsolute;
        pParam->CamTableID.Periodic = pParam->Periodic;

        auto pce = reinterpret_cast<SMC_CAMXYVA*>(pCam->pce);
        pParam->CamTableID.StartMaster = pce[0].dX;
        pParam->CamTableID.EndMaster = pce[pCam->nElements-1].dX;
        pParam->CamTableID.StartSlave = pce[0].dY;
        pParam->CamTableID.EndSlave = pce[pCam->nElements-1].dY;
        fbStat = FB_STAT_DONE;
    }
    else {
        onError(ZMC_CAM_INVALID_ELEMENT_COUNT);
        // MC_CAM_UNSUPPORTED_TYPE;
    }
}

FbtCamTableSelect::FbtCamTableSelect() { 
    make_impl<FbtCamTableSelectImpl, FbtCamTableSelectStruct>(*this);
    Periodic = true;
    MasterAbsolute = true;
    SlaveAbsolute = true;
}

FbtCamTableSelect::~FbtCamTableSelect() {
    if (pImpl) {
        delete (FbtCamTableSelectImpl*) pImpl;
        pImpl = nullptr;
    }
}
void FbtCamTableSelect::runCycle() {
    if (!pImpl) return;
    auto impl = static_cast<FbtCamTableSelectImpl*>(pImpl);
    ((FbtCamTableSelectImpl*) pImpl)->runCycle();    
}



//  T=FbtCamIn or FbtGetCamSlaveSetPosition
template<typename T>
PosVelAcc calcCamSlavePva(PosVelAcc masterPva, const T& param, const CamRelStruct& camRel){
    double adjustedMasterPos;
    auto pFitter = reinterpret_cast<CamTable*>(param.CamTableID.pCT);
    
    adjustedMasterPos = (masterPva.pos - (camRel.masterAbsolute?0:camRel.masterEngagePos)) * param.MasterScaling + param.MasterOffset;


    PosVelAcc pvaCam, pvaOut;
    pFitter->getPosVelAcc(adjustedMasterPos, pvaCam);
    
    pvaOut.pos = pvaCam.pos * param.SlaveScaling + param.SlaveOffset + (camRel.slaveAbsolute? 0: camRel.slaveEngagePos);
    
    double A = param.MasterScaling;
    double C = param.SlaveScaling;

    pvaOut.vel = pvaCam.vel * masterPva.vel * A * C;
    pvaOut.acc = (A * A * C * pvaCam.acc * masterPva.vel * masterPva.vel) + 
                 (A * C * pvaCam.vel * masterPva.acc);
    
    return pvaOut;
}

class FbtCamInImpl:public FbExeAbortingTmpl<FbtCamInImpl, FbtCamInStruct>  {
public:
    CamTappetChecker camFitter;
    CamAffineTransform affine;
    bool hasReachEnd = false;
    int periodCnt_ = 0;
    double posZeroPoint = 0;
    double slavePosZeroPt = 0;
    double masterPosLast = 0;

    Vel4Wrap trapzVel;
    bool rampInActive = false;  // Ramp-in execution flag
    
    FbtCamInImpl(){
        fbType = FB_TYPE_CAMIN;
    }
    void onStarted() {
        auto& param = *this->pParam;
        if (fbStat != FB_STAT_IDLE) return;
        if (!param.Master || !param.Slave) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }

        pushToBufferAndActivate(mcSynchronizedMotion);
    }
    void onPlan(){
        auto& param = *this->pParam;
        auto pSlave = ((AxisImplBase*) param.Slave);
        auto pMaster = ((AxisImplBase*) param.Master);

        if (param.CamTableID.pCT == nullptr) {
            this->onErrorWhenBuild(ZMC_CI_NO_CAM_SELECTED);
            return;
        }
        #if 1
        auto pCam = (MC_CAM_TABLE*) param.CamTableID.pCT;
        auto errCode = static_cast<CamTable*>(pCam)->validate();
        if (errCode){
            return onErrorWhenBuild((MC_ERROR_CODE) errCode);
        }
        #endif
        // auto pFitter = reinterpret_cast<PoliFitterWrap*>(param.CamTableID.pCT);
        // pFitter->build();
        trapzVel.set_mode(pMaster->eRampType);
        // param.StartMode == mcRampIn:
        CamRelStruct camRel = CamRelStruct{
            param.CamTableID.MasterAbsolute,
            param.CamTableID.SlaveAbsolute && (param.StartMode!=mcRelative),
            pMaster->fSavePosition,
            pSlave->fSavePosition
        };
        affine.fromCamRel(camRel, param);
        this->hasVelPlan = true;
        this->hasEmitFirst = false;
        camFitter.pCamTable = reinterpret_cast<CamTable*>(param.CamTableID.pCT);

        if (pMaster->is_modulo_type()) {
            posZeroPoint = pMaster->calc_zero_point(pMaster->fSavePosition);
        }else{
            posZeroPoint = 0;
        }
        if (pSlave->is_modulo_type()) {
            slavePosZeroPt = pSlave->calc_zero_point(pSlave->fSavePosition);
        }else{
            slavePosZeroPt = 0;
        }
        
    }
    void onForward(){
        auto& param = *this->pParam;
        auto& ax = getAxImpl();
        auto pSlave = ((AxisImplBase*) param.Slave);
        auto pMaster = ((AxisImplBase*) param.Master);
        if (isError())
            return;
        if (this->isReached() && !param.CamTableID.Periodic){
            pSlave->cmd = AXIS_CMD_MOTION;
            pSlave->fSetVelocity = 0;
            pSlave->fSetAcceleration = 0;
            this->inVelocity = false;
            return;
        }
        double masterLinearPos = pMaster->fSetPosition - posZeroPoint;
        // double tableLookupPos = camFitter.calc_modulo(masterLinearPos);
        auto pvaMaster = PosVelAcc{masterLinearPos, pMaster->fSaveVelocity, pMaster->fSaveAcceleration};
        // auto pFitter = reinterpret_cast<CamTableFitter*>(param.CamTableID.pCT);
        // auto pvaOut = calcCamSlavePva(pvaMaster, param, camRel);
    
        PosVelAcc pvaOut;
        int ret = camFitter.get_pva_or_build_affine(pvaMaster, pvaOut, affine);

        // double difPos = pvaOut.pos - slavePosLast;
        // if (pvaOut.vel > 0){
        //     pSlave->fSetPosition = pSlave->fSavePosition + (difPos<0? (difPos + camFitter.getRange()):difPos);
        // }else{
        //     pSlave->fSetPosition = pSlave->fSavePosition + (difPos>0? (difPos - camFitter.getRange()):difPos);
        // }

        // Calculate cam target position
        double camTargetPos = pvaOut.pos + slavePosZeroPt;
        
        // Check and handle ramp-in logic (after pvaOut calculation)
        if (!rampInActive && (param.StartMode == mcRampIn || param.StartMode == mcRampInPos || param.StartMode == mcRampInNeg) && !this->hasEmitFirst) {
            // Calculate position difference S: slave start position - cam calculated slave start position
            double rampInOffset = pSlave->fSavePosition - camTargetPos;  // Position difference S
            
            // Check if there's significant position difference
            if (std::abs(rampInOffset) > 1e-6) {

                // Add modulo type check for slave axis
                if (pSlave->is_modulo_type()) {
                    if (param.StartMode == mcRampInPos && rampInOffset > 0 ) {
                        slavePosZeroPt -= pSlave->fPositionPeriod;
                        rampInOffset -= pSlave->fPositionPeriod;
                        camTargetPos -= pSlave->fPositionPeriod;
                    }
                    else if (param.StartMode == mcRampInNeg && rampInOffset < 0) {
                        slavePosZeroPt += pSlave->fPositionPeriod;
                        rampInOffset += pSlave->fPositionPeriod;
                        camTargetPos -= pSlave->fPositionPeriod;
                    }
                }
                
                // Check if compensation is still needed after filtering
                if (std::abs(rampInOffset) > 1e-6) {
                    rampInActive = true;
                    
                    // Calculate velocity difference for smooth transition
                    double slaveStartVel = pSlave->fSaveVelocity;
                    double camStartVel = pvaOut.vel;
                    double velDiff = slaveStartVel - camStartVel;  // Velocity difference
                    
                    if (param.VelocityDiff <= 0 || param.Acceleration <= 0 || param.Deceleration <= 0){
                        return onError(ZMC_INVALID_VELACCJERK_VALUES);
                    }
                    // Use TrapzVel to plan motion compensation from S to 0
                    trapzVel.set_config(param.VelocityDiff, param.Acceleration, param.Deceleration, param.Jerk);
                    trapzVel.set_begin_pva(rampInOffset, velDiff);  // Start from position S and velocity difference
                    trapzVel.set_end_pva(0, 0);                     // End at position 0 and velocity 0
                    int ret = trapzVel.build();  // Build motion plan
                    if (ret != 0) return onError(ZMC_ERR_VEL_BUILD_FAILED);
                    this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
                    this->hasVelPlan = true;
                }
            }
        }
        
        // Execute ramp-in motion compensation (if active)
        PosVelAcc pvaCompensation = {0, 0, 0};
        if (rampInActive) {
            double currentTime = pSlave->get_offset_time(this->startTick);
            if (currentTime <= trapzVel.get_end_time()) {
                // Still in ramp-in phase, get compensation position
                trapzVel.get_slice(currentTime, pvaCompensation);
            } else {
                // Ramp-in completed
                rampInActive = false;
            }
        }
        // Add to cam calculation result: slave.pos = camOut.pos + trapz.pos
        pSlave->fSetPosition = camTargetPos + pvaCompensation.pos;
        pSlave->fSetVelocity = pvaOut.vel + pvaCompensation.vel;
        pSlave->fSetAcceleration = pvaOut.acc + pvaCompensation.acc;
        this->inVelocity = !rampInActive;

        int tappetTrgNum = camFitter.checkProcessTappet(&param.Tappets);
        if (tappetTrgNum) param.Tappets.dwCycleTime = pSlave->cycleTime * 1e6;
        pSlave->cmd = AXIS_CMD_MOTION;

        double masterLinearAffinePos = masterLinearPos * affine.masterScale + affine.masterIntercept;
        int crossBound = camFitter.detect_cross_bound(masterLinearAffinePos, masterPosLast);
        // periodCnt_ += crossBound;
        // pSlave->fSetPosition += periodCnt_ * camFitter.getYRange();
        if (!param.CamTableID.Periodic) {
            if (crossBound && this->hasEmitFirst) {
                // fbStat = FB_STAT_DONE; 
                this->inReached_ = true;
            }else{
                // fbStat = FB_STAT_BUSY_ACTIVE;
                this->inReached_ = false;
            }
        } else if (param.CamTableID.Periodic){
            if (crossBound && this->hasEmitFirst) {
                this->inReached_ = true;
                
            }else{
                // fbStat = FB_STAT_BUSY_ACTIVE;
                this->inReached_ = false;
            }
        }
        // if (!this->hasEmitFirst) 
        this->hasEmitFirst = true;
        masterPosLast = masterLinearAffinePos;
    }
    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->bNodeStat_ = 0;
        this->startTick = 0;
        this->hasVelPlan = false;
        this->posZeroPoint = 0;
        this->slavePosZeroPt = 0;
        this->periodCnt_ = 0;

        // Reset ramp-in state
        rampInActive = false;

    }
    void onCapture(){
        this->execute = this->pParam->Execute;
    }
    void onUpdate(){
        // InSync
        this->pParam->InSync = this->isInXxx();
        this->pParam->EndOfProfile = this->isReached();
        this->pParam->Busy = this->isBusy();
        this->pParam->CommandAborted = this->isAborted();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};


FbtCamIn::FbtCamIn() {     
    make_impl<FbtCamInImpl, FbtCamInStruct>(*this);
    MasterScaling = 1.0;
    SlaveScaling = 1.0;
}

FbtCamIn::~FbtCamIn() {
    if (pImpl) {
        delete (FbtCamInImpl*) pImpl;
        pImpl = nullptr;
    }
}

void FbtCamIn::runCycle() {
    if (!pImpl) return;
    auto impl = static_cast<FbtCamInImpl*>(pImpl);
    ((FbtCamInImpl*) pImpl)->runCycle();
}


class FbtImplCamOut 
    : public FbExeAbortingTmpl<FbtImplCamOut, FbtCamOutStruct> {
public:
    FbtImplCamOut(){
        fbType = FB_TYPE_CAMOUT;
    }
    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        auto& slave = get_axis_impl_ref(this->pParam->Slave);

        if (slave.axisStat != mcSynchronizedMotion)
            return onError(ZMC_ERR_AXIS_NOT_IN_SYNC_MOTION);
        if (slave.pNode.isEmpty())
            return onError(ZMC_ERR_AXIS_NOT_IN_CAM);
        auto& fb = slave.pNode.currentNode();
        if (fb.fbType == FB_TYPE_GEARIN || fb.fbType == FB_TYPE_GEARINPOS)
            return onError(ZMC_ERR_AXIS_NOT_IN_CAM);
        
        // fbStat = FB_STAT_BUSY_ACTIVE;
        this->pushToBufferAndActivate(this->getAxisStateWhenActivated());
    }

    MC_AXIS_STATES getAxisStateWhenActivated()const{
        return mcSynchronizedMotion;
    }

    void onForward() {
        auto& param = *this->pParam;
        auto& slave = get_axis_impl_ref(this->pParam->Slave);

        slave.cmd = AXIS_CMD_MOTION;
        slave.fSetVelocity = slave.fSaveVelocity;
        slave.fSetPosition = slave.fSavePosition + slave.fSaveVelocity * slave.cycleTime;
        slave.fSetAcceleration = 0;
        slave.pNode.pop_front();
        if (slave.fSetVelocity) {
            slave.setAxisState(mcContinuousMotion, AXIS_STATE_DETAIL_GEAR_OUT);
        } else {
            slave.setAxisState(mcStandstill);
        }
        fbStat = FB_STAT_DONE;
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
    }
    void onCapture(){
        this->execute = this->pParam->Execute;
    }
    void onUpdate(){
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
    }
};


FbtCamOut::FbtCamOut() { make_impl<FbtImplCamOut, FbtCamOutStruct>(*this); }

FbtCamOut::~FbtCamOut() {
    if (pImpl) {
        delete static_cast<FbtImplCamOut*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtCamOut::runCycle() {
    if (!pImpl) return;
    auto impl = static_cast<FbtImplCamOut*>(pImpl);
    impl->runCycle();
}



class FbtReadCamImpl : public FbtExeTmpl<FbtReadCamImpl, FbtImplParamTmpl<FbtReadCAMStruct>> {
public:
    CamTappet camTap;
    MC_CAM_TABLE cam;
    FbtReadCamImpl(){
        ((zmc::CamTable&)cam).init();
    }
    void onStarted() {
        if (fbStat != FB_STAT_IDLE) return;
        // Add file loading logic here
        fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onExecuting() {
        int ret = camTap.parse(pParam->sFileName.value);
        if (ret) {
            return onError(ZMC_NO_CAM_REF_TYPE);
        }
        camTap.toCamRef(&cam);
        pParam->CAM = &cam;
        fbStat = FB_STAT_DONE;
    }
    void onCapture(){
        execute = pParam->bExecute;
    }
    void onUpdate(){
        pParam->bBusy = isBusy();
        pParam->bDone = isDone();
        pParam->bError = isError();
        pParam->ErrorID = getErrorID();
    }
};

// Implementation classes constructors/destructors
FbtReadCAM::FbtReadCAM() { 
    make_impl<FbtReadCamImpl, FbtReadCAMStruct>(*this);
    CAM = &static_cast<FbtReadCamImpl*>(pImpl)->cam;
}
FbtReadCAM::~FbtReadCAM() { 
    delete (FbtReadCamImpl*)pImpl; 
    pImpl = nullptr; 
}


// Run cycle methods
void FbtReadCAM::runCycle() {
    if (!pImpl) return;
    auto impl = static_cast<FbtReadCamImpl*>(pImpl);
    impl->runCycle();
}



class FbtWriteCamImpl : public FbtExeTmpl<FbtWriteCamImpl, FbtImplParamTmpl<FbtWriteCAMStruct>> {
public:
    CamTappet camTap;
    MC_CAM_TABLE cam;
    void onStarted() {
        if (fbStat != FB_STAT_IDLE) return;
        fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onExecuting() {
        int ret = camTap.fromCamRef(pParam->CAM);
        if (ret) {
            return onError(ZMC_NO_CAM_REF_TYPE);
        }
        ret = camTap.serialize(pParam->sFileName.value);
        if (ret) {
            return onError(ZMC_ERR_WRC_DUMP_FAILED);
        }
        fbStat = FB_STAT_DONE;
    }
    void onCapture(){
        execute = pParam->bExecute;
    }
    void onUpdate(){
        pParam->bBusy = isBusy();
        pParam->bDone = isDone();
        pParam->bError = isError();
        pParam->ErrorID = getErrorID();
    }
};

FbtWriteCAM::FbtWriteCAM() { make_impl<FbtWriteCamImpl, FbtWriteCAMStruct>(*this); }
FbtWriteCAM::~FbtWriteCAM() { delete (FbtWriteCamImpl*)pImpl; pImpl = nullptr; }

void FbtWriteCAM::runCycle() {
    if (!pImpl) return;
    auto impl = static_cast<FbtWriteCamImpl*>(pImpl);
    impl->runCycle();
}

class FbtGetCamSlaveSetPositionImpl : public FbtEnTmpl<FbtGetCamSlaveSetPositionImpl, FbtGetCamSlaveSetPosition> {
public:
    void onExecuting() {
        if (!Master || !Slave) {
            return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
        }
        if (CamTableID.pCT == nullptr) {
            return this->onError(ZMC_CI_NO_CAM_SELECTED);
        }
        // Calculate slave position/velocity/acceleration
        // based on current master position and cam profile
        auto pMaster = ((AxisImplBase*) Master);
        auto pSlave = ((AxisImplBase*) Slave);
        PosVelAcc pvaMaster = {pMaster->fSavePosition, pMaster->fSaveVelocity, pMaster->fSaveAcceleration};
        auto camRel = CamRelStruct{
            this->CamTableID.MasterAbsolute,
            this->CamTableID.SlaveAbsolute,
            pvaMaster.pos,
            pSlave->fSavePosition
        };
        auto pvaSlave = calcCamSlavePva(pvaMaster, *this, camRel);
        this->fStartPosition = pvaSlave.pos;
        this->fStartVelocity = pvaSlave.vel;
        this->fStartAcceleration = pvaSlave.acc;
        this->Busy = true;
        this->Error = false;
        this->ErrorID = ZMC_NO_ERROR;
    }
private:
    inline void onError(MC_ERROR_CODE err) {
        this->Busy = false;
        this->Error = true;
        this->ErrorID = err;
    }
};


FbtGetCamSlaveSetPosition::FbtGetCamSlaveSetPosition() { 
    all_zero_init<FbtGetCamSlaveSetPositionStruct>(*this);
}
FbtGetCamSlaveSetPosition::~FbtGetCamSlaveSetPosition() {}

void FbtGetCamSlaveSetPosition::runCycle() {
    auto impl = static_cast<FbtGetCamSlaveSetPositionImpl*>(this);
    impl->runCycle();
}


class FbtCamRegisterImpl : public FbtEnableTmpl<FbtCamRegisterImpl, FbtImplParamTmpl<FbtCamRegisterStruct>> {
public:
    double masterPosLast = 0;
    double tappetHysteresisValues[MAX_NUM_TAPPETS] = {0};


    void onEnable() {
        auto& param = *this->pParam;        
        if (!param.Master) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        
        if (!param.CamTable) {
            this->onError(ZMC_NO_CAM_REF_TYPE);
            return;
        }
        
        if (!param.bTappet) {
            // this->onError(SMC_TAPPET_NOT_DEFINED);
            return;
        }

        fbStat = FB_STAT_BUSY_ACTIVE;
        
        // adjustedMasterPos
        auto pMaster = ((AxisImplBase*)param.Master);
        double masterPos = pMaster->fSetPosition;
        double masterVel = pMaster->fSetVelocity;
        
        double compensatedMasterPos = masterPos * param.MasterScaling + param.MasterOffset;
        if (param.DeadTimeCompensation != 0) {
            compensatedMasterPos -=  masterVel * param.DeadTimeCompensation;
        }

        double endPos = 0;

        auto pXYVA = reinterpret_cast<SMC_CAMXYVA*>(param.CamTable->pce);
        endPos = pXYVA[param.CamTable->nElements - 1].dX;
        this->inReached_  = (compensatedMasterPos >= endPos);
        // FIXME
        if (param.CamTable->nTappets > 0) {
            auto pTappets = reinterpret_cast<Tappet*>(param.CamTable->pt);
            TAPPET_BITS_STRUCT* tappetBits = param.bTappet;
            
            for (int i = 0; i < param.CamTable->nTappets && i < MAX_NUM_TAPPETS; i++) {
                auto& tappet = pTappets[i];
                

                double tappetX = tappet.x;
                double distance = std::abs(compensatedMasterPos - tappetX);

                bool shouldTrigger = false;
                
                if (distance <= param.TappetHysteresis) {
                    continue;
                }
                

                tappetHysteresisValues[i] = distance;
                

                if (compensatedMasterPos >= tappetX) {
  
                    switch (tappet.ctt) {
                        case TAPPET_pos: 
                            if (masterVel >= 0) {
                                shouldTrigger = true;
                            }
                            break;
                        case TAPPET_neg: 
                            if (masterVel < 0) {
                                shouldTrigger = true;
                            }
                            break;
                        case TAPPET_all:
                            shouldTrigger = true;
                            break;
                    }
                    
                    if (shouldTrigger) {
                        switch (tappet.cta) {
                            case TAPPETACTION_on:
                                tappetBits->arr[i] = 1;
                                break;
                            case TAPPETACTION_off:
                                tappetBits->arr[i] = 0;
                                break;
                            case TAPPETACTION_inv:
                                tappetBits->arr[i] = !tappetBits->arr[i];
                                break;
                            case TAPPETACTION_time:
                                tappetBits->arr[i] = 1;
                                break;
                        }
                    }
                }
            }
        }
        
        // masterPosLast = adjustedMasterPos;
    }
    void onClear() {
        FbtBase::onClear();
    }
    void onCapture() {
        this->execute = this->pParam->Enable;
    }
    
    void onUpdate() {
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = this->getErrorID();
        this->pParam->EndOfProfile = this->isReached();
    }
};

FbtCamRegister::FbtCamRegister() { make_impl<FbtCamRegisterImpl, FbtCamRegisterStruct>(*this); }
FbtCamRegister::~FbtCamRegister() { delete (FbtCamRegisterImpl*)pImpl; pImpl = nullptr; }
void FbtCamRegister::runCycle(){
    if (!pImpl) return;
    auto impl = static_cast<FbtCamRegisterImpl*>(pImpl);
    impl->runCycle();
}


#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
class FbtCAMBoundsImpl : public FbtExeTmpl<FbtCAMBoundsImpl, FbtImplParamTmpl<FbtCAMBoundsStruct>> {
public:
    void onStarted() {
        auto& param = *this->pParam;
        if (this->fbStat != FB_STAT_IDLE) return;
        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }
    void onExecuting() {
        auto& param = *this->pParam;
        if (!param.CAM) {
            return onError(ZMC_NO_CAM_REF_TYPE);
        }
        auto pce = (CamXyvaMethod*) param.CAM->pce;
        MaxMinPosVelAcc mxmnPva = {1e9, -1e9, 1e9, -1e9, 1e9, -1e9};
        for (int i=0; i<param.CAM->nElements-1; i++) {
            MaxMinPosVelAcc out;
            pce[i].maxPosVelAcc(pce[i+1], out);
            mxmnPva.maxPos = MAX(mxmnPva.maxPos, out.maxPos);
            mxmnPva.minPos = MIN(mxmnPva.minPos, out.minPos);
            mxmnPva.maxVel = MAX(mxmnPva.maxVel, out.maxVel);
            mxmnPva.minVel = MIN(mxmnPva.minVel, out.minVel);
            mxmnPva.maxAcc = MAX(mxmnPva.maxAcc, out.maxAcc);
            mxmnPva.minAcc = MIN(mxmnPva.minAcc, out.minAcc);
        }     
        param.dMinPos =  mxmnPva.minPos * param.dSlaveScaling;
        param.dMaxPos = mxmnPva.maxPos * param.dSlaveScaling;

        double sc = param.dMasterScaling * param.dSlaveScaling;;
        param.dMaxVel = mxmnPva.maxVel * param.dMasterVelMax * sc;
        param.dMinVel = mxmnPva.minVel * param.dMasterVelMax * sc;
        param.dMaxAccDec = (param.dMasterScaling * mxmnPva.maxAcc * param.dMasterVelMax + mxmnPva.maxVel * param.dMasterAccMax )* sc;
        param.dMinAccDec = (param.dMasterScaling * mxmnPva.minAcc * param.dMasterVelMax + mxmnPva.minVel * param.dMasterAccMax )* sc;

        // if(param.MinPosition >= param.MaxPosition) {
        //     return onError(SMC_CAM_INVALID_BOUNDS);
        // }
        fbStat = FB_STAT_DONE;
    }
    void onCapture(){
        execute = pParam->bExecute;
    }
    void onUpdate(){
        pParam->bBusy = isBusy();
        pParam->bDone = isDone();
        pParam->bError = isError();
        pParam->nErrorID = getErrorID();
    }
};

FbtCAMBounds::FbtCAMBounds() { 
    make_impl<FbtCAMBoundsImpl, FbtCAMBoundsStruct>(*this); 
    dMasterScaling = 1.0;
    dSlaveScaling = 1.0;
    dMasterVelMax = 1.0;
}
FbtCAMBounds::~FbtCAMBounds() { 
    delete (FbtCAMBoundsImpl*)pImpl; 
    pImpl = nullptr; 
}
void FbtCAMBounds::runCycle(){
    if (!pImpl) return;
    auto impl = static_cast<FbtCAMBoundsImpl*>(pImpl);
    impl->runCycle();
}



class FbtCAMBounds_PosImpl : public FbtExeTmpl<FbtCAMBounds_PosImpl, FbtImplParamTmpl<FbtCAMBounds_PosStruct>> {
public:
    void onStarted() {
        auto& param = *this->pParam;
        if (this->fbStat != FB_STAT_IDLE) return;
        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }
    void onExecuting() {
        auto& param = *this->pParam;
        if (!param.CAM || !param.CAM->nElements || param.CAM->nElements < 2) {
            return onError(ZMC_CAM_INVALID_ELEMENT_COUNT);
        }
        auto pce = (CamXyvaMethod*) param.CAM->pce;
        MaxPosVelAcc mxmnPva = {1e9, -1e9};
        for (int i=0; i<param.CAM->nElements-1; i++) {
            MaxPosVelAcc out;
            pce[i].maxPosVelAcc(pce[i+1], out);
            mxmnPva.maxPos = MAX(mxmnPva.maxPos, out.maxPos);
            mxmnPva.minPos = MIN(mxmnPva.minPos, out.minPos);
        }     
        param.dMinPos =  mxmnPva.minPos * param.dSlaveScaling;
        param.dMaxPos =  mxmnPva.maxPos * param.dSlaveScaling;
        fbStat = FB_STAT_DONE;
    }
    void onCapture(){
        execute = pParam->bExecute;
    }
    void onUpdate(){
        pParam->bBusy = isBusy();
        pParam->bDone = isDone();
        pParam->bError = isError();
        pParam->nErrorID = getErrorID();
    }
};

FbtCAMBounds_Pos::FbtCAMBounds_Pos() { 
    make_impl<FbtCAMBounds_PosImpl, FbtCAMBounds_PosStruct>(*this);
    dMasterScaling = 1.0;
    dSlaveScaling = 1.0;
    dMasterVelMax = 1.0;
}
FbtCAMBounds_Pos::~FbtCAMBounds_Pos() { 
    delete (FbtCAMBounds_PosImpl*)pImpl; 
    pImpl = nullptr; 
}
void FbtCAMBounds_Pos::runCycle(){
    if (!pImpl) return;
    auto impl = static_cast<FbtCAMBounds_PosImpl*>(pImpl);
    impl->runCycle();
}


class FbtImplGetTappetValue:public FbtImplParamTmpl<FbtGetTappetValueStruct>{
public:
    bool isNotFirst = false;
    // Timer manager for TAPPETACTION_time
    TimerTappetMgr timerManager;
    void runCycle();

};

FbtGetTappetValue::FbtGetTappetValue(){
    make_impl<FbtImplGetTappetValue, FbtGetTappetValueStruct>(*this);
}

#define MAX_TRIGGERED_IDX 3
void FbtGetTappetValue::runCycle(){
    if (!pImpl) return;
    auto impl = static_cast<FbtImplGetTappetValue*>(pImpl);
    impl->runCycle();
}

void FbtImplGetTappetValue::runCycle(){
    // Increment cycle counter for timing
    timerManager.runCycle();
    
    bool& bTappet = pParam->bTappet;
    if (!isNotFirst){
        bTappet = pParam->bInitValue;
        isNotFirst = true;
        timerManager.clear();
        ;
    }
    auto& Tappets = pParam->Tappets;
    if (Tappets == nullptr) return;
    if (pParam->Tappets->bRestart && pParam->bSetInitValueAtReset){
        bTappet = pParam->bInitValue;
    }
    for (int i=0; i<MAX_TRIGGERED_IDX; i++){
        auto pTaps = Tappets->pTaps[i];
        if (!pTaps) continue;
        if (pTaps->iGroupID != pParam->iID) continue;
        
        int groupId = pTaps->iGroupID;
        bool bTappetBak = bTappet;
        
        switch (pTaps->cta)
        {
        case TAPPETACTION_on:
            bTappet = true;
            timerManager.tryClearGroup(groupId);
            break;
        case TAPPETACTION_off:
            bTappet = false;

            timerManager.tryClearGroup(groupId);
            break;
        case TAPPETACTION_inv:
            bTappet = !bTappet;
            timerManager.tryClearGroup(groupId);
            break;
        case TAPPETACTION_time:
            timerManager.dwCycleTime = pParam->Tappets->dwCycleTime;
            // Process timer-based tappet action
            timerManager.tryClearGroup(groupId);
            timerManager.pushTappetTimer(pTaps, bTappet);
            // Timer will automatically update bTappet when completed
            break;
        default:
            break;
        }
        // printf("pTaps->cta: %d, pTaps->ctt: %d, bTappetBak:%d bTappet: %d\n", pTaps->cta, pTaps->ctt, bTappetBak, bTappet);
        // impl.setInitValueAndApply(bInitValue);
        // }
        // if (impl.value == 0xffffffff){ // onInit
        //     impl.bNotHasInit = false;
        //     impl.bRevTappet = !bInitValue;
        // }else if(!impl.bRevRestartCamIn){ // onRestartCamIn
        //     impl.bRevRestartCamIn = true;
        //     impl.bRevTappet = !bInitValue;
        // }
        // bTappet = !impl.bRevTappet;
    }

}

FbtGetTappetValue::~FbtGetTappetValue(){}


// FbtGetTappetValue::FbtGetTappetValue() { 
//     make_impl<FbtGetTappetValueImpl, FbtGetTappetValueStruct>(*this);
// }
// FbtGetTappetValue::~FbtGetTappetValue() { 
//     delete (FbtGetTappetValueImpl*)pImpl; 
//     pImpl = nullptr; 
// }
// void FbtGetTappetValue::runCycle(){
//     if (!pImpl) return;
//     auto impl = static_cast<FbtGetTappetValueImpl*>(pImpl);
//     impl->runCycle();
//     Error = impl->isError();
//     ErrorID = impl->error_code;
// }
FbtDigitalCamSwitch::FbtDigitalCamSwitch(){}
FbtDigitalCamSwitch::~FbtDigitalCamSwitch(){}
void FbtDigitalCamSwitch::runCycle(){}

class FbtCamCalcImpl : public FbtEnableTmpl<FbtCamCalcImpl, FbtImplParamTmpl<FbtCamCalcStruct>> {
public:
    CamTappetChecker camFitter;
    CamAffineTransform affine;
    bool lastEnable = false;
    
    void onEnable() {
        auto& param = *this->pParam;
        if (param.Enable && !this->lastEnable) {
            if (!param.CamTable) {
                return this->onError(ZMC_CI_NO_CAM_SELECTED);
            }
            
            camFitter.pCamTable = static_cast<CamTable*>(param.CamTable);
            CamRelStruct camRel = {
                param.MasterAbsolute, param.SlaveAbsolute, param.MasterEngagePos, param.SlaveEngagePos
            };
            CamAffineParam affineParam = {
                param.MasterScaling, param.SlaveScaling, param.MasterOffset, param.SlaveOffset
            };
            affine.fromCamRel(camRel, affineParam);
        }
        this->lastEnable = param.Enable;
        
        PosVelAcc masterPva = {param.MasterPosition, param.MasterVelocity, param.MasterAcceleration};
        PosVelAcc slavePva;
        
        int result = camFitter.get_pva_or_build_affine(masterPva, slavePva, affine);
        
        if (1) {
            param.SlavePosition = slavePva.pos;
            param.SlaveVelocity = slavePva.vel;
            param.SlaveAcceleration = slavePva.acc;
            param.Busy = true;
            param.Error = false;
            param.ErrorID = ZMC_NO_ERROR;
        } else {
            this->onError((MC_ERROR_CODE) result);
        }
    }
    
    void onClear() {
        auto& param = *this->pParam;
        param.Busy = false;
        param.Error = false;
        param.ErrorID = ZMC_NO_ERROR;
        this->lastEnable = false;
    }
    
    void onCapture() {
        this->execute = this->pParam->Enable;
    }
    
    void onUpdate() {
        auto& param = *this->pParam;

    }
    
private:
    inline void onError(MC_ERROR_CODE err) {
        auto& param = *this->pParam;
        param.Busy = false;
        param.Error = true;
        param.ErrorID = err;
    }
};

FbtCamCalc::FbtCamCalc() { 
    make_impl<FbtCamCalcImpl, FbtCamCalcStruct>(*this);
    MasterScaling = 1.0;
    SlaveScaling = 1.0;
}

FbtCamCalc::~FbtCamCalc() {
    delete static_cast<FbtCamCalcImpl*>(pImpl);
}

void FbtCamCalc::runCycle() {
    auto impl = static_cast<FbtCamCalcImpl*>(pImpl);
    impl->runCycle();
}

}  // namespace zmc
