#include "pou_memory_task.h"

namespace zmc {

MemoryPouTask::MemoryPouTask() : memoryOperator_(engine_.memoryOperator()), errorCode_(MEMOP_OK) {}

int MemoryPouTask::errorCode() const {
    if (errorCode_ != MEMOP_OK) {
        return errorCode_;
    }
    return engine_.errorCode();
}

int MemoryPouTask::loadTypeLayout(const std::string& content) {
    return engine_.loadTypeLayout(content);
}

int MemoryPouTask::configureInit(const std::string& initConfig) {
    return engine_.configure(initConfig);
}

void MemoryPouTask::bindPouMemory(void* base, std::size_t bytes) {
    engine_.bindMemory(base, bytes);
}

int MemoryPouTask::parseScheduledScript(const std::string& script) {
    return engine_.parseScheduledScript(script);
}

}  // namespace zmc
