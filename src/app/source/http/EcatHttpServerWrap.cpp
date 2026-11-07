/*
 * @ArtifactStatus draft
 * @ArtifactStatusDate 2026-09-14
 * @ArtifactStatusAuthority GPT-5 Codex
 */

#include "EcatHttpServerWrap.hpp"
#include "srv/MotSrv.hpp"
#include "HttpJsonParseDetail.hpp"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <functional>
#include <string>
#include <vector>

#include "common/thirdlib/nlohmann/json.hpp"
#include "common/thirdlib/yhirose/httplib.h"
#include "common/zal_version.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "mhelper/pdo_servo_addr.h"

namespace zmc {

namespace {

using json = nlohmann::json;
using http_json_parse_detail::replyJsonInvalidArg;
using http_json_parse_detail::replyJsonOk;
using http_json_parse_detail::replyJsonResult;

const char* io_alias_kind_name(IoAliasKind kind) {
    switch (kind) {
        case IO_ALIAS_KIND_DI:
            return "di";
        case IO_ALIAS_KIND_DO:
            return "do";
        case IO_ALIAS_KIND_AI:
            return "ai";
        case IO_ALIAS_KIND_AO:
            return "ao";
    }
    return "unknown";
}

const char* io_source_kind_name(IoSourceKind kind) {
    // Keep the HTTP representation stable and independent from enum ordinals.
    switch (kind) {
        case IO_SOURCE_KIND_PDO:
            return "pdo";
        case IO_SOURCE_KIND_NOP:
            return "nop";
        case IO_SOURCE_KIND_VIRTUAL:
            return "virtual";
        case IO_SOURCE_KIND_GPIO:
            return "gpio";
        case IO_SOURCE_KIND_MODBUS:
            return "modbus";
    }
    return "unknown";
}

std::string format_index_hex(uint16_t value) {
    char text[7] = {};
    snprintf(text, sizeof(text), "0x%04X", static_cast<unsigned int>(value));
    return text;
}

std::string format_subindex_hex(uint8_t value) {
    char text[5] = {};
    snprintf(text, sizeof(text), "0x%02X", static_cast<unsigned int>(value));
    return text;
}

std::string path_basename(const std::string& path) {
    const size_t slash = path.find_last_of("/\\");
    if (slash == std::string::npos) {
        return path;
    }
    return path.substr(slash + 1);
}

void reply_file_download(httplib::Response& res, const std::string& path,
                         const std::string& bytes) {
    const std::string name = path_basename(path);
    res.status = 200;
    res.set_header("Content-Disposition", "attachment; filename=\"" + name + "\"");
    res.set_content(bytes, "application/octet-stream");
}

bool parse_json_object(const httplib::Request& req, httplib::Response& res, bool allow_empty,
                       json& body) {
    if (req.body.empty()) {
        if (!allow_empty) {
            replyJsonInvalidArg(res, "missing json body");
            return false;
        }
        body = json::object();
        return true;
    }

    if (!http_json_parse_detail::parseJsonBody(req, res, body)) {
        return false;
    }
    if (!body.is_object()) {
        replyJsonInvalidArg(res, "json body must be an object");
        return false;
    }
    return true;
}

bool get_required_string(const json& body, const char* field, httplib::Response& res,
                         std::string& value) {
    try {
        if (!body.contains(field) || !body[field].is_string()) {
            replyJsonInvalidArg(res, std::string("missing or invalid '") + field + "' field");
            return false;
        }
        value = body[field].get<std::string>();
    } catch (const std::exception& e) {
        replyJsonInvalidArg(res, std::string("invalid '") + field + "' field: " + e.what());
        return false;
    }
    if (value.empty()) {
        replyJsonInvalidArg(res, std::string("empty '") + field + "' field");
        return false;
    }
    return true;
}

bool get_optional_string(const json& body, const char* field, httplib::Response& res,
                         std::string& value) {
    value.clear();
    if (!body.contains(field)) {
        return true;
    }
    return get_required_string(body, field, res, value);
}

bool get_required_state(const json& body, httplib::Response& res, uint8_t& state) {
    try {
        if (!body.contains("state") ||
            (!body["state"].is_number_integer() && !body["state"].is_number_unsigned())) {
            replyJsonInvalidArg(res, "missing or invalid 'state' field");
            return false;
        }
        const int parsed = body["state"].get<int>();
        if (parsed < 0 || parsed > 255) {
            replyJsonInvalidArg(res, "invalid 'state' field");
            return false;
        }
        state = static_cast<uint8_t>(parsed);
    } catch (const std::exception& e) {
        replyJsonInvalidArg(res, std::string("invalid 'state' field: ") + e.what());
        return false;
    }
    return true;
}

bool get_required_uint(const json& body, const char* field, httplib::Response& res,
                       uint64_t maximum, uint64_t& value) {
    try {
        // Keep distinct messages: absent/non-number -> missing or invalid; range -> invalid.
        if (!body.contains(field) ||
            (!body[field].is_number_integer() && !body[field].is_number_unsigned())) {
            replyJsonInvalidArg(res, std::string("missing or invalid '") + field + "' field");
            return false;
        }
        if (!http_json_parse_detail::read_uint64(body[field], maximum, true, value)) {
            replyJsonInvalidArg(res, std::string("invalid '") + field + "' field");
            return false;
        }
    } catch (const std::exception& e) {
        replyJsonInvalidArg(res, std::string("invalid '") + field + "' field: " + e.what());
        return false;
    }
    return true;
}

bool get_required_uint16(const json& body, const char* field, httplib::Response& res,
                         uint16_t& value) {
    uint64_t parsed = 0;
    if (!get_required_uint(body, field, res, UINT16_MAX, parsed)) {
        return false;
    }
    value = static_cast<uint16_t>(parsed);
    return true;
}

bool get_required_query_uint16(const httplib::Request& req, const char* name,
                               httplib::Response& res, uint16_t& value) {
    if (!req.has_param(name)) {
        replyJsonInvalidArg(res, std::string("missing '") + name + "' query parameter");
        return false;
    }
    try {
        const std::string text = req.get_param_value(name);
        std::size_t consumed = 0;
        const unsigned long parsed = std::stoul(text, &consumed, 10);
        if (text.empty() || consumed != text.size() || parsed > 0xffffUL) {
            replyJsonInvalidArg(res, std::string("invalid '") + name + "' query parameter");
            return false;
        }
        value = static_cast<uint16_t>(parsed);
        return true;
    } catch (const std::exception&) {
        replyJsonInvalidArg(res, std::string("invalid '") + name + "' query parameter");
        return false;
    }
}

bool get_required_uint8(const json& body, const char* field, httplib::Response& res,
                        uint8_t& value) {
    uint64_t parsed = 0;
    if (!get_required_uint(body, field, res, UINT8_MAX, parsed)) {
        return false;
    }
    value = static_cast<uint8_t>(parsed);
    return true;
}

bool get_required_uint32(const json& body, const char* field, httplib::Response& res,
                         uint32_t& value) {
    uint64_t parsed = 0;
    if (!get_required_uint(body, field, res, UINT32_MAX, parsed)) {
        return false;
    }
    value = static_cast<uint32_t>(parsed);
    return true;
}

bool get_optional_uint32_with_default(const json& body, const char* field, httplib::Response& res,
                                      uint32_t& value, uint32_t default_value) {
    if (!body.contains(field)) {
        value = default_value;
        return true;
    }
    return get_required_uint32(body, field, res, value);
}

bool get_required_bool(const json& body, httplib::Response& res, const char* field, bool& value) {
    try {
        if (!body.contains(field) || !http_json_parse_detail::read_bool(body[field], value)) {
            replyJsonInvalidArg(res, std::string("missing or invalid '") + field + "' field");
            return false;
        }
    } catch (const std::exception& e) {
        replyJsonInvalidArg(res, std::string("invalid '") + field + "' field: " + e.what());
        return false;
    }
    return true;
}

}  // namespace

void MotHttpEcat::bind_ecat_routes() {
    srv::MotSrv& service = *service_;
    using Handler = std::function<void(const httplib::Request&, httplib::Response&, RpcSession)>;
    const auto locked_post = [this](const std::string& path, Handler handler) {
        pSvr->Post(path, [this, handler](const httplib::Request& req, httplib::Response& res) {
            RpcSession session = 0;
            if (!validate(req, res, &session)) return;
            handler(req, res, session);
        });
    };
    const auto locked_get = [this](const std::string& path, Handler handler) {
        pSvr->Get(path, [this, handler](const httplib::Request& req, httplib::Response& res) {
            RpcSession session = 0;
            if (!validate(req, res, &session)) return;
            handler(req, res, session);
        });
    };

    // AI modification signature: GPT-5 Codex.
    locked_post("/zmot/api/file_download", [](const httplib::Request&, httplib::Response& res,
                                              RpcSession session) { replyJsonResult(res, 0); });

    locked_get("/zmot/api/eni_name",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res,
                               {{"eni_name", value.eni_name}, {"hash", value.eni_hash}});
               });
    // Keep the default ENI fields as empty placeholders until their source is defined.
    locked_get("/zmot/api/default_eni",
               [](const httplib::Request&, httplib::Response& res, RpcSession) {
                   replyJsonOk(res, {{"default_eni_path", ""}, {"hash", ""}});
               });
    locked_get("/zmot/api/ini_name",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res,
                               {{"ini_name", value.ini_name}, {"hash", value.ini_hash}});
               });
    locked_get("/zmot/api/selected_config_files",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   json files = json::array();
                   if (!value.selected_eni_path.empty()) {
                       files.push_back({{"type", "eni"},
                                        {"name", value.selected_eni_path},
                                        {"hash", value.selected_eni_hash}});
                   }
                   if (!value.selected_ini_path.empty()) {
                       files.push_back({{"type", "ini"},
                                        {"name", value.selected_ini_path},
                                        {"hash", value.selected_ini_hash}});
                   }
                   replyJsonOk(res, {{"files", files}});
               });

    locked_get("/zmot/api/device_name",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"device_name", value.device_name}});
               });
    locked_get("/zmot/api/channel_name",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"channel_name", value.channel_name}});
               });
    locked_get("/zmot/api/available_channel_names",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"available_channel_names", value.available_channel_names}});
               });

    locked_get("/zmot/api/eni_json",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity identity = {};
                   const int identity_ret = service.getIdentity(session, identity);
                   if (identity_ret != 0) {
                       replyJsonResult(res, identity_ret);
                       return;
                   }

                   std::string json_text;
                   const int ret = service.eni_json(session, json_text);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   try {
                       json eni = json::parse(json_text);
                       replyJsonOk(res, {{"eni_name", identity.eni_name}, {"eni", eni}});
                   } catch (const std::exception&) {
                       replyJsonResult(res, static_cast<int>(ZMC_ERR_ENI_JSON_INVALID));
                   }
               });

    locked_get("/zmot/api/eni_download",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity identity = {};
                   const int identity_ret = service.getIdentity(session, identity);
                   if (identity_ret != 0) {
                       replyJsonResult(res, identity_ret);
                       return;
                   }

                   std::string bytes;
                   const int ret = service.eni_download(session, bytes);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   reply_file_download(res, identity.eni_name, bytes);
               });

    locked_get("/zmot/api/ini_content",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity identity = {};
                   const int identity_ret = service.getIdentity(session, identity);
                   if (identity_ret != 0) {
                       replyJsonResult(res, identity_ret);
                       return;
                   }

                   std::string content;
                   const int ret = service.ini_content(session, content);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"ini_name", identity.ini_name}, {"content", content}});
               });

    locked_get("/zmot/api/io_metadata",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   std::vector<IoMetadataEntry> metadata;
                   bool layout_ready = false;
                   const int ret = service.io_metadata(session, metadata, layout_ready);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }

                   json entries = json::array();
                   for (size_t i = 0; i < metadata.size(); ++i) {
                       const IoMetadataEntry& entry = metadata[i];
                       entries.push_back({{"io_type", io_alias_kind_name(entry.kind)},
                                          {"source_kind", io_source_kind_name(entry.source_kind)},
                                          {"io_number", entry.io_number},
                                          {"mask", entry.mask},
                                          {"slave_idx", entry.slave_idx},
                                          {"index", entry.index},
                                          {"index_hex", format_index_hex(entry.index)},
                                          {"subindex", entry.subindex},
                                          {"subindex_hex", format_subindex_hex(entry.subindex)},
                                          {"direction", entry.is_input ? "input" : "output"},
                                          {"bit_offset", entry.bit_offset},
                                          {"bit_size", entry.bit_size}});
                   }
                   replyJsonOk(res, {{"layout_ready", layout_ready}, {"io_metadata", entries}});
               });

    locked_get("/zmot/api/ini_download",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity identity = {};
                   const int identity_ret = service.getIdentity(session, identity);
                   if (identity_ret != 0) {
                       replyJsonResult(res, identity_ret);
                       return;
                   }

                   std::string bytes;
                   const int ret = service.ini_content(session, bytes);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   reply_file_download(res, identity.ini_name, bytes);
               });

    pSvr->Get(
        "/zmot/api/device_open",
        [&, this](const httplib::Request& req, httplib::Response& res) {
            const std::string device =
                req.has_param("device") ? req.get_param_value("device") : "0";
            bool opened = false;
            const int ret = service.getDeviceOpen(device, opened);
            if (ret != 0) {
                replyJsonResult(res, ret);
                return;
            }
            replyJsonOk(res, {{"device_open", opened}});
        });
    locked_get("/zmot/api/channel_open",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"channel_open", value.channel_open}});
               });
    locked_get("/zmot/api/ecat_running",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"ecat_running", value.ecat_running}});
               });
    locked_get("/zmot/api/session_status",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.session_status(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"device_open", value.device_open},
                                     {"device_name", value.device_name},
                                     {"channel_name", value.channel_name},
                                     {"available_channel_names", value.available_channel_names},
                                     {"eni_name", value.eni_name},
                                     {"eni_hash", value.eni_hash},
                                     {"channel_open", value.channel_open},
                                     {"ecat_running", value.ecat_running}});
               });
    locked_get("/zmot/api/ecat_bus_is_running",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   bool running = false;
                   const int ret = service.busIsRunning(session, running);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"value", running}});
               });
    locked_get("/zmot/api/ecat_get_master_req_state",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   uint8_t state = 0;
                   const int ret = service.getMasterReqState(session, state);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"value", state}});
               });
    locked_get("/zmot/api/ecat_get_master_state",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   uint8_t state = 0;
                   const int ret = service.getMasterState(session, state);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"value", state}});
               });
    locked_get("/zmot/api/ecat_pi_enable_get",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   bool enable = false;
                   const int ret = service.piEnableGet(session, enable);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"value", enable}});
               });
    locked_get("/zmot/api/ecat_get_slave_count",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   uint16_t count = 0;
                   const int ret = service.getSlaveCount(session, count);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"value", count}});
               });
    locked_get("/zmot/api/ecat_get_active_slave_count",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   uint16_t count = 0;
                   const int ret = service.getActiveSlaveCount(session, count);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"value", count}});
               });
    locked_get("/zmot/api/ecat_get_slave_state",
               [&, this](const httplib::Request& req, httplib::Response& res, RpcSession session) {
                   uint16_t slave_id = 0;
                   if (!get_required_query_uint16(req, "slave_id", res, slave_id)) return;
                   uint8_t state = 0;
                   const int ret = service.getSlaveState(session, slave_id, state);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"value", state}});
               });
    locked_get("/zmot/api/ecat_status",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvEcatStatus value = {};
                   const int ret = service.ecat_status(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"bus_is_running", value.bus_is_running},
                                     {"master_req_state", value.master_req_state},
                                     {"master_state", value.master_state},
                                     {"pi_enable", value.pi_enable},
                                     {"slave_count", value.slave_count},
                                     {"active_slave_count", value.active_slave_count}});
               });
    locked_get("/zmot/api/version",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   const uint32_t number = ZAL_VERSION_MAJOR * 1000000U +
                                           ZAL_VERSION_MINOR * 1000U + ZAL_VERSION_PATCH;
                   replyJsonOk(res, {{"version", value.version},
                                     {"major", ZAL_VERSION_MAJOR},
                                     {"minor", ZAL_VERSION_MINOR},
                                     {"patch", ZAL_VERSION_PATCH},
                                     {"number", number}});
               });
    locked_get("/zmot/api/about",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"version", value.version},
                                     {"author", value.author},
                                     {"description", value.description},
                                     {"code_date", value.code_date},
                                     {"device_name", value.device_name}});
               });
    locked_get("/zmot/api/controller_info",
               [&, this](const httplib::Request&, httplib::Response& res, RpcSession session) {
                   srv::SrvIdentity value = {};
                   const int ret = service.getIdentity(session, value);
                   if (ret != 0) {
                       replyJsonResult(res, ret);
                       return;
                   }
                   replyJsonOk(res, {{"soft_type", value.device_name},
                                     {"soft_version", value.version},
                                     {"controller_id", value.controller_id}});
               });

    pSvr->Post("/zmot/api/scan_device", [&, this](const httplib::Request& req,
                                                  httplib::Response& res) {
        json body;
        if (!parse_json_object(req, res, true, body)) return;
        std::vector<srv::SrvDeviceEntry> values;
        const int ret = service.scanDevice(values);
        if (ret != 0) {
            replyJsonResult(res, ret);
            return;
        }
        json devices = json::array();
        for (const auto& value : values)
            devices.push_back({{"device", value.device}, {"name", value.name}});
        replyJsonOk(res, {{"available_devices", devices}});
    });

    locked_post("/zmot/api/open_channel", [&, this](const httplib::Request& req,
                                                    httplib::Response& res, RpcSession session) {
        json body;
        std::string netname;
        if (!parse_json_object(req, res, false, body) ||
            !get_required_string(body, "netname", res, netname)) {
            return;
        }
        replyJsonResult(res, service.openChannel(session, netname));
    });

    locked_post("/zmot/api/close_channel", [&, this](const httplib::Request& req,
                                                     httplib::Response& res, RpcSession session) {
        json body;
        if (!parse_json_object(req, res, true, body)) {
            return;
        }
        replyJsonResult(res, service.closeChannel(session));
    });

    locked_post("/zmot/api/eni_select", [&, this](const httplib::Request& req,
                                                  httplib::Response& res, RpcSession session) {
        json body;
        std::string path;
        if (!parse_json_object(req, res, false, body) ||
            !get_required_string(body, "remote_file", res, path)) {
            return;
        }
        const int ret = service.eni_select(session, path);
        if (ret != 0) {
            replyJsonResult(res, ret);
            return;
        }
        srv::SrvIdentity value = {};
        const int identity_ret = service.getIdentity(session, value);
        if (identity_ret != 0) {
            replyJsonResult(res, identity_ret);
            return;
        }
        replyJsonOk(res, {{"hash", value.selected_eni_hash}});
    });

    locked_post("/zmot/api/ini_select", [&, this](const httplib::Request& req,
                                                  httplib::Response& res, RpcSession session) {
        json body;
        std::string path;
        if (!parse_json_object(req, res, false, body) ||
            !get_required_string(body, "remote_file", res, path)) {
            return;
        }
        const int ret = service.ini_select(session, path);
        if (ret != 0) {
            replyJsonResult(res, ret);
            return;
        }
        srv::SrvIdentity value = {};
        const int identity_ret = service.getIdentity(session, value);
        if (identity_ret != 0) {
            replyJsonResult(res, identity_ret);
            return;
        }
        replyJsonOk(res, {{"hash", value.selected_ini_hash}});
    });

    locked_post("/zmot/api/startup", [&, this](const httplib::Request& req, httplib::Response& res,
                                               RpcSession session) {
        json body;
        std::string path;
        if (!parse_json_object(req, res, true, body) ||
            !get_optional_string(body, "remote_file", res, path))
            return;
        replyJsonResult(res, service.startup(session, path));
    });

    locked_post("/zmot/api/startup_ini", [&, this](const httplib::Request& req,
                                                   httplib::Response& res, RpcSession session) {
        json body;
        std::string path;
        if (!parse_json_object(req, res, false, body) ||
            !get_required_string(body, "local_file", res, path))
            return;
        replyJsonResult(res, service.startup_ini(session, path));
    });

    locked_post("/zmot/api/ecat_stop", [&, this](const httplib::Request& req,
                                                 httplib::Response& res, RpcSession session) {
        json body;
        if (!parse_json_object(req, res, true, body)) return;
        replyJsonResult(res, service.ecat_stop(session));
    });

    locked_post("/zmot/api/ecat_request_master_state", [&, this](const httplib::Request& req,
                                                                 httplib::Response& res,
                                                                 RpcSession session) {
        json body;
        uint8_t state = 0;
        if (!parse_json_object(req, res, false, body) || !get_required_state(body, res, state)) {
            return;
        }
        replyJsonResult(res, service.requestMasterState(session, state));
    });
    locked_post(
        "/zmot/api/ecat_clear_slave_state_error",
        [&, this](const httplib::Request& req, httplib::Response& res, RpcSession session) {
            json body;
            uint16_t slave_id = 0;
            if (!parse_json_object(req, res, false, body) ||
                !get_required_uint16(body, "slave_id", res, slave_id)) {
                return;
            }
            replyJsonResult(res, service.clearSlaveStateError(session, slave_id));
        });
    locked_post(
        "/zmot/api/ecat_pi_enable_set",
        [&, this](const httplib::Request& req, httplib::Response& res, RpcSession session) {
            json body;
            bool enable = false;
            if (!parse_json_object(req, res, false, body) ||
                !get_required_bool(body, res, "flag", enable)) {
                return;
            }
            replyJsonResult(res, service.piEnableSet(session, enable));
        });
    locked_post(
        "/zmot/api/ecat_coe_sdo_download",
        [&, this](const httplib::Request& req, httplib::Response& res, RpcSession session) {
            json body;
            uint16_t slave_id = 0, index = 0;
            uint8_t subindex = 0;
            uint32_t size = 0, data = 0;
            if (!parse_json_object(req, res, false, body) ||
                !get_required_uint16(body, "slave_id", res, slave_id) ||
                !get_required_uint16(body, "index", res, index) ||
                !get_required_uint8(body, "subindex", res, subindex) ||
                !get_required_uint32(body, "size", res, size) ||
                !get_required_uint32(body, "data", res, data))
                return;
            const int ret =
                service.ecat_coe_sdo_download(session, slave_id, index, subindex, size, data);
            replyJsonResult(res, ret);
        });
    locked_post(
        "/zmot/api/ecat_coe_sdo_upload",
        [&, this](const httplib::Request& req, httplib::Response& res, RpcSession session) {
            json body;
            uint16_t slave_id = 0, index = 0;
            uint8_t subindex = 0;
            uint32_t size = 0, data = 0;
            if (!parse_json_object(req, res, false, body) ||
                !get_required_uint16(body, "slave_id", res, slave_id) ||
                !get_required_uint16(body, "index", res, index) ||
                !get_required_uint8(body, "subindex", res, subindex) ||
                !get_required_uint32(body, "size", res, size))
                return;
            const int ret =
                service.ecat_coe_sdo_upload(session, slave_id, index, subindex, size, data);
            if (ret != 0) {
                replyJsonResult(res, ret);
                return;
            }
            replyJsonOk(res, {{"data", data}});
        });
}

// GPT-6 (Codex), 2026-10-29. Join workers before derived route handlers are destroyed.
MotHttpEcat::~MotHttpEcat() {
    stop();
}

int MotHttpEcat::bind(srv::MotSrv& service) {
    // Register generic motion routes before ECAT-specific routes.
    const int ret = MotHttpQcmd::bindQcmd(service);
    if (ret != 0) return ret;
    bind_ecat_routes();
    return 0;
}

}  // namespace zmc
