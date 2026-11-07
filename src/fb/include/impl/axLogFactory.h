#ifndef _AXIS_LOG_FACTORY_HPP_
#define _AXIS_LOG_FACTORY_HPP_

#include "axLogBase.hpp"
#include <memory>

namespace zmc{
    

enum class AxLoggerType {
    NONE,
    BUFFER,
    CONSOLE,
    FILE,
    SPDLOG,
};

inline AxLoggerType AxLoggerTypeFromString(const std::string& str) {
    if (str == "NONE" || str == "none") return AxLoggerType::NONE;
    if (str == "BUFFER" || str == "buffer") return AxLoggerType::BUFFER;
    if (str == "CONSOLE" || str == "console") return AxLoggerType::CONSOLE;
    if (str == "FILE" || str == "file") return AxLoggerType::FILE;
    if (str == "SPDLOG" || str == "spdlog") return AxLoggerType::SPDLOG;
    return AxLoggerType::NONE;
}

class AxLoggerFactory {
public:
    static std::unique_ptr<AxLogger> createLogger(AxLoggerType type, const std::string& filename = "");
};

}
#endif