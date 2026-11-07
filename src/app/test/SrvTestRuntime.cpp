#include "SrvTestRuntime.hpp"
#include "EcatMotion.h"
#include "EcatMotionCli.hpp"
#include "common/zmc/fbt_errorcode_def.h"

// GPT-6, 2026-10-29. Test lifetime belongs to its own application, without a singleton.
int SrvTestRuntime::start(const std::vector<std::string>& args) {
    zmc::PdoPowerIniArgs parsed;
    // GPT-6 (Codex), 2026-10-29. Only syntax validation precedes production startup.
    result_ = zmc::parse_ecat_motion_args(args, &parsed);
    if (result_ != 0) return result_;
    app_ = zmc::createMotionApp(args);
    if (!app_) return ZMC_ERR_HOST_NOT_IMPLEMENTED;
    thread_ = std::thread(
        [this, args]() { result_ = app_->run(args); });
    // The worker owns result_ until finish() joins it.
    return 0;
}

int SrvTestRuntime::finish() {
    if (thread_.joinable()) {
        app_->requestExit();
        thread_.join();
    }
    return result_;
}

SrvTestRuntime::~SrvTestRuntime() { finish(); }
