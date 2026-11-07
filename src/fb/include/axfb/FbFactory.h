#ifndef _ZMC_FB_FACTORY_H_
#define _ZMC_FB_FACTORY_H_


#include "FbFactoryDef.h"
#include "AxisAttrWrap.h"
#include "FbAttrVisitor.h"
class AxisCiaConfig;

namespace zmc{
class AxFbFactory;
class FbWrapper;


class CommandProcessor {
    AxFbFactory& factory;

   public:
    CommandProcessor();
    ~CommandProcessor();
    void createFb(FB_TYPE fbTyp);
    void createAxis(AX_FACTORY_TYPE axTyp);
    void createAxisArr(int num, AX_FACTORY_TYPE axType);

    void axisRunCycle(int idx);
    void fbRunCycle(int idx);
    AxisAttrWrap* getAxisWr(int id);
    AXIS_REF getAxis(int id);
    FbWrapper* getFb(int id);
    int getFbAttr(int fbId, int attrId, double& val);
    int getAxAttr(int fbId, int attrId, double& val);
    int getFbAttr(int fbId, int attrId, void* val);
    int setFbAttr(int fbId, int attrId, const void* val);
    void process(const CommandStruct& cmd);
    void process(const CommandStruct4& cmd);
    unsigned int getFbNum() const;
    unsigned int getAxisNum() const;
};

typedef CommandProcessor FactoryCmdProc;
void* create_fb(FB_TYPE fbTyp);
FbWrapper* create_fb_wrap(FB_TYPE fbTyp);

}
#endif