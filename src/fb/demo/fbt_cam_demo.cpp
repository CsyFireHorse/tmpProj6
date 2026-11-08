#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "pou/AxisManager.h"
#include "fbt_axis_impl.h"
#include "fbt_axis_base.h"
#include "fbt_axis.h"
#include "fbt_axis_virtual.h"
#include "fb_cpp_def.h"
#include "stdio.h"
#include "camGenHelper.hpp"
#include "cam_tappet_serialize.h"
#include "zmc_cam_table.h"
#include "common/misc/CmdArgsToy.hpp"

using namespace zmc;

#define CAM_XYVA_LEN 21

template<bool bReadCamDefault = false>
class CamTask : public PouTaskBase {
public:
    CamTask() {
        auto& args = CmdArgsToySingleton::get();
        master_axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        slave_axis = AxPouTaskBinderAuto::getAxisByIndex(1);
        
        // Configure move relative
        move.Axis = master_axis;
        move.Distance = args["move.Distance"].get<double>(360);
        move.Velocity = args["move.Velocity"].get<double>(80);
        move.Acceleration = args["move.Acceleration"].get<double>(400);
        move.Deceleration = args["move.Deceleration"].get<double>(400);
        move.Jerk = args["move.Jerk"].get<double>(0);
        
        readCam.sFileName = ZMC_STRING_80{"abc3.cam"};
        
        // Configure cam table select
        camSel.Slave = slave_axis;
        camSel.Master = master_axis;
        camSel.Periodic = args["camSel.Periodic"].get<bool>(false);
        camSel.MasterAbsolute = args["camSel.MasterAbsolute"].get<bool>(true);
        camSel.SlaveAbsolute = args["camSel.SlaveAbsolute"].get<bool>(true);
        
        // Configure cam in
        camIn.Slave = slave_axis;
        camIn.Master = master_axis;
        camIn.Acceleration = args["camIn.Acceleration"].get<double>(1000);
        camIn.Deceleration = args["camIn.Deceleration"].get<double>(1000);
        camIn.VelocityDiff = args["camIn.VelocityDiff"].get<double>(100);
        camIn.MasterOffset = args["camIn.MasterOffset"].get<double>(0);
        camIn.SlaveOffset = args["camIn.SlaveOffset"].get<double>(0);
        camIn.MasterScaling = args["camIn.MasterScaling"].get<double>(1);
        camIn.SlaveScaling = args["camIn.SlaveScaling"].get<double>(1);
        camIn.StartMode = (MC_STARTMODE) args["camIn.StartMode"].get<int>(mcAbsolute);
        
        // Store configuration parameters
        // verbose_flag = args["verbose"].get<int>(9);
        read_cam = args["read_cam"].get<int>(bReadCamDefault);
        int slices = args["slices"].get<int>(1);
        
        // Initialize cam data
        fillXyvaToy((Xyva*)camData, CAM_XYVA_LEN, 90, slices);
        camSel.CamTable = makeCamRef(camData, CAM_XYVA_LEN);
        
        // Power control for both axes
        power_master.Axis = master_axis;
        power_slave.Axis = slave_axis;

        master_axis->bind_extra_data(&fbStArr[0]);
        slave_axis->bind_extra_data(&fbStArr[1]);

    }
    
    void runCycle() override {
        // Power control for master axis
        power_master.Enable = true;
        power_master.bDriveStart = true;
        power_master.bRegulatorOn = true;
        power_master.runCycle();
        
        // Power control for slave axis
        power_slave.Enable = true;
        power_slave.bDriveStart = true;
        power_slave.bRegulatorOn = true;
        power_slave.runCycle();
        
        // Control movement sequence
        if (idx >= 3 && idx <= 4 && read_cam == 1) {
            readCam.bExecute = read_cam == 1;
            camSel.CamTable = readCam.CAM;
        }

        camSel.Execute = idx >= 5;
        move.Execute = power_master.Status && idx >= 7;
        if (camSel.Done) {
            camIn.CamTableID = camSel.CamTableID;
            camIn.Execute = power_slave.Status && idx >= 8;
        }
        
        readCam.runCycle();
        move.runCycle();
        camSel.runCycle();
        camIn.runCycle();

        
        // Print status
        if (1) {
            fbStArr[0] = move.Busy + move.Done*3 + move.Error*4 + move.CommandAborted*5;
            fbStArr[1] = camIn.Busy + camIn.InSync + camIn.EndOfProfile + camIn.CommandAborted*5 + camIn.Error*4;
        }
        // master_axis->print(verbose_flag);
        // slave_axis->print(verbose_flag);
        idx++;
    }
    
private:
    AXIS_REF master_axis;
    AXIS_REF slave_axis;
    FbtPower power_master;
    FbtPower power_slave;
    FbtMoveRelative move;
    FbtCamTableSelect camSel;
    FbtCamIn camIn;
    FbtReadCAM readCam;
    int read_cam;
    //  slices;
    int idx = 0;
    CamXyva camData[CAM_XYVA_LEN] = {0};
    // int verbose_flag;

    int fbStArr[2] = {0};
};

template<bool bReadCamDefault = false>
int _ax_cam_demo(int argc, char** argv) {
    const std::string iniContentDefault = R"(
[base]
axis_num = 2
[axis_0]
cycle_time = 20000
is_axis_virtual = true
id = 0
[axis_1]
cycle_time = 20000
is_axis_virtual = true
id = 1
)";  
    return pou_auto_tmpl_demo<CamTask<bReadCamDefault>>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "9"}});
}

int ax_cam_demo(int argc, char** argv) {
    return _ax_cam_demo<false>(argc, argv);
}
int ax_cam_demo2(int argc, char** argv) {
    return _ax_cam_demo<true>(argc, argv);
}


#define MIN(a, b) ((a) < (b) ? (a) : (b))
void cam_write_read_demo_with_args(CmdArgsToy& argsToy) {
    // ZMC_STRING_80 sFileName = {"abc.cam"}; // compile ok
    #define CAM_XYVA_MAX_LEN 50
    FbtWriteCAM wrCam;
    wrCam.bExecute = true;
    const char* sFileName = argsToy["file_name"].get<const char*>("abc3.cam");
    // wrCam.sFileName = ZMC_STRING_80{"abc.cam"};
    wrCam.sFileName = make_ZMC_STRING_80(sFileName);
    CamXyva dat[CAM_XYVA_MAX_LEN] = {0};
    int len = MIN(argsToy["length"].get<int>(CAM_XYVA_LEN), CAM_XYVA_MAX_LEN);
    if (!argsToy["use_linear"].get<bool>(false))
        fillXyvaToy((Xyva*)dat, len, argsToy["amplitude"].get<double>(90), argsToy["slices"].get<int>(1), argsToy["xmax"].get<double>(360));
    else
        fillXyvaLinear((Xyva*)dat, len, argsToy["amplitude"].get<double>(360), argsToy["xmax"].get<double>(360));
    wrCam.CAM = makeCamRef(dat, len);

    wrCam.runCycle();
    printf("writeCam %s: %d\n", wrCam.sFileName.value, wrCam.bBusy + wrCam.bDone* 2 +  wrCam.bError * 3);

    FbtReadCAM rdCam;
    rdCam.bExecute = true;
    rdCam.sFileName = wrCam.sFileName;
    rdCam.runCycle();
    printf("readCam:%d\n", rdCam.bBusy + rdCam.bDone* 2 +  rdCam.bError * 3);
}

int cam_write_read_demo(int argc, char** argv){
    if (argc>1 && (strcmp(argv[1],"--help")==0 || strcmp(argv[1],"-h")==0)){
        printf("Usage: --help \n [--help] [--file_name <string>] [--length <int>] [--amplitude <double>(90)] [--slices <int>]   [--xmax <double>] [--use_linear <int>]" );
        return 0;
    }
    CmdArgsToy argsToy;
    parse_args(argc-1, argv+1, argsToy);
    cam_write_read_demo_with_args(argsToy);
    return 0;
}


void cam_bounds_demo() {
    CamXyva dat[CAM_XYVA_LEN] = {0};
    fillXyvaToy((Xyva*)dat, CAM_XYVA_LEN, 90);
    auto CAM = makeCamRef(dat, CAM_XYVA_LEN);

    FbtCAMBounds bndCam;
    bndCam.bExecute = true;
    bndCam.CAM = CAM;
    bndCam.runCycle();
    printf("%d\n", bndCam.bBusy + bndCam.bDone* 2 +  bndCam.bError * 3);
    printf("bounds: %f, %f,%f, %f\n", bndCam.dMaxPos, bndCam.dMinPos, bndCam.dMaxVel, bndCam.dMaxAccDec);
}


void CamTableTappet_demo(){
    AxisMotEmu axisArr[2];

    FbtMoveVelocity fbMov;
    FbtCamTableSelect fbTbl;
    FbtCamIn camIn;
    #define TAPPET_ID_NUM 5
    FbtGetTappetValue getTappetValue[TAPPET_ID_NUM];

    fbMov.Axis = &axisArr[0];
    fbMov.Acceleration = 1000;
    fbMov.Deceleration = 1000;
    fbMov.Velocity = 100;

    fbTbl.Master = &axisArr[0];
    fbTbl.Slave = &axisArr[1];
    camIn.Master = &axisArr[0];
    camIn.Slave = &axisArr[1];

    #define CAM_XYVA_LEN 21
    CamXyva dat[CAM_XYVA_LEN] = {0};
    fillXyvaToy((Xyva*)dat, CAM_XYVA_LEN, 90);

    SMC_CAMTappet tappets[] = {
        // Type, action, delay, duration, group ID, position, activation.
        {TAPPET_pos, TAPPETACTION_on, 0, 0, 1, 10., 0xffffffff}, 
        {TAPPET_pos, TAPPETACTION_on, 0, 0, 2, 20., 0xffffffff}, 
        {TAPPET_all, TAPPETACTION_off, 0, 0, 1, 30., 0xffffffff}, 
        {TAPPET_all, TAPPETACTION_on, 0, 0, 3, 50., 0xffffffff}, 
        {TAPPET_all, TAPPETACTION_inv, 0, 0, 4, 50., 0xffffffff}, 
        {TAPPET_all, TAPPETACTION_inv, 0, 0, 5, 60., 0xffffffff}, 
        {TAPPET_neg, TAPPETACTION_off, 0, 0, 4, 70., 0xffffffff}, 
        {TAPPET_pos, TAPPETACTION_off, 0, 0, 2, 90., 0xffffffff}  
    };

    CamTable table(dat, CAM_XYVA_LEN/2+1, (const Tappet*)tappets, sizeof(tappets)/sizeof(tappets[0]));
    
    auto camRef = (MC_CAM_REF) &table;
    fbTbl.CamTable = camRef;


    for (int i = 0; i < TAPPET_ID_NUM; i++){
        getTappetValue[i].iID = i+1;
        getTappetValue[i].Tappets = &camIn.Tappets;
    }
    getTappetValue[3].bInitValue = true;

    int verbose = 1;
    int fbStat;
    for (int i = 0; i <= 300; i++) {
        if (i==3) fbTbl.Execute = true;
        if (i==6){
            fbMov.Execute = true;
            camIn.Execute = true;
            camIn.CamTableID = fbTbl.CamTableID;
        }
        // fbPow.runCycle();
        // fbPow2.runCycle();
        fbMov.runCycle();
        fbTbl.runCycle();
        camIn.runCycle();
        int tappetBitValue = 0;
        for (int j = 0; j < TAPPET_ID_NUM; j++) {
            getTappetValue[j].runCycle();
            tappetBitValue |= getTappetValue[j].bTappet << j;
        }
        for (int j=0; j<2; j++){
            auto ax =  (AxisImplBase&) axisArr[j];
            if (j==0){
                fbStat = fbMov.Busy * 1 + fbMov.InVelocity * 2 + fbMov.Error * 4 + fbMov.CommandAborted * 5;
            }else{
                fbStat = camIn.EndOfProfile * 3 + camIn.Busy * 2 + camIn.InSync * 1 + camIn.Error * 4;
            }
            PosVelAcc pva = {ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration};
            printf("%d, %d, %d, %d, %f, %f, %f, 0x%x\n", i, j, ax.axisStat, fbStat, pva.pos, pva.vel, pva.acc, tappetBitValue);
        }
        axisArr[0].runCycle();
        axisArr[1].runCycle();
    }
}
