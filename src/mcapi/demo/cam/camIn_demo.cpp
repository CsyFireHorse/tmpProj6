#include "mc_fb_api.h"
#include "axis_cia402_api.h"
#include "axis_config_api.h"
#include "axis_virtual_api.h"
#include "stdio.h"

class McCamInDemo {
public:
    axis_cia402_handle_t axis_master;
    axis_cia402_handle_t axis_slave;
    McPower fbPowM, fbPowS;
    McMoveVelocity fbVel;
    McCamTableSelect fbTbl;
    McReadCAM fbRdCam;
    McCamIn fbCamIn;

    int idx = 0;
    int count_to_exit = 100;

    McCamInDemo() {
        axis_master = axis_cia402_new();
        axis_slave = axis_cia402_new();

        AxisCiaConfig* cfgM = axis_cia402_get_config_addr(axis_master);
        axis_config_default_init(cfgM);
        cfgM->is_axis_virtual = true;
        cfgM->cycle_time = 10000;
        axis_cia402_set_config(axis_master, cfgM);


        AxisCiaConfig* cfgS = axis_cia402_get_config_addr(axis_slave);
        axis_config_default_init(cfgS);
        cfgS->is_axis_virtual = true;
        cfgS->cycle_time = 10000;
        axis_cia402_set_config(axis_slave, cfgS);


        MC_Power_create(&fbPowM);
        MC_Power_create(&fbPowS);
        MC_MoveVelocity_create(&fbVel);
        MC_CamTableSelect_create(&fbTbl);
        MC_ReadCAM_create(&fbRdCam);
        MC_CamIn_create(&fbCamIn);


        fbPowM.Axis = axis_cia402_get_ref(axis_master);
        fbPowS.Axis = axis_cia402_get_ref(axis_slave);
        fbVel.Axis = axis_cia402_get_ref(axis_master);
        fbTbl.Master = axis_cia402_get_ref(axis_master);
        fbTbl.Slave = axis_cia402_get_ref(axis_slave);
        fbCamIn.Master = axis_cia402_get_ref(axis_master);
        fbCamIn.Slave = axis_cia402_get_ref(axis_slave);


        fbVel.Velocity = 50;
        fbVel.Acceleration = 500;
        fbVel.Deceleration = 500;
        fbVel.Jerk = 0;
        fbVel.Direction = 1;
        fbVel.BufferMode = 0;

        fbTbl.Periodic = 0;
        fbTbl.MasterAbsolute = 0;
        fbTbl.SlaveAbsolute = 0;

        fbCamIn.MasterOffset = 0;
        fbCamIn.SlaveOffset = 0;
        fbCamIn.MasterScaling = 1;
        fbCamIn.SlaveScaling = 1;
        fbCamIn.Acceleration = 1000;
        fbCamIn.Deceleration = 1000;
        fbCamIn.Jerk = 0;
    }

    ~McCamInDemo() {
        MC_Power_destroy(&fbPowM);
        MC_Power_destroy(&fbPowS);
        MC_MoveVelocity_destroy(&fbVel);
        MC_CamTableSelect_destroy(&fbTbl);
        MC_ReadCAM_destroy(&fbRdCam);
        MC_CamIn_destroy(&fbCamIn);
        axis_cia402_delete(axis_master);
        axis_cia402_delete(axis_slave);
    }

    void preRunCycle() {
        if (idx == 1) {
            fbPowM.Enable = true;
            fbPowM.bRegulatorOn = true;
            fbPowM.bDriveStart = true;
            fbPowS.Enable = true;
            fbPowS.bRegulatorOn = true;
            fbPowS.bDriveStart = true;
        }
        if (idx == 5) {
            fbRdCam.bExecute = true;
            snprintf(fbRdCam.sFileName.value, sizeof(fbRdCam.sFileName.value), "abc.cam");
        }
        if (idx == 8) {
            fbTbl.Execute = true;
            fbTbl.CamTable = fbRdCam.CAM;
        }
        if (idx == 10) {
            fbVel.Execute = true;
        }
        if (idx == 15) {
            fbCamIn.Execute = true;
            fbCamIn.CamTableID = fbTbl.CamTableID;
        }
    }

    void run() {
        for (idx = 0; idx < count_to_exit; ++idx) {
            preRunCycle();
            MC_Power_exec(&fbPowM);
            MC_Power_exec(&fbPowS);
            MC_ReadCAM_exec(&fbRdCam);
            MC_CamTableSelect_exec(&fbTbl);
            MC_MoveVelocity_exec(&fbVel);
            MC_CamIn_exec(&fbCamIn);
            axis_virtual_print((AxisStorage*)axis_master, 1);
            axis_virtual_print((AxisStorage*)axis_slave, 1);
            axis_cia402_runcycle(axis_master);
            axis_cia402_runcycle(axis_slave);
        }
    }
};

void mc_camIn_demo() {
    McCamInDemo demo;
    demo.run();
}
