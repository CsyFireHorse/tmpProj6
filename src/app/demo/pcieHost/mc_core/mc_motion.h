#ifndef MC_PCIEHOST_MC_MOTION_H
#define MC_PCIEHOST_MC_MOTION_H

/*
 * Motion shell exposed to mc_pdo_thread.
 *
 * Holds a process-global AxisFbBridgeWithThread (embedded pio_layout_addr and
 * SdoRt with MC_EcatCoeSDO*Ex adapter). MC_motionRunCycle forwards raw PI
 * buffers to AxisFbBridge::run_cycle.
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Construct the global AxisFbBridgeWithThread shell.
 *   ini_path : path to the axis ini (may be empty / NULL)
 *   axis_num : number of logical axes managed by the bridge
 * Returns 0 on success.
 */
int MC_motionInit(const char* ini_path, unsigned int axis_num);

/* Destroy bridge and SDO worker. Safe to call repeatedly. */
void MC_motionShutdown(void);

/*
 * Parse ENI and apply ZmcPioOffsetLayout to the bridge.
 * Must be called after MC_EcatHostOpen (bus running).
 * Returns 0 on success.
 */
int MC_motionLoadEni(const char* eni_path);

/* Auto-bind logical axes to servos starting at bind_begin. Returns 0 on success. */
int MC_motionAutoBind(int bind_begin);

/*
 * Manual bind: axis_arr[i] is the logical axis index bound to servo slot i.
 * count must be > 0. Returns 0 on success.
 */
int MC_motionManualBind(int count, const int* axis_arr);

/* Bind MC SDO adapter and start the bridge SDO worker. Returns 0 on success. */
int MC_motionStartSdo(void);

/* Stop SDO worker. */
void MC_motionStopSdo(void);

/*
 * Add SDO upload/download watches for opmode when not mapped in PDO.
 * init_opmode is written by download watch (CiA402 mode byte); 0 skips watches.
 * Returns 0 on success.
 */
int MC_motionAddOpmodeWatch(int init_opmode);

/*
 * One PDO cycle: map inp/oup raw PI buffers, run AxisFbBridge::run_cycle.
 *   inp : PI input (slave -> master) byte buffer
 *   oup : PI output (master -> slave) byte buffer
 */
int MC_motionRunCycle(const void* inp, void* oup);

void MC_motionPrintAddress(void);
#ifdef __cplusplus
}

#include <string>

int MC_motionDispatchRunMode(int run_mode, int port, const std::string* startup_script,
                             bool script_exit);

#endif /* __cplusplus */

#endif /* MC_PCIEHOST_MC_MOTION_H */
