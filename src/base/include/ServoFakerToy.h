/*
 * @Author: ChenSy
 * @Date: 2024-04-02 11:07:38 
 * @Last Modified by: ChenSy
 * @Last Modified time: 2024-08-21 11:52:57
 * @Last Modified time: 2024-08-21 11:52:57 add csv, csp
 */
#ifndef _ZMC_SERVO_FAKER_TOY_H_
#define _ZMC_SERVO_FAKER_TOY_H_

#include "stdint.h"
#include "common/ecat/ecat_pdo_key.h"
// #include "common/ecat/Cia402Easy.hpp"
// #include "common/zal_dbg.h"

namespace zmc{

struct ServoFakerToyCfg{
    double cycleTime;
    double posInit = 0;
    double velInit = 0;
    int smInit = 0;
    double rcfilter_scale = 0;
    double decel = 10000.;
};

struct ServoFakerToyStruct{
    void* pImpl = nullptr;
    // input
    uint16_t wCtrlword = 0;
    uint8_t byOpMode = OP_MODE_CSP;
    bool will_fault = false;
    int diSetPosition = 0;
    int diSetVelocity = 0;
    int diSetAcceleration = 0;
    int diSetTorque = 0;
    // output
    uint16_t wStatusword = 0;
    uint8_t byOpModeDisplay = OP_MODE_CSP;
    uint16_t errCode = 0;
    int diActPosition = 0;
    int diActVelocity = 0;
    int diActAcceleration = 0;
    int diActTorque = 0;
};

struct SdoDataItem{
    uint16_t index;
    uint8_t subindex;
    uint8_t len;
    int32_t val;
};

class ServoFakerToy:public ServoFakerToyStruct{
public:
    ServoFakerToy();
    ~ServoFakerToy();
    bool set_sdo_data(const SdoDataItem& item);
    bool get_sdo_data(uint16_t index, uint8_t subindex, SdoDataItem& out) const;
    void set_config(const ServoFakerToyCfg& cfg);
    void runCycle();
private:
    bool _stop();
    void _syncPosVelAcc();
};

}

#endif