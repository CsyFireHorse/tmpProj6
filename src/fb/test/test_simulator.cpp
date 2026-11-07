#if 0
#include "../include/axfb/AxFbSimulator.h"
#include "../include/axfb/AxFbCmdSimulator.h"
#include "../include/axfb/AxFbHttpSimulator.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace zmc;

// 测试基本的AxFbSimulator功能
void testBasicSimulator() {
    std::cout << "=== Testing AxFbSimulator ===" << std::endl;
    
    // 创建基本仿真器实例
    AxFbSimulator simulator;
    
    // 配置仿真器
    RunConfig config;
    config.cycleTime = 0.001;   // 1ms cycle time
    config.exitCount = 0;       // Run indefinitely
    config.verbose = 1;         // Enable basic verbosity
    config.verboseModulo = 100; // Print status every 100 ticks
    simulator.setRunConfig(config);
    
    // 创建轴和功能块
    simulator.createAxis(static_cast<AX_FACTORY_TYPE>(0)); // Create axis 0
    simulator.createAxis(static_cast<AX_FACTORY_TYPE>(1)); // Create axis 1
    simulator.createFb(static_cast<FB_TYPE>(0)); // Create function block 0
    
    // 运行仿真
    std::cout << "Running simulation for 0.5 seconds..." << std::endl;
    simulator.delayTime(0.5);
    std::cout << "Current time: " << simulator.getCurrentTime() << " seconds" << std::endl;
    
    // 运行到特定时间
    std::cout << "Running to time 1.0 seconds..." << std::endl;
    simulator.runToTime(1.0);
    std::cout << "Current time: " << simulator.getCurrentTime() << " seconds" << std::endl;
    
    std::cout << "Basic simulator test completed." << std::endl << std::endl;
}

// 测试命令行交互式仿真器
void testCmdSimulator() {
    std::cout << "=== Testing AxFbCmdSimulator ===" << std::endl;
    
    // 创建命令行仿真器实例
    AxFbCmdSimulator cmdSimulator;
    
    // 配置仿真器
    RunConfig config;
    config.cycleTime = 0.001;
    config.exitCount = 0;
    config.verbose = 1;
    config.verboseModulo = 100;
    cmdSimulator.setRunConfig(config);
    
    // 创建轴和功能块
    cmdSimulator.createAxis(static_cast<AX_FACTORY_TYPE>(0));
    cmdSimulator.createFb(static_cast<FB_TYPE>(0));
    
    // 运行一些周期
    std::cout << "Running 10 cycles..." << std::endl;
    for (int i = 0; i < 10; i++) {
        cmdSimulator.runCycle();
    }
    std::cout << "Current time: " << cmdSimulator.getCurrentTime() << " seconds" << std::endl;
    
    // 启动命令行交互
    std::cout << "Starting command line interface..." << std::endl;
    std::cout << "Type 'exit' to continue with the test." << std::endl;
    cmdSimulator.runCommandInteractor("Command Line Simulator Test - Type 'help' for available commands");
    
    std::cout << "Command simulator test completed." << std::endl << std::endl;
}

// 测试HTTP仿真器
void testHttpSimulator() {
    std::cout << "=== Testing AxFbHttpSimulator ===" << std::endl;
    
    // 创建HTTP仿真器实例
    AxFbHttpSimulator::HttpConfig httpConfig;
    httpConfig.port = 8080;
    AxFbHttpSimulator httpSimulator(httpConfig);
    
    // 配置仿真器
    RunConfig config;
    config.cycleTime = 0.001;
    config.exitCount = 0;
    config.verbose = 1;
    config.verboseModulo = 100;
    httpSimulator.setRunConfig(config);
    
    // 创建轴和功能块
    httpSimulator.createAxis(static_cast<AX_FACTORY_TYPE>(0));
    httpSimulator.createFb(static_cast<FB_TYPE>(0));
    
    // 启动HTTP服务器
    std::cout << "Starting HTTP server..." << std::endl;
    httpSimulator.startServer();
    
    // 运行一些周期
    std::cout << "Running simulation for 0.5 seconds..." << std::endl;
    httpSimulator.delayTime(0.5);
    std::cout << "Current time: " << httpSimulator.getCurrentTime() << " seconds" << std::endl;
    
    // 保持服务器运行一段时间
    std::cout << "HTTP server is running. Press Enter to stop..." << std::endl;
    std::cin.get();
    
    // 停止HTTP服务器
    httpSimulator.stopServer();
    
    std::cout << "HTTP simulator test completed." << std::endl << std::endl;
}

int main(int argc, char** argv) {
    // 测试基本仿真器
    testBasicSimulator();
    
    // 测试命令行仿真器
    testCmdSimulator();
    
    // 测试HTTP仿真器
    testHttpSimulator();
    
    std::cout << "All tests completed." << std::endl;
    return 0;
} 
#endif