#ifndef ZMC_FB_DEMO_MEMORY_POU_TASK_HPP
#define ZMC_FB_DEMO_MEMORY_POU_TASK_HPP

#include "common/misc/pou_base.hpp"
#include "memory/MemoryPouEngine.hpp"

#include <cstddef>
#include <string>

namespace zmc {

/* Demo PouTask that delegates image geometry/script to MemoryPouEngine. */
class MemoryPouTask : public PouTaskBase {
   public:
    MemoryPouTask();

    int errorCode() const;
    void cmdAdvance() { engine_.cmdAdvance(); }

   protected:
    int loadTypeLayout(const std::string& content);
    int configureInit(const std::string& initConfig);
    void bindPouMemory(void* base, std::size_t bytes);
    int parseScheduledScript(const std::string& script);

    MemoryPouEngine engine_;
    MemoryOperator& memoryOperator_;
    int errorCode_;
};

}  // namespace zmc

#endif
