#include <stdio.h>
// #include "Foo.h"
// #ifndef _NS_ZMC_
// #define _NS_ZMC_
// #endif
#include "FooCppDef.h"
// #include "FooCppDef.h"
void zmc_foo_demo(){
    zmc::Foo foo;
    foo.a = 1;
    foo.b = 2;
    foo.boo = zmc::ZmcBoo1;
    printf("foo.a=%d, foo.b=%d\n", foo.a, foo.b);
}
// void foo_demo2(){
//     Foo foo;
//     foo.a = 1;
//     foo.b = 2;
//     printf("foo.a=%d, foo.b=%d\n", foo.a, foo.b);
// }