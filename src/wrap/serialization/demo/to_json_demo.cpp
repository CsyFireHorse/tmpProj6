/*
 * @Author: ChenShengyong
 * @Date: 2024-08-02 16:42:28
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-08-02 18:01:17
 */
#include "servoFak_to_json.h"
#include "vel_to_json.h"

using namespace zmc;

void vel_2_json_demo() {
    VelocityWrap vel;
    vel.set_config(5, 3, 4, 0);
    vel.set_begin_end_pos_vel(0, 10, 0, 1);
    vel.build();

    json js;
    to_json(js, vel);
    auto jstr = js.dump(2);
    printf("%s\n", jstr.c_str());

    VelocityWrap vel2;
    auto js2 = json::parse(jstr);
    from_json(js2, vel2);
    vel2.get_end_rate();
}

void vel_load_demo() {
    VelocityWrap vel;
    // std::string fname("");
    std::string jstr = R"({ "impl": { "fw": { "_vs": 0.0, "vm": 1000000.0, "am": 3.0, "an": 3.0, "Varr": [ 2.6999999999999993, 1000000.0, 1000000.0, 1.8000000000465661, 0.0, 333332.43333333335, -0.8999874599976465, 333331.83334587334 ], "cTarr": [ 1.2149999999999999, 166666666666.6667, -166666666654.12665, 12.000030517578125, 0.0, 0.0, 0.0, 0.0 ], "cSarr": [ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 ] }, "param": { "A_0": 0.0, "A_d": 0.0, "A_m": 3.0, "A_n": 3.0, "J": 0.0, "J2": 0.0, "S_0": 1.3537499999999998, "S_d": 12.0, "Tc": 2.5188888888888887, "V_0": 2.849999999999999, "V_d": 1.8, "V_m": 1000000.0, "policy": 0 } } })";
    from_jsonstr(jstr, vel);
    int ret = vel.build();
    double vd = vel.get_end_rate();
    printf("%f\n", vel.check());

}

void AxisChecker_2_json_demo() {
    AxisChecker axCh;
    json js;
    to_json(js, axCh);
    auto jstr = js.dump(2);
    printf("%s\n", jstr.c_str());
}

void ServoSgFaker_2_json_demo() {
    ServoSgFaker svFk;
    json js;
    to_json(js, svFk);
    auto jstr = js.dump(2);
    printf("%s\n", jstr.c_str());
}