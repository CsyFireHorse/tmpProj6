#include "fbt_fb_base.hpp"
#include "fbt_fb_impl.h"

namespace zmc{
int get_fb_impl_stat(void* pFb){
    struct FooImpl{
        FbtBase* pFb;
    };
    FooImpl* pImpl = (FooImpl*)pFb;
    if (pImpl->pFb->fbType) 
        return pImpl->pFb->fbStat;
    else
        return -1;
}
}