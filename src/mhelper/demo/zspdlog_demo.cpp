#include "common/zal_config.h"

#ifdef ZAL_DEBUG_USE_SPDLOG

#include "zspdlog.h"

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"  // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h" // support for user defined types
#include "spdlog/fmt/ranges.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace zmc;

void zspdlog_demo(){
    // Log levels can be loaded from argv/env using "SPDLOG_LEVEL"
    spdlog::cfg::load_env_levels();

    spdlog::info("Welcome to spdlog version {}.{}.{}  !", SPDLOG_VER_MAJOR, SPDLOG_VER_MINOR, SPDLOG_VER_PATCH);

    spdlog::warn("Easy padding in numbers like {:08d}", 12);

    // // Create basic file logger (not rotated).
    // auto my_logger = spdlog::basic_logger_mt("file_logger", "logs/basic-log2.txt", false);
    // spdlog::critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    // spdlog::info("Support for floats {:03.2f}", 1.23456);

    // my_logger->warn("Easy padding in numbers like {:08d}", 12);
    // my_logger->critical("Support for int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    // my_logger->info("Support for floats {:03.2f}", 1.23456);
    
    // spdlog::set_default_logger(my_logger);
    logger_init();
    auto log = get_logger();
    log->info("Support for floats {:03.2f}", 5.678);
    get_logger()->trace("trace log");
    get_logger()->debug("debug log");
    get_logger()->info("Support for floats {:03.2f} {}", 9.87654321, "wert");
    get_logger()->warn("Easy padding in numbers like {:08d}", 13);
    get_logger()->error("error log");
}

void spdlog_vector_example()
{
    std::vector<int> vec = {1, 2, 3};
    spdlog::info("Vector example: {}", vec);
    int vec2[] = {1, 2, 3, 4, 5};
    // int a = 4;
    // spdlog::info("Vector example: {}", (int (&) [a]) vec2);

    spdlog::info("Vector example: {}", (int (&) [3]) vec2);
    spdlog::info("Array example: {}", fmt::join(vec2, vec2+3, ","));

    // fmt::print("{}", fmt::join(vec, ", "));
}

void fmt_demo(){
    std::vector<int> v = {1, 2, 3};
    fmt::print("{}", fmt::join(v, ", "));
    int v2[] = {4, 5, 6, 7};
    fmt::print("{}", fmt::join(v2, v2+4, ", "));
}
#else 

void zspdlog_demo(){}
#endif