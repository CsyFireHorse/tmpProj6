#ifndef _ZMC_ECAT_FAKER_HPP_
#define _ZMC_ECAT_FAKER_HPP_

#include "../zecm_zh.h"
#include <thread>
#include <chrono>
#include "stdio.h"
#include "string.h"
#include <unordered_map>
#include <vector>
// #include "ServoSgFaker.hpp"
#include "ServoFakAlloc.hpp"

#include "EcatSlaveSimulatorClient.h"
#include "pdoPreparer.h"
#include "ServoFakFileState.h"


using std::this_thread::sleep_for;
using std::this_thread::sleep_until;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::steady_clock;

namespace zmc{
/*
 * TODO: add pdo generate
 * TODO: SDO 
 * TODO: remove allocatedStartIndex 
 *
 * */
class EcatFaker{
    public:
        bool exitFlag = false;
        EcatExtCtrlTaskHandler pFunc = nullptr;
        EcatExtCtrlTaskHandler pfPreparer = nullptr;
        ECAT_LPVOID pvCtrlTaskArg = nullptr;
        ECAT_LPVOID pvPreparerArg = nullptr;
        ECAT_LPVOID pServoFak = nullptr;
        volatile ECAT_BYTE bRequestState = EcatStateNotSet;
        volatile ECAT_BYTE bState = EcatStateNotSet;
        uint8_t request_cnt = 0;
        ECAT_SIMPLE_START_CONTEXT ctx;
        bool _will_trig_fault = false;
        int _trig_fault_stat = 0;
        int SlaveCount = 0;
        // Slave state storage is allocated once after the ENI slave count is known.
        std::vector<ECAT_BYTE> slave_states_;
        std::vector<ECAT_BYTE> slave_request_states_;
        std::vector<uint8_t> slave_request_counts_;
        int allocatedStartIndex = -1; // Starting index of allocated ServoSgFaker array
        std::unordered_map<int, uint32_t> reg;
        // PDO lengths (master <-> slave), bytes
        // rxpdoLen: PDO input length (slave receives, master outputs)
        // txpdoLen: PDO output length (slave sends, master inputs)
        uint32_t rxpdoLen = 64;
        uint32_t txpdoLen = 64;
        // Servo layout information from preparer
        uint32_t servoSize = 0;   // number of servo slaves (<=32)
        uint32_t servoMask = 0;   // bit mask, bit[i] == 1 means slave i is servo

        // Generic user context and optional free function hook
        ECAT_LPVOID pUserCtx = nullptr;
        typedef void (*UserCtxFreeFn)(ECAT_LPVOID);
        UserCtxFreeFn pUserCtxFreeFn = nullptr;
        // Cycle timing members
        steady_clock::time_point next_cycle_time_;
        microseconds cycle_period_;
        unsigned char* pPdoPi = new unsigned char[4096];
        unsigned char* pPdoPo = new unsigned char[4096];
        EcatSlaveSimulatorClient* pSlaveClient = nullptr;
        EcatFaker(){
            memset(pPdoPi, 0, 4096);
            memset(pPdoPo, 0, 4096);
            pSlaveClient = new EcatSlaveSimulatorClient();
            // Connection will be attempted in guess_use_preparer when ENI is available
        }
        ~EcatFaker(){
            // release user context if needed
            if (pUserCtx && pUserCtxFreeFn){
                pUserCtxFreeFn(pUserCtx);
            }
            pUserCtx = nullptr;
            pUserCtxFreeFn = nullptr;

            delete [] pPdoPi;
            delete [] pPdoPo;
            if (pSlaveClient) {
                delete pSlaveClient;
                pSlaveClient = nullptr;
            }
            unbind();
        }
        void setUserCtx(ECAT_LPVOID ctx, UserCtxFreeFn fn = nullptr){
            if (pUserCtx && pUserCtxFreeFn && pUserCtx != ctx){
                pUserCtxFreeFn(pUserCtx);
            }
            pUserCtx = ctx;
            pUserCtxFreeFn = fn;
        }
        void clearUserCtx(){
            if (pUserCtx && pUserCtxFreeFn){
                pUserCtxFreeFn(pUserCtx);
            }
            pUserCtx = nullptr;
            pUserCtxFreeFn = nullptr;
        }
        int setTaskHandler(EcatExtCtrlTaskHandler pfnHandler, ECAT_LPVOID pvCtrlArg){
            if (!pfnHandler || !pvCtrlArg) return -1;
            pFunc = pfnHandler;
            pvCtrlTaskArg = pvCtrlArg;
            return 0;
        }
        int setPdoPreparer(EcatExtCtrlTaskHandler pfnHandler, ECAT_LPVOID pArg){
            pfPreparer = pfnHandler;
            pvPreparerArg = pArg;
            return 0;
        }
        void _ctrlOnce(){
            if (pFunc)
                pFunc(pvCtrlTaskArg, pPdoPi, pPdoPo);
        }
        void _prepareOnce(){
            if (pfPreparer)
                pfPreparer(pvPreparerArg, pPdoPi, pPdoPo);
        }
        void _switchState(){
            if (bRequestState == EcatStateO){
                if (++request_cnt >= 2){
                    request_cnt = 0;
                    switch (bState)
                    {
                    case EcatStateI: bState = EcatStateP; break;
                    case EcatStateP: bState = EcatStateS; break;
                    case EcatStateS: bState = EcatStateO; break;
                    case EcatStateO: ; break;
                    default: bState = EcatStateI; break;
                    }
                }
            }else{
                bState = bRequestState;
                request_cnt = 0;
            }
            // Advance each slave with the same two-cycle cadence as the master.
            for (std::size_t i = 0; i < slave_states_.size(); ++i){
                _switchSlaveState(i);
            }
        }
        void _switchSlaveState(std::size_t index){
            ECAT_BYTE& state = slave_states_[index];
            const ECAT_BYTE request_state = slave_request_states_[index];
            uint8_t& request_count = slave_request_counts_[index];
            const ECAT_BYTE state_flags =
                static_cast<ECAT_BYTE>(state & ~EcatStateMask);

            if ((request_state & EcatStateMask) == EcatStateO){
                if (++request_count >= 2){
                    request_count = 0;
                    switch (state & EcatStateMask)
                    {
                    case EcatStateI:
                        state = static_cast<ECAT_BYTE>(state_flags | EcatStateP);
                        break;
                    case EcatStateP:
                        state = static_cast<ECAT_BYTE>(state_flags | EcatStateS);
                        break;
                    case EcatStateS:
                        state = static_cast<ECAT_BYTE>(state_flags | EcatStateO);
                        break;
                    case EcatStateO:
                        break;
                    default:
                        state = static_cast<ECAT_BYTE>(state_flags | EcatStateI);
                        break;
                    }
                }
            }else{
                state = static_cast<ECAT_BYTE>(state_flags | (request_state & EcatStateMask));
                request_count = 0;
            }
        }
        int requestMasterState(ECAT_BYTE byState){
            bRequestState = byState;
            for (std::size_t i = 0; i < slave_request_states_.size(); ++i){
                slave_request_states_[i] = byState;
                slave_request_counts_[i] = 0;
            }
            return ECAT_S_OK;
        }
        int getMasterState(ECAT_BYTE* pbyState) const{
            if (!pbyState){
                return ECAT_E_INVALIDARG;
            }
            *pbyState = bState;
            return ECAT_S_OK;
        }
        void initializeState(ECAT_BYTE byState){
            bRequestState = byState;
            bState = byState;
            request_cnt = 0;
            for (std::size_t i = 0; i < slave_states_.size(); ++i){
                slave_states_[i] = byState;
                slave_request_states_[i] = byState;
                slave_request_counts_[i] = 0;
            }
        }
        void initializeSlaveStates(int slave_count){
            SlaveCount = slave_count > 0 ? slave_count : 0;
            const std::size_t state_count = static_cast<std::size_t>(SlaveCount);
            const ECAT_BYTE initial_state = bState;
            const ECAT_BYTE initial_request_state = bRequestState;
            slave_states_.assign(state_count, initial_state);
            slave_request_states_.assign(state_count, initial_request_state);
            slave_request_counts_.assign(state_count, 0u);
        }
        void clearSlaveStates(){
            slave_states_.clear();
            slave_request_states_.clear();
            slave_request_counts_.clear();
            SlaveCount = 0;
        }
        bool isOperational() const{
            return (bState & EcatStateMask) == EcatStateO;
        }
        int requestSlaveState(ECAT_WORD wSlaveId, ECAT_BYTE byState){
            if (wSlaveId >= slave_states_.size()){
                return ECAT_E_INVALIDARG;
            }
            slave_request_states_[wSlaveId] = byState;
            slave_request_counts_[wSlaveId] = 0;
            return ECAT_S_OK;
        }
        int getSlaveState(ECAT_WORD wSlaveId, ECAT_BYTE* pbyState) const{
            if (!pbyState || wSlaveId >= slave_states_.size()){
                return ECAT_E_INVALIDARG;
            }
            *pbyState = slave_states_[wSlaveId];
            return ECAT_S_OK;
        }
        int getSlaveReqState(ECAT_WORD wSlaveId, ECAT_BYTE* pbyState) const{
            if (!pbyState || wSlaveId >= slave_request_states_.size()){
                return ECAT_E_INVALIDARG;
            }
            *pbyState = slave_request_states_[wSlaveId];
            return ECAT_S_OK;
        }
        int clearSlaveStateError(ECAT_WORD wSlaveId){
            if (wSlaveId >= slave_states_.size()){
                return ECAT_E_INVALIDARG;
            }
            slave_states_[wSlaveId] =
                static_cast<ECAT_BYTE>(slave_states_[wSlaveId] & ~EcatStateErrorFlag);
            return ECAT_S_OK;
        }
        int getSlaveStatesByPosIndex(ECAT_WORD wCount, const ECAT_WORD* pwSlavePosIndexArr,
                                     ECAT_BYTE* pbyState) const{
            if (wCount != 0u && (!pwSlavePosIndexArr || !pbyState)){
                return ECAT_E_INVALIDARG;
            }
            for (ECAT_WORD i = 0; i < wCount; ++i){
                int ret = getSlaveState(pwSlavePosIndexArr[i], &pbyState[i]);
                if (ret != ECAT_S_OK){
                    return ret;
                }
            }
            return ECAT_S_OK;
        }
        int getSlaveDLStatus(ECAT_WORD wSlaveId, ECAT_WORD* pwDLStatus) const{
            if (!pwDLStatus || wSlaveId >= SlaveCount){
                return ECAT_E_INVALIDARG;
            }
            // No topology or DL register model exists in the faker.
            *pwDLStatus = 0;
            return ECAT_S_OK;
        }
        void runOnce(){
            // Execute tasks
            _switchState();
            if (bState == EcatStateO);
            _ctrlOnce();
            _prepareOnce();
            
            // Wait until next cycle time
            if (steady_clock::now() < next_cycle_time_) {
                sleep_until(next_cycle_time_);
            }
            
            // Advance next cycle time
            next_cycle_time_ += cycle_period_;
        }
        int run(){
            
            // Initialize cycle timing
            cycle_period_ = microseconds(ctx.MainCycleTimeUs);
            next_cycle_time_ = steady_clock::now() + cycle_period_;
            
            while(!exitFlag){
                runOnce();
            }
            return 0;
        }
        int stop(){
            exitFlag = true;
            sleep_for(milliseconds(3));
            return ECAT_S_OK;
        }
        void bind(zmc::ServoSgFaker* pServo){
            // pServo->pEcat = this;
            pServoFak = pServo;
        }
        void unbind(){
            if (!pServoFak || allocatedStartIndex < 0){
                clearSlaveStates();
                return;
            }
            zmc::ServoSgFaker* pServo = (zmc::ServoSgFaker*) pServoFak;
            if (servo_fak_state_enabled() && servoSize > 0) {
                if (ctx.ENI) {
                    servo_fak_state_set_extra_string("eni", ctx.ENI);
                }
                servo_fak_state_save(pServo, (int)servoSize);
            }
            // pServo->pEcat = nullptr;
            pServoFak = nullptr;
            
            zmc::ServoSgFakerEager::deallocate_consecutive(allocatedStartIndex, SlaveCount);
            allocatedStartIndex = -1;
            clearSlaveStates();
        }
    
        void _trig_fault(int idx){
            // depreciated
            return ;
            static const int _trig_fault_sgn[] = {0x9099, 0x9099, 0x9088, 0x9088};
            switch (_trig_fault_stat)
            {
            case 0:
                if (idx == _trig_fault_sgn[0])
                    _trig_fault_stat = 1;
                else
                    _trig_fault_stat = 0;
                break;
            case 1:
                if (idx == _trig_fault_sgn[1])
                    _trig_fault_stat = 2;
                else
                    _trig_fault_stat = 0;
                break;
            case 2:
                if (idx == _trig_fault_sgn[2])
                    _trig_fault_stat = 3;
                else
                    _trig_fault_stat = 0;
                break;
            case 3:
                if (idx == _trig_fault_sgn[2]){
                    _trig_fault_stat = 0;
                    _will_trig_fault = true;
                    // dosth
                }else
                    _trig_fault_stat = 0;
                break;
            default:
                _trig_fault_stat = 0;
                break;
            }
        }
        int read_reg(int idx){
            int val = 0;
            auto it = reg.find(idx);
            if (it != reg.end()){
                val = it->second;
                // printf("read_reg %x %d %d\n", idx, is_match_op(idx-0x100), val);
            }
            return val;
        }
    
        void write_reg(int idx, int val){
            // printf("write_reg %x %d\n", idx, is_match_op(idx));
            if (is_match_op(idx))
                reg[idx+ 0x100] = val;
            reg[idx] = val;  
        }

        int read_reg_len(int idx, int len, void* pbyData){
            if (len == 1) {
                *((uint8_t*)pbyData) = (uint8_t)read_reg(idx);
            } else if (len == 2) {
                *((uint16_t*)pbyData) = (uint16_t)read_reg(idx);
            } else if (len == 4) {
                *((uint32_t*)pbyData) = (uint32_t)read_reg(idx);
            } else {
                // fallback: just return 1 byte as before
                *((uint8_t*)pbyData) = (uint8_t)read_reg(idx);
            }
            return 0;
        }

        int write_reg_len(int idx, int len, const void* pbyData){
            if (len == 1) {
                write_reg(idx, *((uint8_t*)pbyData));
            } else if (len == 2) {
                uint16_t val = *((uint16_t*)pbyData);
                write_reg(idx, val);
            } else if (len == 4) {
                uint32_t val = *((uint32_t*)pbyData);
                write_reg(idx, val);
            } else {
                // fallback: just write the first byte
                write_reg(idx, *((uint8_t*)pbyData));
            }
            return 0;
        }
        inline int index_to_mix(int wSlaveId, int wIndex, int bySubIndex)const{
            return (wSlaveId << 24) | (wIndex << 8) | bySubIndex;
        }
        inline bool is_match_op(int idx)const{
            idx >>= 8;
            idx &= 0xffff;
            if( idx == 0x6060 || idx == 0x6860 || idx == 0x7060 || idx == 0x7860)
                return true;
            return false;
        }
    };
    
}


using zmc::EcatFaker;
inline 
// PDO wrapper function for slave simulator client
int ecatSlavePreparePdoWrapper(void* obj, uint8_t* pi, uint8_t* po){
    auto* client = (EcatSlaveSimulatorClient*)obj;
    if (client && client->isConnected()) {
        // swap po & pi
        return client->preparePdo(po, pi);
    }
    return -1;
}



inline 
int guess_use_preparer(zmc::EcatFaker* hMaster){
    auto pEcat = hMaster;

    PdoPreparerConfig config = guess_preparer(pEcat->ctx.ENI);
    pEcat->initializeSlaveStates(config.slaveCount);
    pEcat->rxpdoLen   = config.rxpdoLen;
    pEcat->txpdoLen   = config.txpdoLen;
    pEcat->servoSize  = config.servoSize;
    pEcat->servoMask  = config.servoMask;

    if (config.ctx) {
        uint32_t cycleTime = eni_pdo_ctx_get_cycle_time(config.ctx);
        if (cycleTime > 0) {
            pEcat->ctx.MainCycleTimeUs = cycleTime;
        }
        pEcat->setUserCtx(config.ctx, freeEniPdoPrepareCtx);
    }

    // Disable external simulator connections while testing local startup timing.
#if 0
    // Try to connect to slave simulator service and configure it
    if (pEcat->pSlaveClient && !pEcat->pSlaveClient->isConnected()) {
        if (config.handler) {
            pEcat->pSlaveClient->set_config(ECAT_SLAVE_SIMULATOR_DEFAULT_PORT, pEcat->ctx.ENI, config.rxpdoLen, config.txpdoLen);
            if (pEcat->pSlaveClient->connect() == ECAT_RESULT_SUCCESS) {
                pEcat->setPdoPreparer(ecatSlavePreparePdoWrapper, pEcat->pSlaveClient);
                return 0;
            }
        }
    } else if (pEcat->pSlaveClient && pEcat->pSlaveClient->isConnected()) {
        pEcat->setPdoPreparer(ecatSlavePreparePdoWrapper, pEcat->pSlaveClient);
        return 0;
    }

#endif
    // Fallback to local ServoSgFaker simulation
    if (!pEcat->pfPreparer && config.handler && config.ctx) {
        int startIndex = zmc::ServoSgFakerEager::allocate_consecutive(pEcat->SlaveCount);
        if (startIndex < 0) {
            printf("Failed to allocate %d consecutive ServoSgFaker slots\n", pEcat->SlaveCount);
            return -1;
        }

        zmc::ServoSgFaker* svArr = &zmc::ServoSgFakerEager::Instance(startIndex);
        eni_pdo_ctx_set_sv_arr(config.ctx, svArr);
        pEcat->allocatedStartIndex = startIndex;
        pEcat->bind(svArr);
        pEcat->setPdoPreparer(config.handler, config.ctx);
        if (zmc::servo_fak_state_enabled() && pEcat->servoSize > 0) {
            double cycleTime = pEcat->ctx.MainCycleTimeUs * 1e-6;
            if (pEcat->ctx.ENI) {
                zmc::servo_fak_state_set_extra_string("eni", pEcat->ctx.ENI);
            }
            zmc::servo_fak_state_load(svArr, (int)pEcat->servoSize, cycleTime);
        }
    }
    return 0;
}

inline
int ecatFakeRun(ECAT_HANDLE hMaster){
    auto pEcat = (zmc::EcatFaker*)hMaster;
    guess_use_preparer(pEcat);
    return pEcat->run();
}
#endif
