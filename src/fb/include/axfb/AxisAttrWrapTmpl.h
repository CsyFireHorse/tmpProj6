#ifndef _ZMC_AXIS_ATTR_WRAP_TMPL_H_
#define _ZMC_AXIS_ATTR_WRAP_TMPL_H_
#include "AxisAttrWrap.h"

namespace zmc{

class AxisAttrNop {
    char a[4];
};

template <typename T>
class AxisAttrWrapTmpl : public AxisAttrWrap {
   public:
    T ax;
    AxisAttrWrapTmpl() = default;
    ~AxisAttrWrapTmpl() = default;
    void set_config(const AxisCiaConfig* config)override{
        ax.set_config(config);
    }
    int setAttr(int name, double value) override {
        return AxAttrVisitorWrite((AxisImplBase&) ax, (AxAttr)name, value);
    };
    int getAttr(int name, double& value) const override {
        return AxAttrVisitorRead((const AxisImplBase&) ax, (AxAttr)name, value);
    };
    int setPtrAttr(int name, const void* value) override {
        return AxAttrVisitorWrite((AxisImplBase&) ax, (AxAttr)name, value);
    };
    int getPtrAttr(int name, void*& value)  const override {
        return AxAttrVisitorRead((const AxisImplBase&) ax, (AxAttr)name, value);
    };

    AXIS_REF getAxis() override { return (AXIS_REF) &ax; }
    void runCycle() override{ ax.runCycle();}
};

template <>
class AxisAttrWrapTmpl<AxisAttrNop> : public AxisAttrWrap {
    AxisAttrNop ax;
    int setAttr(int name, double value) override { return -1;};
    int getAttr(int name, double& value) const override { return -1;};
    int setPtrAttr(int name, const void* value) override { return -1;};
    int getPtrAttr(int name, void*&) const override { return -1;};

    AXIS_REF getAxis() override { return nullptr; }
    void set_config(const AxisCiaConfig* config)override{ }
    void runCycle() override { }
};
}
#endif