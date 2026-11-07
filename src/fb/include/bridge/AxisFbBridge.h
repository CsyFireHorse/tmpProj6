#ifndef _AXIS_FB_BRIDGE_H_
#define _AXIS_FB_BRIDGE_H_

#include <memory>
#include <string>

#include "ecat/ecat_state_adapter.h"
#include "mhelper/pdo_servo_addr.h"
#include "sdo/sdo_adapter.h"
#include "utils/zmc_pio_offset_layout.h"
#include "io/IoMetadata.hpp"
#include <vector>
namespace zmc {

class SdoRt;
class IoManager;
class MotionCmdQueueBase;

class AxisFbBridge {
   public:
    AxisFbBridge();
    ~AxisFbBridge();
    AxisFbBridge(const AxisFbBridge&) = delete;
    AxisFbBridge& operator=(const AxisFbBridge&) = delete;

    int reset();
    int configure(const char* ini_path, const char* ini_content, unsigned int axis_num);
    // Parse ENI independently and apply the resulting MC PDO layout.
    int load_eni(const char* eni_path);
    // int bind_sdo(SdoRt* sdo);
    int bind_sdo_adapter(const SdoCAdapter* adapter);
    // Bind EtherCAT state callbacks and enable per-cycle communication checks.
    int bind_ecat_state_adapter(const EcatStateAdapter* adapter);
    SdoRt* native_sdo();
    const SdoRt* native_sdo() const;
    IoManager* native_io();
    const IoManager* native_io() const;
    int register_opmode_watches(uint8_t init_opmode);
    int apply_pio_layout(const ZmcPioOffsetLayout* layout);
    int get_io_metadata(std::vector<IoMetadataEntry>& out) const;

    int auto_bind(int bind_begin, int servo_num);
    int manual_bind(int axis_count, const int* bind_axis_arr, int servo_num);
    void print_address() const;

    // Raw PI byte buffers. Caller must have called apply_pio_layout(); inp/oup are valid PI
    // buffers.
    int run_cycle(const void* inp, void* oup);
    // Parsed proc_img_* from pdo_get_input/output.
    int run_cycle_parsed(const proc_img_i_addr* inp, proc_img_o_addr* oup);

    int axis_count() const;
    int get_address(int axis_idx) const;
    int get_actual_position(int axis_idx) const;
    int servo_count() const;
    bool pdo_layout_ready() const;
    MotionCmdQueueBase& command_interface();
    const MotionCmdQueueBase& command_interface() const;

   private:
    int read_by_sdo(int servo_idx, int key_idx, int& val) const;
    int write_by_sdo(int servo_idx, int key_idx, int val, int sz);
    int read_opmode_by_sdo(int servo_idx, int* opmode) const;
    int write_opmode_by_sdo(int servo_idx, int opmode);

    struct Impl;
    std::unique_ptr<Impl> p_;
};

}  // namespace zmc

#endif
