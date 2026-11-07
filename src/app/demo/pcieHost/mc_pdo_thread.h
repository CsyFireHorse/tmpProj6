// you can`t edit this file without permission
#ifndef __MC_PDO_THREAD_H
#define __MC_PDO_THREAD_H

#include <unistd.h>

#include "delay/delay.h"
#include "mc_core/mc_eni_blob.h"
#include "mc_core/mc_motion.h"
#include "mc_zecm.h"

/******************************************************/
#define EXEC_ABNORMAL_DELAY_MS 10
#define SWITCH_MASTER_STATUS_RETRY_CNT 100
#define SWITCH_MASTER_STATUS_DELAY_MS 100
/******************************************************/

static __inline int32_t _switchMasterStatus(uint8_t status, int32_t retryCount) {
    uint8_t curStatus;

    MC_EcatRequestMasterState(status);
    do {
        delay_ms(SWITCH_MASTER_STATUS_DELAY_MS);
        MC_EcatGetMasterState(&curStatus);
        if (status == curStatus) return 0;
    } while (retryCount-- > 0);
    return -1;
}

void mc_pdo_thread(void) {
    uint8_t txPdo[256];
    uint8_t rxPdo[256];
    ecat_eni_info_ex eniInfo;
    bool runFlag;
    bool eniLoaded = false;
    for (;;) {
        if (MC_EcatBusIsRunning(&runFlag) || !runFlag) {
            eniLoaded = false;
            usleep(EXEC_ABNORMAL_DELAY_MS);
            continue;
        }
        if (_switchMasterStatus(MC_EcatStateO, SWITCH_MASTER_STATUS_RETRY_CNT)) {
            usleep(EXEC_ABNORMAL_DELAY_MS);
            continue;
        }
        if (MC_EcatGetEniInfo(&eniInfo)) {
            usleep(EXEC_ABNORMAL_DELAY_MS);
            continue;
        }
        if (!eniLoaded) {
            eniLoaded = (mc_eni_load_pending_descriptor() == 0);
        }
        if (MC_EcatPIEnableSet(true)) {
            usleep(EXEC_ABNORMAL_DELAY_MS);
            continue;
        }
        for (; (!MC_EcatBusIsRunning(&runFlag) && runFlag);) {
            MC_EcatPIInputWaitNotify(1);
            if (MC_EcatPIRead(0, sizeof(rxPdo), rxPdo)) break;
            MC_motionRunCycle(rxPdo, txPdo);
            if (MC_EcatPIWrite(0, sizeof(txPdo), txPdo)) break;
        }
    }
}

#endif  //__MC_PDO_THREAD_H
