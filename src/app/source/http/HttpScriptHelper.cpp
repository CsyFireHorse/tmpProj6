#include "HttpScriptHelper.hpp"

#include "srv/MotSrv.hpp"
#include "HttpJsonParseDetail.hpp"

#include "fbt_errorcode_def.h"
#include "impl/fbt_impl_enum_def.h"

#include <cctype>
#include <cstdlib>
#include <functional>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

#include "common/thirdlib/nlohmann/json.hpp"
#include "common/thirdlib/yhirose/httplib.h"
#include "common/zmc/zmc_predef.h"
#include "fb/mc_parameter_enum_def.h"

using json = nlohmann::json;

namespace zmc {

namespace {

using http_json_parse_detail::applyMotionApiRet;
using http_json_parse_detail::makeJsonEnvelope;
using http_json_parse_detail::parseJsonBody;
using http_json_parse_detail::replyJsonError;
using http_json_parse_detail::replyJsonInvalidArg;
using http_json_parse_detail::replyJsonOk;
using http_json_parse_detail::setJsonResponse;

void replyNotImplemented(httplib::Response& res, const std::string& route) {
    res.status = 501;
    setJsonResponse(res, makeJsonEnvelope(501, "not implemented", {{"route", route}}));
}


bool parseHttpPosList(const std::string& text, std::vector<double>& out) {
    out.clear();

    const char* p = text.c_str();
    while (*p) {
        while (*p && (std::isspace(static_cast<unsigned char>(*p)) || *p == ',')) {
            ++p;
        }
        if (!*p) {
            break;
        }

        char* end = nullptr;
        const double value = std::strtod(p, &end);
        if (end == p) {
            out.clear();
            return false;
        }

        out.push_back(value);
        p = end;
        if (*p && !std::isspace(static_cast<unsigned char>(*p)) && *p != ',') {
            out.clear();
            return false;
        }
    }

    return !out.empty();
}

bool parseHttpIntList(const std::string& text, std::vector<int>& out) {
    out.clear();

    const char* p = text.c_str();
    while (*p) {
        while (*p && (std::isspace(static_cast<unsigned char>(*p)) || *p == ',')) {
            ++p;
        }
        if (!*p) {
            break;
        }

        char* end = nullptr;
        const long value = std::strtol(p, &end, 10);
        if (end == p) {
            out.clear();
            return false;
        }

        out.push_back(static_cast<int>(value));
        p = end;
        if (*p && !std::isspace(static_cast<unsigned char>(*p)) && *p != ',') {
            out.clear();
            return false;
        }
    }

    return !out.empty();
}

bool getJsonPosList(const json& body, httplib::Response& res, std::vector<double>& out) {
    out.clear();
    try {
        if (!body.contains("pos")) {
            replyJsonInvalidArg(res, "missing 'pos' field");
            return false;
        }
        const json& pos = body["pos"];
        if (pos.is_array()) {
            for (const auto& item : pos) {
                out.push_back(item.get<double>());
            }
        } else if (pos.is_string()) {
            if (!parseHttpPosList(pos.get<std::string>(), out)) {
                replyJsonInvalidArg(res, "invalid 'pos' field");
                return false;
            }
        } else if (pos.is_number()) {
            out.push_back(pos.get<double>());
        } else {
            replyJsonInvalidArg(res, "invalid 'pos' field");
            return false;
        }
        if (out.empty()) {
            replyJsonInvalidArg(res, "empty 'pos' field");
            return false;
        }
    } catch (const std::exception& e) {
        replyJsonInvalidArg(res, std::string("invalid 'pos' field: ") + e.what());
        return false;
    }
    return true;
}

bool getJsonSinglePos(const json& body, httplib::Response& res, double& out) {
    std::vector<double> posList;
    if (!getJsonPosList(body, res, posList)) {
        return false;
    }
    if (posList.size() != 1) {
        replyJsonInvalidArg(res, "invalid single 'pos' field");
        return false;
    }
    out = posList[0];
    return true;
}

bool getJsonDoubleField(const json& body, const char* name, httplib::Response& res, double& out) {
    try {
        if (!body.contains(name)) {
            replyJsonInvalidArg(res, std::string("missing '") + name + "' field");
            return false;
        }
        out = body[name].get<double>();
    } catch (const std::exception& e) {
        replyJsonInvalidArg(res, std::string("invalid '") + name + "' field: " + e.what());
        return false;
    }
    return true;
}

bool getJsonDoubleAnyField(const json& body, const char* const* names, size_t count,
                           httplib::Response& res, double& out) {
    for (size_t i = 0; i < count; ++i) {
        if (body.contains(names[i])) {
            return getJsonDoubleField(body, names[i], res, out);
        }
    }

    replyJsonInvalidArg(res, "missing numeric field");
    return false;
}

bool getJsonIntField(const json& body, const char* name, httplib::Response& res, int& out) {
    try {
        if (!body.contains(name)) {
            replyJsonInvalidArg(res, std::string("missing '") + name + "' field");
            return false;
        }
        out = body[name].get<int>();
    } catch (const std::exception& e) {
        replyJsonInvalidArg(res, std::string("invalid '") + name + "' field: " + e.what());
        return false;
    }
    return true;
}

bool getJsonIntAnyField(const json& body, const char* const* names, size_t count,
                        httplib::Response& res, int& out) {
    for (size_t i = 0; i < count; ++i) {
        if (body.contains(names[i])) {
            return getJsonIntField(body, names[i], res, out);
        }
    }

    replyJsonInvalidArg(res, "missing integer field");
    return false;
}

bool getJsonAxisList(const json& body, httplib::Response& res, std::vector<int>& out) {
    out.clear();
    try {
        const char* field = nullptr;
        if (body.contains("axes")) {
            field = "axes";
        } else if (body.contains("axis")) {
            field = "axis";
        } else {
            replyJsonInvalidArg(res, "missing 'axes' field");
            return false;
        }

        const json& axes = body[field];
        if (axes.is_array()) {
            for (const auto& item : axes) {
                out.push_back(item.get<int>());
            }
        } else if (axes.is_string()) {
            if (!parseHttpIntList(axes.get<std::string>(), out)) {
                replyJsonInvalidArg(res, "invalid axis-list field");
                return false;
            }
        } else if (axes.is_number_integer()) {
            out.push_back(axes.get<int>());
        } else {
            replyJsonInvalidArg(res, "invalid axis-list field");
            return false;
        }

        if (out.empty()) {
            replyJsonInvalidArg(res, "empty axis-list field");
            return false;
        }
    } catch (const std::exception& e) {
        replyJsonInvalidArg(res, std::string("invalid axis-list field: ") + e.what());
        return false;
    }
    return true;
}

}  // namespace

bool parse_script_path_from_argv(int argc, char** argv, std::string& scriptPath) {
    for (int i = 0; i < argc; ++i) {
        const std::string arg = argv[i] ? argv[i] : "";
        if (arg.rfind("--script=", 0) == 0) {
            scriptPath = arg.substr(std::string("--script=").size());
            return !scriptPath.empty();
        }
        if (arg == "--script" && i + 1 < argc) {
            scriptPath = argv[++i] ? argv[i] : "";
            return !scriptPath.empty();
        }
    }
    return false;
}

bool parse_script_path_from_args(const std::vector<std::string>& args, size_t beginIndex,
                                 std::string& scriptPath) {
    for (size_t i = beginIndex; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg.rfind("--script=", 0) == 0) {
            scriptPath = arg.substr(std::string("--script=").size());
            return !scriptPath.empty();
        }
        if (arg == "--script" && i + 1 < args.size()) {
            scriptPath = args[++i];
            return !scriptPath.empty();
        }
    }
    return false;
}

bool parse_port_value(const std::string& value, int* port) {
    if (!port || value.empty()) {
        return false;
    }
    try {
        size_t pos = 0;
        int parsed = std::stoi(value, &pos);
        if (pos != value.size() || parsed <= 0 || parsed > 65535) {
            return false;
        }
        *port = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

bool read_text_file_all(const std::string& path, std::string& out) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        return false;
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    out = oss.str();
    return true;
}

void http_server_bind_lifecycle(httplib::Server& svr, MotHttpQcmd& wrap) {
    // POST /zmot/api/server_exit
    svr.Post("/zmot/api/server_exit", [&](const httplib::Request& req, httplib::Response& res) {
        // The listener observes exit after this worker prepares its response.
        if (!wrap.validate(req, res)) return;
        replyJsonOk(res);
        wrap.service_->requestExit();
    });
}

void http_server_bind_zmc(httplib::Server& svr, MotHttpQcmd& wrap) {
    srv::MotSrv& service = *wrap.service_;
    using Handler = std::function<void(const httplib::Request&, httplib::Response&, RpcSession)>;
    auto lockedGet = [&](const std::string& pattern, Handler handler) {
        svr.Get(pattern, [&wrap, handler](const httplib::Request& req, httplib::Response& res) {
            RpcSession session = 0;
            if (!wrap.validate(req, res, &session)) return;
            handler(req, res, session);
        });
    };
    auto lockedPost = [&](const std::string& pattern, Handler handler) {
        svr.Post(pattern, [&wrap, handler](const httplib::Request& req, httplib::Response& res) {
            RpcSession session = 0;
            if (!wrap.validate(req, res, &session)) return;
            handler(req, res, session);
        });
    };

    // Parameter getters share axis parsing and preserve the existing response shape.
    // Added by GPT-6 Codex, 2026-10-12.
    using ParameterGetter = int (srv::SrvQcmd::*)(RpcSession, int32_t, double&);
    auto bindParameterGet = [&](const char* path, ParameterGetter getter) {
        lockedGet(path, [&, getter](const httplib::Request& req, httplib::Response& res,
                                    RpcSession session) {
            int32_t axis = 0;
            try {
                if (req.has_param("axis")) axis = std::stoi(req.get_param_value("axis"));
            } catch (const std::exception& e) {
                replyJsonInvalidArg(res, std::string("invalid axis parameter: ") + e.what());
                return;
            }
            double value = 0;
            const int ret = (service.*getter)(session, axis, value);
            if (!applyMotionApiRet(res, ret)) return;
            replyJsonOk(res, {{"value", value}});
        });
    };
    bindParameterGet("/zmot/api/units", &srv::SrvQcmd::getunits);
    bindParameterGet("/zmot/api/speed", &srv::SrvQcmd::getspeed);
    bindParameterGet("/zmot/api/accel", &srv::SrvQcmd::getaccel);
    bindParameterGet("/zmot/api/decel", &srv::SrvQcmd::getdecel);
    bindParameterGet("/zmot/api/jerk", &srv::SrvQcmd::getjerk);

    // Typed read-parameter routes share one helper with a fixed ParameterNo.
    // GLM-5.3 (opencode), 2026-10-23.
    auto bindTypedRead = [&](const char* path, ParameterNo para_number, bool integer_value) {
        lockedGet(path, [&, para_number, integer_value](const httplib::Request& req,
                                                        httplib::Response& res,
                                                        RpcSession session) {
            int32_t axis = 0;
            try {
                if (req.has_param("axis")) axis = std::stoi(req.get_param_value("axis"));
            } catch (const std::exception& e) {
                replyJsonInvalidArg(res, std::string("invalid axis parameter: ") + e.what());
                return;
            }
            double value = 0;
            const int ret = service.read_parameter(session, axis, para_number, value);
            if (!applyMotionApiRet(res, ret)) return;
            if (integer_value)
                replyJsonOk(res, {{"value", static_cast<int32_t>(value)}});
            else
                replyJsonOk(res, {{"value", value}});
        });
    };
    bindTypedRead("/zmot/api/read_axis_state", PARA_nAxisState, true);
    bindTypedRead("/zmot/api/read_set_position", PARA_fSetPosition, false);
    bindTypedRead("/zmot/api/read_act_position", PARA_fActPosition, false);
    bindTypedRead("/zmot/api/read_sw_limit_negative", PARA_fSWLimitNegative, false);
    bindTypedRead("/zmot/api/read_sw_limit_enable", PARA_bSWLimitEnable, true);
    bindTypedRead("/zmot/api/read_sw_limit_positive", PARA_fSWLimitPositive, false);
    bindTypedRead("/zmot/api/read_units_num", PARA_iRatioTechUnitsNum, true);
    bindTypedRead("/zmot/api/read_units_den", PARA_dwRatioTechUnitsDenom, true);

    // Generic numeric parameter read: GET /zmot/api/read_parameter?axis=&parameter_number=
    lockedGet("/zmot/api/read_parameter", [&](const httplib::Request& req,
                                              httplib::Response& res, RpcSession session) {
        int32_t axis = 0;
        int32_t parameter_number = 0;
        try {
            if (req.has_param("axis")) axis = std::stoi(req.get_param_value("axis"));
            parameter_number = std::stoi(req.get_param_value("parameter_number"));
        } catch (const std::exception& e) {
            replyJsonInvalidArg(res, std::string("invalid query parameter: ") + e.what());
            return;
        }
        double value = 0;
        const int ret = service.read_parameter(session, axis, parameter_number, value);
        if (!applyMotionApiRet(res, ret)) return;
        replyJsonOk(res, {{"value", value}});
    });

    // Numeric writes return only after the motion-cycle ACK. GPT-6 Codex.
    lockedPost("/zmot/api/write_parameter", [&](const httplib::Request& req,
                                                httplib::Response& res, RpcSession session) {
        json body;
        if (!parseJsonBody(req, res, body)) return;
        int axis = 0;
        int parameter_number = 0;
        double value = 0;
        if ((body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) ||
            !getJsonIntField(body, "parameter_number", res, parameter_number) ||
            !getJsonDoubleField(body, "value", res, value)) return;
        if (!applyMotionApiRet(res, service.write_parameter(session, axis, parameter_number,
                                                            value))) return;
        replyJsonOk(res);
    });

    // GET /zmot/api/status_latest?axis=<int>
    lockedGet("/zmot/api/status_latest", [&](const httplib::Request& req, httplib::Response& res,
                                             RpcSession session) {
        int axis = req.has_param("axis") ? std::stoi(req.get_param_value("axis")) : 0;
        srv::SrvLatestStatus snapshot = {};
        if (!applyMotionApiRet(res, service.status_latest(session, axis, snapshot))) return;
        json response;
        if (snapshot.has_axis) {
            const RpcAxisState& state = snapshot.axis;
            response["axis"] = {{"axis_id", state.axis_id},   {"axis_state", state.axis_state},
                                {"error_id", state.error_id}, {"tick", state.tick},
                                {"act_pos", state.act_pos},   {"set_pos", state.set_pos}};
        } else
            response["axis"] = nullptr;
        response["cmd_results"] = json::array();
        for (const auto& item : snapshot.cmd_results)
            response["cmd_results"].push_back({{"id", item.id},
                                               {"command_type", item.command_type},
                                               {"target_index", item.target_index},
                                               {"target_kind", item.target_kind},
                                               {"status", item.status},
                                               {"io_count", item.io_count},
                                               {"error_code", item.error_code},
                                               {"tick", item.tick},
                                               {"data", item.data},
                                               {"data_ext", item.data_ext}});
        replyJsonOk(res, response);
    });

    // GET /zmot/api/axis_state?axis=<int>
    lockedGet("/zmot/api/axis_state", [&](const httplib::Request& req, httplib::Response& res,
                                          RpcSession session) {
        int axis = 0;
        if (req.has_param("axis")) {
            axis = std::stoi(req.get_param_value("axis"));
        }

        json response;
        RpcAxisState state;
        if (service.getaxisstate(session, axis, state, true) == 0) {
            response["axis"] = {{"axis_id", state.axis_id},   {"axis_state", state.axis_state},
                                {"error_id", state.error_id}, {"tick", state.tick},
                                {"act_pos", state.act_pos},   {"set_pos", state.set_pos}};
        } else {
            response["axis"] = nullptr;
        }

        replyJsonOk(res, response);
    });

    // GET /zmot/api/cmdresult
    lockedGet("/zmot/api/cmdresult",
              [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                  json response;
                  std::vector<RpcCmdResultInfo> results;
                  if (service.getcmdresults(session, results, true) == 0) {
                      json cmdResultsArray = json::array();
                      for (const auto& item : results) {
                          cmdResultsArray.push_back({{"id", item.id},
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
                      response["cmd_results"] = cmdResultsArray;
                  } else {
                      response["cmd_results"] = json::array();
                  }

                  replyJsonOk(res, response);
              });

    // GET /zmot/api/axis_state2?axis=<int>
    lockedGet("/zmot/api/axis_state2", [&](const httplib::Request& req, httplib::Response& res,
                                           RpcSession session) {
        int axis = 0;
        if (req.has_param("axis")) {
            axis = std::stoi(req.get_param_value("axis"));
        }

        json response;
        RpcAxisState2 state2;
        if (service.getaxisstate2(session, axis, state2, true) == 0) {
            response["axis"] = {
                {"axis_id", state2.axis_id},           {"axis_state", state2.axis_state},
                {"error_id", state2.error_id},         {"tick", state2.tick},
                {"act_pos", state2.act_pos},           {"set_pos", state2.set_pos},
                {"set_vel", state2.set_vel},           {"set_acc", state2.set_acc},
                {"set_position", state2.set_position}, {"act_position", state2.act_position}};
        } else {
            response["axis"] = nullptr;
        }

        replyJsonOk(res, response);
    });

    // GET /zmot/api/flow_status
    lockedGet("/zmot/api/flow_status", [&](const httplib::Request& req, httplib::Response& res,
                                           RpcSession session) {
        RpcFlowCtrlStatus status;
        int ret = service.getflowstatus(session, status, true);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
        json response = {{"period", status.period},           {"batchSize", status.batch_size},
                         {"verboseFlag", status.verbose_flag}, {"latency", status.latency},
                         {"axis_num", status.axis_num},       {"cycleTime", status.cycle_time}};

        replyJsonOk(res, response);
    });

    // GET /zmot/api/dashboard - Aggregated dashboard data
    // Returns all axes states, flow status, and optionally command results in one request
    // Parameters:
    //   axes: comma-separated axis IDs (e.g., "0,1,2"), default: "0,1,2"
    //   include_flow: include flow control status (default: true)
    //   include_cmd_results: include command results (default: true)
    lockedGet("/zmot/api/dashboard", [&](const httplib::Request& req, httplib::Response& res,
                                         RpcSession session) {
        std::vector<int32_t> axes;
        if (req.has_param("axes")) {
            std::istringstream input(req.get_param_value("axes"));
            std::string value;
            while (std::getline(input, value, ',')) {
                try {
                    const int axis = std::stoi(value);
                    if (axis >= 0 && axis < MC_MOTION_MAX_AXES) axes.push_back(axis);
                } catch (...) { /* Preserve the existing query filtering. */
                }
            }
        }
        if (axes.empty()) axes = {0, 1, 2};
        const bool include_flow = !req.has_param("include_flow") ||
                                  req.get_param_value("include_flow") == "true" ||
                                  req.get_param_value("include_flow") == "1";
        const bool include_results = !req.has_param("include_cmd_results") ||
                                     req.get_param_value("include_cmd_results") == "true" ||
                                     req.get_param_value("include_cmd_results") == "1";
        srv::SrvDashboard snapshot = {};
        if (!applyMotionApiRet(
                res, service.dashboard(session, axes, include_flow, include_results, snapshot)))
            return;
        json response = {{"axes", json::array()}};
        for (const auto& entry : snapshot.axes) {
            json axis;
            const auto& st = entry.state;
            if (entry.is_extended) {
                axis = {{"axis_id", st.axis_id},           {"axis_state", st.axis_state},
                        {"error_id", st.error_id},         {"tick", st.tick},
                        {"set_pos", st.set_pos},           {"act_pos", st.act_pos},
                        {"set_vel", st.set_vel},           {"set_acc", st.set_acc},
                        {"set_position", st.set_position}, {"act_position", st.act_position}};
            } else {
                axis = {{"axis_id", st.axis_id},   {"axis_state", st.axis_state},
                        {"error_id", st.error_id}, {"tick", st.tick},
                        {"act_pos", st.act_pos},   {"set_pos", st.set_pos}};
            }
            response["axes"].push_back(axis);
        }
        if (include_flow) {
            const auto& f = snapshot.flow_status;
            response["flow_status"] = {{"period", f.period},           {"batchSize", f.batch_size},
                                       {"verboseFlag", f.verbose_flag}, {"latency", f.latency},
                                       {"axis_num", f.axis_num},       {"cycleTime", f.cycle_time}};
        }
        if (include_results) {
            response["cmd_results"] = json::array();
            for (const auto& r : snapshot.cmd_results)
                response["cmd_results"].push_back({{"id", r.id},
                                                   {"command_type", r.command_type},
                                                   {"target_index", r.target_index},
                                                   {"target_kind", r.target_kind},
                                                   {"status", r.status},
                                                   {"io_count", r.io_count},
                                                   {"error_code", r.error_code},
                                                   {"tick", r.tick},
                                                   {"data", r.data},
                                                   {"data_ext", r.data_ext}});
        }
        replyJsonOk(res, response);
    });

    // POST /zmot/api/dashboard_clear
    lockedPost("/zmot/api/dashboard_clear",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   (void)req;
                   service.dashboard_clear(session);
                   replyJsonOk(res);
               });

    // POST /zmot/api/cache_reset
    lockedPost("/zmot/api/cache_reset",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   (void)req;
                   service.cache_reset(session);
                   replyJsonOk(res);
               });

    // GET /zmot/api/flow_config
    lockedGet("/zmot/api/flow_config",
              [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                  RpcFlowCtrlConfig config;
                  int ret = service.getflowconfig(session, config);
                  if (!applyMotionApiRet(res, ret)) {
                      return;
                  }
                  json response = {{"period", config.period},
                                   {"batchSize", config.batch_size},
                                   {"verboseFlag", config.verbose_flag}};

                  replyJsonOk(res, response);
              });

    // POST /zmot/api/flow_config
    lockedPost("/zmot/api/flow_config", [&](const httplib::Request& req, httplib::Response& res,
                                            RpcSession session) {
        RpcFlowCtrlConfig cfg;
        // Use current config as defaults
        int ret = service.getflowconfig(session, cfg);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }

        json body;
        if (!parseJsonBody(req, res, body)) {
            return;
        }
        try {
            if (body.contains("period")) {
                cfg.period = body["period"].get<uint32_t>();
            }
            if (body.contains("batchSize")) {
                cfg.batch_size = body["batchSize"].get<uint32_t>();
            }
            if (body.contains("verboseFlag")) {
                cfg.verbose_flag = body["verboseFlag"].get<uint8_t>();
            }
        } catch (const std::exception& e) {
            replyJsonInvalidArg(res, std::string("invalid flow_config body: ") + e.what());
            return;
        }

        ret = service.setflowconfig(session, cfg);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    });
    auto handleSingleMoveRel = [&](const httplib::Request& req, httplib::Response& res,
                                   RpcSession session) {
        double pos = 0.0;
        int axis = 0;
        json body;
        if (!parseJsonBody(req, res, body)) {
            return;
        }
        if (!getJsonSinglePos(body, res, pos)) {
            return;
        }
        if (body.contains("axis")) axis = body["axis"].get<int>();

        const int ret = service.movesgrel(session, axis, pos);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // POST /zmot/api/movesg
    lockedPost("/zmot/api/movesg", handleSingleMoveRel);

    // POST /zmot/api/movesgrel
    lockedPost("/zmot/api/movesgrel", handleSingleMoveRel);

    auto handleSingleMoveAdd = [&](const httplib::Request& req, httplib::Response& res,
                                   RpcSession session) {
        double pos = 0.0;
        int axis = 0;
        json body;
        if (!parseJsonBody(req, res, body)) {
            return;
        }
        if (!getJsonSinglePos(body, res, pos)) {
            return;
        }
        if (body.contains("axis")) axis = body["axis"].get<int>();

        const int ret = service.moveadd(session, axis, pos);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // POST /zmot/api/movesgadd
    lockedPost("/zmot/api/movesgadd", handleSingleMoveAdd);

    // POST /zmot/api/moveadd
    lockedPost("/zmot/api/moveadd", handleSingleMoveAdd);

    // POST /zmot/api/movesgabs
    lockedPost("/zmot/api/movesgabs",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   double pos = 0.0;
                   int axis = 0;
                   json body;
                   if (!parseJsonBody(req, res, body)) {
                       return;
                   }
                   if (!getJsonSinglePos(body, res, pos)) {
                       return;
                   }
                   if (body.contains("axis")) axis = body["axis"].get<int>();

                   const int ret = service.movesgabs(session, axis, pos);
                   if (!applyMotionApiRet(res, ret)) {
                       return;
                   }
               });

    auto handleMultiMoveRel = [&](const httplib::Request& req, httplib::Response& res,
                                  RpcSession session) {
        std::vector<double> posList;
        std::vector<int> axes;
        json body;
        if (!parseJsonBody(req, res, body)) {
            return;
        }
        if (!getJsonPosList(body, res, posList)) {
            return;
        }
        if (!getJsonAxisList(body, res, axes) || axes.size() != posList.size()) {
            replyJsonInvalidArg(res, "axes and pos must have the same length");
            return;
        }

        const int ret = service.moverel(session, static_cast<int>(posList.size()), axes.data(),
                                        posList.data());
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // POST /zmot/api/mmove
    lockedPost("/zmot/api/mmove", handleMultiMoveRel);

    // POST /zmot/api/mmoverel
    lockedPost("/zmot/api/mmoverel", handleMultiMoveRel);

    // Compatibility aliases for multi-axis relative moves.
    lockedPost("/zmot/api/move", handleMultiMoveRel);
    lockedPost("/zmot/api/moverel", handleMultiMoveRel);
    lockedPost("/zmot/api/multimoverel", handleMultiMoveRel);

    auto handleMultiMoveAbs = [&](const httplib::Request& req, httplib::Response& res,
                                  RpcSession session) {
        std::vector<double> posList;
        std::vector<int> axes;
        json body;
        if (!parseJsonBody(req, res, body)) {
            return;
        }
        if (!getJsonPosList(body, res, posList)) {
            return;
        }
        if (!getJsonAxisList(body, res, axes) || axes.size() != posList.size()) {
            replyJsonInvalidArg(res, "axes and pos must have the same length");
            return;
        }

        const int ret = service.moveabs(session, static_cast<int>(posList.size()), axes.data(),
                                        posList.data());
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // POST /zmot/api/mmoveabs
    lockedPost("/zmot/api/mmoveabs", handleMultiMoveAbs);

    // Compatibility aliases for multi-axis absolute moves.
    lockedPost("/zmot/api/moveabs", handleMultiMoveAbs);
    lockedPost("/zmot/api/moveabs_multi", handleMultiMoveAbs);

    auto handleCircular = [&](const httplib::Request& req, httplib::Response& res,
                              RpcSession session, bool threePoint, bool absolute) {
        std::vector<int> axes;
        json body;
        if (!parseJsonBody(req, res, body) || !getJsonAxisList(body, res, axes)) {
            return;
        }
        if (axes.size() != 2) {
            replyJsonInvalidArg(res, "circular moves require exactly two axes");
            return;
        }

        const char* firstField = threePoint ? "mid1" : "end1";
        const char* secondField = threePoint ? "mid2" : "end2";
        const char* thirdField = threePoint ? "end1" : "centre1";
        const char* fourthField = threePoint ? "end2" : "centre2";
        double first = 0.0;
        double second = 0.0;
        double third = 0.0;
        double fourth = 0.0;
        int direction = 0;
        if (!getJsonDoubleField(body, firstField, res, first) ||
            !getJsonDoubleField(body, secondField, res, second) ||
            !getJsonDoubleField(body, thirdField, res, third) ||
            !getJsonDoubleField(body, fourthField, res, fourth) ||
            (!threePoint && !getJsonIntField(body, "direction", res, direction))) {
            return;
        }

        int ret = 0;
        if (threePoint) {
            ret = absolute
                      ? service.movecirc2abs(session, 2, axes.data(), first, second, third, fourth)
                      : service.movecirc2(session, 2, axes.data(), first, second, third, fourth);
        } else {
            ret = absolute ? service.movecircabs(session, 2, axes.data(), first, second, third,
                                                 fourth, direction)
                           : service.movecirc(session, 2, axes.data(), first, second, third,
                                              fourth, direction);
        }
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // POST /zmot/api/movecirc
    lockedPost("/zmot/api/movecirc", [handleCircular](const httplib::Request& req,
                                                      httplib::Response& res, RpcSession session) {
        handleCircular(req, res, session, false, false);
    });

    // POST /zmot/api/movecircabs
    lockedPost(
        "/zmot/api/movecircabs",
        [handleCircular](const httplib::Request& req, httplib::Response& res, RpcSession session) {
            handleCircular(req, res, session, false, true);
        });

    // POST /zmot/api/movecirc2
    lockedPost(
        "/zmot/api/movecirc2",
        [handleCircular](const httplib::Request& req, httplib::Response& res, RpcSession session) {
            handleCircular(req, res, session, true, false);
        });

    // POST /zmot/api/movecirc2abs
    lockedPost(
        "/zmot/api/movecirc2abs",
        [handleCircular](const httplib::Request& req, httplib::Response& res, RpcSession session) {
            handleCircular(req, res, session, true, true);
        });

    // POST /zmot/api/vmove
    lockedPost("/zmot/api/vmove",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       int dir = 0;
                       if (!getJsonIntField(body, "dir", res, dir)) {
                           return;
                       }
                       int axis = 0;
                       if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis))
                           return;
                       const int ret = service.movevelocity(session, axis, dir);
                       if (!applyMotionApiRet(res, ret)) {
                           return;
                       }
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });

    // POST /zmot/api/forward
    lockedPost("/zmot/api/forward",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body = json::object();
                   if (!req.body.empty() && !parseJsonBody(req, res, body)) return;

                   int axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   const int ret = service.forward(session, axis);
                   if (!applyMotionApiRet(res, ret)) {
                       return;
                   }
               });

    // POST /zmot/api/reverse
    lockedPost("/zmot/api/reverse",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body = json::object();
                   if (!req.body.empty() && !parseJsonBody(req, res, body)) return;

                   int axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   const int ret = service.reverse(session, axis);
                   if (!applyMotionApiRet(res, ret)) {
                       return;
                   }
               });

    // ====================================================================
    // Control Commands (7 endpoints)
    // ====================================================================

    // POST /zmot/api/stop
    lockedPost("/zmot/api/stop", [&](const httplib::Request& req, httplib::Response& res,
                                     RpcSession session) {
        int axis = 0;
        json body = json::object();
        if (!req.body.empty() && !parseJsonBody(req, res, body)) {
            return;
        }
        if (body.contains("axis")) {
            try {
                axis = body["axis"].get<int>();
            } catch (const std::exception& e) {
                replyJsonInvalidArg(res, std::string("invalid axis parameter: ") + e.what());
                return;
            }
        }
        const int ret = service.stop(session, axis);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    });

    // POST /zmot/api/flush
    lockedPost("/zmot/api/flush",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   const int ret = service.flush(session);
                   if (!applyMotionApiRet(res, ret)) {
                       return;
                   }
               });

    // POST /zmot/api/halt
    lockedPost("/zmot/api/halt", [&](const httplib::Request& req, httplib::Response& res,
                                     RpcSession session) {
        int axis = 0;
        json body = json::object();
        if (!req.body.empty() && !parseJsonBody(req, res, body)) {
            return;
        }
        if (body.contains("axis")) {
            try {
                axis = body["axis"].get<int>();
            } catch (const std::exception& e) {
                replyJsonInvalidArg(res, std::string("invalid axis parameter: ") + e.what());
                return;
            }
        }
        const int ret = service.halt(session, axis);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    });

    // POST /zmot/api/home
    lockedPost("/zmot/api/home",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body = json::object();
                   if (!req.body.empty() && !parseJsonBody(req, res, body)) return;

                   int axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   double pos = 0.0;
                   if (body.contains("pos") && !getJsonDoubleField(body, "pos", res, pos)) return;
                   const int ret = service.home(session, axis, pos);
                   if (!applyMotionApiRet(res, ret)) {
                       return;
                   }
               });

    // POST /zmot/api/home_start
    lockedPost("/zmot/api/home_start", [&](const httplib::Request& req, httplib::Response& res,
                                           RpcSession session) {
        json body;
        if (!parseJsonBody(req, res, body)) {
            return;
        }
        RpcHomeSdoConfig config{};
        int axis = 0;
        try {
            if (body.contains("axis")) axis = body.at("axis").get<int>();
            config.mode = body.at("mode").get<int32_t>();
            config.offset = body.at("offset").get<double>();
            config.velocity_fast = body.at("velocity_fast").get<double>();
            config.velocity_slow = body.at("velocity_slow").get<double>();
            config.acceleration = body.at("acceleration").get<double>();
            config.timeout_ms = body.at("timeout_ms").get<int32_t>();
        } catch (const std::exception& e) {
            replyJsonInvalidArg(res, std::string("invalid home_start parameter: ") + e.what());
            return;
        }
        const int ret = service.home_start(session, axis, config);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    });

    // GET /zmot/api/home_get_status?axis=<int>
    lockedGet("/zmot/api/home_get_status",
              [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                  int32_t axis = 0;
                  try {
                      if (req.has_param("axis")) axis = std::stoi(req.get_param_value("axis"));
                  } catch (const std::exception& e) {
                      replyJsonInvalidArg(res, std::string("invalid axis parameter: ") + e.what());
                      return;
                  }
                  int32_t status = 0, error_id = 0;
                  const int ret = service.home_get_status(session, axis, status, error_id);
                  if (!applyMotionApiRet(res, ret)) return;
                  replyJsonOk(res, {{"status", status}, {"error_id", error_id}});
              });

    // POST /zmot/api/homing
    lockedPost("/zmot/api/homing",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body = json::object();
                   if (!req.body.empty() && !parseJsonBody(req, res, body)) return;

                   int axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   const int ret = service.homing(session, axis);
                   if (!applyMotionApiRet(res, ret)) {
                       return;
                   }
               });

    // POST /zmot/api/reset
    lockedPost("/zmot/api/reset",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body = json::object();
                   if (!req.body.empty() && !parseJsonBody(req, res, body)) return;

                   int axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   const int ret = service.reset(session, axis);
                   if (!applyMotionApiRet(res, ret)) {
                       return;
                   }
               });

    auto handleSetPosition = [&](const httplib::Request& req, httplib::Response& res,
                                 RpcSession session) {
        double pos = 0.0;
        json body;
        if (!parseJsonBody(req, res, body)) {
            return;
        }
        if (!getJsonSinglePos(body, res, pos)) {
            return;
        }
        int axis = 0;
        if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
        const int ret = service.setposition(session, axis, pos);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // ====================================================================
    // Power Commands (3 endpoints)
    // ====================================================================

    auto handlePowerOn = [&](const httplib::Request& req, httplib::Response& res,
                             RpcSession session) {
        int axis = 0;
        json body = json::object();
        if (!req.body.empty() && !parseJsonBody(req, res, body)) {
            return;
        }
        if (body.contains("axis")) {
            try {
                axis = body["axis"].get<int>();
            } catch (const std::exception& e) {
                replyJsonInvalidArg(res, std::string("invalid 'axis' parameter: ") + e.what());
                return;
            }
        }
        const int ret = service.poweron(session, axis);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // POST /zmot/api/poweron
    lockedPost("/zmot/api/poweron", handlePowerOn);

    // Compatibility alias.
    lockedPost("/zmot/api/powerOn", handlePowerOn);

    auto handlePowerOff = [&](const httplib::Request& req, httplib::Response& res,
                              RpcSession session) {
        int axis = 0;
        json body = json::object();
        if (!req.body.empty() && !parseJsonBody(req, res, body)) {
            return;
        }
        if (body.contains("axis")) {
            try {
                axis = body["axis"].get<int>();
            } catch (const std::exception& e) {
                replyJsonInvalidArg(res, std::string("invalid 'axis' parameter: ") + e.what());
                return;
            }
        }
        const int ret = service.poweroff(session, axis);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // POST /zmot/api/poweroff
    lockedPost("/zmot/api/poweroff", handlePowerOff);

    // Compatibility alias.
    lockedPost("/zmot/api/powerOff", handlePowerOff);

    auto handleAxisEnable = [&](const httplib::Request& req, httplib::Response& res,
                                RpcSession session) {
        json body;
        if (!parseJsonBody(req, res, body)) {
            return;
        }
        int axis = 0;
        if (body.contains("axis")) {
            try {
                axis = body["axis"].get<int>();
            } catch (const std::exception& e) {
                replyJsonInvalidArg(res, std::string("invalid 'axis' parameter: ") + e.what());
                return;
            }
        }
        try {
            int enable = 0;
            if (!getJsonIntField(body, "enable", res, enable)) {
                return;
            }
            const int ret = service.axisenable(session, axis, enable != 0);
            if (!applyMotionApiRet(res, ret)) {
                return;
            }
        } catch (const std::exception& e) {
            replyJsonInvalidArg(res, e.what());
        }
    };

    // POST /zmot/api/axisenable
    lockedPost("/zmot/api/axisenable", handleAxisEnable);

    // Compatibility alias.
    lockedPost("/zmot/api/axis_enable", handleAxisEnable);

    // ====================================================================
    // Parameter Setting Commands (7 endpoints)
    // ====================================================================

    // POST /zmot/api/pos
    lockedPost("/zmot/api/pos",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       double value = 0.0;
                       if (!getJsonDoubleField(body, "value", res, value)) {
                           return;
                       }
                       // Note: pos is now set directly in move commands, this endpoint is kept for
                       // compatibility
                       replyJsonOk(res);
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });

    // POST /zmot/api/speed
    lockedPost("/zmot/api/speed",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       double value = 0.0;
                       const char* const names[] = {"speed", "value"};
                       if (!getJsonDoubleAnyField(body, names, 2, res, value)) {
                           return;
                       }
                       int axis = 0;
                       if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis))
                           return;
                       const int ret = service.setspeed(session, axis, value);
                       if (!applyMotionApiRet(res, ret)) {
                           return;
                       }
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });

    // POST /zmot/api/speedratio
    lockedPost("/zmot/api/speedratio",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       double value = 0.0;
                       const char* const names[] = {"ratio", "value"};
                       if (!getJsonDoubleAnyField(body, names, 2, res, value)) {
                           return;
                       }
                       int axis = 0;
                       if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis))
                           return;
                       const int ret = service.setspeedratio(session, axis, value);
                       if (!applyMotionApiRet(res, ret)) {
                           return;
                       }
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });

    // POST /zmot/api/accel
    lockedPost("/zmot/api/accel",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       double value = 0.0;
                       const char* const names[] = {"accel", "value"};
                       if (!getJsonDoubleAnyField(body, names, 2, res, value)) {
                           return;
                       }
                       int axis = 0;
                       if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis))
                           return;
                       const int ret = service.setaccel(session, axis, value);
                       if (!applyMotionApiRet(res, ret)) {
                           return;
                       }
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });

    // POST /zmot/api/decel
    lockedPost("/zmot/api/decel",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       double value = 0.0;
                       const char* const names[] = {"decel", "value"};
                       if (!getJsonDoubleAnyField(body, names, 2, res, value)) {
                           return;
                       }
                       int axis = 0;
                       if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis))
                           return;
                       const int ret = service.setdecel(session, axis, value);
                       if (!applyMotionApiRet(res, ret)) {
                           return;
                       }
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });

    // POST /zmot/api/sparam
    lockedPost("/zmot/api/sparam",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       double value = 0.0;
                       const char* const names[] = {"sparam", "jerk", "value"};
                       if (!getJsonDoubleAnyField(body, names, 3, res, value)) {
                           return;
                       }
                       int axis = 0;
                       if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis))
                           return;
                       const int ret = service.setjerk(session, axis, value);
                       if (!applyMotionApiRet(res, ret)) {
                           return;
                       }
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });

    auto handleFastDecel = [&](const httplib::Request& req, httplib::Response& res,
                               RpcSession session) {
        try {
            json body;
            if (!parseJsonBody(req, res, body)) {
                return;
            }
            double value = 0.0;
            const char* const names[] = {"fast_decel", "value"};
            if (!getJsonDoubleAnyField(body, names, 2, res, value)) {
                return;
            }
            int axis = 0;
            if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
            const int ret = service.setfastdecel(session, axis, value);
            if (!applyMotionApiRet(res, ret)) {
                return;
            }
        } catch (const std::exception& e) {
            replyJsonInvalidArg(res, e.what());
        }
    };

    // POST /zmot/api/fastdecel
    lockedPost("/zmot/api/fastdecel", handleFastDecel);

    // Compatibility alias.
    lockedPost("/zmot/api/setfastdecel", handleFastDecel);

    // Units carry the explicit num/den ratio body. GLM-5.3 (opencode), 2026-10-23.
    auto handleUnits = [&](const httplib::Request& req, httplib::Response& res,
                           RpcSession session) {
        try {
            json body;
            if (!parseJsonBody(req, res, body)) {
                return;
            }
            int num = 0;
            int den = 0;
            if (!getJsonIntField(body, "num", res, num) ||
                !getJsonIntField(body, "den", res, den)) {
                return;
            }
            int axis = 0;
            if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
            const int ret = service.setunits_num_den(session, axis, num, den);
            if (!applyMotionApiRet(res, ret)) {
                return;
            }
        } catch (const std::exception& e) {
            replyJsonInvalidArg(res, e.what());
        }
    };

    // POST /zmot/api/units
    lockedPost("/zmot/api/units", handleUnits);

    // Compatibility alias.
    lockedPost("/zmot/api/setunits", handleUnits);

    // POST /zmot/api/direction
    lockedPost("/zmot/api/direction",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       int value = 0;
                       const char* const names[] = {"dir", "value"};
                       if (!getJsonIntAnyField(body, names, 2, res, value)) {
                           return;
                       }
                       int axis = 0;
                       if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis))
                           return;
                       const int ret = service.setdirection(session, axis, value);
                       if (!applyMotionApiRet(res, ret)) {
                           return;
                       }
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });

    // POST /zmot/api/buffermode
    lockedPost("/zmot/api/buffermode",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       int value = 0;
                       const char* const names[] = {"buffer_mode", "value"};
                       if (!getJsonIntAnyField(body, names, 2, res, value)) {
                           return;
                       }
                       int axis = 0;
                       if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis))
                           return;
                       const int ret = service.setbuffermode(session, axis, value);
                       if (!applyMotionApiRet(res, ret)) {
                           return;
                       }
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });

    // ====================================================================
    // Axis Selection Commands (2 endpoints)
    // ====================================================================

    // POST /zmot/api/base
    lockedPost("/zmot/api/base",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   try {
                       json body;
                       if (!parseJsonBody(req, res, body)) {
                           return;
                       }
                       int axis = 0;
                       if (!getJsonIntField(body, "axis", res, axis)) {
                           return;
                       }
                       const int ret = service.base(session, axis);
                       if (!applyMotionApiRet(res, ret)) {
                           return;
                       }
                   } catch (const std::exception& e) {
                       replyJsonInvalidArg(res, e.what());
                   }
               });
    // POST /zmot/api/master
    lockedPost("/zmot/api/master",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body;
                   if (!parseJsonBody(req, res, body)) return;
                   int axis = 0, master_axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   if (!getJsonIntField(body, "master_axis", res, master_axis)) return;
                   applyMotionApiRet(res, service.setmasteraxis(session, axis, master_axis));
               });

    // ====================================================================
    // Gear Commands (2 endpoints)
    // ====================================================================

    auto handleGearIn = [&](const httplib::Request& req, httplib::Response& res,
                            RpcSession session) {
        json body = json::object();
        if (!req.body.empty() && !parseJsonBody(req, res, body)) return;

        (void)req;
        int axis = 0;
        if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
        const int ret = service.gearin(session, axis);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // POST /zmot/api/gearin
    lockedPost("/zmot/api/gearin", handleGearIn);

    // Compatibility alias.
    lockedPost("/zmot/api/gear_in", handleGearIn);

    auto handleGearOut = [&](const httplib::Request& req, httplib::Response& res,
                             RpcSession session) {
        json body = json::object();
        if (!req.body.empty() && !parseJsonBody(req, res, body)) return;

        (void)req;
        int axis = 0;
        if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
        const int ret = service.gearout(session, axis);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    };

    // POST /zmot/api/gearout
    lockedPost("/zmot/api/gearout", handleGearOut);

    // Compatibility alias.
    lockedPost("/zmot/api/gear_out", handleGearOut);

    // POST /zmot/api/datum
    lockedPost("/zmot/api/datum",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body;
                   if (!parseJsonBody(req, res, body)) {
                       return;
                   }
                   int mode = 0;
                   if (!getJsonIntField(body, "mode", res, mode)) {
                       return;
                   }
                   int axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   const int ret = service.datum(session, axis, mode);
                   if (!applyMotionApiRet(res, ret)) {
                       return;
                   }
               });

    // POST /zmot/api/defpos
    lockedPost("/zmot/api/defpos", handleSetPosition);

    // Compatibility alias.
    lockedPost("/zmot/api/setposition", handleSetPosition);

    // POST /zmot/api/groupselect
    lockedPost("/zmot/api/groupselect",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body;
                   if (!parseJsonBody(req, res, body)) {
                       return;
                   }
                   int idx = 0;
                   if (!getJsonIntField(body, "idx", res, idx)) {
                       return;
                   }
                   const int ret = service.groupselect(session, idx);
                   if (!applyMotionApiRet(res, ret)) {
                       return;
                   }
               });

    // POST /zmot/api/groupsetaxes
    lockedPost("/zmot/api/groupsetaxes", [&](const httplib::Request& req, httplib::Response& res,
                                             RpcSession session) {
        std::vector<int> axes;
        json body;
        if (!parseJsonBody(req, res, body)) {
            return;
        }
        if (!getJsonAxisList(body, res, axes)) {
            return;
        }
        const int ret = service.groupsetaxes(session, static_cast<int>(axes.size()), axes.data());
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    });

    // POST /zmot/api/groupenable
    lockedPost("/zmot/api/groupenable", [&](const httplib::Request& req, httplib::Response& res,
                                            RpcSession session) {
        std::vector<int> axes;
        json body;
        if (!parseJsonBody(req, res, body) || !getJsonAxisList(body, res, axes)) {
            return;
        }
        const int ret = service.groupenable(session, static_cast<int>(axes.size()), axes.data());
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    });

    // POST /zmot/api/groupdisable
    lockedPost("/zmot/api/groupdisable", [&](const httplib::Request& req, httplib::Response& res,
                                             RpcSession session) {
        std::vector<int> axes;
        json body;
        if (!parseJsonBody(req, res, body) || !getJsonAxisList(body, res, axes)) {
            return;
        }
        const int ret = service.groupdisable(session, static_cast<int>(axes.size()), axes.data());
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    });

    // POST /zmot/api/groupreset
    lockedPost("/zmot/api/groupreset", [&](const httplib::Request& req, httplib::Response& res,
                                           RpcSession session) {
        std::vector<int> axes;
        json body;
        if (!parseJsonBody(req, res, body) || !getJsonAxisList(body, res, axes)) {
            return;
        }
        const int ret = service.groupreset(session, static_cast<int>(axes.size()), axes.data());
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    });

    // POST /zmot/api/execute_command - Execute string command
    // Only accepts command string via JSON body (not query parameters)
    lockedPost("/zmot/api/execute_command", [&](const httplib::Request& req,
                                                httplib::Response& res, RpcSession session) {
        std::string cmd;

        if (!req.body.empty()) {
            try {
                json body = json::parse(req.body);
                if (body.contains("cmd") && body["cmd"].is_string()) {
                    cmd = body["cmd"].get<std::string>();
                } else {
                    replyJsonError(res, static_cast<int>(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG),
                                   "cmd field missing or not a string in json body");
                    return;
                }
            } catch (const json::exception& e) {
                (void)e;
                cmd = req.body;
            }
        }

        if (cmd.empty()) {
            replyJsonError(res, static_cast<int>(ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG),
                           "missing 'cmd' in json body");
            return;
        }

        int ret = service.executecommand(session, cmd);
        if (!applyMotionApiRet(res, ret)) {
            return;
        }
    });

    lockedPost("/zmot/api/waitticks",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body;
                   if (!parseJsonBody(req, res, body)) return;
                   int axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   uint64_t parsed = 0;
                   if (!body.contains("nTick") || !http_json_parse_detail::read_uint64(
                                                      body["nTick"], UINT32_MAX, true, parsed)) {
                       replyJsonInvalidArg(res, "invalid nTick");
                       return;
                   }
                   const uint32_t value = static_cast<uint32_t>(parsed);
                   applyMotionApiRet(res, service.waitticks(session, value, axis, 0, 2));
               });
    lockedPost("/zmot/api/waitseconds",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body;
                   if (!parseJsonBody(req, res, body)) return;
                   int axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   double value = 0.0;
                   if (!getJsonDoubleField(body, "seconds", res, value)) return;
                   applyMotionApiRet(res, service.waitseconds(session, value, axis, 0, 2));
               });
    lockedPost("/zmot/api/waitaxisidle",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body;
                   if (!parseJsonBody(req, res, body)) return;
                   int axis = 0;
                   if (body.contains("axis") && !getJsonIntField(body, "axis", res, axis)) return;
                   uint64_t parsed = 0;
                   if (!body.contains("timeout_ms") || !http_json_parse_detail::read_uint64(
                                                       body["timeout_ms"], UINT32_MAX, true,
                                                       parsed)) {
                       replyJsonInvalidArg(res, "invalid timeout_ms");
                       return;
                   }
                   applyMotionApiRet(res, service.waitaxisidle(session, axis,
                                                               static_cast<uint32_t>(parsed)));
               });
    // POST /zmot/api/ping; wait for the same motion-cycle confirmation as RPC ping.
    lockedPost("/zmot/api/ping",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body;
                   if (!parseJsonBody(req, res, body)) return;
                   uint64_t parsed = 0;
                   if (!body.contains("timeout_ms") || !http_json_parse_detail::read_uint64(
                                                               body["timeout_ms"], UINT32_MAX, true,
                                                               parsed)) {
                       replyJsonInvalidArg(res, "invalid timeout_ms");
                       return;
                   }
                   applyMotionApiRet(res, service.ping(session, static_cast<uint32_t>(parsed)));
               });

    // Echo through EcatMotion using the existing session service. GPT-6 Codex.
    lockedPost("/zmot/api/mailbox_ping",
               [&](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   json body;
                   if (!parseJsonBody(req, res, body)) return;
                   uint64_t parsed = 0;
                   if (!body.contains("ping_value") || !http_json_parse_detail::read_uint64(
                                                           body["ping_value"], UINT32_MAX, true,
                                                           parsed)) {
                       replyJsonInvalidArg(res, "invalid ping_value");
                       return;
                   }
                   uint32_t pongValue = 0;
                   if (applyMotionApiRet(res, service.mailboxPing(
                                                 session, static_cast<uint32_t>(parsed), pongValue)))
                       replyJsonOk(res, {{"pong_value", pongValue}});
               });

    auto addPcieCompatGetStub = [&](const std::string& path) {
        lockedGet(path, [path](const httplib::Request&, httplib::Response& res,
                               RpcSession session) { replyNotImplemented(res, path); });
    };

    auto addPcieCompatJsonPostStub = [&](const std::string& path) {
        lockedPost(
            path, [path](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                if (!req.body.empty()) {
                    json body;
                    if (!parseJsonBody(req, res, body)) {
                        return;
                    }
                }
                replyNotImplemented(res, path);
            });
    };

    auto addPcieCompatPostStub = [&](const std::string& path) {
        lockedPost(path, [path](const httplib::Request&, httplib::Response& res,
                                RpcSession session) { replyNotImplemented(res, path); });
    };

    // PCIe compatibility routes without src/app support are registered as explicit stubs.
    // Routes implemented by MotHttpEcat must not appear here: httplib dispatches
    // duplicate method/path registrations in insertion order.
    const char* pcieCompatGetStubs[] = {
        "/zmot/api/mirror_stats",
    };
    for (const char* path : pcieCompatGetStubs) {
        addPcieCompatGetStub(path);
    }

    // addPcieCompatPostStub("/zmot/api/file_download");

    const char* pcieCompatJsonPostStubs[] = {
        "/zmot/api/mc_exit",
        "/zmot/api/setup_command",
        "/zmot/api/setup_process_control",
        "/zmot/api/setup_query_post",
        "/zmot/api/setup_query_pop",
        "/zmot/api/write_tx",
        "/zmot/api/read_rx",
        "/zmot/api/read_tx",
        "/zmot/api/query_config",
        "/zmot/api/query",
    };
    for (const char* path : pcieCompatJsonPostStubs) {
        addPcieCompatJsonPostStub(path);
    }

    // The mailbox ping route above replaces the unsupported compatibility alias.
}

std::string http_sever_mount_web_client(httplib::Server& svr) {
    // Mount static file directory for web client
    // Try multiple possible paths (relative to executable location)
    std::vector<std::string> webClientPaths = {
        "dist",
        "web_client",
        "web_client/dist",
        "src/app/demo/web_client/dist",
        "../src/app/demo/web_client/dist",
        "../../src/app/demo/web_client/dist",
        "../src/app/demo/web_client"
        // ".",
    };
    for (const auto& path : webClientPaths) {
        if (svr.set_mount_point("/", path)) {
            return path;
        }
    }
    return std::string();
}

int MotHttpQcmd::bindQcmd(srv::MotSrv& service) {
    service_ = &service;
    const std::string web_client_path = http_sever_mount_web_client(*pSvr);
    if (log_level_ >= 1) {
        if (web_client_path.empty())
            std::cerr << "Warning: Failed to mount web client directory" << std::endl;
        else
            std::cout << "Web client mounted from: " << web_client_path << std::endl;
    }
    http_server_bind_lifecycle(*pSvr, *this);
    bindSessions();
    http_server_bind_zmc(*pSvr, *this);
    bindIoRoutes();
    return bindFiles();
}

}  // namespace zmc
