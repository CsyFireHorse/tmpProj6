#include "AxFbVarMgr.h"
#include "stdio.h"
#include "mc_ref_def.h"
#include "AxFbVarMgr2.h"


using namespace zmc;
void VariableManager_demo() {
    zmc::VariableManager varMgr;
    varMgr.registerVar({"speed", sizeof(double), 0x01});

    varMgr.setValue("speed", 3.1415);
    double currentSpeed = varMgr.getValue<double>("speed");
    printf("Current speed: %f\n", currentSpeed);

    varMgr.registerVar({"path", sizeof(ZMC_STRING_80), 0x02});
    varMgr.setValue("path", ZMC_STRING_80{"path/3.1415"});
    auto path = varMgr.getValueCRef<ZMC_STRING_80>("path");
    printf("path: %s\n", path.value);
}


void VariableVectManager_demo() {
    zmc::VariableVectManager_v2 varMgr;
    varMgr.registerVar("speed", VarTag::VAR_DOUBLE);

    varMgr.setDouble(0, 3.1415);
    double currentSpeed = varMgr.getDouble(0);
    printf("[0]: %f\n", currentSpeed);

    varMgr.registerObjVar("www", sizeof(ZMC_STRING_80));
    ZMC_STRING_80 path0 = {"path/3.1415"};
    // varMgr.setObjByAddr(1, ZMC_STRING_80{"path/3.1415"}, sizeof(ZMC_STRING_80));
    varMgr.setObjValue(1, path0);
    auto path = varMgr.getObjValue<ZMC_STRING_80>(1);
    printf("[1]: %s\n", path.value);
}