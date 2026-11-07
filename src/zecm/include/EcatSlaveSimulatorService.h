#ifndef _ECAT_SLAVE_SIMULATOR_SERVICE_H_
#define _ECAT_SLAVE_SIMULATOR_SERVICE_H_

#include "stdint.h"
#include "EcatSlaveSimulatorProtocol.h"

class EcatSlaveSimulatorService {
public:
    EcatSlaveSimulatorService();
    ~EcatSlaveSimulatorService();
    
    // Start service (single thread)
    int start(int port = ECAT_SLAVE_SIMULATOR_DEFAULT_PORT);
    
    // Stop service
    void stop();
    
    // Run service loop (blocking)
    void run();
    
private:
    // Runtime state is hidden to keep socket and PDO internals out of this API.
    struct Impl;
    Impl* impl;

    EcatSlaveSimulatorService(const EcatSlaveSimulatorService&);
    EcatSlaveSimulatorService& operator=(const EcatSlaveSimulatorService&);
};

#endif
