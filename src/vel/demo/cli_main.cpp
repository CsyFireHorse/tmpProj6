/*
 * @Author: ChenSy
 * @LastEditors: Chen Sy
 * @Date: 2022-02-28 11:15:50
 * @LastEditTime: 2022-11-08 17:47:10
 * @Description: Modify here please
 */
#include "string.h"
#include <iostream>
#include "math.h"
#include "VelCWrap.h"
#include "TrapzVel.h"
#include "VelUnCppWrap.h"
#include "velocity_utils.hpp"
#include "vel_sin_square.hpp"

using namespace std;

static const char* EXE_NAME = "vel_cli";

static
int printVersion(int argc, char **argv){
    printf("%s: velocity cmdline tool\n"
            "version 1.0.0\n"
            "create by csy on 2022/11/01\n"
            , EXE_NAME);
    return 0;
}

static 
int printHelp(int argc, char **argv){
    printf("%s --version/-V\n"
        " -s 1 -vm 1 -acc 5 -dec 5 -j 10\n"
        " -s 1 -vm 1 -v0 0 -vd 0\n"
        " -s 1 -vm 1 -vt 1\n"
        " -ts 0.5 \n"
        " -s0 -1 s0 2 \n"
        " --help/-h\n", EXE_NAME);
    return 0;
}

typedef struct _ZalParams_t{
    unsigned char is_verbose = 0; 
    int vtype = 0; 

    double s0 = 0; 
    double s = 1; 
    double v0 = 0; 
    double vd = 0; 
    double vmax = 1; 
    double acc = 1; 
    double dec = 1; 
    double a0 = 0; 
    double ad = 0;
    double jerk = 0; 
    double tc = 0; 
    double ts = 0.01; 
    int max_tick = 0;
}_ZalParams;

static _ZalParams _g_params;

int parse_params(int argc, char** argv){
    int i = 0;
    while (i < argc){
        if(0 == strcmp(argv[i], "-vt") || 0 == strcmp(argv[i], "--vtype")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.vtype = atoi(argv[i]);
        }else if(0 == strcmp(argv[i], "-ts")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.ts = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-s") || 0 == strcmp(argv[i], "--s")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.s = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-s0") || 0 == strcmp(argv[i], "--s0")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.s0 = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-v0") || 0 == strcmp(argv[i], "--v0")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.v0 = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-vd") || 0 == strcmp(argv[i], "--vd")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.vd = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-a0")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.a0 = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-vm") || 0 == strcmp(argv[i], "--vmax")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.vmax = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-acc") || 0 == strcmp(argv[i], "--acc")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.acc = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-dec") || 0 == strcmp(argv[i], "--dec")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.dec = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-j") || 0 == strcmp(argv[i], "--jerk")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.jerk = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "-tc") || 0 == strcmp(argv[i], "--tc")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.tc = atof(argv[i]);
        }else if(0 == strcmp(argv[i], "--verbose") || 0 == strcmp(argv[i], "-v")) {
            _g_params.is_verbose = 1;
        }else if(0 == strcmp(argv[i], "-max_tick") || 0 == strcmp(argv[i], "--max_tick")) {
            i++;
            if (i>=argc){
                return 1;
            }
            _g_params.max_tick = atoi(argv[i]);
        }
        i++;
    }
    return 0;
}

struct Foo{ 
    void* ptr;
};




template<typename T>
void vel_build_print(){
    T vel = T();
    vel.set_config(_g_params.vmax, _g_params.acc, _g_params.dec, _g_params.jerk);
    vel.set_begin_pva(_g_params.s0, _g_params.v0, _g_params.a0);
    vel.set_end_pva(_g_params.s, _g_params.vd, _g_params.ad);
    // vel.set_total_time(_g_params.tc);
    int errCode = vel.build();
    if (errCode == 0){
        zmc::prt_vel(vel, _g_params.ts, _g_params.max_tick);
        // printf("%f\n",vel.get_end_time());
    }else{
        printf("build failed\n");
    }   
}

void vel_interfaces_demo0(){
    vel_handle handle = vel_new();
    vel_set_config(handle, _g_params.vmax, _g_params.acc, _g_params.dec, _g_params.jerk);
    vel_set_begin_pos_vel(handle, _g_params.s0, _g_params.v0);
    vel_set_end_pos_vel(handle, _g_params.s, _g_params.vd);
    // vel_set_begin_end_acc(handle, _g_params.a0, 0);
    // vel_set_total_time(handle, _g_params.tc);
    int errCode = vel_build(handle);
    Foo foo = {handle};
    if (errCode == 0){
        // zmc::prt_vel(*(zmc::TrapzVel*) handle, _g_params.ts);
        // printVel(*(zmc::TrapzVel*) handle, vel_get_end_time(handle), _g_params.ts);
    }else{
        printf("build failed\n");
    }   
    vel_delete(handle);
}
class VelocityWrapSinquare:public zmc::VelocityWrap{
public:
    VelocityWrapSinquare(){ set_sinsquare(true);}
};

void vel_interfaces_demo(){
    if (_g_params.vtype == 0)
        vel_build_print<zmc::TrapzVel>();
    else if (_g_params.vtype == 1)
        vel_build_print<zmc::VelSinSquarePlanner>();
    else if (_g_params.vtype == 11)
        vel_build_print<VelocityWrapSinquare>();
    else
        vel_build_print<zmc::VelocityWrap>();
}

int runForward(int argc, char** argv){
    int err = parse_params(argc, argv);
    if (err)
        return err;
    else{
        vel_interfaces_demo();
    }
    return err;
}

int parse_args(int argc, char** argv){
    if (argc == 1){
        return printHelp(argc-1, argv+1);
    }else if(argc >= 2 && (strcmp( argv[1], "-h")==0 || strcmp(argv[1], "--help")==0)){
        return printHelp(argc-1, argv+1);
    }else if(argc >= 2 && (strcmp( argv[1], "-V")==0 || strcmp(argv[1], "--version")==0)){
        return printVersion(argc-1, argv+1);
    }else if(argc >= 2){
        return runForward(argc-1, argv+1);
    }else{
        return printHelp(argc-1, argv+1);
    }
    return 0;
}

int cli_main(int argc, char** argv){
    if (argc == 1) {
        #define LEN(x) sizeof(x)/sizeof(*(x))
        // char * argv2[] = {"", "-s","60","-vm","3","-v0","1","--acc","2","--dec","1","-j","3","--vtype","3"};
        // char * argv2[] = {"", "-s", "30", "-vm", "3", "-v0", "4", "-tc", "10"}; // nseg31
        // char * argv2[] = {"", "-s", "30", "-vm", "3", "-v0", "3", "-vd", "3"}; // nseg10
        // char * argv2[] = {"", "-s", "0.1", "-vm", "3", "-v0", "-2"};
        // char * argv2[] = {"", "-s", "10", "-vm", "3", "-v0", "3", "-vd", "4"}; // nseg25
        // char * argv2[] = {"", "-s", "30", "-vm", "3", "-v0", "2", "-vd", "1", "-tc", "40"}; // nseg30 T
        char * argv2[] = {"", "-s", "10", "-vm", "3", "-v0", "1", "-vd", "1.5", "--acc", "5", "--dec", "3", "-tc", "2.625"}; // nseg30 T faild
        int argc2 = LEN(argv2);
        return parse_args(argc2, argv2);
    }
    return parse_args(argc, argv);
}