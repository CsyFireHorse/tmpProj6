#include "mc_get_version.h"
#include <stdio.h>

void zmc_get_version_major_demo(){
    int major = zmc_get_version_major();
    int minor = zmc_get_version_minor();
    int patch = zmc_get_version_patch();
    printf("version:%d.%d.%d\n",major, minor, patch);   
    const char* version = zmc_get_full_version();
    printf("version:%s\n",version);   
}