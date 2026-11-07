/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-18
@ArtifactStatusAuthority GPT-5.6 Sol
*/
#include "mc_core/mc_motion.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>

#include "AxisFbBridgeWithThread.h"
#include "EcatMotion.h"
#include "FbWrapCmdV2.h"
#include "mc_zecm.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "sdo_key.hpp"

namespace {

std::unique_ptr<zmc::AxisFbBridgeWithThread> g_bridge;

int32_t mc_sdo_upload(void* /*ctx*/, const SdoCKey* key, uint32_t* len, void* data,
                      uint32_t timeout_ms) {
    if (!key) {
        return -1;
    }
    return MC_EcatCoeSDOUploadEx(key->slave_id, key->index, key->subindex, 0, len, data,
                                 timeout_ms);
}

int32_t mc_sdo_download(void* /*ctx*/, const SdoCKey* key, uint32_t len, const void* data,
                        uint32_t timeout_ms) {
    if (!key) {
        return -1;
    }
    return MC_EcatCoeSDODownloadEx(key->slave_id, key->index, key->subindex, 0, len, data,
                                   timeout_ms);
}

int32_t mc_get_slave_state(void* /*ctx*/, uint16_t slave_id, uint8_t* slave_state) {
    return MC_EcatGetSlaveState(slave_id, slave_state);
}

int32_t mc_get_slave_count(void* /*ctx*/, uint16_t* slave_count) {
    return MC_EcatGetSlaveCount(slave_count);
}

int32_t mc_get_master_state(void* /*ctx*/, uint8_t* master_state) {
    return MC_EcatGetMasterState(master_state);
}

struct RunDispatchContext {
    int port;
    const std::string* startupScript;
    bool scriptExit;
};

void runRpcServer(const RunDispatchContext* rpc) {
    (void)rpc;
    std::printf("RPC mode not implemented yet\n");
}

zmc::AxisFbBridge* bridge_or_null() { return g_bridge ? &g_bridge->bridge() : nullptr; }

}  // namespace

extern "C" {

int MC_motionInit(const char* ini_path, unsigned int axis_num) {
    if (g_bridge) {
        return 0;
    }
    g_bridge.reset(new zmc::AxisFbBridgeWithThread());
    if (g_bridge->bridge().configure(ini_path, nullptr, axis_num)) {
        g_bridge.reset();
        return -1;
    }
    return 0;
}

void MC_motionShutdown(void) {
    MC_motionStopSdo();
    g_bridge.reset();
}

int MC_motionLoadEni(const char* eni_path) {
    zmc::AxisFbBridge* bridge = bridge_or_null();
    if (!eni_path || !bridge) {
        return -1;
    }
    return bridge->load_eni(eni_path);
}

int MC_motionAutoBind(int bind_begin) {
    zmc::AxisFbBridge* bridge = bridge_or_null();
    if (!bridge || !bridge->pdo_layout_ready()) {
        return -1;
    }
    const int servo_num = bridge->servo_count();
    if (servo_num <= 0) {
        return -1;
    }
    return bridge->auto_bind(bind_begin, servo_num);
}

int MC_motionManualBind(int count, const int* axis_arr) {
    zmc::AxisFbBridge* bridge = bridge_or_null();
    if (!bridge || !bridge->pdo_layout_ready() || !axis_arr || count <= 0) {
        return -1;
    }
    const int servo_num = bridge->servo_count();
    return bridge->manual_bind(count, axis_arr, servo_num);
}

int MC_motionStartSdo(void) {
    if (!g_bridge) {
        return -1;
    }
    zmc::AxisFbBridge& bridge = g_bridge->bridge();
    EcatStateAdapter ecat_adapter = {};
    ecat_adapter.ctx = nullptr;
    ecat_adapter.get_slave_count = mc_get_slave_count;
    ecat_adapter.get_slave_state = mc_get_slave_state;
    ecat_adapter.get_master_state = mc_get_master_state;
    if (bridge.bind_ecat_state_adapter(&ecat_adapter)) {
        return -1;
    }

    SdoCAdapter adapter = {};
    adapter.ctx = nullptr;
    adapter.upload = mc_sdo_upload;
    adapter.download = mc_sdo_download;
    adapter.get_slave_state = mc_get_slave_state;
    if (bridge.bind_sdo_adapter(&adapter)) {
        return -1;
    }
    return g_bridge->start_sdo();
}

void MC_motionStopSdo(void) {
    if (g_bridge) {
        g_bridge->stop_sdo();
    }
}

int MC_motionAddOpmodeWatch(int init_opmode) {
    zmc::AxisFbBridge* bridge = bridge_or_null();
    if (!bridge || !g_bridge->sdo_started() || init_opmode == 0) {
        return 0;
    }
    return bridge->register_opmode_watches(static_cast<uint8_t>(init_opmode & 0xFF));
}

int MC_motionRunCycle(const void* inp, void* oup) {
    zmc::AxisFbBridge* bridge = bridge_or_null();
    if (!bridge) {
        return -1;
    }
    return bridge->run_cycle(inp, oup);
}

}  // extern "C"

void MC_motionPrintAddress(void) {
    zmc::AxisFbBridge* bridge = bridge_or_null();
    if (bridge) {
        bridge->print_address();
    }
}

int MC_motionDispatchRunMode(int run_mode, int port, const std::string* startup_script,
                             bool script_exit) {
    zmc::AxisFbBridge* bridge = bridge_or_null();
    if (!bridge) {
        return -1;
    }
    const std::string emptyScript;
    const std::string& startupScript = startup_script ? *startup_script : emptyScript;
    switch (run_mode) {
        case zmc::MODE_HTTP:
            return ZMC_ERR_HOST_NOT_IMPLEMENTED;
#if 0  // Disabled external queue attachment; retained legacy code. GPT-6.
        case zmc::MODE_HTTP: {
            int ret = zmc::runHttpServer(port);
            return ret;
        }
#endif
        case zmc::MODE_RPC: {
            RunDispatchContext ctx = {port, startup_script, script_exit};
            runRpcServer(&ctx);
            return 0;
        }
        case zmc::MODE_CMD:
            return zmc::runCmdLoopV2(bridge->command_interface(), startupScript, script_exit);
        default:
            return -1;
    }
}
