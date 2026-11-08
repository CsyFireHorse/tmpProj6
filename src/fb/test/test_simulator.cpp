#if 0
#include "../include/axfb/AxFbSimulator.h"
#include "../include/axfb/AxFbCmdSimulator.h"
#include "../include/axfb/AxFbHttpSimulator.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace zmc;

// Test basic AxFbSimulator functionality.
void testBasicSimulator() {
    std::cout << "=== Testing AxFbSimulator ===" << std::endl;
    
    // Create a basic simulator instance.
    AxFbSimulator simulator;
    
    // Configure the simulator.
    RunConfig config;
    config.cycleTime = 0.001;   // 1ms cycle time
    config.exitCount = 0;       // Run indefinitely
    config.verbose = 1;         // Enable basic verbosity
    config.verboseModulo = 100; // Print status every 100 ticks
    simulator.setRunConfig(config);
    
    // Create an axis and function block.
    simulator.createAxis(static_cast<AX_FACTORY_TYPE>(0)); // Create axis 0
    simulator.createAxis(static_cast<AX_FACTORY_TYPE>(1)); // Create axis 1
    simulator.createFb(static_cast<FB_TYPE>(0)); // Create function block 0
    
    // Run the simulation.
    std::cout << "Running simulation for 0.5 seconds..." << std::endl;
    simulator.delayTime(0.5);
    std::cout << "Current time: " << simulator.getCurrentTime() << " seconds" << std::endl;
    
    // Run until a specified time.
    std::cout << "Running to time 1.0 seconds..." << std::endl;
    simulator.runToTime(1.0);
    std::cout << "Current time: " << simulator.getCurrentTime() << " seconds" << std::endl;
    
    std::cout << "Basic simulator test completed." << std::endl << std::endl;
}

// Test the interactive command-line simulator.
void testCmdSimulator() {
    std::cout << "=== Testing AxFbCmdSimulator ===" << std::endl;
    
    // Create a command-line simulator instance.
    AxFbCmdSimulator cmdSimulator;
    
    // Configure the simulator.
    RunConfig config;
    config.cycleTime = 0.001;
    config.exitCount = 0;
    config.verbose = 1;
    config.verboseModulo = 100;
    cmdSimulator.setRunConfig(config);
    
    // Create an axis and function block.
    cmdSimulator.createAxis(static_cast<AX_FACTORY_TYPE>(0));
    cmdSimulator.createFb(static_cast<FB_TYPE>(0));
    
    // Run a few cycles.
    std::cout << "Running 10 cycles..." << std::endl;
    for (int i = 0; i < 10; i++) {
        cmdSimulator.runCycle();
    }
    std::cout << "Current time: " << cmdSimulator.getCurrentTime() << " seconds" << std::endl;
    
    // Start the command-line interaction.
    std::cout << "Starting command line interface..." << std::endl;
    std::cout << "Type 'exit' to continue with the test." << std::endl;
    cmdSimulator.runCommandInteractor("Command Line Simulator Test - Type 'help' for available commands");
    
    std::cout << "Command simulator test completed." << std::endl << std::endl;
}

// Test the HTTP simulator.
void testHttpSimulator() {
    std::cout << "=== Testing AxFbHttpSimulator ===" << std::endl;
    
    // Create an HTTP simulator instance.
    AxFbHttpSimulator::HttpConfig httpConfig;
    httpConfig.port = 8080;
    AxFbHttpSimulator httpSimulator(httpConfig);
    
    // Configure the simulator.
    RunConfig config;
    config.cycleTime = 0.001;
    config.exitCount = 0;
    config.verbose = 1;
    config.verboseModulo = 100;
    httpSimulator.setRunConfig(config);
    
    // Create an axis and function block.
    httpSimulator.createAxis(static_cast<AX_FACTORY_TYPE>(0));
    httpSimulator.createFb(static_cast<FB_TYPE>(0));
    
    // Start the HTTP server.
    std::cout << "Starting HTTP server..." << std::endl;
    httpSimulator.startServer();
    
    // Run a few cycles.
    std::cout << "Running simulation for 0.5 seconds..." << std::endl;
    httpSimulator.delayTime(0.5);
    std::cout << "Current time: " << httpSimulator.getCurrentTime() << " seconds" << std::endl;
    
    // Keep the server running for a while.
    std::cout << "HTTP server is running. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    // Stop the HTTP server.
    httpSimulator.stopServer();
    
    std::cout << "HTTP simulator test completed." << std::endl << std::endl;
}

int main(int argc, char** argv) {
    // Test the basic simulator.
    testBasicSimulator();
    
    // Test the command-line simulator.
    testCmdSimulator();
    
    // Test the HTTP simulator.
    testHttpSimulator();
    
    std::cout << "All tests completed." << std::endl;
    return 0;
} 
#endif
