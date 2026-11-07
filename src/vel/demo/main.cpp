#include "stdio.h"
#include "string.h"

#include "common/misc/CommandDispatcher.h"

void VelFollower_demo();
void VelFollower_low_acc_demo();
void vel_sin_demo();
void velTrapz_demo();
void velTrapz_invert_demo();

void VelPosFollower_demo();
void VelPosFollower_nonzero_pos_demo();
void VelPosFollower_nonzero_vel_demo();

void velToyTrapz_demo();
void VelToyPosFollower_demo();
int VelToyPouFollower_demo(int argc, char** argv); 
int VelToyPouPosFollower_demo(int argc, char** argv);
int VelToyPouPvFollower_demo(int argc, char** argv);
int VelToyPouPvaFollower_demo(int argc, char** argv);

void lookaheadV2_demo();
void lookahead_vel_demo();

void velQueueDemo();
int cli_main(int argc, char** argv);
void vel_forwarder2_demo();

static const char* EXE_NAME = "zmc_vel_demo";

int printHelp() {
    printf(" --version\n"
        " cli\n"
        " follower\n"
        " follower_low_acc\n"
        " pou_follower\n"
        " pou_pos_follower\n"
        " pou_pv_follower\n"
        " pou_pva_follower\n"
        " --help\n");
    return 0;
}

int printVersion() {
    printf("%s: velocity demo tool\n"
           "version 1.0.0\n"
           "create by csy\n",
           EXE_NAME);
    return 0;
}

int main(int argc, char** argv){
    zmc::CommandDispatcher dispatcher;
    dispatcher.bind("--help", zmc::makeVoidHandler(printHelp));
    dispatcher.bind("-h", zmc::makeVoidHandler(printHelp));
    dispatcher.bind("--version", zmc::makeVoidHandler(printVersion));
    dispatcher.bind("-V", zmc::makeVoidHandler(printVersion));

    dispatcher.bind("cli", &cli_main);
    dispatcher.bind("forwarder", zmc::makeVoidHandler(vel_forwarder2_demo));
    dispatcher.bind("trapz", zmc::makeVoidHandler(velTrapz_demo));
    dispatcher.bind("trapz_inv", zmc::makeVoidHandler(velTrapz_invert_demo));
    dispatcher.bind("queue", zmc::makeVoidHandler(velQueueDemo));
    dispatcher.bind("follower", zmc::makeVoidHandler(VelFollower_demo));
    dispatcher.bind("follower_low_acc", zmc::makeVoidHandler(VelFollower_low_acc_demo));
    dispatcher.bind("pos_follower", zmc::makeVoidHandler(VelPosFollower_demo));
    dispatcher.bind("pos_follower_nonzero_pos", zmc::makeVoidHandler(VelPosFollower_nonzero_pos_demo));
    dispatcher.bind("pos_follower_nonzero_vel", zmc::makeVoidHandler(VelPosFollower_nonzero_vel_demo));
    dispatcher.bind("sin", zmc::makeVoidHandler(vel_sin_demo));
    dispatcher.bind("pou_follower", &VelToyPouFollower_demo);
    dispatcher.bind("pou_pos_follower", &VelToyPouPosFollower_demo);
    dispatcher.bind("pou_pv_follower", &VelToyPouPvFollower_demo);
    dispatcher.bind("pou_pva_follower", &VelToyPouPvaFollower_demo);
    dispatcher.bind("lookahead", zmc::makeVoidHandler(lookahead_vel_demo));
    dispatcher.bind("lookahead_v2", zmc::makeVoidHandler(lookaheadV2_demo));
    dispatcher.setDefaultHandler([](const zmc::CommandDispatcher::ArgList&) -> int {
        return printHelp();
    });

    int ret = dispatcher.parse(argc, argv);
    if (ret == zmc::CommandDispatcher::ERR_NOT_FOUND) {
        printHelp();
        return 0;
    }
    return ret;
}
