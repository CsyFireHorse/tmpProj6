#include "ServoFakerToy.h"
#include "common/misc/un_int64.hpp"
#include "common/ecat/Cia402Easy.hpp"
#include "math.h"
#include "vel/include/vel_stop_online.hpp"
#include <vector>

namespace zmc{


inline double RcFilterCalc(double valNew, double valLast, double scale) {
    return valLast + scale * (valNew - valLast);
}


double calc_next_homing_pos(double pos, double pos_bak, double decMax,
                                          double ratio) {
    if (pos == 0) {
        return pos;
    } else if (pos - pos_bak == 0) {
        if (pos > 0) {
            if (decMax > pos / 4) decMax = pos / 4;
            return pos - decMax;
        } else {
            if (decMax > -pos / 4) decMax = -pos / 4;
            return pos + decMax;
        }
    } else {
        if (pos > 0 && pos > pos_bak)
            return pos - (pos - pos_bak) * ratio;
        else if (pos > 0 && pos < pos_bak)
            return pos + (pos - pos_bak);
        else if (pos < 0 && pos > pos_bak)
            return pos + pos - pos_bak;
        else if (pos < 0 && pos < pos_bak)
            return pos - (pos - pos_bak) * ratio;
        return pos;
    }
}


struct ServoFakerToyImpl {
    double fCurPosition = 0;
    double fCurVelocity = 0;
    double fCurAcceleration = 0;
    un_I64 i64CurPos = {0};
    double cycleTime = 0.02;
    double rcScaleRes = 0;
    std::vector<SdoDataItem> sdoTable = {
        {PDO_PO_HOMING_SPEED, 0x01, 4, 500},
        {PDO_PO_HOMING_SPEED, 0x02, 4, 50},
        {PDO_PO_QUICK_STOP_DECEL, 0x00, 4, 5000},
        {PDO_PO_HOMING_ACCEL, 0x00, 4, 500},
    };

    SdoDataItem* findSdoItem(uint16_t index, uint8_t sub) {
        for (auto& item : sdoTable) {
            if (item.index == index && item.subindex == sub) {
                return &item;
            }
        }
        return nullptr;
    }

    const SdoDataItem* findSdoItem(uint16_t index, uint8_t sub) const {
        for (const auto& item : sdoTable) {
            if (item.index == index && item.subindex == sub) {
                return &item;
            }
        }
        return nullptr;
    }

    void setSdoValue(uint16_t index, uint8_t sub, int32_t value) {
        if (auto* item = findSdoItem(index, sub)) {
            item->val = value;
        }
    }

    double getSdoValueAsDouble(uint16_t index, uint8_t sub) const {
        if (const auto* item = findSdoItem(index, sub)) {
            return static_cast<double>(item->val);
        }
        return 0.0;
    }

    double decelStopValue() const {
        return getSdoValueAsDouble(PDO_PO_QUICK_STOP_DECEL, 0x00);
    }

void set_config(const ServoFakerToyCfg& cfg){
    cycleTime = cfg.cycleTime;
    setSdoValue(PDO_PO_QUICK_STOP_DECEL, 0, static_cast<int32_t>(round(cfg.decel)));
    rcScaleRes = cfg.rcfilter_scale;
    fCurPosition = cfg.posInit;
    fCurVelocity = 0;
    fCurAcceleration = 0;
}
    inline double prevPosition()const{
        return fCurPosition - fCurVelocity * cycleTime;
    }
    void updateSetPosition(int pos){
        i64CurPos.update(pos);
        double fPosNew = RcFilterCalc(pos, fCurPosition, 1 - rcScaleRes);
        double fVelNew = (fPosNew - fCurPosition) / cycleTime;;
        fCurAcceleration = (fVelNew - fCurVelocity) / cycleTime;
        fCurVelocity = fVelNew;
        fCurPosition = fPosNew;
        i64CurPos.val = round(fPosNew);
    }
    void updateSetVelocity(int vel){
        double fPosCalc = fCurPosition + (fCurVelocity + vel) * cycleTime / 2;
        double fPosNew = RcFilterCalc(fPosCalc, fCurPosition, 1 - rcScaleRes);
        fCurAcceleration = (vel - fCurVelocity) / cycleTime;
        fCurPosition = fPosNew;
        fCurVelocity = vel;
        i64CurPos.val = round(fPosNew);
    }
    void updateDirectPosition(double fPosNew){
        // double fPosNew = RcFilterCalc(pos, fCurPosition, 1 - rcScaleRes);
        double fVelNew = (fPosNew - fCurPosition) / cycleTime;;
        fCurAcceleration = (fVelNew - fCurVelocity) / cycleTime;
        fCurVelocity = fVelNew;
        fCurPosition = fPosNew;
        i64CurPos.val = round(fPosNew);
    }

    void stop(){
        double decel = decelStopValue();
        auto pva = calc_next_decel_pva(PosVelAcc{fCurPosition, fCurVelocity, fCurAcceleration}, decel, cycleTime);
        fCurPosition = pva.pos;
        fCurVelocity = pva.vel;
        fCurAcceleration = pva.acc;
    }
};

ServoFakerToy::ServoFakerToy() {
    pImpl = new ServoFakerToyImpl();
}

ServoFakerToy::~ServoFakerToy() {
    delete (ServoFakerToyImpl*)pImpl;
}

void ServoFakerToy::set_config(const ServoFakerToyCfg& cfg){
    auto impl = static_cast<ServoFakerToyImpl*>(pImpl);
    impl->set_config(cfg);
    diActPosition = cfg.posInit;
    diActVelocity = 0;
    diActAcceleration = 0;
    wStatusword = Cia402Easy::get_stat((Cia402Easy::axis_sm_t)(cfg.smInit & 7), 0);
}

bool ServoFakerToy::_stop(){
    auto impl = static_cast<ServoFakerToyImpl*>(pImpl);
    impl->stop();
    _syncPosVelAcc();
    return impl->fCurVelocity == 0;
}

void ServoFakerToy::_syncPosVelAcc(){
    auto impl = static_cast<ServoFakerToyImpl*>(pImpl);
    diActPosition = impl->i64CurPos.iLow;
    diActVelocity = round(impl->fCurVelocity);
    diActAcceleration = round(impl->fCurAcceleration);
}

const int pdo_key_support [] = {
    PDO_PO_HOMING_ACCEL <<8,
    (PDO_PO_HOMING_SPEED <<8) +1,
    (PDO_PO_HOMING_SPEED <<8) +2,
    PDO_PO_QUICK_STOP_DECEL <<8,
    PDO_PO_HOMING_METHOD <<8,
};
#define LEN(arr) (sizeof(arr) / sizeof(arr[0]))
// calculate new state
// check new state 
// calculate new pos
bool ServoFakerToy::set_sdo_data(const SdoDataItem& item){
    int mkey = (item.index<<8) + item.subindex;

    bool found = false;
    for (int i = 0; i < LEN(pdo_key_support); i++) {
        if (pdo_key_support[i] ==  mkey) {
            found = true;
            break;
        }
    }
    if (!found) {
        return false;
    }
    auto impl = static_cast<ServoFakerToyImpl*>(pImpl);
    for (auto& exist : impl->sdoTable) {
        if (exist.index == item.index && exist.subindex == item.subindex) {
            exist = item;
            return true;
        }
    }
    impl->sdoTable.push_back(item);
    return true;
}

bool ServoFakerToy::get_sdo_data(uint16_t index, uint8_t subindex, SdoDataItem& out) const {
    auto impl = static_cast<const ServoFakerToyImpl*>(pImpl);
    int mkey = (index<<8) + subindex;
    bool found = false;
    for (int i = 0; i < LEN(pdo_key_support); i++) {
        if (pdo_key_support[i] ==  mkey) {
            found = true;
            break;
        }
    }
    if (!found) {
        return false;
    }
    for (const auto& exist : impl->sdoTable) {
        if (exist.index == index && exist.subindex == subindex) {
            out = exist;
            return true;
        }
    }
    return false;
}


void ServoFakerToy::runCycle(){
    auto impl = static_cast<ServoFakerToyImpl*>(pImpl);
    uint16_t wStatuswordNew = wStatusword;
    if (will_fault){
        wStatuswordNew = Cia402Easy::get_stat((Cia402Easy::axis_sm_t) 7, 0);
    }else{
        // wStatuswordNew = Cia402Easy::get_new_state(wCtrlword, wStatusword);
        Cia402Easy::axis_flags flg = {impl->fCurVelocity != 0, impl->i64CurPos.low != diSetPosition,
                                    byOpModeDisplay == OP_MODE_HOME, 0};
        wStatuswordNew = Cia402Easy::get_new_stateEx(wCtrlword, wStatusword, flg);
    }
    auto sm = Cia402Easy::get_sm(wStatusword);
    auto smNew = Cia402Easy::get_sm(wStatuswordNew);
    if (sm != smNew){
        if (smNew == Cia402Easy::axis_sm_operation_enabled && diSetPosition != diActPosition){
            // abort transition
            return;
        }else if (smNew == Cia402Easy::axis_sm_quick_stop_active && sm != Cia402Easy::axis_sm_quick_stop_active){ // Transition 11
            _stop();
        }else if(smNew == Cia402Easy::axis_sm_switch_on_disabled && sm != Cia402Easy::axis_sm_quick_stop_active){ // Transition 12
            if (diActVelocity != 0){
                if (!_stop()){
                    return; // abort transition
                }
            }
        }else if(smNew == Cia402Easy::axis_sm_fault_reaction_active && sm != Cia402Easy::axis_sm_fault_reaction_active){ // // Transition 13
            _stop();
        }else if(smNew == Cia402Easy::axis_sm_fault && sm == Cia402Easy::axis_sm_fault_reaction_active){  // Transition 14
            if (diActVelocity != 0){
                if (!_stop()){
                    return; // abort transition
                }
            }
        }
    }
    wStatusword = wStatuswordNew;
    byOpModeDisplay = byOpMode;
    if (smNew == Cia402Easy::axis_sm_operation_enabled){
        if (byOpModeDisplay == OP_MODE_CSP){
            impl->updateSetPosition(diSetPosition);
            _syncPosVelAcc();
        }else if (byOpModeDisplay == OP_MODE_CSV){
            impl->updateSetVelocity(diSetVelocity);
            _syncPosVelAcc();
        }else if (byOpModeDisplay == OP_MODE_CST){
            diActTorque = diSetTorque;
        }else if (byOpModeDisplay == OP_MODE_HOME){
            if (Cia402Easy::is_ctrlw_home(wCtrlword)) {
                double pos_new =
                    calc_next_homing_pos(impl->fCurPosition, impl->prevPosition(), impl->getSdoValueAsDouble(PDO_PO_HOMING_ACCEL, 0x00) / 10, 0.37);
                impl->updateDirectPosition(pos_new);
                _syncPosVelAcc();
            }
        }else{
            if (diActVelocity != 0){
                _stop();
                wStatusword = Cia402Easy::get_stat((Cia402Easy::axis_sm_t) 7, 0);
            }
        }
    }
}
}
