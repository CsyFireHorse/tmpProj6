#include "EcatWrap.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "mhelper/eni_servo_parser.h"
#include "common/ecat/ecat_pdo_key.h"
#include "chrono"
#include <thread>

using std::this_thread::sleep_for;
using std::chrono::milliseconds;

inline void sleep_for_ms(unsigned int ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

using namespace zmc;

EcatServo::EcatServo(const char* eth):EcatBase(eth){
    pdoAddr.pServo = new EniServoMapper();
}
EcatServo::EcatServo(const std::string& eth):EcatBase(eth){
    pdoAddr.pServo = new EniServoMapper();
}
EcatServo::~EcatServo(){
    delete pdoAddr.pServo;
    pdoAddr.pServo = nullptr;
}
int EcatServo::load_eni_json(const char* eni){
    auto& eniCfg = *pdoAddr.pServo;
    int ret = load_pdo_offset_from_file(eni, eniCfg);
    if (ret) return ret;
    pdoAddr.bind_servo(eniCfg);
    ctx.ENI = (char*) eni;
    ctx.MainCycleTimeUs = eniCfg.master_pdo.cycle_time;
    ctx.SubCycleTimeUs = eniCfg.master_pdo.mailbox_cycle_time;
    ctx.AutoRecoveryTimeoutMs = eniCfg.master_pdo.auto_recovery_timeout;
    ctx.RpcServerPort = 5000;
    return 0;
}

static int __EcatServo_handler(void* obj, uint8_t* inp, uint8_t* oup){
    EcatServo* pEcat = (EcatServo*) obj;

    // static bool pdo_first_flag = false;
    // if (!pdo_first_flag){
    //     pdo_first_flag = true;
    // }
    pdo_get_input(pEcat->pdoAddr.txpdo_addr, *pEcat->pdoAddr.pServo, inp);
    pdo_get_output(pEcat->pdoAddr.rxpdo_addr, *pEcat->pdoAddr.pServo, oup);
    int ret = 0;
    if (pEcat->_fPdoHandler)
        ret = pEcat->_fPdoHandler(pEcat->_pObj, (const void*) &pEcat->pdoAddr.txpdo_addr, (void*) &pEcat->pdoAddr.rxpdo_addr);
    return ret;
}

int EcatServo::_bindDefaultHandler(){
    return bind(__EcatServo_handler, this);
}


// Configure operation mode via SDO for all servos
int EcatServo::config_servo_op_mode(uint32_t op_mode){
    int fail_try = 10;
    int ret = 0;
    int axLen = pdoAddr.pServo->servo_size();
    for (int i = 0; i < axLen && fail_try > 0; ) {
        int32_t mode_read = 0;
        
        // Write operation mode to 0x6060:0
        int idx = pdoAddr.get_servo_slave_idx(i);
        int ofs = pdoAddr.get_servo_offset(i);
        ret = write_by_sdo(idx, PDO_PO_MODE + ofs, 0x0, 1, op_mode);
        if (ret != 0) {
            _ERR_("Failed to write op mode for servo %d, ret=%d", i, ret);
            sleep_for_ms(100);
            fail_try--;
            continue;
        }
        sleep_for_ms(100);
        
        // Read back mode of operation display from 0x6061:0
        ret = query_by_sdo(idx, PDO_PI_MODE + ofs, 0x0, 1, &mode_read);
        if (ret != 0 || mode_read != (int32_t)op_mode) {
            // Keep transport errors distinct from a successful mismatched read. GPT-6, 2026-10-21.
            if (ret == 0) ret = ZMC_ERR_ECAT_SERVO_OP_MODE_MISMATCH;
            _INF_("Mode mismatch for servo %d: requested=%d, current=%d", i, op_mode, mode_read);
            sleep_for_ms(100);
            fail_try--;
            continue;
        }
        
        _INF_("Servo %d op mode configured successfully: %d", i, op_mode);
        fail_try = 10; // reset retry counter for next servo
        i++;
    }
    
    if (fail_try <= 0) {
        _ERR_("Failed to configure op mode for all servos");
        return ret;
    }
    return 0;
}

int EcatServo::servo_size()const{
    return pdoAddr.pServo->servo_size();
}
