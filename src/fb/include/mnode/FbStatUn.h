/*
 * @Author: ChenShengyong
 * @Date: 2024-03-29 11:46:47
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-30 13:39:42
 */
#ifndef _FB_STAT_UN_
#define _FB_STAT_UN_

// #include "McCommon.h"
#include "stdint.h"
#include "FbTypeDef.h"

namespace zmc {

union FbCtrlUn {
    uint8_t iCtrl;
    struct {
        uint8_t enable_ : 1;
        uint8_t execute_ : 1;
        uint8_t ContinuousUpdate_ : 1;
        uint8_t _pad : 1;
        uint8_t _pad2 : 1;
        uint8_t bufferMode : 3;
    };
};



enum E_FBSTOP_STAT {
    E_FBSTOP_IDLE,         // idle_unexecute
    E_FBSTOP_BUSY_EXEC,    // busy_execute
    E_FBSTOP_BUSY_UNEXEC,  // busy_unexecute
    E_FBSTOP_DONE_EXEC,    // done_execute
    E_FBSTOP_DONE_UNEXEC,  // done_unexecute  (edge)
    E_FBSTOP_IDLE_RM,      // idle (edge)
};


enum FbOutStat {
    FB_OUT_STAT_BUSY,
    FB_OUT_STAT_DONE,
    FB_OUT_STAT_ACTIVE,
    FB_OUT_STAT_VALID,
    FB_OUT_STAT_ABORTED,
    FB_OUT_STAT_INXXX,
    FB_OUT_STAT_ERROR,
};

enum FbMidStat {
    FB_MID_STAT_IDLE,
    FB_MID_STAT_BUSY_UNVALID,
    FB_MID_STAT_VALID,
    FB_MID_STAT_ERROR,

    // IDLE->BUSY -> DONE
    // IDLE->BUSY -> ERROR
    // IDLE->BUSY -> ABORT
    FB_MID_STAT_EXE_IDLE,
    FB_MID_STAT_EXE_BUSY,
    FB_MID_STAT_EXE_DONE,
    FB_MID_STAT_EXE_ERROR,
    FB_MID_STAT_EXE_ABORT,
};

// 0x1a ~ executed high edge & busy
// 0x18 =  active_ & busy_
// 0x10 = busy
// 0x20 =32  ~ done
// 0x26 =38  ~ done + executed
// 0x48 ~ inxxx
// 0x1E = 30 busy with executed_ high
// 0x18 = 24 busy  with executed_ low
// 0x80 = aborted
union FbStatUn {
    uint16_t ostat;
    struct {
        uint16_t valid_ : 1;
        uint16_t executed_ : 1;
        uint16_t executed_bak : 1;
        uint16_t active_ : 1;  // fb is selected by motion buffer
        uint16_t busy_ : 1;
        uint16_t done_ : 1;
        uint16_t inxxx_ : 1;
        uint16_t aborted_ : 1;
        uint16_t error_ : 1;
        // use for fbpower only
        uint16_t enabled_ : 1;
        uint16_t covered_ : 1;
        uint16_t force_ : 1;  // force output done, error, aborted_
        uint16_t needInit_: 1;
    };
    inline bool isActive() const { return active_;}
    inline bool isError() const { return error_;}
    inline bool is_rising_edge() const { return executed_ & !executed_bak; }
    inline bool is_falling_edge() const { return !executed_ & executed_bak; }
    void reset() { ostat = 0; }
    void set_idle() {
        ostat = 0;
        done_ = aborted_ = busy_ = active_ = false;
        inxxx_ = covered_ = false;
        force_ = false;
    }
    void set_inxxx() {
        inxxx_ = active_ = true;
        busy_ = true;
        aborted_ = error_ = false;
    }
    void set_done() {
        done_ = true;
        active_ = busy_ = aborted_ = error_ = false;
        force_ = true;
    }
    void set_error() {
        error_ = true;
        inxxx_ = false;
        covered_ = false;
        active_ = busy_ = done_ = aborted_ = false;
        force_ = true;
        valid_ = false;
    }
    void set_busy() {
        busy_ = true;
        done_ = aborted_ = error_ = false;
    }
    void set_active_busy() {
        active_ = busy_ = true;
        done_ = aborted_ = error_ = false;
    }
    void set_aborted() {
        aborted_ = true;
        inxxx_ = false;
        covered_ = false;
        active_ = busy_ = done_ = error_ = false;
        force_ = true;
    }
    void set_valid() {
        valid_ = true;
        busy_ = error_ = false;
    }
    void set_busy_unvalid() {
        busy_ = true;
        done_ = error_ = false;
    }
    bool is_done_abort_error() const { return done_ || aborted_ || error_; }
    bool not_busy() const { return (done_ || aborted_ || error_) && !busy_; }
    inline void update_stat(FbMidStat sm) {
        switch (sm) {
            case FB_MID_STAT_IDLE:
                valid_ = busy_ = error_ = false;
                break;
            case FB_MID_STAT_BUSY_UNVALID:
                valid_ = error_ = false;
                busy_ = true;
                break;
            case FB_MID_STAT_VALID:
                error_ = false;
                busy_ = true;
                valid_ = true;
                break;
            case FB_MID_STAT_ERROR:
                busy_ = valid_ = false;
                error_ = true;
                break;
            default:
                break;
        }
    }
    inline void update(FbMidStat sm) {
        switch (sm) {
            case FB_MID_STAT_EXE_IDLE:
                set_idle();
                break;
            case FB_MID_STAT_EXE_BUSY:
                set_active_busy();
                break;
            case FB_MID_STAT_EXE_DONE:
                set_done();
                break;
            case FB_MID_STAT_EXE_ERROR:
                set_error();
                break;
            case FB_MID_STAT_EXE_ABORT:
                set_aborted();
                break;
            default:
                break;
        }
    }
    inline FbMidStat midStat() const {
        if (error_)
            return FB_MID_STAT_EXE_ERROR;
        else if (done_)
            return FB_MID_STAT_EXE_DONE;
        else if (aborted_)
            return FB_MID_STAT_EXE_ABORT;
        else if (busy_)
            return FB_MID_STAT_EXE_BUSY;
        else
            return FB_MID_STAT_EXE_IDLE;
    }
};


} /* namespace zmc */
#endif