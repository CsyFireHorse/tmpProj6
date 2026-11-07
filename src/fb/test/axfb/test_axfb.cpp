#include "AxFbCmdManager.h"
#include "AxisCiaConfig.hpp"
#include "stdio.h"
#include "gtest/gtest.h"
#include "fbt_impl_enum_def.h"

#include "AxFbCmdParser.hpp"
#include "common/get_file_path.hpp"

using namespace zmc;

#define LEN(x) (sizeof(x) / sizeof(x[0]))
TEST(axfb, base) {
    AxFbCmdManager axFbCmdMgr;
    AxFbCmdConfig cfg = {
        {0.01, 100, false, 0}, {AX_FACTORY_VIRTUAL}, {nullptr}, {FB_TYPE_POWER, FB_TYPE_MOVEABSOLUTE},{},
        {{"axisPos", 8, VAR_DOUBLE}, {"axisPos2", 8, VAR_DOUBLE}, {"busy", 8, VAR_INT}, {"done", 8, VAR_INT}, {"axisStat", 8, VAR_INT}, {"axisStat2", 8, VAR_INT}} };
    axFbCmdMgr.set_config(cfg);
    axFbCmdMgr.varMgr.setDouble(0, 250.0);
    double endPos = 15;
    std::vector<CommandStruct> cmds = {
        {1, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},
        {2, FB_ATTR_AUTORUN, AXFB_NOP, 0, 1},
        {1, FB_ATTR_AXIS, AXFB_NOP, 0, 1},
        {1, FB_ATTR_ENABLE, AXFB_NOP, 0, true},
        {1, FB_ATTR_BREGULATORON, AXFB_NOP, 0, true},
        {1, FB_ATTR_BDRIVESTART, AXFB_NOP, 0, true},
        {2, FB_ATTR_AXIS, AXFB_NOP, 0, 1},
        {2, FB_ATTR_POSITION, AXFB_NOP, 0, endPos},
        {2, FB_ATTR_VELOCITY, AXFB_NOP, 0, 20},
        {2, FB_ATTR_ACCELERATION, AXFB_NOP, 0, 250},
        {2, FB_ATTR_DECELERATION, AXFB_NOP, 0, 250},

        // capture axis & fb status
        {1, AX_ATTR_DISETPOSITION - AX_ATTR_NOP, AX_ATTR_GET_EX, 4, 0},
        {2, FB_ATTR_EXECUTE, AXFB_NOP, 6, true}, // run
        {2, FB_ATTR_BUSY, FB_ATTR_GET_EX, 16, 2},
        {1, AX_ATTR_AXISSTAT - AX_ATTR_NOP, AX_ATTR_GET_EX, 18, 4},

        {1, AX_ATTR_DISETPOSITION - AX_ATTR_NOP, AX_ATTR_GET_EX, 95, 1},
        {1, AX_ATTR_AXISSTAT - AX_ATTR_NOP, AX_ATTR_GET_EX, 95, 5},
        {2, FB_ATTR_DONE, FB_ATTR_GET_EX, 95, 3},
    };
    axFbCmdMgr.pushCmds(cmds);
    axFbCmdMgr.run();
    auto val = axFbCmdMgr.varMgr.getValue<double>(1);
    double expected[] = { 0, endPos, true, true, mcDiscreteMotion, mcStandstill};
    for (int i=0; i<LEN(expected); i++)
        EXPECT_EQ(axFbCmdMgr.varMgr.getValue<double>(i), expected[i]);

}


TEST(axfb, load) {
    auto eni_file = zmc::_get_file_path("fb/demo/axfb/motbuf_cmds.json");
    AxFbCmdConfig cfg;
    int ret = load_AxFbCmdConfig_from_json(cfg, eni_file.c_str());
    if (ret) {
        printf("load failed\n");
        return;
    }
    AxFbCmdManager axFbCmdMgr;
    cfg.vars = {{"axisPos", 8, VAR_DOUBLE}, {"axisPos2", 8, VAR_DOUBLE}, {"busy", 8, VAR_INT}, {"done", 8, VAR_INT}, {"axisStat", 8, VAR_INT}, {"axisStat2", 8, VAR_INT}};
    cfg.runCfg.verbose = 0;
    cfg.cmds.resize(cfg.cmds.size()-1);
    axFbCmdMgr.set_config(cfg);
    // axFbCmdMgr.varMgr.at(8);

    std::vector<CommandStruct> cmds = {
        {1, AX_ATTR_DISETPOSITION - AX_ATTR_NOP, AX_ATTR_GET_EX, 4, 0},
        {2, FB_ATTR_EXECUTE, AXFB_NOP, 6, 1},
        {2, FB_ATTR_BUSY, FB_ATTR_GET_EX, 16, 2},
        {1, AX_ATTR_AXISSTAT - AX_ATTR_NOP, AX_ATTR_GET_EX, 18, 4},
        {1, AX_ATTR_DISETPOSITION - AX_ATTR_NOP, AX_ATTR_GET_EX, 95, 1},
        {1, AX_ATTR_AXISSTAT - AX_ATTR_NOP, AX_ATTR_GET_EX, 95, 5},
        {2, FB_ATTR_DONE, FB_ATTR_GET_EX, 95, 3},
    };
    axFbCmdMgr.pushCmds(cmds);
    axFbCmdMgr.run();
    auto val = axFbCmdMgr.varMgr.getValue<double>(1);
    double expected[] = { 0, 15, true, true, mcDiscreteMotion, mcStandstill};
    for (int i=0; i<LEN(expected); i++)
        EXPECT_EQ(axFbCmdMgr.varMgr.getValue<double>(i), expected[i]);

}

void _test_axfb(const char* zfile) {
    auto zfile_full = zmc::_get_file_path(zfile);
    AxFbCmdConfig cfg;
    int ret = load_AxFbCmdConfig_from_json(cfg, zfile_full);
    if (ret) {
        printf("load failed\n");
        return;
    }
    cfg.runCfg.verbose = false;
    AxFbCmdManager axFbCmdMgr;
    axFbCmdMgr.set_config(cfg);
    axFbCmdMgr.run();
    // auto val = axFbCmdMgr.varMgr.getValue<double>(1);
    // double expected[] = { 0, 15, true, true, mcDiscreteMotion, mcStandstill};
    // for (int i=0; i<LEN(expected); i++)
    //     EXPECT_EQ(axFbCmdMgr.varMgr.getValue<double>(i), expected[i]);
}

TEST(axfb, load_batch) {
    const char* files[] = { 
        "fb/demo/axfb/motbuf_gear.json", 
        "fb/demo/axfb/motbuf_gear_out.json", 
        "fb/demo/axfb/motbuf_cmds2.json" 
    };
    for (auto& file : files) {
        _test_axfb(file);
    }
}