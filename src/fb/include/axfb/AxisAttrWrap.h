#ifndef _ZMC_AXIS_ATTR_WRAP_H_
#define _ZMC_AXIS_ATTR_WRAP_H_

#include "FbAttrVisitor.h"
#include "AxisRef.h"

class AxisCiaConfig;

namespace zmc{
    

class AxisAttrWrap :public AxFbAttrBase{
    public:
     inline int setAxAttr(AxAttr name, double value){
         return setAttr(name, value);
     };
     inline int getAxAttr(AxAttr name, double& value) const{
         return getAttr(name, value);
     };
     // virtual int setPtrAttr(int name, void* value) = 0;
     // virtual int getPtrAttr(int name, void*&) const = 0;
     virtual void set_config(const AxisCiaConfig* config) = 0;
     virtual AXIS_REF getAxis() { return nullptr; }
 };
 
 } // namespace zmc
#endif