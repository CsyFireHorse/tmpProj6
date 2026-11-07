#ifndef _ECAT_SLAVE_SIMULATOR_PROTOCOL_H_
#define _ECAT_SLAVE_SIMULATOR_PROTOCOL_H_

// Shared protocol defaults for the EtherCAT slave simulator.
#define ECAT_SLAVE_SIMULATOR_DEFAULT_PORT 8765

// Command codes are non-zero to avoid ambiguity with empty data.
enum EcatSimCmd {
    ECAT_CMD_HELLO = 0x01,
    ECAT_CMD_HELLO_ACK = 0x02,
    ECAT_CMD_CONFIG = 0x10,
    ECAT_CMD_CONFIG_ACK = 0x11,
    ECAT_CMD_PDO = 0x20,
    ECAT_CMD_PDO_ACK = 0x21,
    ECAT_CMD_FIN = 0x30,
    ECAT_CMD_FIN_ACK = 0x31,
    ECAT_CMD_SDO_READ = 0x40,
    ECAT_CMD_SDO_READ_ACK = 0x41,
    ECAT_CMD_SDO_WRITE,
    ECAT_CMD_SDO_WRITE_ACK,
};

// Error codes are non-zero to avoid ambiguity with empty data.
enum EnumServoSimuErrCode {
    ECAT_RESULT_SUCCESS = 0x01,
    ECAT_S_INVALID_SOCKET = 0x10,
    ECAT_S_NOT_CONNECTED = 0x11,
    ECAT_S_SEND_FAILED = 0x20,
    ECAT_S_RECV_FAILED = 0x21,
    ECAT_S_RECV_OVERFLOW = 0x22,
    ECAT_S_CONFIG_FAILED = 0x30,
    ECAT_S_INVALID_CMD = 0x40,
    ECAT_S_INVALID_LEN = 0x41,
};

#endif
