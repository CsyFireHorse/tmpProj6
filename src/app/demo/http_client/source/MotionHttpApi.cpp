// Model signature: GPT-6. Preserve the approved RPC/HTTP axis and SDO contracts.
/*
 * @ArtifactStatus draft
 * @ArtifactStatusDate 2026-09-14
 * @ArtifactStatusAuthority GPT-5 Codex
 */

#include "MotionHttpApi.h"
#include "ZmlHttpContext.hpp"
#include "rpc/MotionRpcTypes.h"

#include <chrono>
#include <cstddef>
#include <ctime>
#include <cstring>
#include <exception>
#include <fstream>
#include <limits>
#include <new>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "common/thirdlib/nlohmann/json.hpp"
#include "common/thirdlib/yhirose/httplib.h"
#include "fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"

namespace {

using json = nlohmann::json;

const uint64_t kHttpFileMaxBytes = 16U * 1024U * 1024U;
const uint32_t kHttpConfigFileTimeoutMs = 5000U;
const uint32_t kHttpSessionTimeoutMs = 5000U;
// Read margin lets the server deliver its own wait-timeout reply before the transport gives up.
const uint32_t kHttpWaitTimeoutMarginMs = 500U;
const char kHttpEniFileName[] = "rpc_eni.xml";
const char kHttpSyscfgFileName[] = "rpc_syscfg.ini";
const char kHttpEniServerPath[] = "server_files/rpc_eni.xml";
const char kHttpSyscfgServerPath[] = "server_files/rpc_syscfg.ini";

inline int32_t httpClientRet(MC_ERROR_CODE code) { return static_cast<int32_t>(code); }

inline int32_t invalidArgRet() { return httpClientRet(ZMC_ERR_RPC_CLIENT_INVALID_ARG); }

void setFileTimeout(httplib::Client* client, uint32_t timeout_ms) {
    const time_t seconds = static_cast<time_t>(timeout_ms / 1000U);
    const time_t microseconds = static_cast<time_t>((timeout_ms % 1000U) * 1000U);
    client->set_connection_timeout(seconds, microseconds);
    client->set_read_timeout(seconds, microseconds);
    client->set_write_timeout(seconds, microseconds);
}

ZML_RESULT readLocalFile(const char* file_name, std::string& data) {
    data.clear();
    if (!file_name || !*file_name) return invalidArgRet();
    std::ifstream input(file_name, std::ios::in | std::ios::binary);
    if (!input.is_open()) return httpClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
    input.seekg(0, std::ios::end);
    const std::streampos end = input.tellg();
    if (end < 0) return httpClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
    const uint64_t size = static_cast<uint64_t>(end);
    if (size > kHttpFileMaxBytes ||
        size > static_cast<uint64_t>((std::numeric_limits<size_t>::max)())) {
        return httpClientRet(ZMC_ERR_RPC_FILE_TOO_LARGE);
    }
    input.seekg(0, std::ios::beg);
    data.resize(static_cast<size_t>(size));
    if (size != 0U) {
        input.read(&data[0], static_cast<std::streamsize>(size));
        if (!input || static_cast<uint64_t>(input.gcount()) != size) {
            data.clear();
            return httpClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
        }
    }
    return httpClientRet(ZMC_NO_ERROR);
}

ZML_RESULT createLocalFile(const char* file_name, const std::string& data) {
    if (!file_name || !*file_name) return invalidArgRet();
    std::ifstream existing(file_name, std::ios::in | std::ios::binary);
    if (existing.is_open()) return httpClientRet(ZMC_ERR_RPC_FILE_ALREADY_EXISTS);
    std::ofstream output(file_name, std::ios::out | std::ios::binary);
    if (!output.is_open()) return httpClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
    if (!data.empty()) output.write(data.data(), static_cast<std::streamsize>(data.size()));
    output.close();
    return output ? httpClientRet(ZMC_NO_ERROR) : httpClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
}

std::string encodePathSegment(const char* value) {
    static const char kHex[] = "0123456789ABCDEF";
    std::string encoded;
    for (const unsigned char* current = reinterpret_cast<const unsigned char*>(value); *current;
         ++current) {
        const unsigned char ch = *current;
        const bool plain = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                           (ch >= '0' && ch <= '9') || ch == '.' || ch == '_' || ch == '-';
        if (plain) {
            encoded.push_back(static_cast<char>(ch));
        } else {
            encoded.push_back('%');
            encoded.push_back(kHex[(ch >> 4) & 0x0fU]);
            encoded.push_back(kHex[ch & 0x0fU]);
        }
    }
    return encoded;
}

httplib::Client* getClientOrNull(ZML_HANDLE client) {
    if (!client) {
        return nullptr;
    }
    return client->client;
}

std::string numberToString(double value) {
    std::ostringstream oss;
    oss.precision(17);
    oss << value;
    return oss.str();
}

std::string numberToString(int32_t value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

std::string numberToString(uint32_t value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

std::string moveArgsToCsv(int axNum, const double* distanceArr) {
    std::ostringstream oss;
    oss.precision(17);
    const int32_t count = (axNum > ZMC_RPC_MAX_AXES) ? ZMC_RPC_MAX_AXES : axNum;
    for (int32_t i = 0; i < count; ++i) {
        if (i != 0) {
            oss << ',';
        }
        oss << distanceArr[i];
    }
    return oss.str();
}

json moveArgsToJson(int32_t ax_num, const int32_t* axes_in, const double* distance) {
    json body;
    json pos = json::array();
    const int32_t count = (ax_num > ZMC_RPC_MAX_AXES) ? ZMC_RPC_MAX_AXES : ax_num;
    for (int32_t i = 0; i < count; ++i) {
        pos.push_back(distance[i]);
    }
    body["pos"] = pos;
    json axes = json::array();
    for (int32_t i = 0; i < count; ++i) axes.push_back(axes_in[i]);
    body["axes"] = axes;
    body["len"] = count;
    return body;
}

std::string axisListToCsv(int32_t ax_num, const int32_t* axes) {
    std::ostringstream oss;
    const int32_t count = (ax_num > ZMC_RPC_MAX_AXES) ? ZMC_RPC_MAX_AXES : ax_num;
    for (int32_t i = 0; i < count; ++i) {
        if (i != 0) {
            oss << ',';
        }
        oss << axes[i];
    }
    return oss.str();
}

json axisListToJson(int32_t ax_num, const int32_t* axes_in) {
    json body;
    json axis = json::array();
    const int32_t count = (ax_num > ZMC_RPC_MAX_AXES) ? ZMC_RPC_MAX_AXES : ax_num;
    for (int32_t i = 0; i < count; ++i) {
        axis.push_back(axes_in[i]);
    }
    body["axes"] = axis;
    body["len"] = count;
    return body;
}

std::string appendQuery(const std::string& path, const std::string& name,
                        const std::string& value) {
    return path + "?" + name + "=" + value;
}

std::string appendQuery3(const std::string& path, const std::string& n1, const std::string& v1,
                         const std::string& n2, const std::string& v2, const std::string& n3,
                         const std::string& v3) {
    return path + "?" + n1 + "=" + v1 + "&" + n2 + "=" + v2 + "&" + n3 + "=" + v3;
}

int32_t retFromResponse(const httplib::Response& res) {
    // HTTP business errors are encoded in the JSON code field even when status is 200.
    try {
        const json body = json::parse(res.body);
        if (body.is_object() && body.contains("code") && body["code"].is_number_integer()) {
            return body["code"].get<int32_t>();
        }
    } catch (const std::exception&) {
        // Preserve the legacy non-JSON error handling below.
    }
    if (res.status < 200 || res.status >= 300) {
        const std::string prefix = "FAIL: ";
        const std::size_t pos = res.body.find(prefix);
        if (pos != std::string::npos) {
            try {
                return static_cast<int32_t>(std::stoi(res.body.substr(pos + prefix.size())));
            } catch (const std::exception&) {
            }
        }
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    return httpClientRet(ZMC_NO_ERROR);
}

int32_t responseCode(const json& root) {
    if (!root.is_object() || !root.contains("code") || !root["code"].is_number_integer()) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    return root["code"].get<int32_t>();
}

// IO values retain their declared boolean or complete unsigned 32-bit wire type.
bool parseIoValue(const json& field, bool& value) {
    if (!field.is_boolean()) return false;
    value = field.get<bool>();
    return true;
}

bool parseIoValue(const json& field, uint32_t& value) {
    if (!field.is_number_unsigned() ||
        field.get<uint64_t>() > (std::numeric_limits<uint32_t>::max)()) return false;
    value = field.get<uint32_t>();
    return true;
}

template <typename T>
ZML_RESULT readIo(ZML_HANDLE handle, const char* path, uint32_t io, T* value) {
    httplib::Client* client = getClientOrNull(handle);
    if (!client) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!value || io >= MC_IO_MAX_COUNT) return invalidArgRet();
    const auto response = client->Post(path, json({{"io_number", io}}).dump(), "application/json");
    handle->connectionState = response ? 1 : 2;
    if (!response) return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    try {
        const json body = json::parse(response->body);
        const int32_t ret = responseCode(body);
        if (ret != 0) return ret;
        T decoded = 0;
        if (!parseIoValue(body.at("value"), decoded)) {
            return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
        }
        *value = decoded;
        return 0;
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

// Preserve session transport errors while retaining other query failure behavior.
int32_t queryFailure(const httplib::Result& res) {
    return res && (res->status == 400 || res->status == 401)
               ? retFromResponse(*res)
               : httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
}

int32_t postPath(ZML_HANDLE client, const std::string& path) {
    httplib::Client* cli = getClientOrNull(client);
    if (!cli) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    }
    httplib::Result res = cli->Post(path.c_str(), "", "text/plain");
    if (!res) {
        client->connectionState = 2;
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    client->connectionState = 1;
    return retFromResponse(*res);
}

int32_t postJson(ZML_HANDLE client, const std::string& path, const json& body) {
    httplib::Client* cli = getClientOrNull(client);
    if (!cli) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    }
    httplib::Result res = cli->Post(path.c_str(), body.dump(), "application/json");
    if (!res) {
        client->connectionState = 2;
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    client->connectionState = 1;
    return retFromResponse(*res);
}

httplib::Result getPath(ZML_HANDLE client, const std::string& path) {
    httplib::Client* cli = getClientOrNull(client);
    if (!cli) {
        return httplib::Result(nullptr, httplib::Error::Unknown);
    }
    httplib::Result res = cli->Get(path.c_str());
    client->connectionState = res ? 1 : 2;
    return res;
}

int32_t commandPath(ZML_HANDLE client, const std::string& path) { return postPath(client, path); }

int32_t commandValue(ZML_HANDLE client, const std::string& path, double value) {
    json body;
    body["value"] = value;
    return postJson(client, path, body);
}

int32_t commandAxis(ZML_HANDLE client, const std::string& path, int32_t axis) {
    json body;
    body["axis"] = axis;
    return postJson(client, path, body);
}

int32_t executeCommandText(ZML_HANDLE client, const std::string& cmd) {
    json body;
    body["cmd"] = cmd;
    return postJson(client, "/zmot/api/execute_command", body);
}

int32_t parseAxisState(const json& root, RpcAxisState& out) {
    const int32_t ret = responseCode(root);
    if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
    json::const_iterator it = root.find("axis");
    if (it == root.end() || it->is_null()) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    out.axis_id = static_cast<uint8_t>((*it)["axis_id"].get<int>());
    out.axis_state = static_cast<uint8_t>((*it)["axis_state"].get<int>());
    out.error_id = static_cast<uint16_t>((*it)["error_id"].get<int>());
    out.tick = (*it)["tick"].get<uint32_t>();
    out.act_pos = (*it)["act_pos"].get<double>();
    out.set_pos = (*it)["set_pos"].get<double>();
    return httpClientRet(ZMC_NO_ERROR);
}

int32_t parseAxisState2(const json& root, RpcAxisState2& out) {
    const int32_t ret = responseCode(root);
    if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
    json::const_iterator it = root.find("axis");
    if (it == root.end() || it->is_null()) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    out.axis_id = static_cast<uint8_t>((*it)["axis_id"].get<int>());
    out.axis_state = static_cast<uint8_t>((*it)["axis_state"].get<int>());
    out.error_id = static_cast<uint16_t>((*it)["error_id"].get<int>());
    out.tick = (*it)["tick"].get<uint32_t>();
    out.act_pos = (*it)["act_pos"].get<double>();
    out.set_pos = (*it)["set_pos"].get<double>();
    out.set_vel = (*it)["set_vel"].get<double>();
    out.set_acc = (*it)["set_acc"].get<double>();
    out.set_position = (*it)["set_position"].get<int32_t>();
    out.act_position = (*it)["act_position"].get<int32_t>();
    return httpClientRet(ZMC_NO_ERROR);
}

int32_t parseCmdResults(const json& root, std::vector<ZML_CmdResultInfo>& out) {
    const int32_t ret = responseCode(root);
    if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
    json::const_iterator it = root.find("cmd_results");
    if (it == root.end() || !it->is_array()) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    out.clear();
    for (json::const_iterator item = it->begin(); item != it->end(); ++item) {
        ZML_CmdResultInfo info = {};
        info.id = (*item)["id"].get<uint32_t>();
        info.command_type = (*item)["command_type"].get<uint16_t>();
        info.target_index = (*item)["target_index"].get<uint8_t>();
        info.target_kind = (*item)["target_kind"].get<uint8_t>();
        info.status = (*item)["status"].get<uint8_t>();
        info.io_count = (*item)["io_count"].get<uint8_t>();
        info.error_code = (*item)["error_code"].get<uint16_t>();
        info.tick = (*item)["tick"].get<uint32_t>();
        info.data = (*item)["data"].get<int32_t>();
        info.data_ext = (*item)["data_ext"].get<int32_t>();
        out.push_back(info);
    }
    return httpClientRet(ZMC_NO_ERROR);
}
// AI modification: GPT-6 Codex, 2026-10-12. Preserve output storage on query errors.
ZML_RESULT readNumericValue(ZML_HANDLE client, const std::string& path, double* value) {
    const httplib::Result res = getPath(client, path);
    if (!res) return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        // Motion errors retain their JSON code even when the server reports HTTP 500.
        if (ret != 0) return ret;
        if (res->status < 200 || res->status >= 300) {
            return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
        }
        *value = root.at("value").get<double>();
        return 0;
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT readAxisParameter(ZML_HANDLE client, const char* path, int32_t axis, double* value) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!value) return invalidArgRet();
    return readNumericValue(client, appendQuery(path, "axis", numberToString(axis)), value);
}

void copyTextField(const json& root, const char* name, char* output, std::size_t capacity) {
    const std::string value = root.at(name).get<std::string>();
    const std::size_t copyLength = (value.size() < capacity - 1U) ? value.size() : capacity - 1U;
    std::memcpy(output, value.data(), copyLength);
    output[copyLength] = '\0';
}

}  // namespace

ZML_RESULT zml_ecat_start(ZML_HANDLE client) {
    const ZML_RESULT open_ret = commandPath(client, "/zmot/api/open_device");
    if (open_ret != httpClientRet(ZMC_NO_ERROR)) return open_ret;
    return commandPath(client, "/zmot/api/startup");
}

ZML_RESULT zml_ecat_stop(ZML_HANDLE client) { return commandPath(client, "/zmot/api/ecat_stop"); }

ZML_RESULT zml_get_version_info(ZML_HANDLE client, char* version) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!version) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/version");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        copyTextField(root, "version", version, ZML_VERSION_INFO_MAX);
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_about(ZML_HANDLE client, char* about) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!about) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/about");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        copyTextField(root, "description", about, ZML_ABOUT_INFO_MAX);
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_controller_info(ZML_HANDLE client, char* soft_type, char* soft_version,
                                   char* controller_id) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!soft_type || !soft_version || !controller_id) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/controller_info");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        copyTextField(root, "soft_type", soft_type, ZML_CONTROLLER_INFO_MAX);
        copyTextField(root, "soft_version", soft_version, ZML_CONTROLLER_INFO_MAX);
        copyTextField(root, "controller_id", controller_id, ZML_CONTROLLER_INFO_MAX);
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_bus_is_running(ZML_HANDLE client, bool* run_flag) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!run_flag) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/ecat_bus_is_running");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        *run_flag = root.at("value").get<bool>();
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_request_master_state(ZML_HANDLE client, uint8_t state) {
    json body;
    body["state"] = state;
    return postJson(client, "/zmot/api/ecat_request_master_state", body);
}

ZML_RESULT zml_ecat_get_master_req_state(ZML_HANDLE client, uint8_t* state) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!state) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/ecat_get_master_req_state");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        *state = root.at("value").get<uint8_t>();
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_get_master_state(ZML_HANDLE client, uint8_t* state) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!state) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/ecat_get_master_state");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        *state = root.at("value").get<uint8_t>();
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_pi_enable_set(ZML_HANDLE client, bool flag) {
    json body;
    body["flag"] = flag;
    return postJson(client, "/zmot/api/ecat_pi_enable_set", body);
}

ZML_RESULT zml_ecat_pi_enable_get(ZML_HANDLE client, bool* flag) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!flag) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/ecat_pi_enable_get");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        *flag = root.at("value").get<bool>();
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_coe_sdo_download(ZML_HANDLE client, uint16_t slave_id, uint16_t index,
                                     uint8_t sub_index, uint32_t data_len, const void* data) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if ((data_len != 1U && data_len != 2U && data_len != 4U) || !data) return invalidArgRet();
    json body;
    body["slave_id"] = slave_id;
    body["index"] = index;
    body["subindex"] = sub_index;
    uint32_t value = 0;
    std::memcpy(&value, data, data_len);
    body["data"] = value;
    body["size"] = data_len;
    return postJson(client, "/zmot/api/ecat_coe_sdo_download", body);
}

ZML_RESULT zml_ecat_coe_sdo_upload(ZML_HANDLE client, uint16_t slave_id, uint16_t index,
                                   uint8_t sub_index, uint32_t* data_len, void* data) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!data_len || (*data_len != 1U && *data_len != 2U && *data_len != 4U) || !data)
        return invalidArgRet();
    const uint32_t capacity = *data_len;
    json body;
    body["slave_id"] = slave_id;
    body["index"] = index;
    body["subindex"] = sub_index;
    body["size"] = capacity;
    httplib::Client* cli = getClientOrNull(client);
    httplib::Result res =
        cli->Post("/zmot/api/ecat_coe_sdo_upload", body.dump(), "application/json");
    if (!res) {
        client->connectionState = 2;
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    client->connectionState = 1;
    const int32_t response_ret = retFromResponse(*res);
    if (response_ret != static_cast<int32_t>(ZMC_NO_ERROR)) return response_ret;
    try {
        const json root = json::parse(res->body);
        const uint32_t value = root.at("data").get<uint32_t>();
        std::memcpy(data, &value, capacity);
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_get_slave_count(ZML_HANDLE client, uint16_t* slave_count) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!slave_count) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/ecat_get_slave_count");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        *slave_count = root.at("value").get<uint16_t>();
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_get_active_slave_count(ZML_HANDLE client, uint16_t* slave_count) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!slave_count) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/ecat_get_active_slave_count");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        *slave_count = root.at("value").get<uint16_t>();
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_get_slave_state(ZML_HANDLE client, uint16_t slave_id, uint8_t* state) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!state) return invalidArgRet();
    const std::string path =
        appendQuery("/zmot/api/ecat_get_slave_state", "slave_id", numberToString(slave_id));
    httplib::Result res = getPath(client, path);
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        *state = root.at("value").get<uint8_t>();
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_clear_slave_state_error(ZML_HANDLE client, uint16_t slave_id) {
    json body;
    body["slave_id"] = slave_id;
    return postJson(client, "/zmot/api/ecat_clear_slave_state_error", body);
}

ZML_RESULT zml_open_eth(ZML_HANDLE* out_handle, const char* ipaddr) {
    return zml_open(out_handle, ipaddr);
}

ZML_RESULT zml_open_pci(ZML_HANDLE* out_handle, uint32_t alias) {
    if (alias > 9U) {
        if (out_handle) *out_handle = nullptr;
        return invalidArgRet();
    }
    /* HTTP uses the local default service for every accepted PCI alias. */
    return zml_open(out_handle, "127.0.0.1");
}

ZML_RESULT zml_open(ZML_HANDLE* out_handle, const char* device) {
    if (!device || !out_handle) return invalidArgRet();
    std::string endpoint(device);
    bool numericDevice = !endpoint.empty();
    for (std::size_t i = 0; i < endpoint.size(); ++i) {
        if (endpoint[i] < '0' || endpoint[i] > '9') numericDevice = false;
    }
    if (numericDevice) return invalidArgRet();
    std::string host = endpoint;
    int port = ZMC_MOTAPP_HTTP_DEFAULT_PORT;
    const std::size_t separator = endpoint.rfind(':');
    if (separator != std::string::npos && endpoint.find(':') == separator) {
        host = endpoint.substr(0, separator);
        const std::string portText = endpoint.substr(separator + 1);
        if (host.empty() || portText.empty()) return invalidArgRet();
        try {
            port = std::stoi(portText);
        } catch (const std::exception&) {
            return invalidArgRet();
        }
        if (port <= 0 || port > 65535) return invalidArgRet();
    }
    *out_handle = nullptr;
    ZML_Context* client = new (std::nothrow) ZML_Context();
    if (!client) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CONNECT_FAILED);
    }
    client->client = nullptr;
    client->connectionState = 0;
    const std::string normalizedHost = (host == "localhost") ? "127.0.0.1" : host;
    client->client = new (std::nothrow) httplib::Client(normalizedHost.c_str(), port);
    if (!client->client) {
        delete client;
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CONNECT_FAILED);
    }
    // AI modification signature: GPT-6. Publish the handle only after session creation.
    setFileTimeout(client->client, kHttpSessionTimeoutMs);
    const httplib::Result result =
        client->client->Post("/zmot/api/open", "{}", "application/json");
    int32_t ret =
        result ? retFromResponse(*result) : httpClientRet(ZMC_ERR_RPC_CLIENT_CONNECT_FAILED);
    if (ret == httpClientRet(ZMC_NO_ERROR)) {
        const json body = json::parse(result->body, nullptr, false);
        if (body.is_object() && body.contains("session") && body["session"].is_string() &&
            !body["session"].get_ref<const std::string&>().empty()) {
            // Default headers are the sole stored copy and cover every HTTP request path.
            client->client->set_default_headers(
                {{"X-Zmot-Session", body["session"].get<std::string>()}});
            client->connectionState = 1;
            *out_handle = client;
            return ret;
        }
        ret = httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    delete client->client;
    delete client;
    return ret;
}

ZML_RESULT zml_read_di(ZML_HANDLE handle, uint32_t io_number, bool* value) {
    return readIo(handle, "/zmot/api/read_di_sync", io_number, value);
}

ZML_RESULT zml_read_do(ZML_HANDLE handle, uint32_t io_number, bool* value) {
    return readIo(handle, "/zmot/api/read_do_sync", io_number, value);
}

ZML_RESULT zml_write_do(ZML_HANDLE handle, uint32_t io_number, bool value) {
    if (!getClientOrNull(handle)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (io_number >= MC_IO_MAX_COUNT) return invalidArgRet();
    return postJson(handle, "/zmot/api/write_do", {{"io_number", io_number}, {"value", value}});
}

ZML_RESULT zml_read_ai(ZML_HANDLE handle, uint32_t io_number, uint32_t* value) {
    return readIo(handle, "/zmot/api/read_ai_sync", io_number, value);
}

ZML_RESULT zml_read_ao(ZML_HANDLE handle, uint32_t io_number, uint32_t* value) {
    return readIo(handle, "/zmot/api/read_ao_sync", io_number, value);
}

ZML_RESULT zml_write_ao(ZML_HANDLE handle, uint32_t io_number, uint32_t value) {
    if (!getClientOrNull(handle)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (io_number >= MC_IO_MAX_COUNT) return invalidArgRet();
    return postJson(handle, "/zmot/api/write_ao", {{"io_number", io_number}, {"value", value}});
}

ZML_RESULT zml_write_virtual_di(ZML_HANDLE handle, uint32_t io_number, bool value) {
    if (!getClientOrNull(handle)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (io_number >= MC_IO_MAX_COUNT) return invalidArgRet();
    return postJson(handle, "/zmot/api/write_virtual_di",
                    {{"io_number", io_number}, {"value", value}});
}

ZML_RESULT zml_write_virtual_ai(ZML_HANDLE handle, uint32_t io_number, uint32_t value) {
    if (!getClientOrNull(handle)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (io_number >= MC_IO_MAX_COUNT) return invalidArgRet();
    return postJson(handle, "/zmot/api/write_virtual_ai",
                    {{"io_number", io_number}, {"value", value}});
}

ZML_RESULT zml_get_connection_state(ZML_HANDLE client) {
    if (!client) {
        return 0;
    }
    return client->connectionState;
}

ZML_RESULT zml_wait_until_connection_ready(ZML_HANDLE client, int32_t cnt) {
    if (!getClientOrNull(client)) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    }
    while (cnt-- > 0) {
        httplib::Result res = getPath(client, "/zmot/api/flow_status");
        if (res && (res->status == 400 || res->status == 401)) return retFromResponse(*res);
        if (res && res->status >= 200 && res->status < 300) {
            return httpClientRet(ZMC_NO_ERROR);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return httpClientRet(ZMC_ERR_RPC_CLIENT_CONNECT_TIMEOUT);
}

ZML_RESULT zml_close(ZML_HANDLE* handle) {
    if (!handle || !*handle) return httpClientRet(ZMC_NO_ERROR);
    ZML_Context* context = static_cast<ZML_Context*>(*handle);
    setFileTimeout(context->client, kHttpSessionTimeoutMs);
    const int32_t ret = postJson(context, "/zmot/api/close", json::object());
    // Always clear the local handle, matching the RPC client's close ownership.
    delete context->client;
    context->client = nullptr;
    delete context;
    *handle = nullptr;
    return ret;
}

ZML_RESULT zml_ping(ZML_HANDLE client, uint32_t timeout_ms) {
    httplib::Client* http_client = getClientOrNull(client);
    if (!http_client) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (timeout_ms == 0U) return invalidArgRet();
    setFileTimeout(http_client, timeout_ms + kHttpWaitTimeoutMarginMs);
    return postJson(client, "/zmot/api/ping", {{"timeout_ms", timeout_ms}});
}

// Publish the server's complete unsigned echo value on success. GPT-6 Codex.
ZML_RESULT zml_mailbox_ping(ZML_HANDLE handle, uint32_t pingValue, uint32_t* pongValue) {
    httplib::Client* client = getClientOrNull(handle);
    if (!client) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!pongValue) return invalidArgRet();
    const auto response = client->Post("/zmot/api/mailbox_ping",
                                      json({{"ping_value", pingValue}}).dump(), "application/json");
    handle->connectionState = response ? 1 : 2;
    if (!response) return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    try {
        const json body = json::parse(response->body);
        const int32_t ret = responseCode(body);
        if (ret != 0) return ret;
        uint32_t decoded = 0;
        if (!parseIoValue(body.at("pong_value"), decoded)) {
            return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
        }
        *pongValue = decoded;
        return 0;
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_download_file(ZML_HANDLE client, const char* local_file_name,
                             const char* remote_file_name, uint32_t timeout_ms) {
    httplib::Client* http_client = getClientOrNull(client);
    if (!http_client) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!remote_file_name || !*remote_file_name || timeout_ms == 0U) return invalidArgRet();
    std::string data;
    const ZML_RESULT read_ret = readLocalFile(local_file_name, data);
    if (read_ret != httpClientRet(ZMC_NO_ERROR)) return read_ret;

    setFileTimeout(http_client, timeout_ms);
    httplib::MultipartFormDataItems items;
    items.push_back({"file", data, remote_file_name, "application/octet-stream"});
    httplib::Result result = http_client->Post("/zmot/api/files", items);
    client->connectionState = result ? 1 : 2;
    if (!result) return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    int32_t ret = retFromResponse(*result);
    if (ret != static_cast<int32_t>(ZMC_ERR_RPC_FILE_ALREADY_EXISTS)) return ret;

    result = http_client->Put("/zmot/api/files/" + encodePathSegment(remote_file_name), items);
    client->connectionState = result ? 1 : 2;
    return result ? retFromResponse(*result) : httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
}

ZML_RESULT zml_upload_file(ZML_HANDLE client, const char* local_file_name,
                           const char* remote_file_name, uint32_t timeout_ms) {
    httplib::Client* http_client = getClientOrNull(client);
    if (!http_client) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!local_file_name || !*local_file_name || !remote_file_name || !*remote_file_name ||
        timeout_ms == 0U) {
        return invalidArgRet();
    }
    std::ifstream existing(local_file_name, std::ios::in | std::ios::binary);
    if (existing.is_open()) return httpClientRet(ZMC_ERR_RPC_FILE_ALREADY_EXISTS);

    setFileTimeout(http_client, timeout_ms);
    const httplib::Result result =
        http_client->Get(("/zmot/api/files/" + encodePathSegment(remote_file_name)).c_str());
    client->connectionState = result ? 1 : 2;
    if (!result) return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    const std::string content_type = result->get_header_value("Content-Type");
    const int32_t ret = content_type.find("application/octet-stream") != std::string::npos
                            ? httpClientRet(ZMC_NO_ERROR)
                            : retFromResponse(*result);
    if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
    return createLocalFile(local_file_name, result->body);
}

ZML_RESULT zml_download_eni(ZML_HANDLE client, const char* path_name) {
    const ZML_RESULT ret =
        zml_download_file(client, path_name, kHttpEniFileName, kHttpConfigFileTimeoutMs);
    if (ret != httpClientRet(ZMC_NO_ERROR)) return ret;
    json body;
    body["remote_file"] = kHttpEniServerPath;
    return postJson(client, "/zmot/api/eni_select", body);
}

ZML_RESULT zml_download_syscfg(ZML_HANDLE client, const char* path_name) {
    const ZML_RESULT ret =
        zml_download_file(client, path_name, kHttpSyscfgFileName, kHttpConfigFileTimeoutMs);
    if (ret != httpClientRet(ZMC_NO_ERROR)) return ret;
    json body;
    body["remote_file"] = kHttpSyscfgServerPath;
    return postJson(client, "/zmot/api/ini_select", body);
}

ZML_RESULT zml_upload_eni(ZML_HANDLE client, const char* path_name) {
    return zml_upload_file(client, path_name, kHttpEniFileName, kHttpConfigFileTimeoutMs);
}

ZML_RESULT zml_upload_syscfg(ZML_HANDLE client, const char* path_name) {
    return zml_upload_file(client, path_name, kHttpSyscfgFileName, kHttpConfigFileTimeoutMs);
}

ZML_RESULT zml_ax_move_abs(ZML_HANDLE client, int32_t axis, double pos) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    json body;
    body["axis"] = axis;
    body["pos"] = pos;
    return postJson(client, "/zmot/api/movesgabs", body);
}

ZML_RESULT zml_moveabs(ZML_HANDLE client, int32_t ax_num, const int32_t* axes,
                       const double* pos) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes || !pos)
        return invalidArgRet();
    return postJson(client, "/zmot/api/mmoveabs", moveArgsToJson(ax_num, axes, pos));
}

ZML_RESULT zml_ax_power_on(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/poweron", axis);
}

ZML_RESULT zml_ax_power_off(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/poweroff", axis);
}

ZML_RESULT zml_ax_get_state(ZML_HANDLE client, int32_t axis, ZML_AxisState* out_state) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_state) return invalidArgRet();
    httplib::Result res =
        getPath(client, appendQuery("/zmot/api/axis_state", "axis", numberToString(axis)));
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        return parseAxisState(json::parse(res->body), *out_state);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_get_state_ex(ZML_HANDLE client, int32_t axis, ZML_AxisStateEx* out_state) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_state) return invalidArgRet();
    httplib::Result res =
        getPath(client, appendQuery("/zmot/api/axis_state2", "axis", numberToString(axis)));
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        return parseAxisState2(json::parse(res->body), *out_state);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_set_enable(ZML_HANDLE client, int32_t axis, int32_t enable) {
    json body;
    body["axis"] = axis;
    body["enable"] = enable ? 1 : 0;
    return postJson(client, "/zmot/api/axisenable", body);
}

ZML_RESULT zml_ax_get_enable(ZML_HANDLE client, int32_t axis, int32_t* enable) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!enable) return invalidArgRet();
    ZML_AxisState state = {};
    const ZML_RESULT ret = zml_ax_get_state(client, axis, &state);
    if (ret != httpClientRet(ZMC_NO_ERROR)) return ret;
    /* The public axis-state byte carries the extended power-off error state. */
    *enable = state.axis_state == 0x00U || state.axis_state == 0x17U ? 0 : 1;
    return ret;
}

ZML_RESULT zml_ax_get_units(ZML_HANDLE client, int32_t axis, double* units) {
    return readAxisParameter(client, "/zmot/api/units", axis, units);
}
ZML_RESULT zml_ax_get_speed(ZML_HANDLE client, int32_t axis, double* speed) {
    return readAxisParameter(client, "/zmot/api/speed", axis, speed);
}
ZML_RESULT zml_ax_get_accel(ZML_HANDLE client, int32_t axis, double* accel) {
    return readAxisParameter(client, "/zmot/api/accel", axis, accel);
}
ZML_RESULT zml_ax_get_decel(ZML_HANDLE client, int32_t axis, double* decel) {
    return readAxisParameter(client, "/zmot/api/decel", axis, decel);
}
ZML_RESULT zml_ax_get_jerk(ZML_HANDLE client, int32_t axis, double* jerk) {
    return readAxisParameter(client, "/zmot/api/jerk", axis, jerk);
}

ZML_RESULT zml_ax_read_parameter(ZML_HANDLE client, int32_t axis, int32_t parameter_number,
                              double* value) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!value) return invalidArgRet();
    const std::string path =
        appendQuery("/zmot/api/read_parameter", "axis", numberToString(axis)) +
        "&parameter_number=" + numberToString(parameter_number);
    return readNumericValue(client, path, value);
}

// The response carries the motion-cycle execution ACK. GPT-6 Codex.
ZML_RESULT zml_ax_write_parameter(ZML_HANDLE client, int32_t axis, int32_t parameter_number,
                               double value) {
    return postJson(client, "/zmot/api/write_parameter",
                    {{"axis", axis}, {"parameter_number", parameter_number}, {"value", value}});
}

// Typed single-value reads reuse the dedicated read-parameter routes; each call
// performs exactly one parameter query. GLM-5.3 (opencode), 2026-10-23.
ZML_RESULT zml_ax_read_state(ZML_HANDLE client, int32_t axis, int32_t* state) {
    if (!state) return invalidArgRet();
    double value = 0;
    const ZML_RESULT ret =
        readAxisParameter(client, "/zmot/api/read_axis_state", axis, &value);
    if (ret == httpClientRet(ZMC_NO_ERROR)) *state = static_cast<int32_t>(value);
    return ret;
}

ZML_RESULT zml_ax_read_set_position(ZML_HANDLE client, int32_t axis, double* position) {
    return readAxisParameter(client, "/zmot/api/read_set_position", axis, position);
}

ZML_RESULT zml_ax_read_act_position(ZML_HANDLE client, int32_t axis, double* position) {
    return readAxisParameter(client, "/zmot/api/read_act_position", axis, position);
}

ZML_RESULT zml_ax_read_sw_limit_negative(ZML_HANDLE client, int32_t axis, double* limit) {
    return readAxisParameter(client, "/zmot/api/read_sw_limit_negative", axis, limit);
}

ZML_RESULT zml_ax_read_sw_limit_enable(ZML_HANDLE client, int32_t axis, int32_t* enable) {
    if (!enable) return invalidArgRet();
    double value = 0;
    const ZML_RESULT ret =
        readAxisParameter(client, "/zmot/api/read_sw_limit_enable", axis, &value);
    if (ret == httpClientRet(ZMC_NO_ERROR)) *enable = static_cast<int32_t>(value);
    return ret;
}

ZML_RESULT zml_ax_read_sw_limit_positive(ZML_HANDLE client, int32_t axis, double* limit) {
    return readAxisParameter(client, "/zmot/api/read_sw_limit_positive", axis, limit);
}

ZML_RESULT zml_ax_read_units_num(ZML_HANDLE client, int32_t axis, int32_t* num) {
    if (!num) return invalidArgRet();
    double value = 0;
    const ZML_RESULT ret = readAxisParameter(client, "/zmot/api/read_units_num", axis, &value);
    if (ret == httpClientRet(ZMC_NO_ERROR)) *num = static_cast<int32_t>(value);
    return ret;
}

ZML_RESULT zml_ax_read_units_den(ZML_HANDLE client, int32_t axis, int32_t* den) {
    if (!den) return invalidArgRet();
    double value = 0;
    const ZML_RESULT ret = readAxisParameter(client, "/zmot/api/read_units_den", axis, &value);
    if (ret == httpClientRet(ZMC_NO_ERROR)) *den = static_cast<int32_t>(value);
    return ret;
}

// Units post the explicit num/den ratio body. GLM-5.3 (opencode), 2026-10-23.
ZML_RESULT zml_ax_set_units(ZML_HANDLE client, int32_t axis, int32_t num, int32_t den) {
    json body;
    body["axis"] = axis;
    body["num"] = num;
    body["den"] = den;
    return postJson(client, "/zmot/api/units", body);
}

ZML_RESULT zml_ax_set_velocity(ZML_HANDLE client, int32_t axis, double speed) {
    json body;
    body["axis"] = axis;
    body["value"] = speed;
    return postJson(client, "/zmot/api/speed", body);
}

ZML_RESULT zml_ax_set_accel(ZML_HANDLE client, int32_t axis, double accel) {
    json body;
    body["axis"] = axis;
    body["value"] = accel;
    return postJson(client, "/zmot/api/accel", body);
}

ZML_RESULT zml_ax_set_decel(ZML_HANDLE client, int32_t axis, double decel) {
    json body;
    body["axis"] = axis;
    body["value"] = decel;
    return postJson(client, "/zmot/api/decel", body);
}

ZML_RESULT zml_ax_set_jerk(ZML_HANDLE client, int32_t axis, double jerk) {
    json body;
    body["axis"] = axis;
    body["value"] = jerk;
    return postJson(client, "/zmot/api/sparam", body);
}

ZML_RESULT zml_ax_set_fastdecel(ZML_HANDLE client, int32_t axis, double fast_decel) {
    json body;
    body["axis"] = axis;
    body["value"] = fast_decel;
    return postJson(client, "/zmot/api/fastdecel", body);
}

ZML_RESULT zml_ax_set_direction(ZML_HANDLE client, int32_t axis, int32_t dir) {
    json body;
    body["axis"] = axis;
    body["value"] = dir;
    return postJson(client, "/zmot/api/direction", body);
}

ZML_RESULT zml_ax_set_buffermode(ZML_HANDLE client, int32_t axis, int32_t buffer_mode) {
    json body;
    body["axis"] = axis;
    body["value"] = buffer_mode;
    return postJson(client, "/zmot/api/buffermode", body);
}

ZML_RESULT zml_set_flow_config(ZML_HANDLE client, const ZML_FlowCtrlConfig* cfg) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!cfg) return invalidArgRet();
    json body;
    body["period"] = cfg->period;
    body["batchSize"] = cfg->batch_size;
    body["verboseFlag"] = cfg->verbose_flag;
    return postJson(client, "/zmot/api/flow_config", body);
}

ZML_RESULT zml_ax_stop(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/stop", axis);
}

ZML_RESULT zml_ax_halt(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/halt", axis);
}

ZML_RESULT zml_ax_reset(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/reset", axis);
}

ZML_RESULT zml_set_master_axis(ZML_HANDLE client, int32_t axis, int32_t master_axis) {
    return postJson(client, "/zmot/api/master", {{"axis", axis}, {"master_axis", master_axis}});
}

ZML_RESULT zml_ax_move_rel(ZML_HANDLE client, int32_t axis, double pos) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    json body;
    body["axis"] = axis;
    body["pos"] = pos;
    return postJson(client, "/zmot/api/movesg", body);
}

ZML_RESULT zml_ax_moveadd(ZML_HANDLE client, int32_t axis, double pos) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    json body;
    body["axis"] = axis;
    body["pos"] = pos;
    return postJson(client, "/zmot/api/moveadd", body);
}

ZML_RESULT zml_ax_home_start(ZML_HANDLE client, int32_t axis, const ZML_HomeSdoConfig* config) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!config) return httpClientRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    json body;
    body["axis"] = axis;
    body["mode"] = config->mode;
    body["offset"] = config->offset;
    body["velocity_fast"] = config->velocity_fast;
    body["velocity_slow"] = config->velocity_slow;
    body["acceleration"] = config->acceleration;
    body["timeout_ms"] = config->timeout_ms;
    return postJson(client, "/zmot/api/home_start", body);
}

ZML_RESULT zml_ax_home_get_status(ZML_HANDLE client, int32_t axis, int32_t* status,
                               int32_t* error_id) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!status || !error_id) return invalidArgRet();
    const std::string path =
        appendQuery("/zmot/api/home_get_status", "axis", numberToString(axis));
    httplib::Result res = getPath(client, path);
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        *status = root.at("status").get<int32_t>();
        *error_id = root.at("error_id").get<int32_t>();
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_home_stop(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/stop", axis);
}

ZML_RESULT zml_ax_home(ZML_HANDLE client, int32_t axis, double pos) {
    json body;
    body["axis"] = axis;
    body["pos"] = pos;
    return postJson(client, "/zmot/api/home", body);
}

ZML_RESULT zml_ax_homing(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/homing", axis);
}

ZML_RESULT zml_gear_in(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/gearin", axis);
}

ZML_RESULT zml_gear_out(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/gearout", axis);
}

ZML_RESULT zml_ax_set_position(ZML_HANDLE client, int32_t axis, double pos) {
    json body;
    body["axis"] = axis;
    body["pos"] = pos;
    return postJson(client, "/zmot/api/defpos", body);
}

ZML_RESULT zml_ax_set_speed_ratio(ZML_HANDLE client, int32_t axis, double ratio) {
    return postJson(client, "/zmot/api/speedratio", {{"axis", axis}, {"value", ratio}});
}

ZML_RESULT zml_ax_datum(ZML_HANDLE client, int32_t axis, int32_t mode) {
    return postJson(client, "/zmot/api/datum", {{"axis", axis}, {"mode", mode}});
}

ZML_RESULT zml_ax_move_velocity(ZML_HANDLE client, int32_t axis, int32_t dir) {
    json body;
    body["axis"] = axis;
    body["dir"] = dir;
    return postJson(client, "/zmot/api/vmove", body);
}

ZML_RESULT zml_ax_forward(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/forward", axis);
}

ZML_RESULT zml_ax_reverse(ZML_HANDLE client, int32_t axis) {
    return commandAxis(client, "/zmot/api/reverse", axis);
}

ZML_RESULT zml_moverel(ZML_HANDLE client, int32_t ax_num, const int32_t* axes,
                       const double* distance) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes || !distance)
        return invalidArgRet();
    return postJson(client, "/zmot/api/mmove", moveArgsToJson(ax_num, axes, distance));
}

ZML_RESULT zml_movecirc(ZML_HANDLE client, int32_t ax_num, const int32_t* axes, double end1,
                        double end2, double centre1, double centre2, int32_t direction) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num != 2 || !axes) return invalidArgRet();
    json body = axisListToJson(ax_num, axes);
    body["end1"] = end1;
    body["end2"] = end2;
    body["centre1"] = centre1;
    body["centre2"] = centre2;
    body["direction"] = direction;
    return postJson(client, "/zmot/api/movecirc", body);
}

ZML_RESULT zml_movecircabs(ZML_HANDLE client, int32_t ax_num, const int32_t* axes, double end1,
                           double end2, double centre1, double centre2, int32_t direction) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num != 2 || !axes) return invalidArgRet();
    json body = axisListToJson(ax_num, axes);
    body["end1"] = end1;
    body["end2"] = end2;
    body["centre1"] = centre1;
    body["centre2"] = centre2;
    body["direction"] = direction;
    return postJson(client, "/zmot/api/movecircabs", body);
}

ZML_RESULT zml_movecirc2(ZML_HANDLE client, int32_t ax_num, const int32_t* axes, double mid1,
                         double mid2, double end1, double end2) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num != 2 || !axes) return invalidArgRet();
    json body = axisListToJson(ax_num, axes);
    body["mid1"] = mid1;
    body["mid2"] = mid2;
    body["end1"] = end1;
    body["end2"] = end2;
    return postJson(client, "/zmot/api/movecirc2", body);
}

ZML_RESULT zml_movecirc2abs(ZML_HANDLE client, int32_t ax_num, const int32_t* axes, double mid1,
                            double mid2, double end1, double end2) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num != 2 || !axes) return invalidArgRet();
    json body = axisListToJson(ax_num, axes);
    body["mid1"] = mid1;
    body["mid2"] = mid2;
    body["end1"] = end1;
    body["end2"] = end2;
    return postJson(client, "/zmot/api/movecirc2abs", body);
}

ZML_RESULT zml_group_select(ZML_HANDLE client, int32_t idx) {
    json body;
    body["idx"] = idx;
    return postJson(client, "/zmot/api/groupselect", body);
}

ZML_RESULT zml_group_setaxes(ZML_HANDLE client, int32_t ax_num, const int32_t* axes) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes) return invalidArgRet();
    return postJson(client, "/zmot/api/groupsetaxes", axisListToJson(ax_num, axes));
}

ZML_RESULT zml_group_enable(ZML_HANDLE client, int32_t ax_num, const int32_t* axes) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes) return invalidArgRet();
    return postJson(client, "/zmot/api/groupenable", axisListToJson(ax_num, axes));
}

ZML_RESULT zml_group_disable(ZML_HANDLE client, int32_t ax_num, const int32_t* axes) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes) return invalidArgRet();
    return postJson(client, "/zmot/api/groupdisable", axisListToJson(ax_num, axes));
}

ZML_RESULT zml_group_reset(ZML_HANDLE client, int32_t ax_num, const int32_t* axes) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes) return invalidArgRet();
    return postJson(client, "/zmot/api/groupreset", axisListToJson(ax_num, axes));
}

ZML_RESULT zml_ax_wait_ticks(ZML_HANDLE client, uint32_t n_tick, int32_t axis) {
    return postJson(client, "/zmot/api/waitticks", {{"axis", axis}, {"nTick", n_tick}});
}

ZML_RESULT zml_ax_wait_seconds(ZML_HANDLE client, double seconds, int32_t axis) {
    return postJson(client, "/zmot/api/waitseconds", {{"axis", axis}, {"seconds", seconds}});
}

ZML_RESULT zml_ax_wait_idle(ZML_HANDLE client, int32_t axis, uint32_t timeout_ms) {
    httplib::Client* http_client = getClientOrNull(client);
    if (!http_client) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (timeout_ms == 0U) return invalidArgRet();
    setFileTimeout(http_client, timeout_ms + kHttpWaitTimeoutMarginMs);
    json body;
    body["axis"] = axis;
    body["timeout_ms"] = timeout_ms;
    return postJson(client, "/zmot/api/waitaxisidle", body);
}

ZML_RESULT zml_sleep_ms(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return httpClientRet(ZMC_NO_ERROR);
}

ZML_RESULT zml_get_cmd_results(ZML_HANDLE client, ZML_CmdResultInfo* out, uint32_t* inout_len) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!inout_len) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/status_latest");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        std::vector<ZML_CmdResultInfo> items;
        const int32_t parseRet = parseCmdResults(json::parse(res->body), items);
        if (parseRet != static_cast<int32_t>(ZMC_NO_ERROR)) {
            return parseRet;
        }
        const uint32_t needed = static_cast<uint32_t>(items.size());
        const uint32_t capacity = *inout_len;
        *inout_len = needed;
        if (needed > capacity || (needed > 0U && !out)) {
            return invalidArgRet();
        }
        for (uint32_t i = 0; i < needed; ++i) {
            out[i] = items[i];
        }
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_io_metadata(ZML_HANDLE client, ZML_IoMetadataEntry* out,
                               uint32_t out_capacity, uint32_t* out_count) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_count || (out_capacity > 0U && !out)) return invalidArgRet();
    *out_count = 0;
    httplib::Result res = getPath(client, "/zmot/api/io_metadata");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        const json& metadata = root.at("io_metadata");
        if (!metadata.is_array() ||
            metadata.size() > (std::numeric_limits<uint32_t>::max)()) {
            return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
        }
        const uint32_t needed = static_cast<uint32_t>(metadata.size());
        *out_count = needed;
        if (needed > out_capacity) return invalidArgRet();

        std::vector<ZML_IoMetadataEntry> entries(needed);
        for (uint32_t i = 0; i < needed; ++i) {
            const json& item = metadata[i];
            ZML_IoMetadataEntry& entry = entries[i];
            entry.mask = item.at("mask").get<uint32_t>();
            entry.io_number = item.at("io_number").get<uint16_t>();
            entry.index = item.at("index").get<uint16_t>();
            entry.bit_offset = item.at("bit_offset").get<uint16_t>();
            entry.slave_idx = item.at("slave_idx").get<uint8_t>();
            entry.subindex = item.at("subindex").get<uint8_t>();
            entry.bit_size = item.at("bit_size").get<uint8_t>();
            const std::string io_type = item.at("io_type").get<std::string>();
            entry.io_type = io_type == "di"   ? ZML_IO_TYPE_DI
                            : io_type == "do" ? ZML_IO_TYPE_DO
                            : io_type == "ai" ? ZML_IO_TYPE_AI
                            : io_type == "ao" ? ZML_IO_TYPE_AO
                                               : ZML_IO_TYPE_UNKNOWN;
            const std::string source_kind = item.at("source_kind").get<std::string>();
            entry.source_kind = source_kind == "nop"      ? ZML_IO_SOURCE_KIND_NOP
                                : source_kind == "pdo"    ? ZML_IO_SOURCE_KIND_PDO
                                : source_kind == "virtual" ? ZML_IO_SOURCE_KIND_VIRTUAL
                                : source_kind == "gpio"   ? ZML_IO_SOURCE_KIND_GPIO
                                : source_kind == "modbus" ? ZML_IO_SOURCE_KIND_MODBUS
                                                           : ZML_IO_SOURCE_KIND_UNKNOWN;
            const std::string direction = item.at("direction").get<std::string>();
            if (direction != "input" && direction != "output") {
                return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
            }
            entry.is_input = direction == "input" ? 1U : 0U;
        }
        for (uint32_t i = 0; i < needed; ++i) out[i] = entries[i];
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_flow_status(ZML_HANDLE client, ZML_FlowCtrlStatus* out_status) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_status) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/flow_status");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        out_status->period = root["period"].get<uint32_t>();
        out_status->batch_size = root["batchSize"].get<uint32_t>();
        out_status->verbose_flag = root["verboseFlag"].get<uint8_t>();
        out_status->reserved[0] = 0;
        out_status->reserved[1] = 0;
        out_status->reserved[2] = 0;
        out_status->latency = root["latency"].get<uint32_t>();
        out_status->axis_num = root["axis_num"].get<uint32_t>();
        out_status->cycle_time = root["cycleTime"].get<uint32_t>();
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_flow_config(ZML_HANDLE client, ZML_FlowCtrlConfig* out_config) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_config) return invalidArgRet();
    httplib::Result res = getPath(client, "/zmot/api/flow_config");
    if (!res || res->status < 200 || res->status >= 300) {
        return queryFailure(res);
    }
    try {
        const json root = json::parse(res->body);
        const int32_t ret = responseCode(root);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        out_config->period = root["period"].get<uint32_t>();
        out_config->batch_size = root["batchSize"].get<uint32_t>();
        out_config->verbose_flag = static_cast<uint8_t>(root["verboseFlag"].get<int>());
        return httpClientRet(ZMC_NO_ERROR);
    } catch (const std::exception&) {
        return httpClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_execute_command(ZML_HANDLE client, const char* cmd) {
    if (!getClientOrNull(client)) return httpClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!cmd) return invalidArgRet();
    return executeCommandText(client, cmd);
}
