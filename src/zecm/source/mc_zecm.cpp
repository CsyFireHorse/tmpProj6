#include "mc_zecm.h"
#include "pci_zecm.h"
#include <cstring>

/* Process-global handle. The demo expects a single PCIe master, which is
 * also how pci_zecm_fak is structured (one EcatFaker per Open). */
static ECAT_HANDLE g_handle = 0;

extern "C" {

int MC_EcatHostOpen(const char* board_if, uint32_t channel, const char* eni_path) {
    if (g_handle) return -1;
    if (!eni_path) return -1;
    const char* board = board_if ? board_if : "ID0";
    int32_t ret = EcatOpen(&g_handle, board, channel);
    if (ret != ECAT_S_OK) {
        g_handle = 0;
        return static_cast<int>(ret);
    }
    ret = EcatBusRun(g_handle, eni_path);
    if (ret != ECAT_S_OK) {
        EcatClose(g_handle);
        g_handle = 0;
        return static_cast<int>(ret);
    }
    return 0;
}

int MC_EcatHostClose(void) {
    if (!g_handle) return 0;
    EcatBusStop(g_handle);
    int32_t ret = EcatClose(g_handle);
    g_handle = 0;
    return static_cast<int>(ret);
}

int MC_EcatRequestMasterState(uint8_t state) {
    if (!g_handle) return -1;
    return static_cast<int>(EcatRequestMasterState(g_handle, state));
}

int MC_EcatGetMasterState(uint8_t* state) {
    if (!g_handle || !state) return -1;
    return static_cast<int>(EcatGetMasterState(g_handle, state));
}

int MC_EcatBusIsRunning(bool* run_flag) {
    if (!run_flag) return -1;
    *run_flag = false;
    if (!g_handle) return -1;
    uint8_t st = 0;
    int32_t ret = EcatGetMasterState(g_handle, &st);
    if (ret != ECAT_S_OK) return static_cast<int>(ret);
    /* Treat any state at or beyond INIT as "running" - the bus thread is
     * alive. The pdo loop tightens the check to EcatStateO separately. */
    *run_flag = ((st & EcatStateMask) >= EcatStateI);
    return 0;
}

int MC_EcatGetEniInfo(ecat_eni_info_ex* info) {
    if (!g_handle || !info) return -1;
    ECAT_ENI_INFO raw;
    std::memset(&raw, 0, sizeof(raw));
    int32_t ret = EcatGetEniInfo(g_handle, &raw);
    if (ret != ECAT_S_OK) return static_cast<int>(ret);
    info->u32MainCycle = raw.u32MainCycle;
    info->u32MailboxCycle = raw.u32MailboxCycle;
    info->u32PIOutputByteSize = raw.u32PIOutputByteSize;
    info->u32PIInputByteSize = raw.u32PIInputByteSize;
    return 0;
}

int MC_EcatPIEnableSet(bool enable) {
    if (!g_handle) return -1;
    if (!enable) return 0;
    return static_cast<int>(EcatPIEnable(g_handle));
}

int MC_EcatPIRead(uint32_t offset, uint32_t len, void* data) {
    if (!g_handle || !data) return -1;
    return static_cast<int>(EcatPIRead(g_handle, PI_AREA_SHADOW_INPUT, offset, len, data));
}

int MC_EcatPIWrite(uint32_t offset, uint32_t len, const void* data) {
    if (!g_handle || !data) return -1;
    return static_cast<int>(EcatPIWrite(g_handle, PI_AREA_FAST_OUTPUT, offset, len, data));
}

int MC_EcatPIInputWaitNotify(uint32_t timeout_ms) {
    if (!g_handle) return -1;
    return static_cast<int>(EcatPIInputWaitNotify(g_handle, timeout_ms));
}

MCCommResult MC_EcatCoeSDODownloadEx(uint16_t u16SlaveId, uint16_t u16Index, uint8_t u8SubIndex,
                                     uint8_t bCompleteAccess, uint32_t u32DataLen,
                                     const void* pvData, uint32_t u32TimeOutMs) {
    if (!g_handle || !pvData) return -1;
    return static_cast<MCCommResult>(EcatCoeSDODownloadEx(g_handle, u16SlaveId, u16Index,
                                                          u8SubIndex, bCompleteAccess, u32DataLen,
                                                          pvData, u32TimeOutMs));
}

MCCommResult MC_EcatCoeSDOUploadEx(uint16_t u16SlaveId, uint16_t u16Index, uint8_t u8SubIndex,
                                   uint8_t bCompleteAccess, uint32_t* pu32DataLen, void* pvData,
                                   uint32_t u32TimeOutMs) {
    if (!g_handle || !pu32DataLen || !pvData) return -1;
    return static_cast<MCCommResult>(EcatCoeSDOUploadEx(g_handle, u16SlaveId, u16Index, u8SubIndex,
                                                        bCompleteAccess, pu32DataLen, pvData,
                                                        u32TimeOutMs));
}

MCCommResult MC_EcatGetSlaveCount(uint16_t* pu16SlaveCount) {
    if (!g_handle || !pu16SlaveCount) return -1;
    return static_cast<MCCommResult>(EcatGetSlaveCount(g_handle, pu16SlaveCount));
}
MCCommResult MC_EcatGetActiveSlaveCount(uint16_t* pu16SlaveCount) {
    if (!g_handle || !pu16SlaveCount) return -1;
    return static_cast<MCCommResult>(EcatGetActiveSlaveCount(g_handle, pu16SlaveCount));
}
MCCommResult MC_EcatGetSlaveState(uint16_t u16SlaveIdx, uint8_t* pu8Status) {
    if (!g_handle || !pu8Status) return -1;
    return static_cast<MCCommResult>(EcatGetSlaveState(g_handle, u16SlaveIdx, pu8Status));
}

MCCommResult MC_EcatGetSlaveStateEx(uint16_t u16SlaveNum, uint8_t* pu8Status) {
    if (!g_handle || (u16SlaveNum != 0u && !pu8Status)) return -1;
    for (uint16_t i = 0; i < u16SlaveNum; ++i) {
        MCCommResult ret = MC_EcatGetSlaveState(i, &pu8Status[i]);
        if (ret != ECAT_S_OK) return ret;
    }
    return ECAT_S_OK;
}

MCCommResult MC_EcatGetSlaveDLStatus(uint16_t u16SlaveIdx, uint16_t* pu16DLStatus) {
    if (!g_handle || !pu16DLStatus) return -1;
    return static_cast<MCCommResult>(EcatGetSlaveDLStatus(g_handle, u16SlaveIdx, pu16DLStatus));
}

MCCommResult MC_EcatGetSlaveALStatus(uint16_t u16SlaveIdx, uint16_t* pu16ALStatus) {
    if (!g_handle || !pu16ALStatus) return -1;

    uint8_t slaveState = 0;
    MCCommResult ret = MC_EcatGetSlaveState(u16SlaveIdx, &slaveState);
    if (ret != ECAT_S_OK) return ret;

    // The faker has no separate AL status register, so expose its state byte.
    *pu16ALStatus = static_cast<uint16_t>(slaveState);
    return ECAT_S_OK;
}

}  // extern "C"
