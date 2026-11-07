#include "FbWrapCmd.h"
// #include "FbMoveWrap.hpp"
#include "AxisFbGraph.hpp"
// #include "base/misc_utils.h"
// #include "common/ecat/ServoSgFaker.hpp"
// #include "common/misc/CmdParser.hpp"
#include "common/misc/CommandParser.h"
#include "common/misc/CommandInteractor.h"
#include "pou/axPouTmplRunner.hpp"
#include "common/container/FbCmdQueue.hpp"
#include <thread>
#include <cstring>

namespace zmc {

// Helper function to push command to queue
static bool pushCmdToQueue(FbMoveWrap2Queue& fbGrp, uint16_t cmdType, const void* param, uint16_t param_len) {
    CmdPacket pkt;
    pkt.reset();
    if (pkt.appendItem(cmdType, param, param_len)) {
        return fbGrp.getCmdQueue().push(pkt);
    }
    return false;
}

int CommandParserBindMove(CommandParser& cps, FbMoveWrap& fbGrp) {
    // cps.bind("", print_help);
    cps.bind("--help", [&](){
        cps.showHelp();
    });

    cps.bind("fault", [&fbGrp]() {
        // ServoSgFakerEager::Instance(0).set_fault();
        printf("fault \n");
    });

    cps.bind("clearAxisExecute", [&fbGrp]() {
        fbGrp.sendAxisLow();
        printf("clearAxisExecute \n");
    });

    cps.bind("clearAxisMotExecute", [&fbGrp]() {
        fbGrp.sendAxisMotLow();
        printf("clearAxisMotExecute \n");
    });
    cps.bind("cls", [&fbGrp]() {
        fbGrp.sendAxisMotLow();
        printf("clearAxisMotExecute \n");
    });

    cps.bind("reset", [&fbGrp]() {
        fbGrp.sendReset();
        printf("reset \n");
    });

    cps.bind("unreset", [&fbGrp]() {
        fbGrp.sendResetLow();
        printf("reset \n");
    });

    cps.bind("powerOn", [&fbGrp]() {
        fbGrp.setPowerOn();
        printf("powerOn\n");
    });

    cps.bind("powerOff", [&fbGrp]() {
        fbGrp.setPowerOff();
        printf("powerOff\n");
    });
    cps.bind("axis_enable", [&fbGrp](int enable) {
        fbGrp.setAxisEnable(enable);
        printf("axisEnable %d\n", enable);
    });
    cps.bind("moveabs", [&fbGrp](double p) {
        fbGrp.setCmdPos(p);
        fbGrp.sendExecuteMoveAbs(true);
        printf("moveabs \n");
    });
    cps.bind("moveabs_low", [&fbGrp]() {
        fbGrp.sendExecuteMoveAbs(false);
        printf("moveabs_low \n");
    });

    cps.bind("moverel", [&fbGrp](double p) {
        fbGrp.setCmdPos(p);
        fbGrp.sendExecuteMoveRel(true);
        printf("moverel \n");
    });

    cps.bind("moverel_low", [&fbGrp]() {
        fbGrp.sendExecuteMoveRel(false);
        printf("moverel_low \n");
    });

    cps.bind("move", [&fbGrp](double p) {
        fbGrp.setCmdPos(p);
        fbGrp.sendExecuteMoveRel(true);
        printf("moverel \n");
    });

    cps.bind("moveadd", [&fbGrp](double p) {
        fbGrp.setCmdPos(p);
        fbGrp.sendExecuteMoveAdd(true);
        printf("moveadd \n");
    });

    cps.bind("moveadd_low", [&fbGrp]() {
        fbGrp.sendExecuteMoveAdd(false);
        printf("movedd_low \n");
    });

    cps.bind("vmove", [&fbGrp](int sel) {
        fbGrp.setDirection(sel);
        fbGrp.sendExecuteMoveVel(true);
        printf("vmove \n");
    });
    cps.bind("forward", [&fbGrp]() {
        fbGrp.setCmdPos(1);
        fbGrp.sendExecuteMoveVel(true);
        printf("forward \n");
    });
    cps.bind("reverse", [&fbGrp]() {
        fbGrp.setCmdPos(-1);
        fbGrp.sendExecuteMoveVel(true);
        printf("reverse \n");
    });
    cps.bind("move_vel_low", [&fbGrp]() {
        fbGrp.sendExecuteMoveVel(false);
        printf("move_vel_low \n");
    });

    cps.bind("select", [&fbGrp](int sel) {
        fbGrp.setAxisSelect(sel);
        printf("select \n");
    });

    cps.bind("master", [&fbGrp](int sel) {
        fbGrp.setMasterAxis(sel);
        printf("master \n");
    });

    cps.bind("pos", [&fbGrp](double p) {
        fbGrp.setCmdPos(p);
        printf("pos \n");
    });

    cps.bind("direction", [&fbGrp](int p) {
        fbGrp.setDirection(p);
        printf("direction \n");
    });

    cps.bind("buffermode", [&fbGrp](int p) {
        fbGrp.setBufferMode(p);
        printf("buffermode \n");
    });

    cps.bind("units", [&fbGrp](double p) {
        fbGrp.sendUnits(p);
        printf("units \n");
    });
    cps.bind("speed", [&fbGrp](double p) {
        fbGrp.setCmdVel(p);
        printf("speed \n");
    });

    cps.bind("accel", [&fbGrp](double p) {
        fbGrp.setCmdAcc(p);
        printf("accel \n");
    });
    cps.bind("decel", [&fbGrp](double p) {
        fbGrp.setCmdDec(p);
        printf("decel \n");
    });
    cps.bind("sparam", [&fbGrp](double p) {
        fbGrp.setCmdJerk(p);
        printf("decel \n");
    });
    cps.bind("mparam", [&fbGrp]() { 
        fbGrp.show_mparam(); 
    });

    cps.bind("stop", [&fbGrp]() {
        fbGrp.sendStop();
        printf("stop \n");
    });

    cps.bind("stop_low", [&fbGrp]() {
        fbGrp.sendStopLow();
        printf("stop_low \n");
    });

    cps.bind("halt", [&fbGrp]() {
        fbGrp.sendHalt();
        printf("halt \n");
    });

    cps.bind("halt_low", [&fbGrp]() {
        fbGrp.sendHaltLow();
        printf("halt_low \n");
    });

    cps.bind("gear_in", [&fbGrp]() {
        fbGrp.sendGearIn();
        printf("gear_in\n");
    });

    cps.bind("gear_in_low", [&fbGrp]() {
        fbGrp.sendGearInLow();
        printf("gear_in_low\n");
    });

    cps.bind("gear_out", [&fbGrp]() {
        fbGrp.sendGearOut();
        printf("gear_out\n");
    });

    cps.bind("gear_out_low", [&fbGrp]() {
        fbGrp.sendGearOutLow();
        printf("gear_out_low\n");
    });

    cps.bind("defpos", [&fbGrp]() {
        fbGrp.sendSetPos();
        printf("defpos\n");
    });

    cps.bind("homing", [&fbGrp]() {
        fbGrp.sendHoming();
        printf("homing\n");
    });

    cps.bind("home", [&fbGrp]() {
        fbGrp.sendHome();
        printf("home\n");
    });

    cps.bind("home_low", [&fbGrp]() {
        fbGrp.sendHomeLow();
        printf("home_low\n");
    });

    cps.bind("home_stat", [&fbGrp](int sel) {
        fbGrp.show_home_fb_sg(sel);
    });
    
    cps.bind("p", [&fbGrp]() {
        fbGrp.show();
        printf("show\n");
    });
    cps.bind("show", [&fbGrp]() {
        fbGrp.show();
        printf("show\n");
    });

    cps.bind("state", [&fbGrp]() {
        fbGrp.show_state();
        printf("state\n");
    });

    cps.bind("cia402", [&fbGrp]() {
        fbGrp.show_power();
        printf("cia402\n");
    });

    cps.bind("motbuf", [&fbGrp]() {
        fbGrp.show_motbuf();
        printf("motbuf\n");
    });

    cps.bind("motnode", [&fbGrp]() {
        fbGrp.show_motnode();
        printf("motnode\n");
    }); 
    cps.bind("cmdresult", [&](){
        fbGrp.show_cmdresult();
        printf("cmdresult \n");
    });


    cps.bind("simu_manual", [&fbGrp](bool val) {
        fbGrp.setManualMode(val);
        printf("simu_manual %d\n", val);
    });
    cps.bind("simu_pause", [&fbGrp]() {
        fbGrp.setPauseFlag(true);
        printf("simu_pause\n");
    });
    cps.bind("simu_resume", [&fbGrp]() {
        fbGrp.setPauseFlag(false);
        printf("simu_resume\n");
    });
    // cps.bind("s", [&fbGrp]() {
    //     fbGrp.sendSteps(1);
    //     printf("step\n");
    // });

    cps.bind("s", [&fbGrp](const std::vector<std::string>& args) {
        int errCode = 1;
        int val = 1;
        if (args.size()){
            val = convertArg<int>(args[0], errCode);
            if (errCode) val = 1;
        }
        fbGrp.sendSteps(val);
        printf("step %d\n", val);
    });

    cps.bind("steps", [&fbGrp](int n) {
        fbGrp.sendSteps(n);
        printf("steps %d\n", n);
    });
    cps.bind("tickto", [&fbGrp](int n) {
        fbGrp.sendTickTo(n);
        printf("tickto %d\n", n);
    });
    cps.bind("timeto", [&fbGrp](double time) {
        fbGrp.sendTimeTo(time);
        printf("timeto %f\n", time);
    });
    cps.bind("delay", [&fbGrp](double time) {
        fbGrp.sendDelay(time);
        printf("delay %f\n", time);
    });
    cps.bind("exit", [&](){
        printf("exit \n");
        fbGrp.sendExitFlag();
        cps.setExitFlag();
    });

    return 0;
}

int runCmdLoop(FbMoveWrap& fbGrp) {
    CommandParser cps;
    CommandInteractor<CommandParser> clp(cps);
    CommandParserBindMove(cps, fbGrp);
    clp.run();
    // zmc::CmdInterLooper clp;
    // clp.run2([&cps](const std::string& st) {
    //     auto strIn = st;
    //     std::vector<std::string> args = stringSplit(strIn, ' ');
    //     int ret = cps.match(args);
    //     // std::cout<< st << " ready\n";
    //     return 0;
    // });
    return 0;
}
 
int runCmdLoop2(int argc, char** argv){
    // int axis_num = 1;
    auto pRunner = make_pou_config<AxisFbGraph>(argc, argv, "");
    if (!pRunner) return -1;

    auto& axfbGrp = *pRunner;
    CommandParser cps;
    CommandInteractor<CommandParser> clp(cps);
    std::thread runner([&](){
        axfbGrp.count_to_exit = 0;
        axfbGrp.run();
    });
    CommandParserBindMove(cps, axfbGrp.fbGrp);
    clp.setErrorHandlerDefault();
    clp.run();

    if (runner.joinable()) {
        runner.join();
    }
    return 0;
}


}  // namespace zmc