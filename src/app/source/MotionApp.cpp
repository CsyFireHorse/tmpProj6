#include "MotionApp.hpp"

#include <cstdio>
#include <system_error>

#include "EcatMotion.h"
#include "EcatMotionCli.hpp"
#include "http/EcatHttpServerWrap.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "rpc/MotRpc.hpp"

// Drafted by GPT-6 (Codex), 2026-10-29. Application startup owns ordering and rollback.
namespace zmc {
namespace {

int runStartupBusiness(const std::vector<std::string>& args, srv::MotSrv& service) {
    std::vector<std::string> motion_args;
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        // Keep paired motion values intact even when a value resembles another option.
        if (arg == "--ini" || arg == "--eni" || arg == "--netname") {
            motion_args.push_back(arg);
            motion_args.push_back(args[++i]);
        } else if (arg == "--script") {
            ++i;
        } else if (arg != "--http" && arg != "--rpc" && arg != "--cmd" &&
                   arg != "--script_exit" && arg.find("--script=") != 0 &&
                   arg.find("--port=") != 0) {
            motion_args.push_back(arg);
        }
    }

    std::string script_path;
    const bool has_script = parse_script_path_from_args(args, 0, script_path);
    if (motion_args.empty() && !has_script) return 0;

    PdoPowerIniArgs parsed;
    std::vector<std::string> prepared;
    int ret = prepare_ecat_motion_cli_args(motion_args, &parsed, &prepared);
    if (ret != 0) return ret;

    RpcSession session = 0;
    ret = service.open("0", session);
    if (ret != 0) return ret;
    if (!motion_args.empty()) ret = service.prepareConfig(session, prepared);
    if (ret == 0 && !parsed.eniName.empty())
        ret = service.startup(session, parsed.eniName, parsed.iniName);
    if (ret == 0 && has_script) {
        std::string text;
        if (!read_text_file_all(script_path, text)) {
            std::fprintf(stderr, "Failed to read startup script: %s\n", script_path.c_str());
            ret = ZMC_ERR_HOST_LOCAL_FILE_OPEN_FAILED;
        } else if (text.empty()) {
            ret = ZMC_ERR_MOTCMD_SCRIPT_EMPTY;
        } else {
            // Trusted startup scripts use the same instance as subsequent heartbeats.
            // The core pump advances the script and retains queue-backpressure retries.
            ret = service.executecommand(session, text);
        }
    }
    const int closed = service.close(session);
    return ret != 0 ? ret : closed;
}

}  // namespace

int MotionApp::configure(const std::vector<std::string>& args) {
    PdoPowerIniArgs parsed;
    const int ret = parse_ecat_motion_args(args, &parsed);
    if (ret != 0) return ret;
    if (parsed.runMode == MODE_HTTP) {
        MotHttp* server = new MotHttp();
        transport_.reset(server);
        server->port = parsed.port;
        server->set_log_level(log_level_);
        return 0;
    }
    if (parsed.runMode == MODE_RPC) {
        transport_.reset(new MotRpc("0.0.0.0", parsed.port));
        return 0;
    }
    return ZMC_ERR_HOST_NOT_IMPLEMENTED;
}

std::unique_ptr<MotionApp> createMotionApp(const std::vector<std::string>& args) {
    std::unique_ptr<MotionApp> app(new MotionApp());
    if (app->configure(args) != 0) return nullptr;
    return app;
}

MotionApp::MotionApp() = default;
MotionApp::~MotionApp() { (void)shutdown(); }
void MotionApp::set_log_level(int level) {
    log_level_ = level;
    if (transport_) transport_->set_log_level(level);
}
void MotionApp::requestExit() { srv_.requestExit(); }

int MotionApp::run(const std::vector<std::string>& args) {
    // Validate syntax without reading optional business files or opening native resources.
    PdoPowerIniArgs parsed;
    int ret = parse_ecat_motion_args(args, &parsed);
    if (ret != 0) return ret == 1 ? 0 : ret;

    if (!transport_) ret = configure(args);
    if (ret != 0) return ret;
    ret = srv_.initialize(args);
    if (ret != 0) return ret;  // A rejected second startup does not own the active run.

    // RPC binding uses throwing Asio bind/listen; protocol start creates native threads.
    // These APIs have no error-code alternative at this application boundary.
    try {
        if (!srv_.exitRequested()) ret = transport_->bind(srv_);
        if (ret == 0 && !srv_.exitRequested()) ret = transport_->start();
    } catch (const std::system_error& error) {
        ret = error.code().value();
    }
    if (ret == 0 && !srv_.exitRequested()) ret = runStartupBusiness(args, srv_);
    // Script submission commits startup; the existing pump executes it asynchronously.
    if (ret == 0) ret = transport_->wait();

    const int cleanup = shutdown();
    if (ret != 0) std::fprintf(stderr, "Motion application failed: %d\n", ret);
    return ret != 0 ? ret : cleanup;
}

int MotionApp::shutdown() {
    // Wake synchronous callers before joining protocol workers or releasing device adapters.
    srv_.requestExit();
    if (transport_) transport_->stop();
    transport_.reset();
    return srv_.shutdown();
}

int runMotionApp(const std::vector<std::string>& args) {
    PdoPowerIniArgs parsed;
    const int ret = parse_ecat_motion_args(args, &parsed);
    if (ret != 0) return ret == 1 ? 0 : ret;
    std::unique_ptr<MotionApp> app = createMotionApp(args);
    return app ? app->run(args) : ZMC_ERR_HOST_NOT_IMPLEMENTED;
}

}  // namespace zmc
