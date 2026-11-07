#include "EcatSlaveSimulatorService.h"
#include "simple_socket.h"
#include "../zecm_zh.h"
#include "stdio.h"
#include "string.h"
// #include "base/include/ServoFakerToy.h"
// #include "common/misc/ZmcBitmap.hpp"
#include "ServoFakAlloc.hpp"
#include "pdoPreparer.h"
#include "ServoFakFileState.h"
#include <time.h>
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

struct EcatSlaveSimulatorService::Impl {
    Impl()
        : serverFd(INVALID_SOCKET),
          running(false),
          slaveCount(0),
          servoArray(nullptr),
          pdoCtx(nullptr),
          allocatedStartIndex(-1),
          pdoHandler(nullptr),
          rxpdoLen(0),
          txpdoLen(0) {
    }

    SOCKET serverFd;
    bool running;
    int slaveCount;
    zmc::ServoSgFaker* servoArray;
    EniPdoPrepareCtx* pdoCtx;
    int allocatedStartIndex;
    PrepareHandler pdoHandler;
    uint32_t rxpdoLen;
    uint32_t txpdoLen;

    // Protocol helper functions
    int sendResponse(SOCKET fd, uint8_t ackCmd, uint8_t errorCode, const uint8_t* data, uint32_t dataLen);
    int recvRequest(SOCKET fd, uint8_t* cmd, uint8_t* data, uint32_t* dataLen, uint32_t maxDataLen);

    // Handle client request
    int handleRequest(SOCKET clientFd);

    // Process requests
    int processHello(SOCKET clientFd);
    int processConfig(SOCKET clientFd, const uint8_t* data, uint32_t dataLen);
    int processPdo(SOCKET clientFd, const uint8_t* data, uint32_t dataLen);
    int processFin(SOCKET clientFd);
};


// zmc::BitmapAllocatorTmpl<32> EcatSlaveSimulatorService::s_allocator;

// static zmc::BitmapAllocatorTmpl<32> s_allocator;

EcatSlaveSimulatorService::EcatSlaveSimulatorService()
    : impl(new Impl()) {
    SimpleSocket::init();
}

EcatSlaveSimulatorService::~EcatSlaveSimulatorService() {
    stop();
    if (impl->servoArray && impl->allocatedStartIndex >= 0 && impl->slaveCount > 0) {
        if (zmc::servo_fak_state_enabled()) {
            zmc::servo_fak_state_save(impl->servoArray, impl->slaveCount);
        }
        zmc::ServoSgFakerEager::deallocate_consecutive(impl->allocatedStartIndex, impl->slaveCount);
    }
    if (impl->pdoCtx) {
        freeEniPdoPrepareCtx(impl->pdoCtx);
        impl->pdoCtx = nullptr;
    }
    delete impl;
    impl = nullptr;
}

// Process CONFIG request
int EcatSlaveSimulatorService::Impl::processConfig(SOCKET clientFd, const uint8_t* data, uint32_t dataLen) {
    if (servoArray) {
        printf("[Server] Already initialized, return success\n");
        // Already initialized, return success
        return sendResponse(clientFd, ECAT_CMD_CONFIG_ACK, ECAT_RESULT_SUCCESS, nullptr, 0);
    }
    
    // Parse payload: [rxpdoLen:4][txpdoLen:4][eniLen:4][eniFile:eniLen]
    if (dataLen < 12) {
        printf("[Server] Invalid CONFIG payload length: %u\n", dataLen);
        return sendResponse(clientFd, ECAT_CMD_CONFIG_ACK, ECAT_S_INVALID_LEN, nullptr, 0);
    }
    
    uint32_t rxpdoLen = ntohl_local(*(uint32_t*)(data + 0));
    uint32_t txpdoLen = ntohl_local(*(uint32_t*)(data + 4));
    uint32_t eniLen = ntohl_local(*(uint32_t*)(data + 8));
    
    if (eniLen > 1023 || dataLen != 12 + eniLen) {
        printf("[Server] Invalid ENI length: %u or payload mismatch\n", eniLen);
        return sendResponse(clientFd, ECAT_CMD_CONFIG_ACK, ECAT_S_INVALID_LEN, nullptr, 0);
    }
    
    char eni_file[1024];
    memcpy(eni_file, data + 12, eniLen);
    eni_file[eniLen] = '\0';
    
    printf("[Server] CONFIG: rxpdoLen=%u, txpdoLen=%u, eniFile=%s\n", 
           rxpdoLen, txpdoLen, eni_file);
    
    PdoPreparerConfig config = guess_preparer(eni_file);
    if (!config.handler || !config.ctx) {
        if (config.ctx) {
            freeEniPdoPrepareCtx(config.ctx);
        }
        printf("[Server] Failed to guess preparer for %s\n", eni_file);
        return sendResponse(clientFd, ECAT_CMD_CONFIG_ACK, ECAT_S_CONFIG_FAILED, nullptr, 0);
    }
    
    // Verify PDO sizes match
    if (config.rxpdoLen != rxpdoLen || config.txpdoLen != txpdoLen) {
        printf("[Server] Warning: PDO size mismatch. Config: input=%u output=%u, Received: input=%u output=%u\n",
               config.rxpdoLen, config.txpdoLen, rxpdoLen, txpdoLen);
        // Use received sizes instead
        config.rxpdoLen = rxpdoLen;
        config.txpdoLen = txpdoLen;
    }
    
    int startIndex = zmc::ServoSgFakerEager::allocate_consecutive(config.slaveCount);
    if (startIndex < 0) {
        freeEniPdoPrepareCtx(config.ctx);
        printf("[Server] Failed to allocate %d consecutive ServoSgFaker slots\n", config.slaveCount);
        return sendResponse(clientFd, ECAT_CMD_CONFIG_ACK, ECAT_S_CONFIG_FAILED, nullptr, 0);
    }
    
    servoArray = &zmc::ServoSgFakerEager::Instance(startIndex);
    allocatedStartIndex = startIndex;
    eni_pdo_ctx_set_sv_arr(config.ctx, servoArray);
    pdoCtx = config.ctx;
    pdoHandler = config.handler;
    slaveCount = config.slaveCount;
    this->rxpdoLen = config.rxpdoLen;
    this->txpdoLen = config.txpdoLen;
    
    printf("[Server] Configured: %d slaves, input=%u bytes, output=%u bytes\n",
           slaveCount, rxpdoLen, txpdoLen);

    if (zmc::servo_fak_state_enabled() && slaveCount > 0) {
        zmc::servo_fak_state_set_extra_string("eni", eni_file);
        uint32_t cycleUs = eni_pdo_ctx_get_cycle_time(pdoCtx);
        double cycleTime = (cycleUs > 0) ? (cycleUs * 1e-6) : 1e-3;
        zmc::servo_fak_state_load(servoArray, slaveCount, cycleTime);
    }
    
    return sendResponse(clientFd, ECAT_CMD_CONFIG_ACK, ECAT_RESULT_SUCCESS, nullptr, 0);
}

int EcatSlaveSimulatorService::start(int port) {
    if (impl->running) return 0;
    
    impl->serverFd = SimpleSocket::createServer(port);
    if (impl->serverFd == INVALID_SOCKET) {
        printf("Failed to create server on port %d\n", port);
        return ECAT_S_INVALID_SOCKET;
    }
    
    impl->running = true;
    printf("EcatSlaveSimulatorService started on port %d\n", port);
    return 0;
}

void EcatSlaveSimulatorService::stop() {
    if (impl->serverFd != INVALID_SOCKET) {
        closesocket(impl->serverFd);
        impl->serverFd = INVALID_SOCKET;
    }
    impl->running = false;
}

void EcatSlaveSimulatorService::run() {
    if (!impl->running || impl->serverFd == INVALID_SOCKET) return;
    
    printf("EcatSlaveSimulatorService waiting for connections...\n");
    
    while (impl->running) {
        SOCKET clientFd = SimpleSocket::acceptClient(impl->serverFd);
        if (clientFd == INVALID_SOCKET) {
            if (impl->running) {
                printf("Accept failed, continuing...\n");
            }
            continue;
        }
        
        printf("Client connected\n");
        impl->handleRequest(clientFd);
        closesocket(clientFd);
        printf("Client disconnected\n");
    }
}

// Protocol helper: Send response [CMD_ACK:1][errorCode:1][LEN:4][DATA:LEN]
int EcatSlaveSimulatorService::Impl::sendResponse(SOCKET fd, uint8_t ackCmd, uint8_t errorCode, const uint8_t* data, uint32_t dataLen) {
    uint32_t len = htonl_local(dataLen);
    
    // printf("[Server] Send ACK=0x%02X, errorCode=0x%02X, LEN=%u\n", ackCmd, errorCode, dataLen);
    
    if (SimpleSocket::sendAll(fd, &ackCmd, 1) != 1) {
        return ECAT_S_SEND_FAILED;
    }
    
    if (SimpleSocket::sendAll(fd, &errorCode, 1) != 1) {
        return ECAT_S_SEND_FAILED;
    }
    
    if (SimpleSocket::sendAll(fd, (uint8_t*)&len, 4) != 4) {
        return ECAT_S_SEND_FAILED;
    }
    
    if (dataLen > 0 && data != nullptr) {
        if (SimpleSocket::sendAll(fd, data, dataLen) != dataLen) {
            return ECAT_S_SEND_FAILED;
        }
    }
    
    return ECAT_RESULT_SUCCESS;
}

// Protocol helper: Receive request [CMD:1][pad:1][LEN:4][DATA:LEN]
int EcatSlaveSimulatorService::Impl::recvRequest(SOCKET fd, uint8_t* cmd, uint8_t* data, uint32_t* dataLen, uint32_t maxDataLen) {
    uint8_t pad = 0;
    uint32_t len = 0;
    
    if (SimpleSocket::recvAll(fd, cmd, 1) != 1) {
        return ECAT_S_RECV_FAILED;
    }
    
    if (SimpleSocket::recvAll(fd, &pad, 1) != 1) {
        return ECAT_S_RECV_FAILED;
    }
    
    if (SimpleSocket::recvAll(fd, (uint8_t*)&len, 4) != 4) {
        return ECAT_S_RECV_FAILED;
    }
    // printf("[Server] recvRequest: len = %u\n", len);
    len = ntohl_local(len);
    
    // printf("[Server] Recv CMD=0x%02X, pad=0x%02X, LEN=%u\n", *cmd, pad, len);
    
    if (len > maxDataLen) {
        printf("[Server] Error: Data length %u exceeds max %u\n", len, maxDataLen);
        return ECAT_S_RECV_OVERFLOW;
    }
    
    if (len > 0 && data != nullptr && dataLen != nullptr) {
        if (SimpleSocket::recvAll(fd, data, len) != len) {
            return ECAT_S_RECV_FAILED;
        }
        *dataLen = len;
    } else if (len > 0) {
        // Skip data if buffer not provided
        uint8_t dummy[4096];
        uint32_t toSkip = len;
        while (toSkip > 0) {
            uint32_t chunk = (toSkip > sizeof(dummy)) ? sizeof(dummy) : toSkip;
            if (SimpleSocket::recvAll(fd, dummy, chunk) != chunk) {
                return ECAT_S_RECV_FAILED;
            }
            toSkip -= chunk;
        }
    }
    
    return ECAT_RESULT_SUCCESS;
}

// Process HELLO request
int EcatSlaveSimulatorService::Impl::processHello(SOCKET clientFd) {
    uint8_t cmd = 0;
    uint8_t data[1];
    uint32_t dataLen = 0;
    printf("[Server] processHello\n");
    int ret = recvRequest(clientFd, &cmd, data, &dataLen, sizeof(data));
    if (ret != ECAT_RESULT_SUCCESS) {
        return ret;
    }
    printf("[Server] processHello: cmd = 0x%02X\n", cmd);
    
    if (cmd != ECAT_CMD_HELLO) {
        printf("[Server] Error: Expected HELLO (0x%02X), got 0x%02X\n", ECAT_CMD_HELLO, cmd);
        sendResponse(clientFd, ECAT_CMD_HELLO_ACK, ECAT_S_INVALID_CMD, nullptr, 0);
        return ECAT_S_INVALID_CMD;
    }
    
    printf("[Server] HELLO received\n");
    ret = sendResponse(clientFd, ECAT_CMD_HELLO_ACK, ECAT_RESULT_SUCCESS, nullptr, 0);
    if (ret != ECAT_RESULT_SUCCESS) {
        return ret;
    }
    
    printf("[Server] HELLO_ACK sent\n");
    return ECAT_RESULT_SUCCESS;
}

int EcatSlaveSimulatorService::Impl::handleRequest(SOCKET clientFd) {
    // First, handle HELLO
    int ret = processHello(clientFd);
    if (ret != ECAT_RESULT_SUCCESS) {
        printf("[Server] HELLO failed: %d\n", ret);
        return ret;
    }
    
    // Process requests loop
    while (running) {
        uint8_t cmd = 0;
        uint8_t data[4096 + 12];  // Max PDO size + config overhead
        uint32_t dataLen = 0;
        
        ret = recvRequest(clientFd, &cmd, data, &dataLen, sizeof(data));
        if (ret != ECAT_RESULT_SUCCESS) {
            printf("[Server] Failed to receive request: %d\n", ret);
            break;
        }
        
        switch (cmd) {
            case ECAT_CMD_CONFIG:
                ret = processConfig(clientFd, data, dataLen);
                if (ret != ECAT_RESULT_SUCCESS) {
                    printf("[Server] CONFIG failed: %d\n", ret);
                    return ret;
                }
                break;
                
            case ECAT_CMD_PDO:
                ret = processPdo(clientFd, data, dataLen);
                if (ret != ECAT_RESULT_SUCCESS) {
                    printf("[Server] PDO failed: %d\n", ret);
                    return ret;
                }
                break;
                
            case ECAT_CMD_FIN:
                processFin(clientFd);
                return ECAT_RESULT_SUCCESS;
                
            default:
                printf("[Server] Unknown command: 0x%02X\n", cmd);
                sendResponse(clientFd, cmd + 1, ECAT_S_INVALID_CMD, nullptr, 0);  // Assume ACK = cmd + 1
                return ECAT_S_INVALID_CMD;
        }
    }
    
    return ECAT_RESULT_SUCCESS;
}

// Process PDO request
int EcatSlaveSimulatorService::Impl::processPdo(SOCKET clientFd, const uint8_t* data, uint32_t dataLen) {
    if (!pdoHandler || !pdoCtx) {
        printf("[Server] PDO handler not configured\n");
        return sendResponse(clientFd, ECAT_CMD_PDO_ACK, ECAT_S_CONFIG_FAILED, nullptr, 0);
    }
    
    // Parse payload: [rxpdoLen:4][pdoInput:rxpdoLen]
    if (dataLen < 4) {
        printf("[Server] Invalid PDO payload length: %u\n", dataLen);
        return sendResponse(clientFd, ECAT_CMD_PDO_ACK, ECAT_S_INVALID_LEN, nullptr, 0);
    }
    
    uint32_t inputLen = ntohl_local(*(uint32_t*)data);
    if (dataLen != 4 + inputLen || inputLen > rxpdoLen) {
        printf("[Server] Invalid PDO input length: %u (expected %u, payload %u)\n", inputLen, rxpdoLen, dataLen);
        return sendResponse(clientFd, ECAT_CMD_PDO_ACK, ECAT_S_INVALID_LEN, nullptr, 0);
    }
    
    const uint8_t* pdoInput = data + 4;
    
    // Log received data to file
    static FILE* logFile = nullptr;
    static int logCounter = 0;
    if (!logFile) {
        logFile = fopen("ecat_pdo_received.log", "a");
        if (!logFile) {
            printf("[Server] Warning: Failed to open log file ecat_pdo_received.log\n");
        }
    }
    if (logFile) {
        time_t now = time(NULL);
        struct tm* timeinfo;
        char timestamp[64];
        struct timespec ts;
#if defined(_WIN32)
        // On Windows, fallback to seconds precision
        timeinfo = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S.000", timeinfo);
#else
        clock_gettime(CLOCK_REALTIME, &ts);
        timeinfo = localtime(&ts.tv_sec);
        int ms = ts.tv_nsec / 1000000;
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
        snprintf(timestamp + strlen(timestamp), sizeof(timestamp) - strlen(timestamp), ".%03d", ms);
#endif
        
        fprintf(logFile, "=== PDO Receive [%s] #%d ===\n", timestamp, ++logCounter);
        fprintf(logFile, "Length: %u bytes\n", inputLen);
        fprintf(logFile, "Data (hex):\n");
        for (uint32_t i = 0; i < inputLen; i++) {
            fprintf(logFile, "%02X ", pdoInput[i]);
            if ((i + 1) % 16 == 0) fprintf(logFile, "\n");
        }
        if (inputLen % 16 != 0) fprintf(logFile, "\n");
        fprintf(logFile, "\n");
        fflush(logFile);
    }
    
    // Process PDO using existing handler
    const int PDO_SIZE = 4096;
    uint8_t pdoOutput[PDO_SIZE];
    memset(pdoOutput, 0, PDO_SIZE);
    pdoHandler(pdoCtx, (uint8_t*)pdoInput, pdoOutput);
    
    // Build response payload: [txpdoLen:4][pdoOutput:txpdoLen]
    uint8_t responseBuf[PDO_SIZE + 4];
    uint32_t* p = (uint32_t*)responseBuf;
    p[0] = htonl_local(txpdoLen);
    memcpy(responseBuf + 4, pdoOutput, txpdoLen);
    
    return sendResponse(clientFd, ECAT_CMD_PDO_ACK, ECAT_RESULT_SUCCESS, responseBuf, 4 + txpdoLen);
}

// Process FIN request
int EcatSlaveSimulatorService::Impl::processFin(SOCKET clientFd) {
    printf("[Server] FIN received, closing connection\n");
    sendResponse(clientFd, ECAT_CMD_FIN_ACK, ECAT_RESULT_SUCCESS, nullptr, 0);
    return ECAT_RESULT_SUCCESS;
}
