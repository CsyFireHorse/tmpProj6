#include "mc_fb_api.h"
#include "axis_cia402_api.h"
#include "stdio.h"
// #include "fbt_axis.h"
#include "fbt_axis_base.h"
#include "axis_config_api.h"

using namespace zmc;

class AxMoveApiDemo{
   public:
    enum {SZ = 1};
    bool _exitFlag = false;
    uint8_t verbose_flag = 0;
    uint16_t verbose_modulo = 1;
    int count_to_exit = 0;
    uint32_t idx = 0;
    axis_cia402_handle_t axArr[SZ];
 
    double targetPos = 10;
    double begPos = 1;
    McPower fbPow;
    McMoveAbsolute fbMov;
    // VelOnlineChecker chk;
    AxMoveApiDemo() {
        for (int i=0; i<SZ; i++){
            axArr[i] = axis_cia402_new();
        }
        AxisCiaConfig cfg;
        axis_config_default_init(&cfg);
        cfg.debug.servo_fak_pos = 1;
        cfg.cycle_time = 50000;
        cfg.is_axis_virtual = true;
        axis_cia402_set_config(axArr[0], &cfg);

        this->count_to_exit = 70;
        // chk.Ts = cfg.cycle_time / 1e6;

        MC_Power_create(&fbPow);
        MC_MoveAbsolute_create(&fbMov);
    }
    ~AxMoveApiDemo() {
        for (int i=0; i<SZ; i++){
            axis_cia402_delete(axArr[i]);
        }
        MC_Power_destroy(&fbPow);
        MC_MoveAbsolute_destroy(&fbMov);
    }
    AxisImplBase& axRef(int i) { return  (AxisImplBase&) axArr[i]; }
    // AxisCiaConfig& getAxisCfgRef(int i) { return axArr[i].get_config_ref(); }
    void preRun(){
        if (verbose_flag)
            printf("idx,axId,time,axSt,mPos,pos,vel,acc\n");
    }
    void runCycle() {
        preRunCycle();
        for (int i = 0; i < SZ; i++) {
            axis_cia402_runcycle(axArr[i]);
        }
        if (verbose_flag && (idx % verbose_modulo == 0)) {
            for (int i = 0; i < SZ; i++) {
                AxisImplBase& ax = *((AxisImplBase*) axis_cia402_get_ref(axArr[i]));
                if (verbose_flag == 2)
                    printf("%d,%d,%d,%d,%d,%f,%f,%f,%f\n", idx, i, ax.axisStat, ax.diActPosition, ax.diSetPosition, ax.fActPosition, ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
                else
                    printf("%d,%d,%f,%d,%f,%f,%f,%f\n", idx, i, idx*ax.cycleTime, ax.axisStat, ax.fActPosition, ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
            }
        }
        idx++;
        _try_exit();
    }
    void run() {
        preRun();
        while (!_exitFlag) {
            runCycle();
        }
    }
    void _try_exit() {
        if (count_to_exit && idx > count_to_exit) _exitFlag = true;
    }
    void preRunCycle() {
        auto& idx = this->idx;
        if (idx == 2) {
            fbPow.Axis = (AXIS_REF)axis_cia402_get_ref(axArr[0]);
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
        } else if (idx == 8) {
            fbMov.Axis = (AXIS_REF)axis_cia402_get_ref(axArr[0]);
            fbMov.Velocity = 20;
            fbMov.Acceleration = 40;
            fbMov.Deceleration = 40;
            fbMov.Execute = true;
        }
        MC_Power_exec(&fbPow);
        MC_MoveAbsolute_exec(&fbMov);
    }
};

void mc_api_axis_moveabs_demo() {
    AxMoveApiDemo axm;
    axm.fbMov.Position = axm.targetPos;
    axm.verbose_flag = true;
    axm.run();
}
