#include "stdint.h"


#ifdef __cplusplus 
    // #define _NS_ZMC_
    // #ifdef _NS_ZMC_
    #ifndef _ZMC_FOO_H_
    #define _ZMC_FOO_H_
    #include "EnumBooCppDef.h"
    namespace zmc{
        #include "FooTmpl.h"
    }
    #endif
    // #endif
#endif

// typedef struct Foo{
//     uint8_t a;
//     uint32_t b;
//     Boo boo;
// }Foo;

