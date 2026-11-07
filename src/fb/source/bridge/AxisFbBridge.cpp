/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#include "AxisFbBridge.h"

#include <array>

#include "AxisCiaConfig.hpp"
#include "AxisFbGraph.hpp"
#include "AxisFbBinding.hpp"
#include "AxisCoeRouter.hpp"
#include "ecat/EcatErrorManager.hpp"
#include "axis/fbt_axis_opaque.h"
#include "common/ecat/ecat_pdo_key.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "io/IoManager.hpp"
#include "mhelper/eni_servo_parser.h"
#include "mhelper/zmc_pio_layout_addr.h"
#include "sdo/sdo_rt.hpp"

namespace zmc {

namespace {

constexpr uint32_t kOpmodeUploadWatchPeriodMs = 50u;
constexpr uint32_t kOpmodeDownloadWatchPeriodMs = 1000u;
constexpr uint32_t kAxisErrorUploadWatchPeriodMs = 50u;

}  // namespace

struct AxisFbBridge::Impl {
    AxisFbGraph graph;
    pio_layout_addr pio_map;
    SdoRt sdo;
    // One bridge-owned axis-to-slave mapping shared by all bridge helpers.
    std::array<uint16_t, MC_MOTION_MAX_AXES> axis_slave_ids_{};
    EcatErrorManager ecat_error;
    AxisCoeRouter coe;

    Impl() : graph() {
        coe.bind(AxisFbBinding{&graph.get_axis_ref_arr(), &axis_slave_ids_}, &sdo);
    }

    void build_axis_slave_ids() {
        axis_slave_ids_.fill(0u);
        const uint32_t axis_count = graph.axis_num();
        for (uint32_t axis_idx = 0u; axis_idx < axis_count && axis_idx < axis_slave_ids_.size();
             ++axis_idx) {
            const int address = graph.getAddress(static_cast<int>(axis_idx));
            if (address <= 0 || address > pio_map.servo_size()) {
                continue;
            }
            const int slave_idx = pio_map.get_servo_slave_idx(address - 1);
            if (slave_idx < 0 || slave_idx >= MAX_NUM_SLAVES) {
                continue;
            }
            axis_slave_ids_[axis_idx] = static_cast<uint16_t>(slave_idx + 1);
        }

        const int eni_slave_num = pio_map.get_slave_num();
        ecat_error.bind(
            AxisFbBinding{&graph.get_axis_ref_arr(), &axis_slave_ids_,
                          static_cast<uint16_t>(eni_slave_num > 0 ? eni_slave_num : 0)});
    }
};

int AxisFbBridge::read_by_sdo(int servo_idx, int key_idx, int& val) const {
    const pio_layout_addr& pio_map = p_->pio_map;
    int ofs = pio_map.get_servo_offset(servo_idx);
    SdoKey key6061;
    key6061.slave_id = static_cast<uint16_t>(pio_map.get_servo_slave_idx(servo_idx));
    key6061.index = static_cast<uint16_t>(key_idx + ofs);
    key6061.subindex = 0x00;
    uint32_t val_u32 = 0;
    const int result = p_->sdo.get_u32(key6061, &val_u32);
    if (result != 0) return result;
    val = static_cast<int>(val_u32);
    return 0;
}

int AxisFbBridge::read_opmode_by_sdo(int servo_idx, int* opmode) const {
    int val = 0;
    auto ret = read_by_sdo(servo_idx, PDO_PI_MODE, val);
    if (ret == 0) {
        *opmode = static_cast<int>(val & 0xFF);
        return 0;
    }
    return ret;
}

int AxisFbBridge::write_by_sdo(int servo_idx, int key_idx, int val, int sz) {
    const pio_layout_addr& pio_map = p_->pio_map;
    int ofs = pio_map.get_servo_offset(servo_idx);
    SdoKey key6060;
    key6060.slave_id = static_cast<uint16_t>(pio_map.get_servo_slave_idx(servo_idx));
    key6060.index = static_cast<uint16_t>(key_idx + ofs);
    key6060.subindex = 0x00;
    return p_->sdo.update_watch_write_value(key6060, val, sz);
}

int AxisFbBridge::write_opmode_by_sdo(int servo_idx, int opmode) {
    return write_by_sdo(servo_idx, PDO_PO_MODE, opmode, 1);
}

int AxisFbBridge::register_opmode_watches(uint8_t init_opmode) {
    const pio_layout_addr& pio_map = p_->pio_map;
    if (!pdo_layout_ready()) {
        return ZMC_ERR_BRIDGE_PDO_UNCONFIGURED;
    }
    const int servo_num = static_cast<int>(pio_map.txpdo_addr.axLen);
    for (int idx = 0; idx < servo_num; ++idx) {
        const int ofs = pio_map.get_servo_offset(idx);
        const uint16_t slave_id = static_cast<uint16_t>(pio_map.get_servo_slave_idx(idx));

        if (!pio_map.contain_key(idx, PDO_PI_MODE)) {
            SdoKey key6061{};
            key6061.slave_id = slave_id;
            key6061.index = static_cast<uint16_t>(PDO_PI_MODE + ofs);
            key6061.subindex = 0x00;
            const int result = p_->sdo.add_watch_upload(key6061, kOpmodeUploadWatchPeriodMs, 1);
            if (result != 0) return result;
        }

        if (!pio_map.contain_key(idx, PDO_PO_MODE)) {
            SdoKey key6060{};
            key6060.slave_id = slave_id;
            key6060.index = static_cast<uint16_t>(PDO_PO_MODE + ofs);
            key6060.subindex = 0x00;
            const int result =
                p_->sdo.add_watch_download(key6060, kOpmodeDownloadWatchPeriodMs, 1, &init_opmode);
            if (result != 0) return result;
        }

        // Avoid a duplicate mailbox query when 0x603F is already mapped by PDO.
        if (!pio_map.contain_key(idx, PDO_PI_ERROR)) {
            SdoKey error_key{};
            error_key.slave_id = slave_id;
            error_key.index = static_cast<uint16_t>(PDO_PI_ERROR + ofs);
            error_key.subindex = 0x00;
            const int result =
                p_->sdo.add_watch_upload(error_key, kAxisErrorUploadWatchPeriodMs, 2);
            if (result != 0) return result;
        }
    }
    return 0;
}

AxisFbBridge::AxisFbBridge() : p_(new Impl()) {}

AxisFbBridge::~AxisFbBridge() {}

int AxisFbBridge::reset() {
    if (p_->sdo.sdo_started()) {
        p_->sdo.pause();
    }
    // Drop opmode watches before rebuilding the bridge and PDO state.
    p_->sdo.clear_watches();
    // Drop CoE one-shot routing and any queued or terminal axis requests.
    p_->coe.reset();
    p_->ecat_error.reset();
    p_->axis_slave_ids_.fill(0u);
    p_->graph.reset_runtime();
    p_->pio_map.reset();
    p_->graph.ctx().io().reset();
    return 0;
}

int AxisFbBridge::configure(const char* ini_path, const char* ini_content, unsigned int axis_num) {
    const std::string path = ini_path ? ini_path : "";
    const std::string content = ini_content ? ini_content : "";
    const int ret = p_->graph.configure(path, content, axis_num);
    p_->build_axis_slave_ids();
    // Rebuild the complete PDO and virtual registry only after the layout is ready.
    if (ret != 0 || !pdo_layout_ready()) {
        return ret;
    }
    const int bind_ret = p_->graph.applyIoBinding();
    return bind_ret;
}

int AxisFbBridge::load_eni(const char* eni_path) {
    if (!eni_path || !eni_path[0]) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    EniServoMapper mapper;
    int ret = load_pdo_offset_from_file(eni_path, mapper);
    if (ret != 0) {
        return ret;
    }
    ZmcPioOffsetLayout layout;
    ret = pio_layout_from_eni_servo_mapper(&mapper, &layout);
    if (ret != 0) {
        return ret;
    }
    return apply_pio_layout(&layout);
}

int AxisFbBridge::bind_sdo_adapter(const SdoCAdapter* adapter) {
    return p_->sdo.bind_adapter(adapter);
}

int AxisFbBridge::bind_ecat_state_adapter(const EcatStateAdapter* adapter) {
    return p_->ecat_error.bind_adapter(adapter);
}

SdoRt* AxisFbBridge::native_sdo() { return &p_->sdo; }

const SdoRt* AxisFbBridge::native_sdo() const { return &p_->sdo; }

IoManager* AxisFbBridge::native_io() { return &p_->graph.ctx().io(); }

const IoManager* AxisFbBridge::native_io() const { return &p_->graph.ctx().io(); }

int AxisFbBridge::apply_pio_layout(const ZmcPioOffsetLayout* layout) {
    if (!layout) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    // An IO-only ENI has PDO cells but no CiA402 servo slot.
    if (layout->descriptor_pod.slave_count == 0 && layout->descriptor_pod.pdo_cell_count == 0) {
        p_->pio_map.bind_layout(*layout);
        p_->graph.ctx().io().reset();
        p_->build_axis_slave_ids();
        return ZMC_ERR_BRIDGE_PDO_EMPTY_LAYOUT;
    }
    const int io_ret = p_->graph.ctx().io().apply_pio_layout(layout);
    if (io_ret != 0) {
        return io_ret;
    }
    p_->pio_map.bind_layout(*layout);
    const int bind_ret = p_->graph.applyIoBinding();
    p_->build_axis_slave_ids();
    return bind_ret;
}
int AxisFbBridge::get_io_metadata(std::vector<IoMetadataEntry>& out) const {
    out.clear();
    return native_io()->get_alias_metadata(out);
}

int AxisFbBridge::auto_bind(int bind_begin, int servo_num) {
    const int ret = p_->graph.bindAddressAutoBatch(bind_begin, 1, servo_num);
    p_->build_axis_slave_ids();
    return ret;
}

int AxisFbBridge::manual_bind(int axis_count, const int* bind_axis_arr, int servo_num) {
    const int ret = p_->graph.binAddressByAxisIdxArr(axis_count, bind_axis_arr, servo_num);
    p_->build_axis_slave_ids();
    return ret;
}

void AxisFbBridge::print_address() const { p_->graph.printAddress(); }

int AxisFbBridge::axis_count() const { return p_->graph.axis_num(); }

int AxisFbBridge::get_address(int axis_idx) const { return p_->graph.getAddress(axis_idx); }

int AxisFbBridge::get_actual_position(int axis_idx) const {
    if (axis_idx < 0 || axis_idx >= p_->graph.axis_num()) {
        return 0;
    }
    return p_->graph.inpArr[axis_idx].actual_position;
}

int AxisFbBridge::servo_count() const { return p_->pio_map.servo_size(); }

bool AxisFbBridge::pdo_layout_ready() const { return !p_->pio_map.is_empty(); }

MotionCmdQueueBase& AxisFbBridge::command_interface() { return p_->graph.fbGrp.getCmdInterface(); }

const MotionCmdQueueBase& AxisFbBridge::command_interface() const {
    return const_cast<AxisFbBridge*>(this)->command_interface();
}

int AxisFbBridge::run_cycle(const void* inp, void* oup) {
    if (!pdo_layout_ready()) {
        return ZMC_ERR_BRIDGE_PDO_UNCONFIGURED;
    }
    p_->pio_map.apply_input(inp);
    p_->pio_map.apply_output(oup);
    return run_cycle_parsed(&p_->pio_map.txpdo_addr, &p_->pio_map.rxpdo_addr);
}

int AxisFbBridge::run_cycle_parsed(const proc_img_i_addr* inp, proc_img_o_addr* oup) {
    if (!inp || !oup) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    const proc_img_i_addr* txpdo = inp;
    proc_img_o_addr* rxpdo = oup;
    AxisFbGraph& axGrp = p_->graph;

    // Route completed Axis CoE results before submitting the next FIFO heads.
    p_->sdo.pump_results();
    SdoResult oneshot_batch{};
    if (p_->sdo.take_oneshot_results(&oneshot_batch)) {
        p_->coe.dispatch(oneshot_batch);
    }
    p_->ecat_error.update();
    p_->ecat_error.apply();
    p_->graph.ctx().io().bind_process_image(txpdo->pBuf, rxpdo->pBuf);

    int axisNum = axGrp.axis_num();
    for (int j = 0; j < axisNum; j++) {
        int address = axGrp.getAddress(j);
        if (address <= 0) continue;
        int i = address - 1;
        if (i >= txpdo->axLen) continue;
        AxisCiaInput& axisInp = axGrp.inpArr[j];
        const servo_stat_addr& servo = txpdo->servos[i];
        axisInp.status_word = *(uint16_t*)servo.stat;
        axisInp.actual_position = *servo.p;
        // Velocity and torque PDO entries are optional in a mapped servo layout.
        if (servo.v) {
            axisInp.actual_velocity = *servo.v;
        }
        if (servo.t) {
            axisInp.actual_torque = *servo.t;
        }
        if (servo.mode) {
            axisInp.mode_of_operation_display = *servo.mode;
        } else {
            int opmode = 0;
            if (read_opmode_by_sdo(i, &opmode) == 0) {
                axisInp.mode_of_operation_display = opmode & 0xFF;
            }
        }

        // PDO is authoritative; use the last successful SDO value only as a fallback.
        AXIS_REF axis = axGrp.axisRef(j);
        if (servo.error_code) {
            axisSetDriveInterfaceError(axis, *servo.error_code);
        } else {
            int error_code = 0;
            if (read_by_sdo(i, PDO_PI_ERROR, error_code) == 0) {
                axisSetDriveInterfaceError(axis, static_cast<uint16_t>(error_code & 0xFFFFu));
            }
        }
    }
    // Age in-flight CoE submits, then submit queued CoE requests before the
    // FB graph runs so requests enqueued last cycle enter the SDO queue now.
    p_->coe.pump_cycle();
    axGrp.run_once();
    int sdo_write_result = 0;
    for (int j = 0; j < axisNum; j++) {
        int address = axGrp.getAddress(j);
        if (address <= 0) continue;
        int i = address - 1;
        if (i >= rxpdo->axLen) continue;
        if (AxisCiaConfig_is_axis_dummy(&axGrp.get_config_ref(j))) continue;
        AxisCiaOutput& oup_axis = axGrp.oupArr[j];
        servo_ctrl_addr& servo = rxpdo->servos[i];
        *servo.p = oup_axis.set_position;
        // Write optional velocity and torque PDO entries only when they are mapped.
        if (servo.v) {
            *servo.v = oup_axis.set_velocity;
        }
        if (servo.t) {
            *servo.t = oup_axis.set_torque;
        }
        servo.ctrl->v = oup_axis.control_word;
        if (servo.mode) {
            *servo.mode = oup_axis.mode_of_operation;
        } else {
            const int result = write_opmode_by_sdo(i, oup_axis.mode_of_operation);
            if (sdo_write_result == 0 && result != 0) sdo_write_result = result;
        }
    }
    p_->graph.ctx().io().restore_retained_outputs();
    return sdo_write_result;
}

}  // namespace zmc
