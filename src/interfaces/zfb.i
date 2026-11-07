/* File: zfb.i 
  author: ChenSy
  date: 2025-07-24
*/
%module zfbWrap

%{
#define SWIG_FILE_WITH_INIT
#include "stdlib.h"
%}

%include "carrays.i"
%array_functions(float, floatArray);
%array_functions(double, doubleArray);
%array_class(float, FloatArr);
%array_class(double, DoubleArr);

%include "cpointer.i"
%pointer_functions(float, Real)
%pointer_functions(double, LReal)
%pointer_class(float, RealPtr)
%pointer_class(double, LrealPtr)

%include "zmcw_common.h"




%inline %{
/* Create a new vector */
inline int get_value(int a){ return a + 1;}
%}


/* Convert from Python --> C */
%typemap(in) int32_t {
  $1 = PyInt_AsLong($input);
}
 
/* Convert from C --> Python */
%typemap(out) int32_t {
  $result = PyInt_FromLong($1);
}

/* Convert from Python --> C */
%typemap(in) uint32_t {
  $1 = PyInt_AsLong($input);
}
 
/* Convert from C --> Python */
%typemap(out) uint32_t {
  $result = PyInt_FromLong($1);
}
/* Convert from Python --> C */
%typemap(in) int16_t {
  $1 = PyInt_AsLong($input);
}
 
/* Convert from C --> Python */
%typemap(out) int16_t {
  $result = PyInt_FromLong($1);
}
/* Convert from Python --> C */
%typemap(in) uint16_t {
  $1 = PyInt_AsLong($input);
}
 
/* Convert from C --> Python */
%typemap(out) uint16_t {
  $result = PyInt_FromLong($1);
}

%typemap(in) ZMC_BOOL {
  $1 = PyObject_IsTrue($input);
}
 
/* Convert from C --> Python */
%typemap(out) ZMC_BOOL {
  $result = PyBool_FromLong($1);
}


/* Convert from Python --> C */
%typemap(in) ZMC_LREAL {
  $1 = PyFloat_AsDouble($input);
}
 
/* Convert from C --> Python */
%typemap(out) ZMC_LREAL {
  $result = PyFloat_FromDouble($1);
}

%include "vel.i"


%{
#include "../fb/include/mc_get_version.h"
#include "../fb/include/FbFactoryDef.h"
#include "../fb/include/pdo_struct.h"
#include "../fb/include/PdoItemFlag.hpp"
#include "../fb/include/AxisCiaConfig.hpp"
#include "../fb/include/pdo_struct.h"
#include "../mcapi/include/axfb_cmd_api.h"
#include "../mcapi/include/cam_interactive_api.h"
#include "../mcapi/include/cam_tappet_ser_api.h"
#include "../mcapi/include/zmc_enum_cdef.h"
#include "../mcapi/include/axis_cia402_api.h"
#include "../mcapi/include/axis_encoder_api.h"
#include "../mcapi/include/axis_virtual_api.h"
#include "../mcapi/include/mc_fb_api.h"
%}

%include "../fb/include/mc_get_version.h"
%include "../fb/include/PdoItemFlag.hpp"
%include "../fb/include/AxisCiaConfig.hpp"
%include "../fb/include/pdo_struct.h"
%include "../fb/include/FbFactoryDef.h"
%include "../mcapi/include/axfb_cmd_api.h"
%include "../mcapi/include/cam_interactive_api.h"
%include "../mcapi/include/cam_tappet_ser_api.h"
%include "../mcapi/include/zmc_enum_cdef.h"
%include "../mcapi/include/axis_cia402_api.h"
%include "../mcapi/include/axis_encoder_api.h"
%include "../mcapi/include/axis_virtual_api.h"
%include "../mcapi/include/mc_fb_api.h"
