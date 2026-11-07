/* File: example.i */


%{
#include "stdlib.h"
#include "../mcapi/include/axfb_attr_api.h"
%}

typedef class AxFbAttrBaseWrap AxFbAttrBaseWrap;
typedef AxFbAttrBaseWrap* AxFbAttrHandle;

// ZMC_DLLPORT AxFbAttrHandle axfb_attr_create();
ZMC_DLLPORT void axfb_attr_destroy(AxFbAttrHandle handle);

ZMC_DLLPORT int axfb_attr_set(AxFbAttrHandle handle, int name, double value);
ZMC_DLLPORT int axfb_attr_set_ptr(AxFbAttrHandle handle, int name, void* value);
ZMC_DLLPORT int axfb_attr_get(AxFbAttrHandle handle, int name, double* out_value);
ZMC_DLLPORT int axfb_attr_get_ptr(AxFbAttrHandle handle, int name, void** out_ptr);

ZMC_DLLPORT void axfb_attr_run_cycle(AxFbAttrHandle handle);