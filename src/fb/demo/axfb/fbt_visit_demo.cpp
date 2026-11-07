#include "stdio.h"
#include "FbAttrVisitorSpec.h"

using namespace zmc;

void FbAttrVisitorWrite2_demo(){
    FbAttrMoveAbsStruct fb;
    // FbAttrVisitorWrite<AttrPairtStruct>(fb, FB_ATTR_AXIS, 100);

    FbAttrVisitorWrite(fb, FB_ATTR_POSITION, 100);
    double pos;
    FbAttrVisitorRead(fb, FB_ATTR_POSITION, pos);
    printf("%f, %f\n", 100., pos);
    FbAttrPowerStruct fb2;
    // FbAttrVisitorWrite(fb2, FB_ATTR_AXIS, 100);
    double val;
    FbAttrVisitorWrite(fb2, FB_ATTR_STATUS, 100);
    FbAttrVisitorRead(fb2, FB_ATTR_STATUS, val);
    printf("%f\n", val);
        //  (double) ((void*) (&fb))
    {
        double ptr =  (double&) fb;
        UnVoidPtrDouble un = {&fb};
        un.d;
        FbAttrVisitorWrite(fb2, FB_ATTR_AXIS, un.d);
        FbAttrVisitorRead(fb2, FB_ATTR_AXIS, val);
        UnDoubleVoidPtr un2 = {val};
        printf("0x%x, 0x%x\n", un2.p, &fb);
    }
    void* ptrIn = (void*) (&fb2);
    void* ptrOut;
    FbAttrVisitorWrite(fb, FB_ATTR_AXIS, ptrIn);
    FbAttrVisitorRead(fb, FB_ATTR_AXIS, ptrOut);
    printf("0x%x, 0x%x\n", ptrIn, ptrOut);
}
