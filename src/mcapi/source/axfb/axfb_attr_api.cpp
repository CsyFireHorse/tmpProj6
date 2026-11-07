#include "axfb_attr_api.h"
#include "include/axfb/FbAttrVisitor.h"

using namespace zmc;

class AxFbAttrBaseWrap:public AxFbAttrBase{
};


void axfb_attr_destroy(AxFbAttrHandle handle){
    delete (AxFbAttrBase*) handle;
}



int axfb_attr_set(AxFbAttrHandle handle, int name, double value) {
    auto obj = static_cast<AxFbAttrBase*>(handle);
    return obj->setAttr(name, value);
}

int axfb_attr_set_ptr(AxFbAttrHandle handle, int name, const void* value) {
    auto obj = static_cast<AxFbAttrBase*>(handle);
    return obj->setPtrAttr(name, value);
}

int axfb_attr_get(AxFbAttrHandle handle, int name, double* out_value) {
    auto obj = static_cast<AxFbAttrBase*>(handle);
    return obj->getAttr(name, *out_value);
}

int axfb_attr_get_ptr(AxFbAttrHandle handle, int name, void** out_ptr) {
    auto obj = static_cast<AxFbAttrBase*>(handle);
    void* ptr = nullptr;
    int ret = obj->getPtrAttr(name, ptr);
    *out_ptr = ptr;
    return ret;
}

void axfb_attr_run_cycle(AxFbAttrHandle handle) {
    auto obj = static_cast<AxFbAttrBase*>(handle);
    obj->runCycle();
}
