#include "zspdlog_init.h"
#include "common/zal_config.h"

#ifdef ZAL_DEBUG_USE_SPDLOG
#include <mutex>
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "zspdlog.h"


namespace zmc {

std::vector<spdlog::sink_ptr> createSinks(const std::string& log_file_name) {
    std::vector<spdlog::sink_ptr> sinks;

    auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // sink1->set_level(MyLoggers::getGlobalLevel());
    sinks.push_back(sink1);

    auto sink2 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        log_file_name, 1024 * 1024 * 10, 100, false);
    // sink2->set_level(spdlog::level::debug);
    sinks.push_back(sink2);
    return sinks;
}

class SpdSinglt {
   public:
    static std::shared_ptr<spdlog::logger> Instance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            auto sinks = createSinks("logs/zmc.log");
            std::string logger_name = "zmc";
            _ptr = std::make_shared<spdlog::logger>(logger_name, begin(sinks), end(sinks));
            _ptr->set_level(spdlog::level::debug);
            spdlog::register_logger(_ptr);
        });

        return _ptr;
    }

   private:
    static std::shared_ptr<spdlog::logger> _ptr;
};

void init_logger(std::shared_ptr<spdlog::logger>& _ptr) {
    auto sinks = createSinks("logs/zmc.log");
    std::string logger_name = "zmc";
    _ptr = std::make_shared<spdlog::logger>(logger_name, begin(sinks), end(sinks));
    _ptr->set_level(spdlog::level::info);
    spdlog::register_logger(_ptr);
}

std::shared_ptr<spdlog::logger> get_logger_bak() {
    // return SpdSinglt::Instance();
    static std::once_flag s_flag;
    static std::shared_ptr<spdlog::logger> _log;
    std::call_once(s_flag, init_logger, _log);
    return _log;
}

static std::shared_ptr<spdlog::logger> _log_async = spdlog::stdout_color_mt("stdout");
inline void _logger_init() { 
    // _log_async = spdlog::stdout_color_mt<spdlog::async_factory>("async_logger"); 
    _log_async = spdlog::basic_logger_mt<spdlog::async_factory>("async_file","zmc_fb.log");
    _log_async->set_level(spdlog::level::info);
}

void logger_init(int level) { 
    static std::once_flag s_flag;
    std::call_once(s_flag, _logger_init);
    _log_async->set_level((spdlog::level::level_enum) level);
    _log_async->info("log init level {}",  level);
    _log_async->flush();
    spdlog::flush_every(std::chrono::seconds(5));
}

std::shared_ptr<spdlog::logger> get_logger() { return _log_async; }



}  // namespace zmc
void zmc_logger_init() { 
    #ifdef ZAL_DEBUG_USE_SPDLOG
    zmc::logger_init();
    #endif
}
void zmc_logger_init_level(int level) { 
    #ifdef ZAL_DEBUG_USE_SPDLOG
    zmc::logger_init(level);
    #endif
}
#else 
void zmc_logger_init() { 
}
void zmc_logger_init_level(int level) { 
}
#endif
