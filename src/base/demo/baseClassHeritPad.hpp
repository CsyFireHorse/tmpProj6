#ifndef ZMC_BASE_BASE_CLASS_HERIT_PAD_HPP_
#define ZMC_BASE_BASE_CLASS_HERIT_PAD_HPP_
namespace zmc {
namespace base {
struct Base {
    void* pNext;  
    char a;       
    char b;       

};

struct Derived : public Base {
    char c; 
    char d; 
};

static_assert(sizeof(Base) == 16, "size not equal");
static_assert(sizeof(Derived) == 24, "size not equal");
// static_assert(sizeof(Base) == sizeof(Derived), "size not equal");


}  // namespace base
}  // namespace zmc
#endif