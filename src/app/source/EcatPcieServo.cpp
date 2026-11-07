#include "EcatPcieServo.h"

#include <chrono>
#include <cstring>
#include <thread>

#include "AxisCiaConfig.hpp"
#include "common/ecat/ecat_pdo_key.h"
#include "mhelper/eni_servo_parser.h"
#include "sdo_backend_pcie.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace zmc {

static void sleep_ms(unsigned int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

EcatPcieServo::EcatPcieServo() { pdoAddr.pServo = new EniServoMapper(); }

EcatPcieServo::~EcatPcieServo() {
    stop();
    if (pdoAddr.pServo) {
        delete pdoAddr.pServo;
        pdoAddr.pServo = nullptr;
    }
    if (ecat_handle_) {
        EcatClose(ecat_handle_);
        ecat_handle_ = nullptr;
    }
}

int EcatPcieServo::open(uint32_t board_index, uint32_t channel) {
    if (ecat_handle_) return -1;
    char index_buff[16];
    int32_t ret = EcatOpen(&ecat_handle_, BOARD_ALIAS(index_buff, board_index), channel);
    return (ret == ECAT_S_OK) ? 0 : static_cast<int>(ret);
}

int EcatPcieServo::load_eni_json(const char* eni) {
    if (!ecat_handle_ || !pdoAddr.pServo) return -1;
    int err = load_pdo_offset_from_file(eni, *pdoAddr.pServo);
    if (err) return err;
    eni_path_ = eni;
    pdoAddr.bind_servo(*pdoAddr.pServo);
    return 0;
}

bool EcatPcieServo::request_state(int stat, int retry) {
    if (!ecat_handle_) return false;
    if (EcatRequestMasterState(ecat_handle_, static_cast<uint8_t>(stat)) != ECAT_S_OK)
        return false;
    for (int i = 0; i < retry; i++) {
        uint8_t state = 0;
        if (EcatGetMasterState(ecat_handle_, &state) != ECAT_S_OK) continue;
        if (state == static_cast<uint8_t>(stat)) return true;
        sleep_ms(500);
    }
    return false;
}

int EcatPcieServo::config_servo_op_mode(uint32_t op_mode) {
    if (!ecat_handle_ || !pdoAddr.pServo) return -1;
    int fail_try = 10;
    init_opmode_ = static_cast<int8_t>(op_mode & 0xFF);
    const int axLen = pdoAddr.pServo->servo_size();
    for (int i = 0; i < axLen;) {
        int idx = pdoAddr.get_servo_slave_idx(i);
        int ofs = pdoAddr.get_servo_offset(i);
        uint8_t mode_byte = static_cast<uint8_t>(op_mode & 0xFF);
        uint32_t len = 1;
        int32_t ret = PcieSdoBackend::Download(ecat_handle_, idx, PDO_PO_MODE + ofs, 0x00, len,
                                               &mode_byte, 1000);
        if (ret != ECAT_S_OK) {
            if (--fail_try <= 0) return -1;
            sleep_ms(100);
            continue;
        }
        sleep_ms(100);
        uint8_t mode_read = 0;
        len = 1;
        ret = PcieSdoBackend::Upload(ecat_handle_, idx, PDO_PI_MODE + ofs, 0x00, &len, &mode_read,
                                     1000);
        if (ret != ECAT_S_OK || mode_read != (op_mode & 0xFF)) {
            if (--fail_try <= 0) return -1;
            sleep_ms(100);
            continue;
        }
        fail_try = 10;
        i++;
    }
    return 0;
}

int EcatPcieServo::servo_size() const { return pdoAddr.pServo ? pdoAddr.pServo->servo_size() : 0; }

int EcatPcieServo::add_opmode_watch() {
    int servo_num = servo_size();
    for (int idx = 0; idx < servo_num; idx++) {
        if (pdoAddr.contain_key(idx, PDO_PI_MODE)) continue;
        int ofs = pdoAddr.get_servo_offset(idx);
        int slave = pdoAddr.get_servo_slave_idx(idx);
        SdoKey key6061;
        key6061.slave_id = static_cast<uint16_t>(slave);
        key6061.index = PDO_PI_MODE + ofs;
        key6061.subindex = 0x00;
        const int upload_result = sdoRt_.add_watch_upload(key6061, 50, 1);
        if (upload_result != 0) return upload_result;

        uint8_t opmode = static_cast<uint8_t>(init_opmode_);
        SdoKey key6060;
        key6060.slave_id = static_cast<uint16_t>(slave);
        key6060.index = PDO_PO_MODE + ofs;
        key6060.subindex = 0x00;
        const int download_result = sdoRt_.add_watch_download(key6060, 1000, 1, &opmode);
        if (download_result != 0) return download_result;
    }
    return 0;
}

int EcatPcieServo::start() {
    if (eni_path_.empty()) {
        return -1;
    }
    int32_t ret = EcatBusRun(ecat_handle_, eni_path_.c_str());
    if (ret != ECAT_S_OK) return static_cast<int>(ret);
    return ret;
}

int EcatPcieServo::stop() {
    stop_pi_sdo();
    return EcatBusStop(ecat_handle_);
}

int EcatPcieServo::start_pi() {
    if (!ecat_handle_ || !pdoAddr.pServo) return -1;
    if (EcatGetEniInfo(ecat_handle_, &piInfo_) != ECAT_S_OK) return -1;
    size_t inSz = piInfo_.u32PIInputByteSize > 0 ? piInfo_.u32PIInputByteSize : PI_BUFFER_SIZE;
    size_t outSz = piInfo_.u32PIOutputByteSize > 0 ? piInfo_.u32PIOutputByteSize : PI_BUFFER_SIZE;
    if (inSz > PI_BUFFER_SIZE) inSz = PI_BUFFER_SIZE;
    if (outSz > PI_BUFFER_SIZE) outSz = PI_BUFFER_SIZE;
    input_buff_ = static_cast<uint8_t*>(malloc(inSz));
    output_buff_ = static_cast<uint8_t*>(malloc(outSz));
    if (!input_buff_ || !output_buff_) {
        if (input_buff_) {
            free(input_buff_);
            input_buff_ = nullptr;
        }
        if (output_buff_) {
            free(output_buff_);
            output_buff_ = nullptr;
        }
        return -1;
    }
    memset(input_buff_, 0, inSz);
    memset(output_buff_, 0, outSz);
    pdo_get_output(pdoAddr.rxpdo_addr, *pdoAddr.pServo, output_buff_);
    if (EcatPIEnable(ecat_handle_) != ECAT_S_OK) {
        free(input_buff_);
        input_buff_ = nullptr;
        free(output_buff_);
        output_buff_ = nullptr;
        return -1;
    }

    is_pi_start_ = true;
    return 0;
}
int EcatPcieServo::start_pi_sdo() {
    int ret = start_pi();
    if (ret) return ret;
    ret = sdoRt_.bindEcat(ecat_handle_);
    if (ret != 0) return ret;
    sdoRt_.start();
    return 0;
}

int EcatPcieServo::stop_pi_sdo() {
    int ret = stop_pi();
    if (ret) return ret;
    sdoRt_.stop();
    return 0;
}

int EcatPcieServo::stop_pi() {
    is_pi_start_ = false;
    if (input_buff_) {
        free(input_buff_);
        input_buff_ = nullptr;
    }
    if (output_buff_) {
        free(output_buff_);
        output_buff_ = nullptr;
    }
    return 0;
}

void EcatPcieServo::run_cycle_task(const proc_img_i_addr* /*txpdo*/, proc_img_o_addr* /*rxpdo*/) {
    // Default: no-op; subclass overrides (e.g. EcatPcieServoMotion).
    is_pi_running_ = is_pi_start_;
}

int EcatPcieServo::cycle_step() { return cycle_step_tmpl<EcatPcieServo>(); }

template <typename T>
int EcatPcieServo::cycle_step_tmpl() {
    if (!ecat_handle_ || !input_buff_ || !output_buff_) return -1;
    uint32_t inLen = piInfo_.u32PIInputByteSize;
    if (inLen == 0 || inLen > PI_BUFFER_SIZE) inLen = static_cast<uint32_t>(PI_BUFFER_SIZE);
    EcatPIInputWaitNotify(ecat_handle_, 1);
    if (EcatPIRead(ecat_handle_, PI_AREA_SHADOW_INPUT, 0, inLen, input_buff_)) return -1;
    pdo_get_input(pdoAddr.txpdo_addr, *pdoAddr.pServo, input_buff_);
    static_cast<T*>(this)->run_cycle_task(&pdoAddr.txpdo_addr, &pdoAddr.rxpdo_addr);
    uint32_t outLen = piInfo_.u32PIOutputByteSize;
    if (outLen == 0 || outLen > PI_BUFFER_SIZE) outLen = static_cast<uint32_t>(PI_BUFFER_SIZE);
    if (EcatPIWrite(ecat_handle_, PI_AREA_FAST_OUTPUT, 0, outLen, output_buff_) != ECAT_S_OK)
        return -1;
    return 0;
}

// --- EcatPcieServoMotion ---

EcatPcieServoMotion::EcatPcieServoMotion(const char* ini_path, const std::string& ini_content)
    : EcatPcieServo() {
    fbGraph_.configure(ini_path ? std::string(ini_path) : "", ini_content, servo_size());
}

EcatPcieServoMotion::EcatPcieServoMotion(int servo_num) : EcatPcieServo(), fbGraph_() {}

void EcatPcieServoMotion::run_cycle_task(const proc_img_i_addr* txpdo, proc_img_o_addr* rxpdo) {
    sdoRt_.pump_results();
    AxisFbGraph& fbAxGrp = fbGraph_;
    const int axLen = txpdo->axLen;
    for (int i = 0; i < axLen; i++) {
        if (AxisCiaConfig_is_axis_deaf(&fbAxGrp.get_config_ref(i))) continue;
        AxisCiaInput& inp = fbAxGrp.inpArr[i];
        inp.status_word = *reinterpret_cast<const uint16_t*>(txpdo->servos[i].stat);
        inp.actual_position = *txpdo->servos[i].p;
        if (txpdo->servos[i].mode) {
            inp.mode_of_operation_display = *txpdo->servos[i].mode;
        } else {
            SdoKey key6061;
            key6061.slave_id = txpdo->servos[i].slave_idx;
            key6061.index = static_cast<uint16_t>(PDO_PI_MODE + pdoAddr.get_servo_offset(i));
            key6061.subindex = 0x00;
            uint32_t mode_u32 = 0;
            if (sdoRt_.get_u32(key6061, &mode_u32) == 0)
                inp.mode_of_operation_display = static_cast<int8_t>(mode_u32 & 0xFF);
        }
    }
    fbAxGrp.run_once();
    for (int i = 0; i < axLen; i++) {
        if (AxisCiaConfig_is_axis_dummy(&fbAxGrp.get_config_ref(i))) continue;
        AxisCiaOutput& oup = fbAxGrp.oupArr[i];
        *rxpdo->servos[i].p = oup.set_position;
        rxpdo->servos[i].ctrl->v = oup.control_word;
        if (rxpdo->servos[i].mode) {
            *rxpdo->servos[i].mode = oup.mode_of_operation;
        } else {
            SdoKey key6060;
            key6060.slave_id = txpdo->servos[i].slave_idx;
            key6060.index = static_cast<uint16_t>(PDO_PO_MODE + pdoAddr.get_servo_offset(i));
            key6060.subindex = 0x00;
            int32_t opmode_target = static_cast<int32_t>(oup.mode_of_operation);
            sdoRt_.update_watch_write_value(key6060, opmode_target, 1);
        }
    }
    is_pi_running_ = is_pi_start_;
}

int EcatPcieServoMotion::cycle_step() { return cycle_step_tmpl<EcatPcieServoMotion>(); }

// Explicit template instantiations for the two supported variants.
template int EcatPcieServo::cycle_step_tmpl<EcatPcieServo>();
template int EcatPcieServo::cycle_step_tmpl<EcatPcieServoMotion>();

}  // namespace zmc
