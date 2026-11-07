#ifndef _ZMC_AX_VERBOSE_FLAG_H_
#define _ZMC_AX_VERBOSE_FLAG_H_

#include <cstdint>
#include <cstring>
#include <cstdio>

#define LOGGER_HEADER_LAG_FULL "idx,axId,time,axSt,actPos,actVel,actAcc,setPos,setVel,setAcc"

namespace zmc {

// Verbose flag enum for axis printing
enum class AxVerboseFlag : uint8_t {
    AX_VFLAG_NOP = 0,      // No printing
    AX_VFLAG_DEFAULT,  // Default mode (same as MOTION)
    AX_VFLAG_MOTION,   // Print fSetPosition, fSetVelocity, fSetAcceleration
    AX_VFLAG_LAG,      // Print fActPosition, fSetPosition, fSetVelocity, fSetAcceleration
    AX_VFLAG_LAG_FULL,      // Print fActPosition, fActVel, fActAcc, fSetPosition, fSetVelocity, fSetAcceleration
    AX_VFLAG_UNIT,     // Print diActPosition, fSetPosition, fSetVelocity, fSetAcceleration
    AX_VFLAG_UNIT_LAG,     // Print diActPosition, diSetPosition fActPosition, fSetPosition
    AX_VFLAG_POWER,    // Print wControlWord, wStatusWord, diActPosition, diSetPosition
    AX_VFLAG_MOTBUF,   // Reserved for motion buffer related
    AX_VFLAG_FB        // Print fbStat related
};

// Convert verbose flag to string
inline const char* verboseFlagToString(AxVerboseFlag flag) {
    switch (flag) {
        case AxVerboseFlag::AX_VFLAG_NOP:
            return "nop";
        case AxVerboseFlag::AX_VFLAG_MOTION:
            return "motion";
        case AxVerboseFlag::AX_VFLAG_LAG:
            return "lag";
        case AxVerboseFlag::AX_VFLAG_LAG_FULL:
            return "lag_full";
        case AxVerboseFlag::AX_VFLAG_UNIT:
            return "unit";
        case AxVerboseFlag::AX_VFLAG_UNIT_LAG:
            return "unit_lag";
        case AxVerboseFlag::AX_VFLAG_POWER:
            return "power";
        case AxVerboseFlag::AX_VFLAG_MOTBUF:
            return "motbuf";
        case AxVerboseFlag::AX_VFLAG_FB:
            return "fb";
        default:
            return "unknown";
    }
}

// Convert string to verbose flag
inline AxVerboseFlag stringToVerboseFlag(const char* str) {
    if (strcmp(str, "nop") == 0) return AxVerboseFlag::AX_VFLAG_NOP;
    if (strcmp(str, "motion") == 0) return AxVerboseFlag::AX_VFLAG_MOTION;
    if (strcmp(str, "default") == 0) return AxVerboseFlag::AX_VFLAG_DEFAULT;
    if (strcmp(str, "lag") == 0) return AxVerboseFlag::AX_VFLAG_LAG;
    if (strcmp(str, "lag_full") == 0) return AxVerboseFlag::AX_VFLAG_LAG_FULL;
    if (strcmp(str, "unit") == 0) return AxVerboseFlag::AX_VFLAG_UNIT;
    if (strcmp(str, "unit_lag") == 0) return AxVerboseFlag::AX_VFLAG_UNIT_LAG;
    if (strcmp(str, "power") == 0) return AxVerboseFlag::AX_VFLAG_POWER;
    if (strcmp(str, "motbuf") == 0) return AxVerboseFlag::AX_VFLAG_MOTBUF;
    if (strcmp(str, "fb") == 0) return AxVerboseFlag::AX_VFLAG_FB;
    return AxVerboseFlag::AX_VFLAG_NOP;
}

// printf("idx,axId,time,axSt,mPos,pos,vel,acc\n");
inline void printVerboseHeader(AxVerboseFlag verbose_flag) {
    if (verbose_flag == AxVerboseFlag::AX_VFLAG_NOP) return;
    switch (verbose_flag) {
        case AxVerboseFlag::AX_VFLAG_DEFAULT:
        case AxVerboseFlag::AX_VFLAG_MOTION:
            printf("idx,axId,time,axSt,setPos,setVel,setAcc\n");
            break;
        case AxVerboseFlag::AX_VFLAG_LAG:
            printf("idx,axId,time,axSt,actPos,setPos,setVel,setAcc\n");
            break;
        case AxVerboseFlag::AX_VFLAG_LAG_FULL:
            printf("%s\n", LOGGER_HEADER_LAG_FULL);
            break;
        case AxVerboseFlag::AX_VFLAG_UNIT:
            printf("idx,axId,time,axSt,diActPos,setPos,setVel,setAcc\n");
            break;
        case AxVerboseFlag::AX_VFLAG_UNIT_LAG:
            printf("idx,axId,time,axSt,diActPos,diSetPos,actPos,setPos\n");
            break;
        case AxVerboseFlag::AX_VFLAG_POWER:
            printf("idx,axId,time,axSt,ctrlWord,statusWord,diActPos,diSetPos\n");
            break;
        case AxVerboseFlag::AX_VFLAG_MOTBUF:
            printf("idx,axId,time,axSt,motBuf,setPos,setVel,setAcc\n");
            break;
        case AxVerboseFlag::AX_VFLAG_FB:
            printf("idx,axId,time,axSt,fbStat,setPos,setVel,setAcc\n");
            break;
        case AxVerboseFlag::AX_VFLAG_NOP:
            break;
        default:
            printf("idx,axId,time,axSt,setPos,setVel,setAcc\n");
            break;
    }
}

template <typename T>
inline void printAxisVerbose(AxVerboseFlag verbose_flag, const T& ax) {
    int axId = ax.wAxisStructID;
    int idx = ax.m_cycleCounter;
    double time = idx * ax.cycleTime;
    switch (verbose_flag) {
        case AxVerboseFlag::AX_VFLAG_MOTION:
            printf("%d,%d,%f,%d,%f,%f,%f\n", idx, axId, time, ax.axisStat, ax.fSetPosition,
                   ax.fSetVelocity, ax.fSetAcceleration);
            break;
        case AxVerboseFlag::AX_VFLAG_LAG:
            printf("%d,%d,%f,%d,%f,%f,%f,%f\n", idx, axId, time, ax.axisStat, ax.fActPosition,
                   ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
            break;
        case AxVerboseFlag::AX_VFLAG_LAG_FULL:
            printf("%d,%d,%f,%d,%f,%f,%f,%f,%f,%f\n", idx, axId, time, ax.axisStat, 
                   ax.fActPosition, ax.fActVelocity, ax.fActAcceleration,
                   ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
            break;
        case AxVerboseFlag::AX_VFLAG_UNIT:
            printf("%d,%d,%f,%d,%d,%f,%f,%f\n", idx, axId, time, ax.axisStat, ax.diActPosition,
                   ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
            break;
        case AxVerboseFlag::AX_VFLAG_UNIT_LAG:
            printf("%d,%d,%f,%d,%d,%d,%f,%f\n", idx, axId, time, ax.axisStat, 
                   ax.diActPosition, ax.diSetPosition, ax.fActPosition, ax.fSetPosition);
            break;
        case AxVerboseFlag::AX_VFLAG_POWER:
            printf("%d,%d,%f,%d,0x%04x,0x%04x,%d,%d\n", idx, axId, time, ax.axisStat,
                   ax.wControlWord, ax.wStatusWord, ax.diActPosition, ax.diSetPosition);
            break;
        case AxVerboseFlag::AX_VFLAG_MOTBUF:
            // Reserved for motion buffer related - using basic format for now
            printf("%d,%d,%f,%d,%d,%f,%f,%f\n", idx, axId, time, ax.axisStat, ax.byMotbufValue,
                   ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
            break;
        case AxVerboseFlag::AX_VFLAG_FB:
            // Print with current fb stat - need to get from derived class
            printf("%d,%d,%f,%d,0x%x,%f,%f,%f\n", idx, axId, time, ax.axisStat, ax.pDwExtraData?ax.pDwExtraData[0]:-1,
                   ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration); // ax.cur_fb_stat()
            break;
        default:
            printf("%d,%d,%f,%d,%f,%f,%f\n", idx, axId, time, ax.axisStat, ax.fSetPosition,
                   ax.fSetVelocity, ax.fSetAcceleration);
            break;
    }
}

}  // namespace zmc

#endif  // _ZMC_VERBOSE_FLAG_H_