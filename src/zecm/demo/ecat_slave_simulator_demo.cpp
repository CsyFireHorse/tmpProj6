/*
 * Example: How to use EcatSlaveSimulatorService
 * 
 * This is a standalone service that simulates EtherCAT slaves.
 * Run this before starting the master (zecmFak).
 */

#include "EcatSlaveSimulatorService.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


int run_servo_simulator(int argc, char* argv[]) {
    int port = ECAT_SLAVE_SIMULATOR_DEFAULT_PORT;
    
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) {
            port = atoi(argv[i + 1]);
            break;
        }
    }

    
    EcatSlaveSimulatorService service;
    
    // Start service (configuration will be done during handshake)
    if (service.start(port) != 0) {
        printf("Failed to start service on port %d\n", port);
        return 1;
    }
    
    printf("EcatSlaveSimulatorService running on port %d\n", port);
    printf("Waiting for client connection and configuration...\n");
    printf("Press Ctrl+C to stop...\n");
    
    // Run service (blocking)
    service.run();
    
    service.stop();
    printf("Service stopped\n");
    
    return 0;
}
