#include "HttpScriptHelper.hpp"
#include "srv/MotSrv.hpp"
#include "HttpJsonParseDetail.hpp"

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

#include "common/thirdlib/nlohmann/json.hpp"
#include "common/thirdlib/yhirose/httplib.h"
#include "common/zmc/zmc_predef.h"

using json = nlohmann::json;

namespace zmc {

namespace {

using http_json_parse_detail::applyMotionApiRet;
using http_json_parse_detail::parseJsonBody;
using http_json_parse_detail::replyJsonInvalidArg;
using http_json_parse_detail::replyJsonOk;

// Read a strictly typed, range-checked unsigned JSON field.
bool getJsonUint32Field(const json& body, const char* name, uint32_t maxValue,
                        httplib::Response& res, uint32_t& out) {
    if (!body.is_object() || !body.contains(name)) {
        replyJsonInvalidArg(res, std::string("missing '") + name + "' field");
        return false;
    }

    uint64_t value = 0;
    if (!http_json_parse_detail::read_uint64(body[name], static_cast<uint64_t>(maxValue), false,
                                             value)) {
        replyJsonInvalidArg(res, std::string("invalid '") + name + "' field");
        return false;
    }
    out = static_cast<uint32_t>(value);
    return true;
}

// Read one strictly typed JSON boolean field.
bool getJsonBoolField(const json& body, const char* name, httplib::Response& res, bool& out) {
    if (!body.is_object() || !body.contains(name)) {
        replyJsonInvalidArg(res, std::string("missing '") + name + "' field");
        return false;
    }
    if (!http_json_parse_detail::read_bool(body[name], out)) {
        replyJsonInvalidArg(res, std::string("invalid '") + name + "' field");
        return false;
    }
    return true;
}

// Read one IO number within the configured logical-port capacity.
bool getJsonIoNumber(const json& body, const char* name, httplib::Response& res, uint32_t& out) {
    return getJsonUint32Field(body, name, MC_IO_MAX_COUNT - 1u, res, out);
}

}  // namespace

void MotHttpQcmd::bindIoRoutes() {
    using Handler = std::function<void(const httplib::Request&, httplib::Response&, RpcSession)>;
    auto lockedGet = [this](const std::string& pattern, Handler handler) {
        pSvr->Get(pattern, [this, handler](const httplib::Request& req, httplib::Response& res) {
            RpcSession session = 0;
            if (!validate(req, res, &session)) return;
            handler(req, res, session);
        });
    };
    auto lockedPost = [this](const std::string& pattern, Handler handler) {
        pSvr->Post(pattern, [this, handler](const httplib::Request& req, httplib::Response& res) {
            RpcSession session = 0;
            if (!validate(req, res, &session)) return;
            handler(req, res, session);
        });
    };

    // Synchronous reads return only the value confirmed for this request.
    lockedPost("/zmot/api/read_di_sync", [this](const httplib::Request& req,
                                           httplib::Response& res, RpcSession session) {
        json body;
        uint32_t io = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, io)) return;
        bool value = false;
        const int ret = service_->read_di_sync(session, io, value);
        if (ret == 0) replyJsonOk(res, {{"value", value}});
        else applyMotionApiRet(res, ret);
    });
    lockedPost("/zmot/api/read_do_sync", [this](const httplib::Request& req,
                                           httplib::Response& res, RpcSession session) {
        json body;
        uint32_t io = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, io)) return;
        bool value = false;
        const int ret = service_->read_do_sync(session, io, value);
        if (ret == 0) replyJsonOk(res, {{"value", value}});
        else applyMotionApiRet(res, ret);
    });
    lockedPost("/zmot/api/read_ai_sync", [this](const httplib::Request& req,
                                           httplib::Response& res, RpcSession session) {
        json body;
        uint32_t io = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, io)) return;
        uint32_t value = 0;
        const int ret = service_->read_ai_sync(session, io, value);
        if (ret == 0) replyJsonOk(res, {{"value", value}});
        else applyMotionApiRet(res, ret);
    });
    lockedPost("/zmot/api/read_ao_sync", [this](const httplib::Request& req,
                                           httplib::Response& res, RpcSession session) {
        json body;
        uint32_t io = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, io)) return;
        uint32_t value = 0;
        const int ret = service_->read_ao_sync(session, io, value);
        if (ret == 0) replyJsonOk(res, {{"value", value}});
        else applyMotionApiRet(res, ret);
    });

    // POST /zmot/api/read_di
    lockedPost("/zmot/api/read_di", [this](const httplib::Request& req, httplib::Response& res,
                                           RpcSession session) {
        json body;
        uint32_t ioNumber = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, ioNumber)) {
            return;
        }
        applyMotionApiRet(res, service_->read_di(session, ioNumber));
    });

    // POST /zmot/api/read_di_multi
    lockedPost("/zmot/api/read_di_multi", [this](const httplib::Request& req,
                                                 httplib::Response& res, RpcSession session) {
        json body;
        uint32_t startIoNumber = 0;
        uint32_t endIoNumber = 0;
        if (!parseJsonBody(req, res, body) ||
            !getJsonIoNumber(body, "start_io_number", res, startIoNumber) ||
            !getJsonIoNumber(body, "end_io_number", res, endIoNumber)) {
            return;
        }
        applyMotionApiRet(res, service_->read_di_multi(session, startIoNumber, endIoNumber));
    });

    // POST /zmot/api/read_do
    lockedPost("/zmot/api/read_do", [this](const httplib::Request& req, httplib::Response& res,
                                           RpcSession session) {
        json body;
        uint32_t ioNumber = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, ioNumber)) {
            return;
        }
        applyMotionApiRet(res, service_->read_do(session, ioNumber));
    });

    // POST /zmot/api/read_do_multi
    lockedPost("/zmot/api/read_do_multi", [this](const httplib::Request& req,
                                                 httplib::Response& res, RpcSession session) {
        json body;
        uint32_t startIoNumber = 0;
        uint32_t endIoNumber = 0;
        if (!parseJsonBody(req, res, body) ||
            !getJsonIoNumber(body, "start_io_number", res, startIoNumber) ||
            !getJsonIoNumber(body, "end_io_number", res, endIoNumber)) {
            return;
        }
        applyMotionApiRet(res, service_->read_do_multi(session, startIoNumber, endIoNumber));
    });

    // POST /zmot/api/write_do
    lockedPost("/zmot/api/write_do", [this](const httplib::Request& req, httplib::Response& res,
                                            RpcSession session) {
        json body;
        uint32_t ioNumber = 0;
        bool value = false;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, ioNumber) ||
            !getJsonBoolField(body, "value", res, value)) {
            return;
        }
        applyMotionApiRet(res, service_->write_do(session, ioNumber, value));
    });

    // POST /zmot/api/write_do_multi
    lockedPost("/zmot/api/write_do_multi", [this](const httplib::Request& req,
                                                  httplib::Response& res, RpcSession session) {
        json body;
        uint32_t startIoNumber = 0;
        uint32_t endIoNumber = 0;
        uint32_t value = 0;
        if (!parseJsonBody(req, res, body) ||
            !getJsonIoNumber(body, "start_io_number", res, startIoNumber) ||
            !getJsonIoNumber(body, "end_io_number", res, endIoNumber) ||
            !getJsonUint32Field(body, "value", std::numeric_limits<uint32_t>::max(), res, value)) {
            return;
        }
        applyMotionApiRet(res,
                          service_->write_do_multi(session, startIoNumber, endIoNumber, value));
    });

    // POST /zmot/api/read_ai
    lockedPost("/zmot/api/read_ai", [this](const httplib::Request& req, httplib::Response& res,
                                           RpcSession session) {
        json body;
        uint32_t ioNumber = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, ioNumber)) {
            return;
        }
        applyMotionApiRet(res, service_->read_ai(session, ioNumber));
    });

    // POST /zmot/api/read_ao
    lockedPost("/zmot/api/read_ao", [this](const httplib::Request& req, httplib::Response& res,
                                           RpcSession session) {
        json body;
        uint32_t ioNumber = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, ioNumber)) {
            return;
        }
        applyMotionApiRet(res, service_->read_ao(session, ioNumber));
    });

    // POST /zmot/api/write_ao
    lockedPost("/zmot/api/write_ao", [this](const httplib::Request& req, httplib::Response& res,
                                            RpcSession session) {
        json body;
        uint32_t ioNumber = 0;
        uint32_t value = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, ioNumber) ||
            !getJsonUint32Field(body, "value", std::numeric_limits<uint32_t>::max(), res, value)) {
            return;
        }
        applyMotionApiRet(res, service_->write_ao(session, ioNumber, value));
    });

    // POST /zmot/api/write_virtual_di
    lockedPost("/zmot/api/write_virtual_di", [this](const httplib::Request& req,
                                                    httplib::Response& res, RpcSession session) {
        json body;
        uint32_t ioNumber = 0;
        bool value = false;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, ioNumber) ||
            !getJsonBoolField(body, "value", res, value)) {
            return;
        }
        applyMotionApiRet(res, service_->write_virtual_di(session, ioNumber, value));
    });

    // POST /zmot/api/write_virtual_ai
    lockedPost("/zmot/api/write_virtual_ai", [this](const httplib::Request& req,
                                                    httplib::Response& res, RpcSession session) {
        json body;
        uint32_t ioNumber = 0;
        uint32_t value = 0;
        if (!parseJsonBody(req, res, body) || !getJsonIoNumber(body, "io_number", res, ioNumber) ||
            !getJsonUint32Field(body, "value", std::numeric_limits<uint32_t>::max(), res, value)) {
            return;
        }
        applyMotionApiRet(res, service_->write_virtual_ai(session, ioNumber, value));
    });

    // GET /zmot/api/io_results
    lockedGet("/zmot/api/io_results",
              [this](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                  (void)req;

                  std::vector<CmdResult> results;
                  const int ret = service_->io_results(session, results);
                  if (ret != static_cast<int>(ZMC_NO_ERROR) &&
                      ret != static_cast<int>(ZMC_ERR_MOTCMD_IO_RESULTS_EMPTY)) {
                      applyMotionApiRet(res, ret);
                      return;
                  }

                  json ioResults = json::array();
                  for (const auto& item : results) {
                      ioResults.push_back({{"id", item.id},
                                           {"command_type", item.command_type},
                                           {"target_index", item.target_index},
                                           {"target_kind", item.target_kind},
                                           {"status", item.status},
                                           {"io_count", item.io_count},
                                           {"error_code", item.error_code},
                                           {"tick", item.tick},
                                           {"data", item.data},
                                           {"data_ext", item.data_ext}});
                  }
                  replyJsonOk(res, {{"io_results", ioResults}});
              });
}

}  // namespace zmc
