#include "FbFactory.h"
#include "FbAttrVisitor.h"
#include "stdio.h"
#include "errorCode2str.hpp"

using namespace zmc;
void fbFactory_demo() {
    FactoryCmdProc processor;

    // 创建Axis实例
    processor.createAxis(AX_FACTORY_VIRTUAL);
    // 创建FB实例
    processor.createFb(FB_TYPE_POWER);
    processor.createFb(FB_TYPE_MOVEABSOLUTE);
    // 处理指令
    CommandStruct cmd{1, FB_ATTR_AXIS, 0, 1};
    processor.process(cmd);

    CommandStruct cmds[] = {
        {1, FB_ATTR_ENABLE, AXFB_NOP, 0, 1},
        {1, FB_ATTR_BREGULATORON, AXFB_NOP, 0, 1},
        {1, FB_ATTR_BDRIVESTART, AXFB_NOP, 0, 1},
        // {2, TTR, FB_ATTR_AXIS, 0, 1},
        // {2, FB_ATTR_POSITION, 0, 3.5},
        // {2, FB_ATTR_VELOCITY, 0, 5},
        // {2, FB_ATTR_ACCELERATION, 0,  20},
        // {2, FB_ATTR_DECELERATION, 0, 20},
        {1, FB_ATTR_RUN, AXFB_NOP},
        {1, FB_ATTR_AXRUN, AXFB_NOP},
        {1, FB_ATTR_RUN, AXFB_NOP},
        {1, FB_ATTR_AXRUN, AXFB_NOP},
        {1, FB_ATTR_RUN, AXFB_NOP},
        {1, FB_ATTR_AXRUN, AXFB_NOP},
        {1, FB_ATTR_RUN, AXFB_NOP},
        {1, FB_ATTR_AXRUN, AXFB_NOP},
        {1, FB_ATTR_RUN, AXFB_NOP},
        {1, FB_ATTR_AXRUN, AXFB_NOP},
        {1, FB_ATTR_RUN, AXFB_NOP},
        {1, FB_ATTR_AXRUN, AXFB_NOP},
    };
    #define LEN(x) (sizeof(x)/sizeof(x[0]))

    auto fb = processor.getFb(1);
    for (int i=0; i<LEN(cmds); i++){
        processor.process(cmds[i]);
        double val, val2;
        fb->getAttr(FB_ATTR_STATUS, val); 
        fb->getAttr(FB_ATTR_BUSY, val2);
        int st = val * 2 + val2;
        printf("%d,%d\n", i, st);
    }    
}
