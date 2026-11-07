// #include "FbAttrVisitor.h"
#include "AxFbCmdManager.h"
#include "AxisCiaConfig.hpp"
#include "stdio.h"
#include "mc_ref_def.h"
// #include "axisAttrVisitor.h"
using namespace zmc;


void axFbCmdMgr_rdwr_demo() {
    AxFbCmdManager axFbCmdMgr;
    const char* keys[] = {"acc", "dec", "busy", "done", "axisStat", "axisStat2"};
    AxFbCmdConfig cfg = {
        {0.01, 100, true, 0}, {AX_FACTORY_VIRTUAL}, {nullptr}, {FB_TYPE_POWER, FB_TYPE_MOVEABSOLUTE},{},
        {{"acc", 8, VAR_DOUBLE}, {"dec", 8, VAR_INT}, {"busy", 8, VAR_INT}, {"done", 8, VAR_INT}, {"axisStat", 8, VAR_INT}, {"axisStat2", 8, VAR_INT}} };
    axFbCmdMgr.set_config(cfg);
    axFbCmdMgr.varMgr.setDouble(0, 250.0);
    std::vector<CommandStruct> cmds = {
        {1, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},
        {2, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},
        {1, FB_ATTR_AXIS, AXFB_NOP, 0, 1},
        {1, FB_ATTR_ENABLE, AXFB_NOP, 0, true},
        {1, FB_ATTR_BREGULATORON, AXFB_NOP, 0, true},
        {1, FB_ATTR_BDRIVESTART, AXFB_NOP, 0, true},
        {2, FB_ATTR_AXIS, AXFB_NOP, 0, 1},
        {2, FB_ATTR_POSITION, AXFB_NOP, 0, 15},
        {2, FB_ATTR_VELOCITY, AXFB_NOP, 0, 20},
        {2, FB_ATTR_ACCELERATION, AXFB_NOP, 0, 200},
        {2, FB_ATTR_DECELERATION, AXFB_NOP, 0, 200},

        {2, FB_ATTR_ACCELERATION, FB_ATTR_SET_EX, 1, 0}, 
        {2, FB_ATTR_DECELERATION, FB_ATTR_GET_EX, 2, 1},
        {2, FB_ATTR_EXECUTE, AXFB_NOP, 6, true},
        {2, FB_ATTR_BUSY, FB_ATTR_GET_EX, 16, 2},
        {1, AX_ATTR_AXISSTAT - AX_ATTR_NOP, AX_ATTR_GET_EX, 18, 4},
        {2, FB_ATTR_DONE, FB_ATTR_GET_EX, 95, 3},
        {1, AX_ATTR_AXISSTAT - AX_ATTR_NOP, AX_ATTR_GET_EX, 96, 5},
    };
    axFbCmdMgr.pushCmds(cmds);
    axFbCmdMgr.run();
    auto val = axFbCmdMgr.varMgr.getDouble(1);
    for (int i=0; i<6; i++)
        printf("%s=%f\t", keys[i], axFbCmdMgr.varMgr.getDouble(i));
}


void axFbCmdMgr_rdwr_demo2() {
    AxFbCmdManager axFbCmdMgr;
    AxFbCmdConfig cfg = {
        {0.01, 200, true, 0}, {AX_FACTORY_VIRTUAL, AX_FACTORY_VIRTUAL}, {nullptr}, {
            FB_TYPE_POWER, FB_TYPE_MOVEVELOCITY, 
            FB_TYPE_POWER, FB_TYPE_READCAM, FB_TYPE_CAMTABLESELECT, FB_TYPE_CAMIN, },{},
        {
        {"sFilename", 80, VAR_OBJ}, {"cam", 8, VAR_DOUBLE}} };
    axFbCmdMgr.set_config(cfg);
    // axFbCmdMgr.varMgr.setValue(0, 250.0);
    axFbCmdMgr.varMgr.setObjValue(0, ZMC_STRING_80{"abc.cam"});
    
    #define AXIS_M (1)
    #define AXIS_S (2)
    std::vector<CommandStruct> cmds = {
        {1, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},
        {2, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},
        {3, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},
        {4, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},
        {5, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},
        {6, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},

        {1, FB_ATTR_AXIS, AXFB_NOP, 0, AXIS_M},
        {1, FB_ATTR_ENABLE, AXFB_NOP, 0, true},
        {1, FB_ATTR_BREGULATORON, AXFB_NOP, 0, true},
        {1, FB_ATTR_BDRIVESTART, AXFB_NOP, 0, true},
        {2, FB_ATTR_AXIS, AXFB_NOP, 0, AXIS_M},
        {2, FB_ATTR_VELOCITY, AXFB_NOP, 0, 20},
        {2, FB_ATTR_ACCELERATION, AXFB_NOP, 0, 200},
        {2, FB_ATTR_DECELERATION, AXFB_NOP, 0, 200},

        {3, FB_ATTR_AXIS, AXFB_NOP, 0, AXIS_S},
        {3, FB_ATTR_ENABLE, AXFB_NOP, 0, true},
        {3, FB_ATTR_BREGULATORON, AXFB_NOP, 0, true},
        {3, FB_ATTR_BDRIVESTART, AXFB_NOP, 0, true},

        {4, FB_ATTR_SFILENAME, FB_ATTR_SET_EX_SPECIAL, 0, 0}, // load from var0
        {5, FB_ATTR_SLAVE, AXFB_NOP, 0, AXIS_S},
        {5, FB_ATTR_MASTER, AXFB_NOP, 0, AXIS_M},
        {6, FB_ATTR_SLAVE, AXFB_NOP, 0, AXIS_S},
        {6, FB_ATTR_MASTER, AXFB_NOP, 0, AXIS_M},

        {4, FB_ATTR_BEXECUTE, AXFB_NOP, 2, true}, 
        {4, FB_ATTR_CAM, FB_ATTR_GET_EX, 3, 1}, // save to var1
        {5, FB_ATTR_CAMTABLE, FB_ATTR_SET_EX, 4, 1},  // load from var1
        {5, FB_ATTR_EXECUTE, AXFB_NOP, 5, true}, 
        {6, FB_ATTR_CAMTABLEID, FB_ATTR_SET_EX_SPECIAL, 6, 5}, // load from camSwitch
        {6, FB_ATTR_EXECUTE, AXFB_NOP, 7, true}, 

        {2, FB_ATTR_EXECUTE, AXFB_NOP, 8, true}, 
    };
    axFbCmdMgr.pushCmds(cmds);
    axFbCmdMgr.run();
    // auto val = axFbCmdMgr.varMgr.getValue<double>(1);
}