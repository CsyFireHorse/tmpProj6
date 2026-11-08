#ifndef _ECAT_PCIE_SERVO_H_
#define _ECAT_PCIE_SERVO_H_

#include "mhelper/pdo_servo_addr.h"
#include <cstdint>
#include <cstdio>
#include <string>

#include "AxisFbGraph.hpp"
#include "pci_zecm.h"
#include "sdo_rt_pcie.hpp"


namespace zmc {

// PCIe EtherCAT master: PDO + SDO (no handler binding). Same-name interface as EcatServo.
// Subclass overrides run_cycle_task() to implement "master + motion" (e.g. AxisFbGraph).
class EcatPcieServo {
   public:
    EcatPcieServo();
    ~EcatPcieServo();

    // Same as EcatServo
    int load_eni_json(const char* eni);
    int config_servo_op_mode(uint32_t op_mode);
    int add_opmode_watch();
    int servo_size() const;
    int start();
    int stop();
    int start_pi();
    int stop_pi();
    int start_pi_sdo();
    int stop_pi_sdo();

    bool is_pi_start() const { return is_pi_start_ || is_pi_running_; }
    // PCIe-specific (no EcatBase)
    int open(uint32_t board_index, uint32_t channel);
    bool request_state(int stat, int retry = 30);

    // SDO: built-in facade, no manager exposure
    void sdo_pump_results() { sdoRt_.pump_results(); }
    void sdo_set_pump_divider(uint32_t n) { sdoRt_.set_pump_divider(n); }
    int sdo_get_u32(const SdoKey& key, uint32_t* out, uint32_t* seq = nullptr) const {
        return sdoRt_.get_u32(key, out, seq);
    }
    int sdo_read_once(const SdoKey& key, uint32_t max_len) {
        return sdoRt_.sdo_read_once(key, max_len);
    }
    int sdo_write(const SdoKey& key, uint32_t len, const void* data) {
        return sdoRt_.sdo_write(key, len, data);
    }
    int sdo_update_watch_write_value(const SdoKey& key, int32_t value, uint32_t value_size) {
        return sdoRt_.update_watch_write_value(key, value, value_size);
    }
    int sdo_add_watch_upload(const SdoKey& key, uint32_t period_ms, uint32_t max_len) {
        return sdoRt_.add_watch_upload(key, period_ms, max_len);
    }
    int sdo_add_watch_download(const SdoKey& key, uint32_t period_ms, uint32_t len,
                               const void* data) {
        return sdoRt_.add_watch_download(key, period_ms, len, data);
    }

    // One cycle: PI read -> pdo_get_input -> run_cycle_task -> PI write
    int cycle_step();

    // Public member, same name as EcatServo
    pdo_addr pdoAddr;

    EcatPcieServo(const EcatPcieServo&) = delete;
    EcatPcieServo& operator=(const EcatPcieServo&) = delete;
    // CRTP helper: Derived must be a subclass of EcatPcieServo.
    template <typename T>
    int cycle_step_tmpl();

   protected:
    // Override in subclass (e.g. EcatPcieServoMotion) to run PDO <-> AxisFbGraph <-> SDO.
    void run_cycle_task(const proc_img_i_addr* txpdo, proc_img_o_addr* rxpdo);

    ECAT_HANDLE ecat_handle_{nullptr};
    ECAT_ENI_INFO piInfo_{};
    PcieSdoRt sdoRt_;
    uint8_t* input_buff_{nullptr};
    uint8_t* output_buff_{nullptr};
    std::string eni_path_;
    bool is_pi_start_{false};    // PI start state
    bool is_pi_running_{false};  // PI running state
    int8_t init_opmode_{0};
    static constexpr size_t PI_BUFFER_SIZE = 1024;
};

// Master + motion: embeds AxisFbGraph, run_cycle_task = pump_results + fill inp + run_once + oup +
// update_watch.
class EcatPcieServoMotion : public EcatPcieServo {
   public:
    explicit EcatPcieServoMotion(const char* ini_path, const std::string& ini_content = "");
    explicit EcatPcieServoMotion(int servo_num = 1);

    int cycle_step();

    AxisFbGraph& fb_graph() { return fbGraph_; }
    const AxisFbGraph& fb_graph() const { return fbGraph_; }

    void run_cycle_task(const proc_img_i_addr* txpdo, proc_img_o_addr* rxpdo);

   private:
    AxisFbGraph fbGraph_;
};

}  // namespace zmc

#endif  // _ECAT_PCIE_SERVO_H_
