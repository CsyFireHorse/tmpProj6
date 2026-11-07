// #include "FbAttrVisitor.h"
#include "AxFbCmdManager.h"
#include "AxisCiaConfig.hpp"
#include "stdio.h"
// #include "axisAttrVisitor.h"
using namespace zmc;

void axFbCmdMgr_demo() {
    AxFbCmdManager axFbCmdMgr;
    AxFbCmdConfig cfg = {
        {0.01, 100, true, 0}, {AX_FACTORY_VIRTUAL}, {nullptr}, {FB_TYPE_POWER, FB_TYPE_MOVEABSOLUTE}};
    axFbCmdMgr.set_config(cfg);

    std::vector<CommandStruct4> cmds = {
        {1, FB_ATTR_AUTORUN, 0, 1},
        {2, FB_ATTR_AUTORUN, 0, 1},
        {1, FB_ATTR_AXIS, 0, 1},
        {1, FB_ATTR_ENABLE, 0, true},
        {1, FB_ATTR_BREGULATORON, 0, true},
        {1, FB_ATTR_BDRIVESTART, 0, true},
        {2, FB_ATTR_AXIS, 0, 1},
        {2, FB_ATTR_POSITION, 0, 15},
        {2, FB_ATTR_VELOCITY, 0, 20},
        {2, FB_ATTR_ACCELERATION, 0, 200},
        {2, FB_ATTR_DECELERATION, 0, 200},
        {2, FB_ATTR_EXECUTE, 6, true},
    };
    axFbCmdMgr.pushCmds(cmds);
    axFbCmdMgr.run();
}

void axFbCmdMgr_encoder_demo() {
    AxFbCmdManager axFbCmdMgr;
    AxFbCmdConfig cfg = {{0.01, 100, true, 0}, {AX_FACTORY_ENCODER}, {nullptr}, {FB_TYPE_POWER}};
    axFbCmdMgr.set_config(cfg);

    std::vector<CommandStruct4> cmds = {{1, FB_ATTR_AUTORUN, 0, 1},
                                       {1, FB_ATTR_AXIS, 0, 1},
                                       {1, FB_ATTR_ENABLE, 0, true},
                                       {1, FB_ATTR_BREGULATORON, 0, true},
                                       {1, FB_ATTR_BDRIVESTART, 0, true}};
    axFbCmdMgr.pushCmds(cmds);
    for (int i = 0; i < 100; i++) {
        if (i >= 5) {
            double pos = (i - 5) * 2.45;
            CommandStruct cmd = {1, (uint8_t) AX_ATTR_DIENCODERPOSITION, AX_ATTR_SET, i, pos};
            axFbCmdMgr.pushCommand(cmd);
        }
        axFbCmdMgr.runCycle();
    }
}
