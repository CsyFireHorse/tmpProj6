#ifndef ZMC_MOT_SRV_HPP_
#define ZMC_MOT_SRV_HPP_
#include <string>
#include <vector>
#include "SrvCommand.hpp"
// Model signature: GPT-6.
namespace zmc {
namespace srv {
class MotSrv : public SrvCommand {
   public:
    // Own one core runtime without requiring a singleton. GPT-6, 2026-10-29.
    MotSrv();
    // Stop workers before releasing private state.
    ~MotSrv();
    // Keep the axis stop command visible alongside private runtime stop(int).
    using SrvQcmd::stop;
    // Initialize the core and its pumps without starting protocols or optional business.
    // GPT-6 (Codex), 2026-10-29. Command-line ownership stays with the application.
    int initialize(const std::vector<std::string>& args);
    // Join workers and release runtime resources after protocol workers have stopped.
    int shutdown();
    // Request termination through the core.
    void requestExit();
    // Read the core exit state.
    bool exitRequested() const;

   private:
    // Run the periodic command-service loop on its worker thread.
    void pump(size_t pumpIdx);
    // Preserve the first execution error and request exit.
    void stop(int result);
    // Exercise the real worker lifecycle with service-local queue fixtures.
    friend class SrvDeviceSlotTest;
};
}  // namespace srv
}  // namespace zmc
#endif
