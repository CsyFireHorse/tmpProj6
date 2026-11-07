#include "axfb/AxFbCmdSimulator.h"
#include <iostream>

using namespace zmc;

int axfb_cmd_simulator_demo(int argc, char* argv[]) {
    // Create simulator instance
    AxFbCmdSimulator simulator;
    
    // Setup initial configuration
    RunConfig config;
    config.cycleTime = 0.001;  // 1ms cycle time
    config.exitCount = 0;      // No automatic exit
    config.verbose = 1;
    config.verboseModulo = 10;
    simulator.set_run_config(config);
    // Create a virtual axis by default
    simulator.createAxis(AX_FACTORY_VIRTUAL);
    
    simulator.printWelcomeMsg();
    
    // Start interactive command interface
    simulator.runCommandInteractor("AxFbCmdSimulator initialized. Ready for commands.");
    
    return 0;
} 