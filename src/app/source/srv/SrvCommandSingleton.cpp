#include "srv/SrvCommandSingleton.hpp"

// Model signature: GPT-6.
namespace zmc {
namespace srv {
SrvCommandSingleton::SrvCommandSingleton() {}
SrvCommandSingleton::~SrvCommandSingleton() {}
SrvCommandSingleton& SrvCommandSingleton::instance() {
    static SrvCommandSingleton service;
    return service;
}
}  // namespace srv
}  // namespace zmc
