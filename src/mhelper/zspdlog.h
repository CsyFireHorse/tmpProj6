#ifndef _ZMC_Z_SPDLOG_H_
#define _ZMC_Z_SPDLOG_H_

#include <chrono>
#define SPDLOG_DISABLE_DEFAULT_LOGGER
#include "spdlog/spdlog.h"
#include "zspdlog_init.h"

namespace zmc {
std::shared_ptr<spdlog::logger> get_logger();

// inline std::shared_ptr<spdlog::logger>& get_logger0()
// {
//     // Create basic file logger (not rotated).
//     static std::shared_ptr<spdlog::logger> my_logger = spdlog::basic_logger_mt("file_logger",
//     "logs/zmc.log", false);
//     // spdlog::flush_every(std::chrono::milliseconds(500));
//     return my_logger;
// }

void logger_init(int level = 2);

// spdlog::flush_every(std::chrono::milliseconds(500));
// get_logger().set_level(spdlog::level::debug);
// spdlog::register_logger(logger);

}  // namespace zmc


#endif