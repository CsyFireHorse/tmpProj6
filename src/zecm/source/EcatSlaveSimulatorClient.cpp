#include "../include/EcatSlaveSimulatorClient.h"
#include "simple_socket.h"
#include "stdio.h"
#include "string.h"
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

// Convert to network byte order (big-endian)
static uint32_t htonl_local(uint32_t hostlong) {
    return htonl(hostlong);
}

// Convert from network byte order
static uint32_t ntohl_local(uint32_t netlong) {
    return ntohl(netlong);
}

struct EcatSlaveSimulatorClient::Impl {
    Impl()
        : clientFd(INVALID_SOCKET),
          connected(false),
          port(ECAT_SLAVE_SIMULATOR_DEFAULT_PORT),
          eni_file(nullptr),
          rxpdoLen(0),
          txpdoLen(0) {
    }

    SOCKET clientFd;
    bool connected;
    int port;
    const char* eni_file;
    uint32_t rxpdoLen;
    uint32_t txpdoLen;

    // Protocol helper functions
    int sendRequest(uint8_t cmd, const uint8_t* data, uint32_t dataLen);
    int recvResponse(uint8_t expectedAck, uint8_t* errorCode, uint8_t* data, uint32_t* dataLen, uint32_t maxDataLen);
};

EcatSlaveSimulatorClient::EcatSlaveSimulatorClient()
    : impl(new Impl()) {
    SimpleSocket::init();
}

EcatSlaveSimulatorClient::~EcatSlaveSimulatorClient() {
    disconnect();
    delete impl;
    impl = nullptr;
}

void EcatSlaveSimulatorClient::set_config(int port , const char* eni_file, uint32_t rxpdoLen, uint32_t txpdoLen){
    impl->port = port;
    impl->eni_file = eni_file;
    impl->rxpdoLen = rxpdoLen;
    impl->txpdoLen = txpdoLen;
}

int EcatSlaveSimulatorClient::connect() {
    if (impl->connected) return ECAT_RESULT_SUCCESS;  // Use ECAT_RESULT_SUCCESS instead of ECAT_S_OK
    
    int ret = ECAT_RESULT_SUCCESS;
    uint8_t errorCode = 0;  // Declare at function start to avoid goto issues
    
    impl->clientFd = SimpleSocket::createClient("127.0.0.1", impl->port);
    
    if (impl->clientFd == INVALID_SOCKET) {
        impl->connected = false;
        return ECAT_S_INVALID_SOCKET;
    }
    
    // Handshake: Send HELLO request
    ret = impl->sendRequest(ECAT_CMD_HELLO, nullptr, 0);
    if (ret != ECAT_RESULT_SUCCESS) {
        printf("[Client] sendRequest: ret = %d\n", ret);
        goto failed;
    }
    
    // Receive HELLO_ACK response
    ret = impl->recvResponse(ECAT_CMD_HELLO_ACK, &errorCode, nullptr, nullptr, 0);
    if (ret != ECAT_RESULT_SUCCESS || errorCode != ECAT_RESULT_SUCCESS) {
        printf("[Client] Handshake failed: ret=%d, errorCode=0x%02X\n", ret, errorCode);
        ret = (ret != ECAT_RESULT_SUCCESS) ? ret : ECAT_S_RECV_FAILED;
        goto failed;
    }
    
    impl->connected = true;
    printf("[Client] Handshake successful\n");
    
    // Send configuration
    ret = _sendConfig();
    if (ret != ECAT_RESULT_SUCCESS) {
        goto failed;
    }
    
    return ECAT_RESULT_SUCCESS;

    failed:
    if (impl->clientFd != INVALID_SOCKET) {
        closesocket(impl->clientFd);
        impl->clientFd = INVALID_SOCKET;
    }
    impl->connected = false;
    return ret;
}

int EcatSlaveSimulatorClient::_sendConfig(){
    // Send configuration request if eni_file is provided
    if (!impl->eni_file) {
        return ECAT_S_CONFIG_FAILED;
    }
    if (!impl->connected) {
        return ECAT_S_NOT_CONNECTED;
    }
    
    // Build payload: [rxpdoLen:4][txpdoLen:4][eniLen:4][eniFile:eniLen]
    uint32_t eniLen = (uint32_t)strlen(impl->eni_file);
    uint32_t payloadLen = 4 + 4 + 4 + eniLen;
    uint8_t payload[1024 + 12];  // Max ENI path + 3 uint32_t
    
    uint32_t* p = (uint32_t*)payload;
    p[0] = htonl_local(impl->rxpdoLen);
    p[1] = htonl_local(impl->txpdoLen);
    p[2] = htonl_local(eniLen);
    memcpy(payload + 12, impl->eni_file, eniLen);
    
    // Send CONFIG request
    int ret = impl->sendRequest(ECAT_CMD_CONFIG, payload, payloadLen);
    if (ret != ECAT_RESULT_SUCCESS) {
        return ret;
    }
    
    // Receive CONFIG_ACK response
    uint8_t errorCode = 0;
    ret = impl->recvResponse(ECAT_CMD_CONFIG_ACK, &errorCode, nullptr, nullptr, 0);
    if (ret != ECAT_RESULT_SUCCESS) {
        return ret;
    }
    
    if (errorCode != ECAT_RESULT_SUCCESS) {
        printf("[Client] Configuration failed: errorCode=0x%02X\n", errorCode);
        return ECAT_S_CONFIG_FAILED;
    }
    
    printf("[Client] Configuration successful\n");
    return ECAT_RESULT_SUCCESS;
}

void EcatSlaveSimulatorClient::disconnect() {
    if (impl->clientFd != INVALID_SOCKET) {
        closesocket(impl->clientFd);
        impl->clientFd = INVALID_SOCKET;
    }
    impl->connected = false;
}

bool EcatSlaveSimulatorClient::isConnected() const {
    return impl->connected;
}

// Protocol helper: Send request [CMD:1][pad:1][LEN:4][DATA:LEN]
int EcatSlaveSimulatorClient::Impl::sendRequest(uint8_t cmd, const uint8_t* data, uint32_t dataLen) {
    // Allow HELLO command to be sent before connection is established
    if (cmd != ECAT_CMD_HELLO) {
        if (!connected || clientFd == INVALID_SOCKET) {
            return ECAT_S_NOT_CONNECTED;
        }
    } else {
        // For HELLO, only check socket validity
        if (clientFd == INVALID_SOCKET) {
            return ECAT_S_INVALID_SOCKET;
        }
    }
    
    uint8_t pad = 0;
    uint32_t len = htonl_local(dataLen);
    
    // printf("[Client] Send CMD=0x%02X, pad=0x%02X, LEN=%u\n", cmd, pad, dataLen);
    
    if (SimpleSocket::sendAll(clientFd, &cmd, 1) != 1) {
        return ECAT_S_SEND_FAILED;
    }
    
    if (SimpleSocket::sendAll(clientFd, &pad, 1) != 1) {
        return ECAT_S_SEND_FAILED;
    }
    
    if (SimpleSocket::sendAll(clientFd, (uint8_t*)&len, 4) != 4) {
        return ECAT_S_SEND_FAILED;
    }
    
    if (dataLen > 0 && data != nullptr) {
        if (SimpleSocket::sendAll(clientFd, data, dataLen) != dataLen) {
            return ECAT_S_SEND_FAILED;
        }
    }
    
    return ECAT_RESULT_SUCCESS;
}

// Protocol helper: Receive response [CMD_ACK:1][errorCode:1][LEN:4][DATA:LEN]
int EcatSlaveSimulatorClient::Impl::recvResponse(uint8_t expectedAck, uint8_t* errorCode, uint8_t* data, uint32_t* dataLen, uint32_t maxDataLen) {
    // Allow HELLO_ACK to be received before connection is established
    if (expectedAck != ECAT_CMD_HELLO_ACK) {
        if (!connected || clientFd == INVALID_SOCKET) {
            return ECAT_S_NOT_CONNECTED;
        }
    } else {
        // For HELLO_ACK, only check socket validity
        if (clientFd == INVALID_SOCKET) {
            return ECAT_S_INVALID_SOCKET;
        }
    }
    
    uint8_t ackCmd = 0;
    uint8_t errCode = 0;
    uint32_t len = 0;
    
    if (SimpleSocket::recvAll(clientFd, &ackCmd, 1) != 1) {
        return ECAT_S_RECV_FAILED;
    }
    
    if (SimpleSocket::recvAll(clientFd, &errCode, 1) != 1) {
        return ECAT_S_RECV_FAILED;
    }
    
    if (SimpleSocket::recvAll(clientFd, (uint8_t*)&len, 4) != 4) {
        return ECAT_S_RECV_FAILED;
    }
    
    len = ntohl_local(len);
    
    // printf("[Client] Recv ACK=0x%02X, errorCode=0x%02X, LEN=%u\n", ackCmd, errCode, len);
    
    if (ackCmd != expectedAck) {
        printf("[Client] Error: Expected ACK=0x%02X, got 0x%02X\n", expectedAck, ackCmd);
        return ECAT_S_INVALID_CMD;
    }
    
    if (errorCode) {
        *errorCode = errCode;
    }
    
    if (len > maxDataLen) {
        printf("[Client] Error: Data length %u exceeds max %u\n", len, maxDataLen);
        return ECAT_S_RECV_OVERFLOW;
    }
    
    if (len > 0 && data != nullptr && dataLen != nullptr) {
        if (SimpleSocket::recvAll(clientFd, data, len) != len) {
            return ECAT_S_RECV_FAILED;
        }
        *dataLen = len;
    } else if (len > 0) {
        // Skip data if buffer not provided
        uint8_t dummy[4096];
        uint32_t toSkip = len;
        while (toSkip > 0) {
            uint32_t chunk = (toSkip > sizeof(dummy)) ? sizeof(dummy) : toSkip;
            if (SimpleSocket::recvAll(clientFd, dummy, chunk) != chunk) {
                return ECAT_S_RECV_FAILED;
            }
            toSkip -= chunk;
        }
    }
    
    return ECAT_RESULT_SUCCESS;
}



int EcatSlaveSimulatorClient::preparePdo(uint8_t* rxpdoBuf, uint8_t* txpdoBuf) {
    if (!impl->connected) return ECAT_S_NOT_CONNECTED;
    
    // Build payload: [rxpdoLen:4][rxpdoBuf:rxpdoLen]
    uint32_t payloadLen = 4 + impl->rxpdoLen;
    uint8_t payload[4096 + 4];
    
    uint32_t* p = (uint32_t*)payload;
    p[0] = htonl_local(impl->rxpdoLen);
    memcpy(payload + 4, rxpdoBuf, impl->rxpdoLen);
    
    // Send PDO request
    int ret = impl->sendRequest(ECAT_CMD_PDO, payload, payloadLen);
    if (ret != ECAT_RESULT_SUCCESS) {
        disconnect();
        return ret;
    }
    
    // Receive PDO_ACK response: [txpdoLen:4][txpdoBuf:txpdoLen]
    uint8_t errorCode = 0;
    uint8_t responseBuf[4096 + 4];
    uint32_t responseLen = 0;
    
    ret = impl->recvResponse(ECAT_CMD_PDO_ACK, &errorCode, responseBuf, &responseLen, sizeof(responseBuf));
    if (ret != ECAT_RESULT_SUCCESS) {
        disconnect();
        return ret;
    }
    
    if (errorCode != ECAT_RESULT_SUCCESS) {
        printf("[Client] PDO request failed: errorCode=0x%02X\n", errorCode);
        return ECAT_S_RECV_FAILED;
    }
    
    // Parse response: [txpdoLen:4][txpdoBuf:txpdoLen]
    if (responseLen < 4) {
        printf("[Client] Invalid PDO response length: %u\n", responseLen);
        return ECAT_S_RECV_FAILED;
    }
    
    uint32_t outputLen = ntohl_local(*(uint32_t*)responseBuf);
    if (outputLen > impl->txpdoLen) {
        printf("[Client] PDO output length %u exceeds max %u\n", outputLen, impl->txpdoLen);
        return ECAT_S_RECV_OVERFLOW;
    }
    
    if (responseLen != 4 + outputLen) {
        printf("[Client] PDO response length mismatch: expected %u, got %u\n", 4 + outputLen, responseLen);
        return ECAT_S_RECV_FAILED;
    }
    
    memcpy(txpdoBuf, responseBuf + 4, outputLen);
    return ECAT_RESULT_SUCCESS;
}

int EcatSlaveSimulatorClient::sdoRead(uint16_t slaveId, uint16_t index, uint8_t subindex,
                                      uint8_t* data, uint32_t* dataLen, uint32_t maxLen) {
    // Future implementation
    return -1;
}

int EcatSlaveSimulatorClient::sdoWrite(uint16_t slaveId, uint16_t index, uint8_t subindex,
                                       const uint8_t* data, uint32_t dataLen) {
    // Future implementation
    return -1;
}

void EcatSlaveSimulatorClient::fin() {
    if (!impl->connected || impl->clientFd == INVALID_SOCKET) {
        return;
    }
    
    // Send FIN request
    impl->sendRequest(ECAT_CMD_FIN, nullptr, 0);
    
    // Receive FIN_ACK
    uint8_t errorCode = 0;
    impl->recvResponse(ECAT_CMD_FIN_ACK, &errorCode, nullptr, nullptr, 0);
    
    disconnect();
}
