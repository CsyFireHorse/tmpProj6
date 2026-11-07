/*
 * @Author: ChenShengyong
 * @Date: 2024-04-16 18:22:45
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-09-27 17:55:11
 */
#ifndef _ZMC_FB_MOVE_WRAP_HPP
#define _ZMC_FB_MOVE_WRAP_HPP

// #include "Axis.h"
// #include "FbMoveBase.h"
#include "fb_cpp_def.h"
#include "fbt_axis_base.h"
#include "common/misc/pou_base.hpp"
#include "AxPou.hpp"
// #include "FbFactoryDef.h"
// #include "common/ecat/Cia402Easy.hpp"
#include "stdio.h"
#include "string.h"
#include <cstring>

namespace zmc {
class FbMoveWrap : public PouTaskBase {
   public:
    enum {
        FBM_POWER,
        FBM_MOVE_RELATIVE,
        FBM_MOVE_ABSOLUTE,
        FBM_MOVE_ADDITIVE,
        FBM_MOVE_VELOCITY,
        FBM_GEAR_IN,
        FBM_GEAR_OUT,
        FBM_RESET,
        FBM_STOP,
        FBM_HALT,
        FBM_HOME,
        FBM_SET_POSITION,
        FBM_Number
    };
    FbMoveWrap(int axNum = 3) : _axNum(axNum) {
        axArr = (AXIS_REF*)(new int*[axNum]);
        // Get axes from AxPouTaskBinderAuto
        for (int i = 0; i < _axNum; i++) {
            axArr[i] = AxPouTaskBinderAuto::getAxisByIndex(i);
        }
        
        fbPow = new FbtPower[axNum];
        fbRest = new FbtReset[axNum];
        fbMovRel = new FbtMoveRelative[axNum];
        fbMovAbs = new FbtMoveAbsolute[axNum];
        fbMovAdd = new FbtMoveAdditive[axNum];
        fbMovVel = new FbtMoveVelocity[axNum];
        fbGearIn = new FbtGearIn[axNum];
        fbGearOut = new FbtGearOut[axNum];
        fbStp = new FbtStop[axNum];
        fbHlt = new FbtHalt[axNum];
        fbHome = new FbtHome[axNum];
        fbSetPos = new FbtSetPosition[axNum];

        // powerEnable = new bool[axNum];
        executeArr = (bool(*)[FBM_Number]) new bool[axNum * FBM_Number];
        memset(executeArr, 0, axNum * FBM_Number);
        for (int i = 0; i < _axNum; i++) {
            bindAxis(axArr[i], i);
            fbMovAdd[i].Distance = 10;
            set_fb_vel_config(fbMovAdd[i], cmdVel, cmdAcc, cmdAcc, 0);

            fbMovRel[i].Distance = 10;
            set_fb_vel_config(fbMovRel[i], cmdVel, cmdAcc, cmdAcc, 0);

            fbMovAbs[i].Position = 10;
            set_fb_vel_config(fbMovAbs[i], cmdVel, cmdAcc, cmdAcc, 0);

            set_fb_vel_config(fbMovVel[i], cmdVel, cmdAcc, cmdAcc, 0);

            fbStp[i].Deceleration = cmdAcc;
            fbStp[i].Jerk = 0;

            fbHlt[i].Deceleration = cmdAcc;
            fbHlt[i].Jerk = 0;
        }
    }
    ~FbMoveWrap() {
        delete[] axArr;
        delete[] fbPow;
        delete[] fbRest;
        delete[] fbMovRel;
        delete[] fbMovAbs;
        delete[] fbMovAdd;
        delete[] fbMovVel;
        delete[] fbGearIn;
        delete[] fbGearOut;
        delete[] fbStp;
        delete[] fbHlt;
        delete[] fbHome;
        delete[] fbSetPos;
        // delete [] powerEnable;
        _axNum = 0;
    }
    void bindAxis(AXIS_REF axRef, int i) {
        axArr[i] = axRef;
        fbRest[i].Axis = axRef;
        fbPow[i].Axis = axRef;
        fbMovAdd[i].Axis = axRef;
        fbMovRel[i].Axis = axRef;
        fbMovAbs[i].Axis = axRef;
        fbMovVel[i].Axis = axRef;
        fbGearIn[i].Slave = axRef;
        fbGearOut[i].Slave = axRef;
        fbStp[i].Axis = axRef;
        fbHlt[i].Axis = axRef;
        fbHome[i].Axis = axRef;
        fbSetPos[i].Axis = axRef;
    }

    void runCycle() override {
        cnt++;
        for (int i = 0; i < _axNum; i++) {
            runCycleSgAx(i);
        }
        if (sendStat) {
            sendStat = false;
        }
    }
    void runCycleSgAx(int i) {
        fbRest[i].Execute = executeArr[i][FBM_RESET];
        fbRest[i].runCycle();

        fbPow[i].Enable = true;
        fbPow[i].bRegulatorOn = executeArr[i][FBM_POWER];
        fbPow[i].bDriveStart = executeArr[i][FBM_POWER];
        fbPow[i].runCycle();

        fbStp[i].Execute = executeArr[i][FBM_STOP];
        fbStp[i].runCycle();

        fbHlt[i].Execute = executeArr[i][FBM_HALT];
        fbHlt[i].runCycle();

        fbMovAbs[i].Execute = executeArr[i][FBM_MOVE_ABSOLUTE];
        if (executeArr[i][FBM_MOVE_ABSOLUTE]) {
            set_fb_vel_config(fbMovAbs[i], cmdVel, cmdAcc, cmdAcc, 0);
            fbMovAbs[i].Position = cmdPos;
        }

        fbMovRel[i].Execute = executeArr[i][FBM_MOVE_RELATIVE];
        if (executeArr[i][FBM_MOVE_RELATIVE]) {
            set_fb_vel_config(fbMovRel[i], cmdVel, cmdAcc, cmdAcc, 0);
            fbMovRel[i].Distance = cmdPos;
        }
        fbMovAdd[i].Execute = executeArr[i][FBM_MOVE_ADDITIVE];
        if (executeArr[i][FBM_MOVE_ADDITIVE]) {
            set_fb_vel_config(fbMovAdd[i], cmdVel, cmdAcc, cmdAcc, 0);
            fbMovAdd[i].Distance = cmdPos;
        }
        fbMovVel[i].Execute = (executeArr[i][FBM_MOVE_VELOCITY]);

        set_fb_vel_config(fbMovVel[i], cmdVel, cmdAcc, cmdAcc, 0);
        // }
        fbMovAbs[i].runCycle();
        fbMovRel[i].runCycle();
        fbMovAdd[i].runCycle();
        fbMovVel[i].runCycle();

        fbHome[i].Execute = executeArr[i][FBM_HOME];
        fbHome[i].Position = cmdPos;
        fbHome[i].runCycle();

        fbGearIn[i].Execute = executeArr[i][FBM_GEAR_IN];
        if (executeArr[i][FBM_GEAR_IN]) {
            fbGearIn[i].Master = axArr[axMasterIdx];
            set_fb_acc_config(fbGearIn[i], cmdAcc, cmdAcc, 0);
        }
        fbGearIn[i].runCycle();
        fbGearOut[i].Execute = executeArr[i][FBM_GEAR_OUT];
        fbGearOut[i].runCycle();
        fbSetPos[i].Execute = executeArr[i][FBM_SET_POSITION];
        fbSetPos[i].Position = cmdPos;
        fbSetPos[i].runCycle();
        // ax[i]->runCycle();
        // if (cnt % 1000 == 1)
        //     show(i);
    }
    void show() {
        for (int i = 0; i < _axNum; i++) show(i);
    }
    AXIS_REF axisRef(int i) { return axArr[i]; };
    AxisImplBase& axRef(int i){
        return *((AxisImplBase*) axArr[i]);
    }
    void show(int i) {
        auto& ax = axRef(i);

        auto axStat = ax.axisStat;
        int posAct = ax.diActPosition;
        // posAct = servo.get_pos();
        LREAL pos = ax.fSetPosition;
        LREAL vel = ax.fSetVelocity;
        LREAL acc = ax.fSetAcceleration;

        printf("%d,%d,%d,%d,%f,%f,%f\n", cnt, i, axStat, posAct, pos, vel, acc);
        // printf("0x%x, 0x%x, %d, %f\n", ctrl, stat, posAct);
    }
    void show_state() {
        for (int i = 0; i < _axNum; i++) show_state(i);
    }
    void show_state(int i) {
        auto& ax = axRef(i);
        int pos = ax.diSetPosition;
        int mpul = ax.diActPosition;
        auto axStat = ax.axisStat;
        printf("%d,%d,%d,0x%x,%d,%d\n", cnt, i, axStat, ax.dwErrorID, pos, mpul);
        // printf("0x%x, 0x%x, %d, %f\n", ctrl, stat, posAct);
    }
    void show_power() {
        // for (int i = 0; i < _axNum; i++) ax[i]->print_power();
    }
    void show_motbuf() {
        for (int i = 0; i < _axNum; i++) show_motbuf_sg(i);
    }
    void show_motnode() {
        for (int i = 0; i < _axNum; i++) show_motnode_sg(i);
    }
    void show_motbuf_sg(int i) { } // ax[i]->print_motbuf(); }
    void show_motnode_sg(int i) { } //ax[i]->print_motnode(); }
    void show_power_sg(int i) { } //ax[i]->print_power(); }
    void show_mparam() {
        printf("%f,%f,%f\n", cmdPos, cmdVel, cmdAcc);
        // printf("0x%x, 0x%x, %d, %f\n", ctrl, stat, posAct);
    }

    void show_home_fb_sg(int i) {
        // printf("home=0x%x, error=%d\n", get_fb_stat(&fbHome[i]), fbHome[i].get_fb_error_id());
        // printf("0x%x, 0x%x, %d, %f\n", ctrl, stat, posAct);
    }
    void setAxisSelect(int f) { axSelect = f; }
    void setMasterAxis(int f) { axMasterIdx = f; }
    void setPowerOn() { executeArr[axSelect][FBM_POWER] = true; }
    void setPowerOff() { executeArr[axSelect][FBM_POWER] = false; }

    void sendReset() {
        executeArr[axSelect][FBM_RESET] = true;
        sendStat = true;
    }
    void sendResetLow() { executeArr[axSelect][FBM_RESET] = false; }

    void sendFault() {}

    void setCmdPos(double v) { cmdPos = v; }
    void setCmdVel(double v) { cmdVel = v; }
    void setCmdAcc(double v) { cmdAcc = v; }
    void sendExecute(int v) {
        executeArr[axSelect][FBM_MOVE_ABSOLUTE] = v & 1;
        executeArr[axSelect][FBM_MOVE_RELATIVE] = v & 2;
        executeArr[axSelect][FBM_MOVE_ADDITIVE] = v & 4;
        executeArr[axSelect][FBM_MOVE_VELOCITY] = v & 8;
        sendStat = true;
    }

    void sendExecuteMoveAbs(bool v) {
        executeArr[axSelect][FBM_MOVE_ABSOLUTE] = v;
    }
    void sendExecuteMoveRel(bool v) {
        executeArr[axSelect][FBM_MOVE_RELATIVE] = v;
    }
    void sendExecuteMoveAdd(bool v) {
        executeArr[axSelect][FBM_MOVE_ADDITIVE] = v;
    }
    void sendExecuteMoveVel(bool v) {
        executeArr[axSelect][FBM_MOVE_VELOCITY] = v;
    }

    void sendAxisLow() { memset(executeArr[axSelect], 0, FBM_Number); }
    void sendAxisMotLow() {
        memset(&executeArr[axSelect][FBM_HOME], 0, FBM_Number - FBM_HOME);
    }
    void sendStop() {
        executeArr[axSelect][FBM_STOP] = true;
        sendStat = true;
    }
    void sendStopLow() { executeArr[axSelect][FBM_STOP] = false; }
    void sendHalt() {
        executeArr[axSelect][FBM_HALT] = true;
        sendStat = true;
    }
    void sendHaltLow() { executeArr[axSelect][FBM_HALT] = false; }
    void sendHome() {
        executeArr[axSelect][FBM_HOME] = true;
    }
    void sendHomeLow() { executeArr[axSelect][FBM_HOME] = false; }

    void sendHoming() {
        // executeArr[axSelect][FBM_HOMING] = true;
        // sendStat = true;
    }
    void sendSetPos() {
        executeArr[axSelect][FBM_SET_POSITION] = true;
        sendStat = true;
    }
    void sendGearIn() {
        executeArr[axSelect][FBM_GEAR_IN] = true;
        sendStat = true;
    }
    void sendGearInLow() { executeArr[axSelect][FBM_GEAR_IN] = false; }
    void sendGearOut() {
        executeArr[axSelect][FBM_GEAR_OUT] = true;
        sendStat = true;
    }
    void sendGearOutLow() { executeArr[axSelect][FBM_GEAR_OUT] = false; }
    inline unsigned int size() const { return _axNum; }

    unsigned int cnt = 0;
    bool exitFlag = false;

    int _axNum;
    AXIS_REF* axArr;
    FbtPower* fbPow;
    FbtMoveRelative* fbMovRel;
    FbtMoveAbsolute* fbMovAbs;
    FbtMoveAdditive* fbMovAdd;
    FbtMoveVelocity* fbMovVel;
    FbtGearIn* fbGearIn;
    FbtGearOut* fbGearOut;
    FbtReset* fbRest;
    FbtStop* fbStp;
    FbtHalt* fbHlt;
    FbtHome* fbHome;
    FbtSetPosition* fbSetPos;

   private:
    bool (*executeArr)[FBM_Number];
    uint8_t axMasterIdx = 0;
    int axSelect = 0;
    // int moveCmd = 0;
    int sendStat = 0;
    double cmdPos = 10.;
    double cmdVel = 5.;
    double cmdAcc = 20;
};

}  // namespace zmc

#endif