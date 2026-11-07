#ifndef ZMC_SRV_COMMAND_SINGLETON_HPP_
#define ZMC_SRV_COMMAND_SINGLETON_HPP_
#include "MotSrv.hpp"
// Model signature: GPT-6.
namespace zmc {
namespace srv {
class SrvCommandSingleton final : public MotSrv {
   public:
    // Return the only service instance.
    static SrvCommandSingleton& instance();

   private:
    // Construct only through instance().
    SrvCommandSingleton();
    // Release the private core state.
    ~SrvCommandSingleton();
    // Prevent another service instance.
    SrvCommandSingleton(const SrvCommandSingleton&) = delete;
    // Prevent replacement of the service instance.
    SrvCommandSingleton& operator=(const SrvCommandSingleton&) = delete;
};
}  // namespace srv
}  // namespace zmc
#endif
