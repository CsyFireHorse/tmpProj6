#include "axfb/AxFbCmdSimulator.h"
#include <iostream>
#include <iomanip>
#include "common/misc/CommandInteractor.h"

namespace zmc {

AxFbCmdSimulator::AxFbCmdSimulator() : AxFbCmdPlayer() {
    initCommandParser();
}

AxFbCmdSimulator::~AxFbCmdSimulator() {
    // No additional cleanup needed
}

void AxFbCmdSimulator::initCommandParser() {
    registerCommands();
}

void AxFbCmdSimulator::printWelcomeMsg()const{
    // Welcome message
    std::cout << "===== AxFbCmdSimulator Demo =====" << std::endl;
    std::cout << "Type 'help' for a list of commands" << std::endl;
    std::cout << std::endl;
    std::cout << "Available commands include:" << std::endl;
    std::cout << "  step        - Execute one simulation cycle" << std::endl;
    std::cout << "  steps N     - Execute N simulation cycles" << std::endl;
    std::cout << "  delay T     - Run simulation for T seconds" << std::endl;
    std::cout << "  timeto T    - Run simulation until time T seconds" << std::endl;
    std::cout << "  tickto N    - Run simulation until tick N" << std::endl;
    std::cout << "  realtime X  - Enable/disable real-time mode (true/false)" << std::endl;
    std::cout << "  rtscale N D - Set real-time scale to N/D" << std::endl;
    std::cout << "  exit        - Exit simulator" << std::endl;
    std::cout << std::endl;
}

void AxFbCmdSimulator::registerCommands() {
    // Basic help and control commands
    _cmdParser.bind("help", [this]() {
        std::cout << "Available commands:" << std::endl;
        for (const auto& cmd : _cmdParser.getRegisteredCommands()) {
            std::string help = _cmdParser.getCommandHelp(cmd);
            std::cout << "  " << cmd << " - " << help << std::endl;
        }
    }, "Display list of available commands");
    
    _cmdParser.bind("exit", [this]() {
        _cmdParser.setExitFlag();
        std::cout << "Exiting simulator..." << std::endl;
    }, "Exit the simulator");
    
    // Standard run commands
    _cmdParser.bind("run", [this]() {
        std::cout << "Starting simulator run..." << std::endl;
        run();
    }, "Run the simulator continuously until exit condition");
    
    _cmdParser.bind("cycle", [this]() {
        step();
        std::cout << "Cycle executed. Tick: " << getCurrentTick() << ", Time: " 
                  << (getCurrentTick() * _runCfg.cycleTime) << " s" << std::endl;
    }, "Run a single simulation cycle");
    
    // Time control commands (manual mode)
    _cmdParser.bind("s", [this]() {
        step();
        std::cout << "Step executed. Tick: " << getCurrentTick() << std::endl;
    }, "Run a single cycle");
    _cmdParser.bind("step", [this]() {
        step();
        std::cout << "Step executed. Tick: " << getCurrentTick() << std::endl;
    }, "Run a single cycle");
    
    _cmdParser.bind("steps", [this](int numSteps) {
        steps(numSteps);
        std::cout << "Executed " << numSteps << " steps. Current tick: " 
                  << getCurrentTick() << std::endl;
    }, "Run multiple cycles");
    
    _cmdParser.bind("tickto", [this](int targetTick) {
        std::cout << "Running to tick " << targetTick << "..." << std::endl;
        tickto(targetTick);
        std::cout << "Current tick: " << getCurrentTick() << std::endl;
    }, "Run simulation until specified tick");
    
    _cmdParser.bind("delay", [this](double time) {
        std::cout << "Delaying for " << time << " s..." << std::endl;
        delay(time);
        std::cout << "Current tick: " << getCurrentTick() << std::endl;
    }, "Delay for specified time in seconds");
    
    _cmdParser.bind("timeto", [this](double targetTime) {
        std::cout << "Running to time " << targetTime << " s..." << std::endl;
        timeto(targetTime);
        std::cout << "Current time: " << (getCurrentTick() * _runCfg.cycleTime) << " s" << std::endl;
    }, "Run simulation to specified time in seconds");
    
    // Real-time mode commands
    _cmdParser.bind("realtime", [this](bool enable) {
        if (enable) {
            set_delay_by_real_time(true);
            std::cout << "Real-time mode enabled" << std::endl;
        } else {
            set_delay_by_real_time(false);
            std::cout << "Real-time mode disabled" << std::endl;
        }
    }, "Enable or disable real-time simulation mode (true/false)");
    
    _cmdParser.bind("u", [this]() {
        delay_by_real_time();
        std::cout << "Step executed. Tick: " << getCurrentTick() << std::endl;
    }, "Run cycles");

    _cmdParser.bind("rtscale", [this](int num, int den) {
        set_real_time_num(num);
        set_real_time_den(den);
        std::cout << "Real-time scale set to " << num << "/" << den << std::endl;
    }, "Set real-time scale factor as numerator/denominator");
    
    // Axis and function block creation
    _cmdParser.bind("createaxis", [this](int type) {
        createAxis(static_cast<AX_FACTORY_TYPE>(type));
        std::cout << "Axis created with type " << type << std::endl;
    }, "Create an axis with specified type");
    
    _cmdParser.bind("createfb", [this](int type) {
        createFb(static_cast<FB_TYPE>(type));
        std::cout << "Function block created with type " << type << std::endl;
    }, "Create a function block with specified type");
    

    _cmdParser.bind("pushcmd", [this](int fbId, int attr, int action, double val) {
        // , int tick
        CommandStruct cmdStruct;
        cmdStruct.fbId = fbId;
        cmdStruct.attr = attr;
        cmdStruct.action = action;
        cmdStruct.tick = 0;
        cmdStruct.val = val;
        pushCommand(cmdStruct);
        std::cout << "Command pushed: " << cmdStruct.fbId << "," << cmdStruct.attr << "," << cmdStruct.action << "," << cmdStruct.tick << "," << cmdStruct.val << std::endl;
    }, "Push a command to the simulator");

    // Configuration settings
    _cmdParser.bind("setcycletime", [this](double time) {
        RunConfig config = _runCfg;
        config.cycleTime = time;
        set_run_config(config);
        std::cout << "Cycle time set to " << time << " s" << std::endl;
    }, "Set simulation cycle time in seconds");
    
    _cmdParser.bind("verbose", [this](int level, int modulo) {
        RunConfig config = _runCfg;
        config.verbose = level;
        config.verboseModulo = modulo;
        set_run_config(config);
        std::cout << "Verbosity set to level " << level 
                  << " with modulo " << modulo << std::endl;
    }, "Set verbosity level and print modulo");

    _cmdParser.bind("quickInstall", [this]() {
        createAxis(AX_FACTORY_VIRTUAL);
        
    }, "Quick install all axes");
}

void AxFbCmdSimulator::runCommandInteractor(const std::string& welcomeMsg) {
    CommandInteractor<CommandParser> interactor(_cmdParser, "simulator> ");
    
    // Set error handler
    interactor.setErrorHandler([](CommandParser::ErrorCode error) {
        switch (error) {
            case CommandParser::COMMAND_NOT_FOUND:
                std::cout << "Error: Command not found" << std::endl;
                break;
            case CommandParser::INVALID_ARGUMENT_COUNT:
                std::cout << "Error: Invalid number of arguments" << std::endl;
                break;
            case CommandParser::ARGUMENT_CONVERSION_ERROR:
                std::cout << "Error: Could not convert argument to required type" << std::endl;
                break;
            case CommandParser::EMPTY_COMMAND:
                // Ignore empty commands
                break;
            case CommandParser::EXECUTION_ERROR:
                std::cout << "Error: Command execution failed" << std::endl;
                break;
            default:
                std::cout << "Error: Unknown error" << std::endl;
        }
    });
    
    // Run the interactor
    interactor.run(welcomeMsg);
}

} // namespace zmc 