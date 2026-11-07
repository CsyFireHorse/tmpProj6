#ifndef _ZMC_SERVO_CMD_DEF_HPP_
#define _ZMC_SERVO_CMD_DEF_HPP_

namespace zmc {

enum DriveCtrlCmd {
    DRIVE_CTRL_CMD_NOP,
    DRIVE_CTRL_CMD_RESET,
    DRIVE_CTRL_CMD_POWEROFF,
    DRIVE_CTRL_CMD_QUICKSTOP,
    DRIVE_CTRL_CMD_POWERON,
    DRIVE_CTRL_CMD_HOME,
    DRIVE_CTRL_CMD_HALT,
    DRIVE_CTRL_CMD_MOVE,
};

enum DriveStatword {
    DRIVE_STAT_POWEROFF,
    DRIVE_STAT_POWERON,
    DRIVE_STAT_QUICKSTOP,
    DRIVE_STAT_FAULT,
    // DRIVE_STAT_NOP,
    // DRIVE_STAT_HOME,
    // DRIVE_STAT_MOVE,
};

inline DriveStatword DriveCtrlcmdUpdateStatword(DriveCtrlCmd cmd, DriveStatword stat) {
    switch (cmd) {
        case DRIVE_CTRL_CMD_RESET:
            return DRIVE_STAT_POWEROFF;
        case DRIVE_CTRL_CMD_POWEROFF:
            return DRIVE_STAT_POWEROFF;
        case DRIVE_CTRL_CMD_POWERON:
            return DRIVE_STAT_POWERON;
        case DRIVE_CTRL_CMD_QUICKSTOP:
            return DRIVE_STAT_QUICKSTOP;
        case DRIVE_CTRL_CMD_HOME:
            return DRIVE_STAT_POWERON;
        case DRIVE_CTRL_CMD_MOVE:
            return DRIVE_STAT_POWERON;
        case DRIVE_CTRL_CMD_NOP:
        default:
            return stat;
    }
}

}  // namespace zmc
#endif