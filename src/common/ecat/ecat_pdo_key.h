/*
 * @Author: ChenShengyong
 * @Date: 2024-04-02 14:14:04
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-01-23 14:06:04
 */
#ifndef _ECAT_PDO_KEY_H_
#define _ECAT_PDO_KEY_H_

// Object 6060h: Modes of operation
// Object 6061h: Modes of operation display
// cia402 operation mode 6060h
enum OP_MODE {
    OP_MODE_PP = 1,  // Profile Position Mode
    OP_MODE_VP = 2,  // Velocity Mode
    OP_MODE_PV = 3,  // Profile Velocity Mode
    OP_MODE_PT = 4,  // Profile Torque Mode
    OP_MODE_RESERVE0 = 5,
    OP_MODE_HOME = 6,  // Homing Mode
    OP_MODE_IPM = 7,   // Interpolated Position Mode
    OP_MODE_CSP = 8,   // Cyclic Synchronous Position Mode
    OP_MODE_CSV = 9,   // Cyclic Synchronous Velocity Mode
    OP_MODE_CST = 10,  // Cyclic Synchronous Torque Mode
};

enum PDO_KEY {
    PDO_MULT_SERVO_OFFSET = 0x800,  // single slave has multiple servo, offset is 0x800

    // master send rx_pdo to slave
    // rx_pdo
    PDO_PO_CTRL = 0x6040,
    PDO_PO_MODE = 0x6060,  // mode of operation, uint8
    PDO_PO_POS = 0x607a,   // target Position
    PDO_PO_VEL = 0x60ff,   // target velocity
    PDO_PO_TORCH = 0x6071,

    PDO_PO_MAX_PROF_VEL = 0x607F,      // Max profile velocity
    PDO_PO_PROF_VEL = 0x6081,          // Profile velocity
    PDO_PO_END_VEL = 0x6082,           // End velocity
    PDO_PO_PROF_ACC = 0x6083,          // Profile acceleration
    PDO_PO_PROF_DEC = 0x6084,          //  Profile deceleration
    PDO_PO_QUICK_STOP_DECEL = 0x6085,  // Quick stop deceleration value
    PDO_PO_MOTION_PROF_TYPE = 0x6086,  //  Motion profile type

    PDO_PO_QUICK_STOP = 0x605A,
    PDO_PO_HALT = 0x605D,  // halt option code
    PDO_PO_SHUTDOWN = 0x605B,
    PDO_PO_DISABLE = 0x605C,

    // Homing mode
    PDO_PO_HOMING_OFFSET = 0x607c,  // Home offset, int32
    PDO_PO_HOMING_METHOD = 0x6098,  // Homing method, int8
    PDO_PO_HOMING_SPEED = 0x6099,   // Homing speeds
                                   // subIndex = 1, Speed during search for switch, int32
                                   // subIndex = 0, Speed during search for zero, int32
    PDO_PO_HOMING_ACCEL = 0x609A,  // Homing acceleration

    PDO_PO_DIGITAL_INPUTS = 0x60FD,  // Digital inputs

    // tx_pdo
    // slave send txpdo to master
    PDO_PI_ERROR = 0x603f,  // error code
    PDO_PI_STAT = 0x6041,
    PDO_PI_MODE = 0x6061,               // mode of operation display, uint8
    PDO_PI_POS_DEMAND = 0x6062,         //  Position demand value
    PDO_PI_POS_ACTUAL_INTERN = 0x6063,  //  Position Actual Internal Value
    PDO_PI_POS = 0x6064,                //  Position actual value
    PDO_PI_VEL = 0x606c,                // velocity actual value
    PDO_PI_CMD_VEL = 0x606b,            // commanded velocity
    PDO_PI_POS_DEMAND_INTERN = 0x60FC,  // Position Demand Internal Value
    PDO_PI_TORCH = 0x6077,

    PDO_PI_FOLLOW_ERROR_ACTUAL = 0x60f4,  // follow error actual value

    PDO_PI_INPUT = 0x6000,
    PDO_PI_OUTPUT = 0x7000,

    PDO_TOUCHPROBE_FUNCTION = 0x60B8,       // Touch Probe function
    PDO_TOUCHPROBE_STATUS = 0x60B9,         // Touch Probe status
    PDO_TOUCHPROBE_POS1_POSITIVE = 0x60BA,  // positive value
    PDO_TOUCHPROBE_POS1_NEGATIVE = 0x60BB,  // negative value
    PDO_TOUCHPROBE_POS2_POSITIVE = 0x60BC,  // positive value
    PDO_TOUCHPROBE_POS2_NEGATIVE = 0x60BD,  // negative value

    PDO_TOUCHPROBE_POS1_POSITIVE_COUNTER = 0x60D5,  // positive value
    PDO_TOUCHPROBE_POS1_NEGATIVE_COUNTER = 0x60D6,  // negative value
    PDO_TOUCHPROBE_POS2_POSITIVE_COUNTER = 0x60D7,  // positive value
    PDO_TOUCHPROBE_POS2_NEGATIVE_COUNTER = 0x60D8,  // negative value
    PDO_PI_DIGITAL_OUTPUTS = 0x60FE,                // Digital inputs
    PDO_TOUCHPROBE_SOURCE = 0x60D0,                 // Touch Probe

    PDO_PI_MOTOR_DATA = 0x6402,  // Motor data, U32
    PDO_PI_DRIVE_MODE = 0x6502,  // Supported drive modes, U32

};

#endif