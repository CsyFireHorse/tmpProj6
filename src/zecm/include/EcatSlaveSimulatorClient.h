#ifndef _ECAT_SLAVE_SIMULATOR_CLIENT_H_
#define _ECAT_SLAVE_SIMULATOR_CLIENT_H_

#include "stdint.h"
#include "EcatSlaveSimulatorProtocol.h"


class EcatSlaveSimulatorClient {
public:
    EcatSlaveSimulatorClient();
    ~EcatSlaveSimulatorClient();
    
    // Try to connect (handshake) and send configuration
    void set_config(int port = ECAT_SLAVE_SIMULATOR_DEFAULT_PORT, const char* eni_file = nullptr, uint32_t rxpdoLen = 4096, uint32_t txpdoLen = 4096);
    int connect();
    int _sendConfig();
    // Disconnect gracefully
    void disconnect();
    void fin();
    
    // Check if connected
    bool isConnected() const;
    
    // PDO preparation (priority)
    int preparePdo(uint8_t* pdoInput, uint8_t* pdoOutput);
    
    // SDO read/write (future implementation)
    int sdoRead(uint16_t slaveId, uint16_t index, uint8_t subindex,
                uint8_t* data, uint32_t* dataLen, uint32_t maxLen);
    int sdoWrite(uint16_t slaveId, uint16_t index, uint8_t subindex,
                 const uint8_t* data, uint32_t dataLen);
    
private:
    // Socket state is hidden to keep platform headers out of this public API.
    struct Impl;
    Impl* impl;

    EcatSlaveSimulatorClient(const EcatSlaveSimulatorClient&);
    EcatSlaveSimulatorClient& operator=(const EcatSlaveSimulatorClient&);
};

#endif
