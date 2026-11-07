#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "pou/AxisManager.h"
#include "fb_cpp_def.h"
#include "common/get_file_path.hpp"
#include <iostream>
#include "common/misc/CmdArgsToy.hpp"
#include "fbt_axis_morph.h"
#include "fbt_impl_enum_def.h"

using namespace zmc;


class PowerTask:public PouTaskBase{
public:
    PowerTask() {
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        power.Axis = axis;
    }
    void runCycle() override {
        if (cnt > 2 && cnt < 15){
            bPow = true;
        }else if(cnt>15) 
            bPow = false;
        power.Enable = true;
        power.bDriveStart = bPow;
        power.bRegulatorOn = bPow;
        power.runCycle();
        axis->print(1);
        cnt++;
    }
    AXIS_REF axis;
    FbtPower power;
    int cnt = 0;
    bool bPow = false;
};

void pou_power_demo(){
    CmdArgsToy args;
    AxPouTaskBinderAuto pou(1);
    pou._setAllAxisVirtual();
    PowerTask task;
    pou.bind(0, &task);
    pou.run();
}


class MoveTask:public PouTaskBase{
public:
    MoveTask() {
        auto& args = CmdArgsToySingleton::get();
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        
        power.Axis = axis;
        move.Axis = axis;
        move.Position = args["move.Position"].get<double>(100.0);
        move.Velocity = args["move.Velocity"].get<double>(10.0);
        move.Acceleration = args["move.Acceleration"].get<double>(20.0);
        move.Deceleration = args["move.Deceleration"].get<double>(20.0);
        move.Jerk = args["move.Jerk"].get<double>(0);
    }
    void runCycle() override {
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();

        move.Execute = power.Status;
        move.runCycle();
        // axis->print(1);
    }
    AXIS_REF axis;
    FbtPower power;
    FbtMoveAbsolute move;
};

void pou_split_demo(){
    CmdArgsToy args;
    AxPouTaskBinderAuto pou(1);
    pou._setAllAxisVirtual();
    MoveTask task;
    pou.bind(0, &task);
    pou.run();
}



template<typename T>
class MoveTmplTask:public PouTaskBase{
public:
    MoveTmplTask() {
        auto& args = CmdArgsToySingleton::get();
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        
        power.Axis = axis;
        move.Axis = axis;
        move.Velocity = args["move.Velocity"].get<double>(10.0);
        move.Acceleration = args["move.Acceleration"].get<double>(20.0);
        move.Deceleration = args["move.Deceleration"].get<double>(20.0);
        move.Jerk = args["move.Jerk"].get<double>(0);
    }
    void runCycle() override {
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();

        move.Execute = power.Status;
        move.runCycle();
        // axis->print(1);
    }
    AXIS_REF axis;
    FbtPower power;
    T move;
};

class MoveVelTask:public MoveTmplTask<FbtMoveVelocity>{
public:
    MoveVelTask() {
        auto& args = CmdArgsToySingleton::get();
        move.Direction = (MC_DIRECTION) args["move.Direction"].get<int>(2);
    }
};
class MoveContAbsTask:public MoveTmplTask<FbtMoveContinuousAbsolute>{
public:
    MoveContAbsTask() {
        auto& args = CmdArgsToySingleton::get();
        move.Position = args["move.Position"].get<double>(100.0);
        move.EndVelocityDirection = (MC_DIRECTION) args["move.EndVelocityDirection"].get<int>(2);
        move.Direction = (MC_DIRECTION) args["move.Direction"].get<int>(2);
        move.EndVelocity = args["move.EndVelocity"].get<double>(10.0);
    }
};
class MoveContRelTask:public MoveTmplTask<FbtMoveContinuousRelative>{
public:
    MoveContRelTask() {
        auto& args = CmdArgsToySingleton::get();
        move.Distance = args["move.Distance"].get<double>(100.0);
        move.EndVelocityDirection = (MC_DIRECTION) args["move.EndVelocityDirection"].get<int>(2);
        move.EndVelocity = args["move.EndVelocity"].get<double>(10.0);
        // move.Direction = (MC_DIRECTION) args["move.Direction"].get<int>(2);
    }
};

int pou_move_cont_abs_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<MoveContAbsTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
}
int pou_move_cont_rel_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<MoveContRelTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
}

int pou_movev_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<MoveVelTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
}

int pou_demo(int argc, char** argv) { 
    return pou_auto_tmpl_demo<MoveTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
} 


class MoveHaltTask:public PouTaskBase{
public:
    MoveHaltTask() {
        auto& args = CmdArgsToySingleton::get();
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        
        power.Axis = axis;
        move.Axis = axis;
        move.Distance = args["move.Distance"].get<double>(1.0);
        move.Velocity = args["move.Velocity"].get<double>(10.0);
        move.Acceleration = args["move.Acceleration"].get<double>(20.0);
        move.Deceleration = args["move.Deceleration"].get<double>(20.0);
        move.Jerk = 0.0;
        // printVerboseHeader((AxVerboseFlag) 1);
    }

    void runCycle() override {
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();

        move.Execute = power.Status;
        move.runCycle();

        mHalt.Execute = !power.Status || index>70 ; // move.Done;
        mHalt.runCycle();
        // axis->print(1);
        index++;
    }
    AXIS_REF axis;
    FbtPower power;
    FbtMoveRelative move;
    FbtHalt mHalt;
    int index=0;
};

int pou_halt_demo(int argc, char** argv) { 
    return pou_auto_tmpl_demo<MoveHaltTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
} 

class MorphTask : public PouTaskBase {
public:
    MorphTask(AxPouTaskBinder& pou) {
        axis = pou.axisRef(0);
    }
    void runCycle() override {
        static int32_t testSequence[] = {0, 10, 20, 40, 60, 80, 100, 200, 250, 255, 0, 1, 50, 100};
        static int sequenceIdx = 0;
        if (sequenceIdx < (int)(sizeof(testSequence)/sizeof(testSequence[0]))) {
            ((AxisMorph*)axis)->set_encoder_pos(testSequence[sequenceIdx]);
            sequenceIdx++;
        }
        axis->print(1);
    }
    AXIS_REF axis;
};

void morphPou_mini_demo() {
    std::string iniContent = R"(
[base]
axis_num = 1
[axis_0]
cycle_time = 100000
axis_index = 3
bit_width = 8
debug.eAxisMtype = AXIS_TYPE_ENCODER
)";
    AxPouTaskBinder pou("", iniContent);
    MorphTask task(pou);
    pou.bind(0, &task);
    pou.count_to_exit = 22;
    pou.run();
}
