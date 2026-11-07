#include <functional>
#include <utility>
#include <vector>

#include "AxisToy.hpp"
#include "TrapzVel.h"
#include "VelGearFollower.h"
#include "VelPosFollower.h"
#include "common/misc/TrapzVelToyForwarder.hpp"
#include "common/misc/CmdArgsToy.hpp"
#include "stdio.h"
using namespace zmc;

void print_help(){
    printf("Usage: --help \n [--help] [--axis_0.vel <double>] [--axis_0.pos <double>] [--move.Position <double>] [--move.EndVelocity <double>] [--move.Velocity <double>] [--move.Acceleration <double>] [--move.Deceleration <double>] [--move.Jerk <double>] [--gear.Velocity <double>] [--gear.Acceleration <double>] [--gear.Deceleration <double>] [--gear.Jerk <double>] [--verbose <int>] [--cycles <int>]" );
}

template<typename T>
class AxToyPouSlaveTmpl : public PouTaskBase{
   public:
    CmdArgsToy args;
    AxToyPouSlaveTmpl() {
        auto& args = CmdArgsToySingleton::get();
        this->args = args;
        double vel = args["move.Velocity"].get<double>(4);
        double acc = args["move.Acceleration"].get<double>(3);
        double dec = args["move.Deceleration"].get<double>(3);
        double jerk = args["move.Jerk"].get<double>(0);
        this->vMove.set_config(vel, acc, dec, jerk);
        double position = args["move.Position"].get<double>(-1e5);
        double end_vel = args["move.EndVelocity"].get<double>(0);
        this->vMove.set_end_pva(position, end_vel);
        
        double gear_vel = args["gear.Velocity"].get<double>(5);
        double gear_acc = args["gear.Acceleration"].get<double>(3);
        double gear_dec = args["gear.Deceleration"].get<double>(3);
        double gear_jerk = args["gear.Jerk"].get<double>(0);
        int num_gear = args["gear.num"].get<int>(1);
        int den_gear = args["gear.den"].get<int>(1);
        this->vGear.set_num_den(num_gear, den_gear);
        this->vGear.set_config(gear_vel, gear_acc, gear_dec, gear_jerk);
    }
    
    void runCycle() override {
        this->vMove.Axis = axisArr[0];
        this->vGear.Master = axisArr[0];
        this->vGear.Slave = axisArr[1];
        this->vMove.Execute = true;
        if (this->getIndex() == 2) {
            this->vGear.Execute = true;
        }
        this->vMove.runCycle();
        this->vGear.runCycle();
        this->idx++;
    }   
    int getIndex() const {
        return this->idx;
    }
    AxisToy* axisArr[2];   
    VelToyMoveTmpl<ToyTrapzVelPlanner> vMove;
    VelToyGearTmpl<T> vGear;
    int idx = 0;
};


template<typename T>
int VelToyPouPosFollowerTmpl(int argc, char** argv) {
    if (argc && (argv[0] == "--help" || argv[0] == "-h")){
        print_help();
        return 0;
    }
    CmdArgsToySingleton::parse(argc, argv);
    auto& argsToy = CmdArgsToySingleton::get();

    AxToyPou runner(2);
    AxToyPouSlaveTmpl<T> pou;
    runner.axisArr[0].pos = argsToy["axis_0.pos"].get<double>(5);
    runner.axisArr[0].vel = argsToy["axis_0.vel"].get<double>(-4);
    runner.verbose_flag = argsToy["verbose"].get<int>(1);
    runner.count_to_exit = argsToy["cycles"].get<int>(200);
    runner.bind(0, &pou);
    pou.axisArr[0] = &runner.axisArr[0];
    pou.axisArr[1] = &runner.axisArr[1];
    runner.run();
    return 0;
}

int VelToyPouFollower_demo(int argc, char** argv) {
    return VelToyPouPosFollowerTmpl<VelGearFollower>(argc, argv);
}

int VelToyPouPosFollower_demo(int argc, char** argv) {
    return VelToyPouPosFollowerTmpl<VelPosFollower>(argc, argv);
}

int VelToyPouPvFollower_demo(int argc, char** argv) {
    return VelToyPouPosFollowerTmpl<VelPvFollower>(argc, argv);
}

int VelToyPouPvaFollower_demo(int argc, char** argv) {
    return VelToyPouPosFollowerTmpl<VelPvaFollower>(argc, argv);
}