#include <map>
#include <memory>
#include <string>
#include "misc/cpp14_backport.hpp"

// #include "fbt_axis_base.h"
#include "stdio.h"
#include "axLog.h"
#include "axLogFactory.h"
#include "axLogSpd.h"

namespace zmc {

std::unique_ptr<AxLogger> AxLoggerFactory::createLogger(AxLoggerType type,
                                                        const std::string& filename) {
    switch (type) {
        case AxLoggerType::BUFFER:
            return make_unique<BufferAxLogger>();
        case AxLoggerType::CONSOLE:
            return make_unique<ConsoleAxLogger>();
        case AxLoggerType::FILE:
            return make_unique<FileAxLogger>(filename);
        case AxLoggerType::SPDLOG:
            return make_unique<SpdlogAxLogger>();
        case AxLoggerType::NONE:
        default:
            return nullptr;
    }
}

}  // namespace zmc
