#include "HttpSessionService.hpp"
#include "HttpJsonParseDetail.hpp"
#include "srv/MotSrv.hpp"
#include "common/zmc/zmc_predef.h"

#include <limits>
#include <iostream>
#include <system_error>
#include <chrono>

namespace zmc {
namespace {

// AI modification signature: GPT-6.
using json = nlohmann::json;
using namespace http_json_parse_detail;

bool rejectSession(httplib::Response& res, int code, const char* message) {
    setJsonResponse(res, makeJsonEnvelope(code, message));
    res.status = code;
    return false;
}

bool mergeSession(const std::string& text, uint64_t& value, httplib::Response& res) {
    uint64_t parsed = 0;
    if (text.empty()) return rejectSession(res, 400, "invalid session format");
    for (char ch : text) {
        if (ch < '0' || ch > '9' ||
            parsed > ((std::numeric_limits<uint64_t>::max)() - (ch - '0')) / 10) {
            return rejectSession(res, 400, "invalid session format");
        }
        parsed = parsed * 10 + (ch - '0');
    }
    if (parsed == 0) return rejectSession(res, 400, "invalid session format");
    if (value != 0 && value != parsed) return rejectSession(res, 400, "conflicting session");
    value = parsed;
    return true;
}

bool readObject(const httplib::Request& req, httplib::Response& res, json& body) {
    body = req.body.empty() ? json::object() : json::parse(req.body, nullptr, false);
    if (!body.is_object()) {
        replyJsonInvalidArg(res, "json body must be an object");
        return false;
    }
    return true;
}

bool readOpenString(const json& body, const char* field, const char* defaultValue,
                    httplib::Response& res, std::string& value) {
    const auto it = body.find(field);
    if (it == body.end()) {
        value = defaultValue;
        return true;
    }
    if (!it->is_string() || it->get_ref<const std::string&>().empty()) {
        replyJsonInvalidArg(res, std::string("invalid '") + field + "' field");
        return false;
    }
    value = it->get<std::string>();
    return true;
}

}  // namespace

bool MotHttpBase::validate(const httplib::Request& req, httplib::Response& res,
                           uint64_t* session) const {
    uint64_t value = 0;
    // Merge header credentials first, then request fields. GPT-6, 2026-10-29.
    const auto headers = req.headers.equal_range("X-Zmot-Session");
    for (auto it = headers.first; it != headers.second; ++it) {
        if (!mergeSession(it->second, value, res)) return false;
    }
    if (req.method == "GET" || req.method == "HEAD") {
        const auto params = req.params.equal_range("session");
        for (auto it = params.first; it != params.second; ++it) {
            if (!mergeSession(it->second, value, res)) return false;
        }
    } else if (req.is_multipart_form_data()) {
        const auto files = req.files.equal_range("session");
        for (auto it = files.first; it != files.second; ++it) {
            const auto& part = it->second;
            if (!part.filename.empty()) return rejectSession(res, 400, "invalid session field");
            if (!mergeSession(part.content, value, res)) return false;
        }
    } else if (!req.body.empty()) {
        const json body = json::parse(req.body, nullptr, false);
        // Leave invalid business bodies to the existing handler, including raw scripts.
        const auto field = body.find("session");
        if (field != body.end()) {
            if (!field->is_string()) {
                return rejectSession(res, 400, "invalid session format");
            }
            if (!mergeSession(field->get_ref<const std::string&>(), value, res)) return false;
        }
    }
    if (value == 0) return rejectSession(res, 401, "session required");
    if (service_->checkSession(value) != 0) return rejectSession(res, 401, "invalid session");
    if (session) *session = value;
    return true;
}

void MotHttpBase::open(const httplib::Request& req, httplib::Response& res) {
    json body;
    std::string device;
    std::string channel;
    if (!readObject(req, res, body) || !readOpenString(body, "device", "0", res, device) ||
        (req.path == "/zmot/api/open" &&
         !readOpenString(body, "channel", ZMC_DEFAULT_NET_NAME, res, channel)))
        return;

    uint64_t session = 0;
    const int ret = service_->open(device, session);
    if (ret != 0) {
        replyJsonResult(res, ret);
        return;
    }
    replyJsonOk(res, {{"session", std::to_string(session)}});
}

void MotHttpBase::close(const httplib::Request& req, httplib::Response& res) {
    uint64_t session = 0;
    json body;
    if (!validate(req, res, &session) || !readObject(req, res, body)) return;
    // Revocation does not close the shared runtime. Model signature: GPT-6.
    replyJsonResult(res, service_->close(session));
}

void MotHttpBase::bindSessions() {
    httplib::Server& server = *pSvr;
    for (const char* path : {"/zmot/api/open", "/zmot/api/open_device"}) {
        server.Post(
            path, [this](const httplib::Request& req, httplib::Response& res) { open(req, res); });
    }
    for (const char* path : {"/zmot/api/close", "/zmot/api/close_device"}) {
        server.Post(path, [this](const httplib::Request& req, httplib::Response& res) {
            close(req, res);
        });
    }
}
MotHttpBase::MotHttpBase() : pSvr(new httplib::Server()), service_(nullptr) {}

MotHttpBase::~MotHttpBase() { delete pSvr; }

// GPT-6 (Codex), 2026-10-29. Actual listener readiness precedes optional business.
int MotHttpBase::start() {
    thread_ = std::thread([this]() {
        // httplib::Server::listen_internal creates ThreadPool workers with std::thread.
        // Catch on this worker boundary; exceptions cannot reach the application thread.
        try {
            result_ = run();
        } catch (const std::system_error& error) {
            result_ = error.code().value();
        }
    });
    pSvr->wait_until_ready();
    return pSvr->is_running() ? 0 : wait();
}

int MotHttpBase::wait() {
    if (thread_.joinable()) {
        // The core drives heartbeat; only the application thread stops the HTTP listener.
        while (pSvr->is_running() && !service_->exitRequested())
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        stop();
    }
    return result_;
}

int MotHttpBase::run() {
    if (log_level_ >= 1) {
        std::cout << "Starting HTTP Motion Control Server..." << std::endl;
        std::cout << "Server will listen on http://" << host << ":" << port << std::endl;
        std::cout << "API endpoints: /zmot/api/*" << std::endl;
    }
    // ====================================================================
    if (!pSvr->listen(host, port)) {
        std::cerr << "HTTP server listen failed: host=" << host << " port=" << port << std::endl;
        return -1;
    }
    return 0;
}
void MotHttpBase::stop() {
    if (!thread_.joinable()) return;
    pSvr->stop();
    // The listener stops accepting; its task queue joins response workers before returning.
    thread_.join();
}

}  // namespace zmc
