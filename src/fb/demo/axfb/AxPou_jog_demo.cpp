#include "AxPou.hpp"
#include "fb_cpp_def.h"
#include <iostream>
#include "AxisCiaConfig.hpp"
#include "fbt_axis_base.h"
#include "fbt_axis.h"
#include "axPouTmplRunner.hpp"

using namespace zmc;

void setFbForward(void* pFb, bool v){
    // FbtJog.JogForward offset to FbtJog is same as FbtInch.InchForward offset to FbtInch
    static_cast<FbtJog*>(pFb)->JogForward = v;
}
void setFbBackward(void* pFb, bool v){
    // FbtJog.JogBackward offset to FbtJog is same as FbtInch.InchBackward offset to FbtInch
    static_cast<FbtJog*>(pFb)->JogBackward = v;
}

template<typename T>
class JogTaskTmpl : public PouTaskBase {
public:
    JogTaskTmpl() {
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        axis->bind_extra_data(&jogStat);
        // printAxisHeader(9);
        set_fb_vel_config(jog, 30, 80, 80, 500);
    }
    void setForward(bool v){
        setFbForward(&jog, v);
    }
    void setBackward(bool v){
        setFbBackward(&jog, v);
    }

    void runCycle() override {
        int cycle = getIndex();
        power.Axis = axis;
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();

        // Jog demo: forward 20-50, halt 50~80, backward 80-120  forward 120~140
        jog.Axis = axis;
        setForward(power.Status && ((cycle >= 20 && cycle < 50) || (cycle >= 120 && cycle < 160)));
        setBackward(power.Status && ((cycle >= 80 && cycle < 120) || (cycle >= 140 && cycle < 190)));
        
        jog.runCycle();
        jogStat = jog.Busy + jog.CommandAborted * 5 + jog.Error * 4;
        // axis->print(9);
        cnt++;
    }
    int getIndex() const { return cnt;}
    AXIS_REF axis;
    FbtPower power;
    T jog;
    int32_t jogStat = 0;
    int cnt = 0;
};

typedef JogTaskTmpl<FbtJog> JogTask;    


static const std::string iniContent = R"(
[base]
axis_num = 1
[axis_0]
cycle_time = 50000
is_axis_virtual = true
)";

template<typename T>
void pou_jog_demo_tmpl() {
    AxPouTaskBinder pou("", iniContent);
    pou.setAxisSimulated(0, true);
    T task(pou);
    pou.bind(0, &task);
    pou.count_to_exit = 450;
    pou.run();
}

int pou_jog_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<JogTask>(argc, argv, iniContent, PouTmplDefaults{{"verbose_flag", "9"}});
}
class InchTask: public JogTaskTmpl<FbtInch>{
public:
    InchTask():JogTaskTmpl(){
        jog.Distance = 20;
    }
};

int pou_inch_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<InchTask>(argc, argv, iniContent, PouTmplDefaults{{"verbose_flag", "9"}});
}