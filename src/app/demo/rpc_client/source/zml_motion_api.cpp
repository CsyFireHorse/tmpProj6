// Model signature: GPT-6. Preserve the approved RPC/HTTP axis and SDO contracts.
// Parameter getters added by GPT-6 Codex, 2026-10-12.
/*
 * @ArtifactStatus draft
 * @ArtifactStatusDate 2026-09-14
 * @ArtifactStatusAuthority GPT-5 Codex
 */

#include "zml_motion_api_ex.h"

#include <chrono>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <future>
#include <limits>
#include <thread>
#include <tuple>
#include <string>
#include <vector>

#include "fb/mc_parameter_enum_def.h"
#include "fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"
#include "rpc/client.h"
#include "rpc/MotionRpcWireTypes.h"
#include "rpc/rpc_error.h"

struct ZML_Context {
    rpc::client* client;
    RpcSession session;
};

namespace {

const uint64_t kRpcFileMaxBytes = 16U * 1024U * 1024U;
const int64_t kRpcSessionTimeoutMs = 5000;
const uint32_t kRpcConfigFileTimeoutMs = 5000U;
const char kRpcEniFileName[] = "rpc_eni.xml";
const char kRpcSyscfgFileName[] = "rpc_syscfg.ini";
const char kRpcEniServerPath[] = "server_files/rpc_eni.xml";
const char kRpcSyscfgServerPath[] = "server_files/rpc_syscfg.ini";

inline int32_t rpcClientRet(MC_ERROR_CODE code) { return static_cast<int32_t>(code); }

inline int32_t invalidArgRet() { return rpcClientRet(ZMC_ERR_RPC_CLIENT_INVALID_ARG); }

rpc::client* getClientOrNull(ZML_HANDLE client) {
    if (!client) {
        return nullptr;
    }
    return client->client;
}

rpc::client& getClient(ZML_HANDLE client) { return *client->client; }

template <typename... Args>
RPCLIB_MSGPACK::object_handle callRpc(ZML_HANDLE client, const char* method, Args... args) {
    return getClient(client).call(method, client->session, args...);
}

// Parameter replies publish a value only after the service reports success.
template <typename... Args>
ZML_RESULT readAxisParameter(ZML_HANDLE handle, const char* method, int32_t axis, double* value,
                             Args... args) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!value) return invalidArgRet();
    try {
        const auto result = callRpc(handle, method, axis, args...)
                                .template as<std::tuple<int32_t, double>>();
        if (std::get<0>(result) == 0) *value = std::get<1>(result);
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

// Typed IO replies update caller storage only after a successful motion-cycle read.
template <typename T>
ZML_RESULT readIo(ZML_HANDLE handle, const char* method, uint32_t io, T* value) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!value || io >= MC_IO_MAX_COUNT) return invalidArgRet();
    try {
        const auto result = callRpc(handle, method, io).template as<std::tuple<int32_t, T>>();
        if (std::get<0>(result) == 0) *value = std::get<1>(result);
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

template <typename T>
ZML_RESULT writeIo(ZML_HANDLE handle, const char* method, uint32_t io, T value) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (io >= MC_IO_MAX_COUNT) return invalidArgRet();
    try {
        return callRpc(handle, method, io, value).template as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

bool parseDecimal(const char* text, uint32_t* value) {
    if (!text || !*text || !value) return false;
    char* end = nullptr;
    errno = 0;
    const unsigned long parsed = std::strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed > 0xffffffffUL) {
        return false;
    }
    *value = static_cast<uint32_t>(parsed);
    return true;
}

bool isDecimalString(const char* text) {
    if (!text || !*text) return false;
    for (const char* current = text; *current; ++current) {
        if (*current < '0' || *current > '9') return false;
    }
    return true;
}

/* Copies text into a caller buffer whose required capacity belongs to the API contract. */
void copyText(char* output, uint32_t capacity, const char* text) {
    std::strncpy(output, text, capacity - 1U);
    output[capacity - 1U] = '\0';
}

ZML_RESULT readLocalFile(const char* file_name, std::vector<char>& data) {
    data.clear();
    if (!file_name || !*file_name) return invalidArgRet();
    std::ifstream input(file_name, std::ios::in | std::ios::binary);
    if (!input.is_open()) return rpcClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
    input.seekg(0, std::ios::end);
    const std::streampos end = input.tellg();
    if (end < 0) return rpcClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
    const uint64_t size = static_cast<uint64_t>(end);
    if (size > kRpcFileMaxBytes ||
        size > static_cast<uint64_t>((std::numeric_limits<size_t>::max)())) {
        return rpcClientRet(ZMC_ERR_RPC_FILE_TOO_LARGE);
    }
    input.seekg(0, std::ios::beg);
    data.resize(static_cast<size_t>(size));
    if (size != 0U) {
        input.read(&data[0], static_cast<std::streamsize>(size));
        if (!input || static_cast<uint64_t>(input.gcount()) != size) {
            data.clear();
            return rpcClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
        }
    }
    return rpcClientRet(ZMC_NO_ERROR);
}

ZML_RESULT createLocalFile(const char* file_name, const std::vector<char>& data) {
    if (!file_name || !*file_name) return invalidArgRet();
    std::ifstream existing(file_name, std::ios::in | std::ios::binary);
    if (existing.is_open()) return rpcClientRet(ZMC_ERR_RPC_FILE_ALREADY_EXISTS);
    std::ofstream output(file_name, std::ios::out | std::ios::binary);
    if (!output.is_open()) return rpcClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
    if (!data.empty()) {
        output.write(&data[0], static_cast<std::streamsize>(data.size()));
    }
    output.close();
    return output ? rpcClientRet(ZMC_NO_ERROR) : rpcClientRet(ZMC_ERR_RPC_FILE_IO_FAILED);
}

ZML_RESULT openRpc(ZML_HANDLE* out_handle, const char* ip, uint16_t port, uint32_t device_id) {
    if (!ip || !out_handle) {
        return invalidArgRet();
    }
    *out_handle = nullptr;
    ZML_Context* client = nullptr;
    ZML_RESULT result = rpcClientRet(ZMC_NO_ERROR);
    try {
        client = new ZML_Context();
        client->client = nullptr;
        client->session = 0;
        const char* host = std::strcmp(ip, "localhost") == 0 ? "127.0.0.1" : ip;
        client->client = new rpc::client(host, port);
        client->client->set_timeout(kRpcSessionTimeoutMs);
        const auto open_result =
            client->client->call("rpc_open", device_id).as<std::tuple<int32_t, RpcSession>>();
        result = std::get<0>(open_result);
        client->session = std::get<1>(open_result);
        client->client->clear_timeout();
        if (result == rpcClientRet(ZMC_NO_ERROR) && client->session == 0) {
            result = rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
        }
    } catch (const rpc::timeout&) {
        result = rpcClientRet(ZMC_ERR_RPC_CLIENT_CONNECT_TIMEOUT);
    } catch (const rpc::rpc_error&) {
        result = rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    } catch (const std::exception&) {
        result = rpcClientRet(ZMC_ERR_RPC_CLIENT_CONNECT_FAILED);
    }
    if (result != rpcClientRet(ZMC_NO_ERROR)) {
        if (client) delete client->client;
        delete client;
        return result;
    }
    *out_handle = client;
    return rpcClientRet(ZMC_NO_ERROR);
}

}  // namespace

ZML_RESULT zml_open_eth(ZML_HANDLE* out_handle, const char* ip) {
    return openRpc(out_handle, ip, static_cast<uint16_t>(ZMC_MOTAPP_HTTP_DEFAULT_PORT), 0U);
}

ZML_RESULT zml_open_pci(ZML_HANDLE* out_handle, uint32_t alias) {
    if (alias > 9U) return invalidArgRet();
    return openRpc(out_handle, "127.0.0.1", static_cast<uint16_t>(ZMC_MOTAPP_HTTP_DEFAULT_PORT),
                   alias);
}

ZML_RESULT zml_open(ZML_HANDLE* out_handle, const char* device) {
    if (!device) return invalidArgRet();
    uint32_t alias = 0;
    if (isDecimalString(device)) {
        if (!parseDecimal(device, &alias)) return invalidArgRet();
        return zml_open_pci(out_handle, alias);
    }

    const char* device_separator = std::strchr(device, '/');
    const char* port_separator = std::strrchr(device, ':');
    if (device_separator && (!port_separator || port_separator > device_separator)) {
        return invalidArgRet();
    }
    if (!port_separator || port_separator == device || *(port_separator + 1) == '\0') {
        if (device_separator) return invalidArgRet();
        return zml_open_eth(out_handle, device);
    }

    const char* port_end = device_separator ? device_separator : device + std::strlen(device);
    const std::string port_text(port_separator + 1,
                                static_cast<std::size_t>(port_end - (port_separator + 1)));
    uint32_t port = 0;
    if (!parseDecimal(port_text.c_str(), &port) || port == 0U || port > 65535U) {
        return invalidArgRet();
    }
    if (device_separator) {
        if (*(device_separator + 1) == '\0' || !parseDecimal(device_separator + 1, &alias) ||
            alias > 9U) {
            return invalidArgRet();
        }
    }
    const std::string host(device, static_cast<std::size_t>(port_separator - device));
    return openRpc(out_handle, host.c_str(), static_cast<uint16_t>(port), alias);
}

ZML_RESULT zml_ecat_start(ZML_HANDLE handle) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(handle, "ecat_start").as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_stop(ZML_HANDLE handle) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(handle, "ecat_stop").as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_version_info(ZML_HANDLE handle, char* version) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!version) return invalidArgRet();
    try {
        const auto result =
            callRpc(handle, "get_version_info").as<std::tuple<int32_t, RpcVersionInfoWire>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR)) {
            const RpcVersionInfoWire& wire = std::get<1>(result);
            copyText(version, ZML_VERSION_INFO_MAX, wire.version);
        }
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_about(ZML_HANDLE handle, char* about) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!about) return invalidArgRet();
    try {
        const auto result =
            callRpc(handle, "get_about").as<std::tuple<int32_t, RpcAboutInfoWire>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR)) {
            const RpcAboutInfoWire& wire = std::get<1>(result);
            copyText(about, ZML_ABOUT_INFO_MAX, wire.about);
        }
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_controller_info(ZML_HANDLE handle, char* soft_type, char* soft_version,
                                   char* controller_id) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!soft_type || !soft_version || !controller_id) return invalidArgRet();
    try {
        const auto result = callRpc(handle, "get_controller_info")
                                .as<std::tuple<int32_t, RpcControllerInfoWire>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR)) {
            const RpcControllerInfoWire& wire = std::get<1>(result);
            copyText(soft_type, ZML_CONTROLLER_INFO_MAX, wire.soft_type);
            copyText(soft_version, ZML_CONTROLLER_INFO_MAX, wire.soft_version);
            copyText(controller_id, ZML_CONTROLLER_INFO_MAX, wire.controller_id);
        }
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_bus_is_running(ZML_HANDLE handle, bool* run_flag) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!run_flag) return invalidArgRet();
    try {
        const auto result = callRpc(handle, "ecat_bus_is_running").as<std::tuple<int32_t, bool>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR)) {
            *run_flag = std::get<1>(result);
        }
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_request_master_state(ZML_HANDLE handle, uint8_t state) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(handle, "ecat_request_master_state", state).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_get_master_req_state(ZML_HANDLE handle, uint8_t* state) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!state) return invalidArgRet();
    try {
        const auto result =
            callRpc(handle, "ecat_get_master_req_state").as<std::tuple<int32_t, uint8_t>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR))
            *state = std::get<1>(result);
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_get_master_state(ZML_HANDLE handle, uint8_t* state) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!state) return invalidArgRet();
    try {
        const auto result =
            callRpc(handle, "ecat_get_master_state").as<std::tuple<int32_t, uint8_t>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR))
            *state = std::get<1>(result);
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_pi_enable_set(ZML_HANDLE handle, bool flag) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(handle, "ecat_pi_enable_set", flag).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_pi_enable_get(ZML_HANDLE handle, bool* flag) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!flag) return invalidArgRet();
    try {
        const auto result = callRpc(handle, "ecat_pi_enable_get").as<std::tuple<int32_t, bool>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR)) *flag = std::get<1>(result);
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_coe_sdo_download(ZML_HANDLE handle, uint16_t slave_id, uint16_t index,
                                     uint8_t sub_index, uint32_t data_len, const void* data) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if ((data_len != 1U && data_len != 2U && data_len != 4U) || !data) return invalidArgRet();
    try {
        uint32_t value = 0;
        std::memcpy(&value, data, data_len);
        return callRpc(handle, "ecat_coe_sdo_download", slave_id, index, sub_index, data_len,
                       value)
            .as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_coe_sdo_upload(ZML_HANDLE handle, uint16_t slave_id, uint16_t index,
                                   uint8_t sub_index, uint32_t* data_len, void* data) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!data_len || (*data_len != 1U && *data_len != 2U && *data_len != 4U) || !data) {
        return invalidArgRet();
    }
    try {
        const auto result =
            callRpc(handle, "ecat_coe_sdo_upload", slave_id, index, sub_index, *data_len)
                .as<std::tuple<int32_t, uint32_t>>();
        const int32_t ret = std::get<0>(result);
        const uint32_t value = std::get<1>(result);
        if (ret == static_cast<int32_t>(ZMC_NO_ERROR)) {
            std::memcpy(data, &value, *data_len);
        }
        return ret;
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_get_slave_count(ZML_HANDLE handle, uint16_t* slave_count) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!slave_count) return invalidArgRet();
    try {
        const auto result =
            callRpc(handle, "ecat_get_slave_count").as<std::tuple<int32_t, uint16_t>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR))
            *slave_count = std::get<1>(result);
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_get_active_slave_count(ZML_HANDLE handle, uint16_t* slave_count) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!slave_count) return invalidArgRet();
    try {
        const auto result =
            callRpc(handle, "ecat_get_active_slave_count").as<std::tuple<int32_t, uint16_t>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR))
            *slave_count = std::get<1>(result);
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_get_slave_state(ZML_HANDLE handle, uint16_t slave_id, uint8_t* state) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!state) return invalidArgRet();
    try {
        const auto result =
            callRpc(handle, "ecat_get_slave_state", slave_id).as<std::tuple<int32_t, uint8_t>>();
        if (std::get<0>(result) == static_cast<int32_t>(ZMC_NO_ERROR))
            *state = std::get<1>(result);
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ecat_clear_slave_state_error(ZML_HANDLE handle, uint16_t slave_id) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(handle, "ecat_clear_slave_state_error", slave_id).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_read_di(ZML_HANDLE handle, uint32_t io_number, bool* value) {
    return readIo(handle, "read_di", io_number, value);
}

ZML_RESULT zml_read_do(ZML_HANDLE handle, uint32_t io_number, bool* value) {
    return readIo(handle, "read_do", io_number, value);
}

ZML_RESULT zml_write_do(ZML_HANDLE handle, uint32_t io_number, bool value) {
    return writeIo(handle, "write_do", io_number, value);
}

ZML_RESULT zml_read_ai(ZML_HANDLE handle, uint32_t io_number, uint32_t* value) {
    return readIo(handle, "read_ai", io_number, value);
}

ZML_RESULT zml_read_ao(ZML_HANDLE handle, uint32_t io_number, uint32_t* value) {
    return readIo(handle, "read_ao", io_number, value);
}

ZML_RESULT zml_write_ao(ZML_HANDLE handle, uint32_t io_number, uint32_t value) {
    return writeIo(handle, "write_ao", io_number, value);
}

ZML_RESULT zml_write_virtual_di(ZML_HANDLE handle, uint32_t io_number, bool value) {
    return writeIo(handle, "write_virtual_di", io_number, value);
}

ZML_RESULT zml_write_virtual_ai(ZML_HANDLE handle, uint32_t io_number, uint32_t value) {
    return writeIo(handle, "write_virtual_ai", io_number, value);
}

ZML_RESULT zml_get_connection_state(ZML_HANDLE client) {
    rpc::client* rpcClient = getClientOrNull(client);
    if (!rpcClient) {
        return 0;
    }
    return static_cast<int>(rpcClient->get_connection_state());
}

ZML_RESULT zml_wait_until_connection_ready(ZML_HANDLE client, int32_t cnt) {
    if (!getClientOrNull(client)) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    }
    while (cnt-- > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        const int st = zml_get_connection_state(client);
        if (st == 1) {
            return rpcClientRet(ZMC_NO_ERROR);
        }
        // rpclib: disconnected means connect attempt failed (do not wait full timeout).
        if (st == 2) {
            return rpcClientRet(ZMC_ERR_RPC_CLIENT_DISCONNECTED);
        }
    }
    return rpcClientRet(ZMC_ERR_RPC_CLIENT_CONNECT_TIMEOUT);
}

ZML_RESULT zml_close(ZML_HANDLE* handle) {
    if (!handle || !*handle) {
        return rpcClientRet(ZMC_NO_ERROR);
    }
    ZML_HANDLE client = *handle;
    ZML_RESULT result = rpcClientRet(ZMC_NO_ERROR);
    try {
        getClient(client).set_timeout(kRpcSessionTimeoutMs);
        result = getClient(client).call("rpc_close", client->session).as<int32_t>();
    } catch (const std::exception&) {
        result = rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
    // Do not call wait_all_responses(): sync call() already completed and
    // iterating ongoing_calls_ races with the io thread erase handler.
    delete client->client;
    client->client = nullptr;
    delete client;
    *handle = nullptr;
    return result;
}

ZML_RESULT zml_ping(ZML_HANDLE handle, uint32_t timeout_ms) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (timeout_ms == 0U) return invalidArgRet();
    try {
        // rpclib assigns call IDs atomically and serializes request posting.
        auto response = getClient(handle).async_call("ping", handle->session, timeout_ms);
        if (response.wait_for(std::chrono::milliseconds(timeout_ms)) ==
            std::future_status::timeout) {
            return rpcClientRet(ZMC_ERR_RPC_CLIENT_CONNECT_TIMEOUT);
        }
        return response.get().as<int32_t>();
    } catch (const rpc::rpc_error&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

// Publish the echoed value only after the service reports success. GPT-6 Codex.
ZML_RESULT zml_mailbox_ping(ZML_HANDLE handle, uint32_t pingValue, uint32_t* pongValue) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!pongValue) return invalidArgRet();
    try {
        const auto result = callRpc(handle, "mailbox_ping", pingValue)
                                .as<std::tuple<int32_t, uint32_t>>();
        if (std::get<0>(result) == 0) *pongValue = std::get<1>(result);
        return std::get<0>(result);
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_download_file(ZML_HANDLE client, const char* local_file_name,
                             const char* remote_file_name, uint32_t timeout_ms) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!remote_file_name || !*remote_file_name || timeout_ms == 0U) return invalidArgRet();
    std::vector<char> data;
    const ZML_RESULT read_ret = readLocalFile(local_file_name, data);
    if (read_ret != rpcClientRet(ZMC_NO_ERROR)) return read_ret;
    try {
        getClient(client).set_timeout(static_cast<int64_t>(timeout_ms));
        return callRpc(client, "file_download", std::string(remote_file_name), data).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_upload_file(ZML_HANDLE client, const char* local_file_name,
                           const char* remote_file_name, uint32_t timeout_ms) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!local_file_name || !*local_file_name || !remote_file_name || !*remote_file_name ||
        timeout_ms == 0U) {
        return invalidArgRet();
    }
    std::ifstream existing(local_file_name, std::ios::in | std::ios::binary);
    if (existing.is_open()) return rpcClientRet(ZMC_ERR_RPC_FILE_ALREADY_EXISTS);
    try {
        getClient(client).set_timeout(static_cast<int64_t>(timeout_ms));
        const std::tuple<int32_t, std::vector<char>> result =
            callRpc(client, "file_upload", std::string(remote_file_name))
                .as<std::tuple<int32_t, std::vector<char>>>();
        if (std::get<0>(result) != static_cast<int32_t>(ZMC_NO_ERROR)) return std::get<0>(result);
        return createLocalFile(local_file_name, std::get<1>(result));
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_download_eni(ZML_HANDLE client, const char* path_name) {
    const ZML_RESULT ret =
        zml_download_file(client, path_name, kRpcEniFileName, kRpcConfigFileTimeoutMs);
    if (ret != rpcClientRet(ZMC_NO_ERROR)) return ret;
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        getClient(client).set_timeout(kRpcConfigFileTimeoutMs);
        return callRpc(client, "eni_select", std::string(kRpcEniServerPath)).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_download_syscfg(ZML_HANDLE client, const char* path_name) {
    const ZML_RESULT ret =
        zml_download_file(client, path_name, kRpcSyscfgFileName, kRpcConfigFileTimeoutMs);
    if (ret != rpcClientRet(ZMC_NO_ERROR)) return ret;
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        getClient(client).set_timeout(kRpcConfigFileTimeoutMs);
        return callRpc(client, "ini_select", std::string(kRpcSyscfgServerPath)).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_upload_eni(ZML_HANDLE client, const char* path_name) {
    return zml_upload_file(client, path_name, kRpcEniFileName, kRpcConfigFileTimeoutMs);
}

ZML_RESULT zml_upload_syscfg(ZML_HANDLE client, const char* path_name) {
    return zml_upload_file(client, path_name, kRpcSyscfgFileName, kRpcConfigFileTimeoutMs);
}

ZML_RESULT zml_ax_move_abs(ZML_HANDLE client, int32_t axis, double pos) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "movesgabs", axis, pos).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_moveabs(ZML_HANDLE client, int32_t ax_num, const int32_t* axes, const double* pos) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes || !pos) {
        return invalidArgRet();
    }
    RpcMoveWire args = {};
    args.len = ax_num;
    for (int32_t i = 0; i < ax_num; ++i) {
        args.axis[i] = axes[i];
        args.distance[i] = pos[i];
    }
    try {
        return callRpc(client, "moveabs", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_power_on(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "poweron", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_power_off(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "poweroff", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_get_state(ZML_HANDLE client, int32_t axis, ZML_AxisState* out_state) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_state) return invalidArgRet();
    try {
        const auto result =
            callRpc(client, "getaxisstate", axis).as<std::tuple<int32_t, RpcAxisStateWire>>();
        const int32_t ret = std::get<0>(result);
        if (ret == static_cast<int32_t>(ZMC_NO_ERROR)) {
            *out_state = rpc_from_wire(std::get<1>(result));
        }
        return ret;
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_get_state_ex(ZML_HANDLE client, int32_t axis, ZML_AxisStateEx* out_state) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_state) return invalidArgRet();
    try {
        const auto result =
            callRpc(client, "getaxisstate2", axis).as<std::tuple<int32_t, RpcAxisState2Wire>>();
        const int32_t ret = std::get<0>(result);
        if (ret == static_cast<int32_t>(ZMC_NO_ERROR)) {
            *out_state = rpc_from_wire(std::get<1>(result));
        }
        return ret;
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_set_enable(ZML_HANDLE client, int32_t axis, int32_t enable) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "axisenable", axis, enable != 0).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_get_enable(ZML_HANDLE client, int32_t axis, int32_t* enable) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!enable) return invalidArgRet();
    ZML_AxisState state = {};
    const ZML_RESULT ret = zml_ax_get_state(client, axis, &state);
    if (ret != rpcClientRet(ZMC_NO_ERROR)) return ret;
    /* The public axis-state byte carries the extended power-off error state. */
    *enable = state.axis_state == 0x00U || state.axis_state == 0x17U ? 0 : 1;
    return ret;
}

// Reuse the numeric reply decoder. Model signature: GPT-6 Codex.
ZML_RESULT zml_ax_read_parameter(ZML_HANDLE handle, int32_t axis, int32_t parameter_number,
                              double* value) {
    return readAxisParameter(handle, "read_parameter", axis, value, parameter_number);
}

// The service returns the motion-cycle execution ACK. GPT-6 Codex.
ZML_RESULT zml_ax_write_parameter(ZML_HANDLE handle, int32_t axis, int32_t parameter_number,
                               double value) {
    if (!getClientOrNull(handle)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(handle, "write_parameter", axis, parameter_number, value).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

/* Typed reads map one fixed ParameterNo onto the existing read-parameter cycle;
   the wire parameter number is the enum cast to its int32 value.
   Model signature: GLM-5.3 (opencode), 2026-10-23. */
ZML_RESULT zml_ax_read_state(ZML_HANDLE handle, int32_t axis, int32_t* state) {
    if (!state) return invalidArgRet();
    double value = 0;
    const ZML_RESULT ret = readAxisParameter(handle, "read_parameter", axis, &value,
                                             static_cast<int32_t>(PARA_nAxisState));
    if (ret == rpcClientRet(ZMC_NO_ERROR)) *state = static_cast<int32_t>(value);
    return ret;
}

ZML_RESULT zml_ax_read_set_position(ZML_HANDLE handle, int32_t axis, double* position) {
    return readAxisParameter(handle, "read_parameter", axis, position, static_cast<int32_t>(PARA_fSetPosition));
}

ZML_RESULT zml_ax_read_act_position(ZML_HANDLE handle, int32_t axis, double* position) {
    return readAxisParameter(handle, "read_parameter", axis, position, static_cast<int32_t>(PARA_fActPosition));
}

ZML_RESULT zml_ax_read_sw_limit_negative(ZML_HANDLE handle, int32_t axis, double* limit) {
    return readAxisParameter(handle, "read_parameter", axis, limit, static_cast<int32_t>(PARA_fSWLimitNegative));
}

ZML_RESULT zml_ax_read_sw_limit_enable(ZML_HANDLE handle, int32_t axis, int32_t* enable) {
    if (!enable) return invalidArgRet();
    double value = 0;
    const ZML_RESULT ret = readAxisParameter(handle, "read_parameter", axis, &value,
                                             static_cast<int32_t>(PARA_bSWLimitEnable));
    if (ret == rpcClientRet(ZMC_NO_ERROR)) *enable = static_cast<int32_t>(value);
    return ret;
}

ZML_RESULT zml_ax_read_sw_limit_positive(ZML_HANDLE handle, int32_t axis, double* limit) {
    return readAxisParameter(handle, "read_parameter", axis, limit, static_cast<int32_t>(PARA_fSWLimitPositive));
}

ZML_RESULT zml_ax_read_units_num(ZML_HANDLE handle, int32_t axis, int32_t* num) {
    if (!num) return invalidArgRet();
    double value = 0;
    const ZML_RESULT ret =
        readAxisParameter(handle, "read_parameter", axis, &value, static_cast<int32_t>(PARA_iRatioTechUnitsNum));
    if (ret == rpcClientRet(ZMC_NO_ERROR)) *num = static_cast<int32_t>(value);
    return ret;
}

ZML_RESULT zml_ax_read_units_den(ZML_HANDLE handle, int32_t axis, int32_t* den) {
    if (!den) return invalidArgRet();
    double value = 0;
    const ZML_RESULT ret =
        readAxisParameter(handle, "read_parameter", axis, &value, static_cast<int32_t>(PARA_dwRatioTechUnitsDenom));
    if (ret == rpcClientRet(ZMC_NO_ERROR)) *den = static_cast<int32_t>(value);
    return ret;
}

ZML_RESULT zml_ax_get_units(ZML_HANDLE client, int32_t axis, double* units) {
    return readAxisParameter(client, "getunits", axis, units);
}
ZML_RESULT zml_ax_get_speed(ZML_HANDLE client, int32_t axis, double* speed) {
    return readAxisParameter(client, "getspeed", axis, speed);
}
ZML_RESULT zml_ax_get_accel(ZML_HANDLE client, int32_t axis, double* accel) {
    return readAxisParameter(client, "getaccel", axis, accel);
}
ZML_RESULT zml_ax_get_decel(ZML_HANDLE client, int32_t axis, double* decel) {
    return readAxisParameter(client, "getdecel", axis, decel);
}
ZML_RESULT zml_ax_get_jerk(ZML_HANDLE client, int32_t axis, double* jerk) {
    return readAxisParameter(client, "getjerk", axis, jerk);
}

ZML_RESULT zml_ax_set_units(ZML_HANDLE client, int32_t axis, int32_t num, int32_t den) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
         return callRpc(client, "setunits", axis, num, den).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_set_velocity(ZML_HANDLE client, int32_t axis, double speed) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setspeed", axis, speed).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_set_accel(ZML_HANDLE client, int32_t axis, double accel) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setaccel", axis, accel).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_set_decel(ZML_HANDLE client, int32_t axis, double decel) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setdecel", axis, decel).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_set_jerk(ZML_HANDLE client, int32_t axis, double jerk) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setjerk", axis, jerk).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_set_fastdecel(ZML_HANDLE client, int32_t axis, double fast_decel) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setfastdecel", axis, fast_decel).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_set_direction(ZML_HANDLE client, int32_t axis, int32_t dir) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setdirection", axis, dir).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_set_buffermode(ZML_HANDLE client, int32_t axis, int32_t buffer_mode) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setbuffermode", axis, buffer_mode).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_set_flow_config(ZML_HANDLE client, const ZML_FlowCtrlConfig* cfg) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!cfg) return invalidArgRet();
    try {
        return callRpc(client, "setflowconfig", rpc_to_wire(*cfg)).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_stop(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "stop", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_halt(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "halt", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_reset(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "reset", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_set_master_axis(ZML_HANDLE client, int32_t axis, int32_t master_axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setmasteraxis", axis, master_axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_move_rel(ZML_HANDLE client, int32_t axis, double pos) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "movesg", axis, pos).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_moveadd(ZML_HANDLE client, int32_t axis, double pos) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "moveadd", axis, pos).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_home_start(ZML_HANDLE client, int32_t axis, const ZML_HomeSdoConfig* config) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!config) return invalidArgRet();
    RpcHomeSdoConfigWire configWire = {};
    rpc_from_wire(configWire) = *config;
    try {
        return callRpc(client, "home_start", axis, configWire).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_home_get_status(ZML_HANDLE client, int32_t axis, int32_t* status,
                               int32_t* error_id) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!status || !error_id) return invalidArgRet();
    try {
        const auto result =
            callRpc(client, "home_get_status", axis).as<std::tuple<int32_t, int32_t, int32_t>>();
        const int32_t ret = std::get<0>(result);
        if (ret == static_cast<int32_t>(ZMC_NO_ERROR)) {
            *status = std::get<1>(result);
            *error_id = std::get<2>(result);
        }
        return ret;
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_home_stop(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "home_stop", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_home(ZML_HANDLE client, int32_t axis, double pos) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "home", axis, pos).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_homing(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "homing", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_gear_in(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "gearin", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_gear_out(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "gearout", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_set_position(ZML_HANDLE client, int32_t axis, double pos) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setposition", axis, pos).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_set_speed_ratio(ZML_HANDLE client, int32_t axis, double ratio) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "setspeedratio", axis, ratio).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_datum(ZML_HANDLE client, int32_t axis, int32_t mode) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "datum", axis, mode).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_move_velocity(ZML_HANDLE client, int32_t axis, int32_t dir) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "movevelocity", axis, dir).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_forward(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "forward", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_ax_reverse(ZML_HANDLE client, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "reverse", axis).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_moverel(ZML_HANDLE client, int32_t ax_num, const int32_t* axes,
                       const double* distance) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes || !distance) {
        return invalidArgRet();
    }
    RpcMoveWire args = {};
    args.len = ax_num;
    for (int32_t i = 0; i < ax_num; ++i) {
        args.axis[i] = axes[i];
        args.distance[i] = distance[i];
    }
    try {
        return callRpc(client, "move", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_movecirc(ZML_HANDLE client, int32_t ax_num, const int32_t* axes, double end1,
                        double end2, double centre1, double centre2, int32_t direction) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num != 2 || !axes) return invalidArgRet();
    RpcMoveCircWire args = {};
    args.len = ax_num;
    args.axis[0] = axes[0];
    args.axis[1] = axes[1];
    args.end1 = end1;
    args.end2 = end2;
    args.centre1 = centre1;
    args.centre2 = centre2;
    args.direction = direction;
    try {
        return callRpc(client, "movecirc", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_movecircabs(ZML_HANDLE client, int32_t ax_num, const int32_t* axes, double end1,
                           double end2, double centre1, double centre2, int32_t direction) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num != 2 || !axes) return invalidArgRet();
    RpcMoveCircWire args = {};
    args.len = ax_num;
    args.axis[0] = axes[0];
    args.axis[1] = axes[1];
    args.end1 = end1;
    args.end2 = end2;
    args.centre1 = centre1;
    args.centre2 = centre2;
    args.direction = direction;
    try {
        return callRpc(client, "movecircabs", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_movecirc2(ZML_HANDLE client, int32_t ax_num, const int32_t* axes, double mid1,
                         double mid2, double end1, double end2) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num != 2 || !axes) return invalidArgRet();
    RpcMoveCirc2Wire args = {};
    args.len = ax_num;
    args.axis[0] = axes[0];
    args.axis[1] = axes[1];
    args.mid1 = mid1;
    args.mid2 = mid2;
    args.end1 = end1;
    args.end2 = end2;
    try {
        return callRpc(client, "movecirc2", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_movecirc2abs(ZML_HANDLE client, int32_t ax_num, const int32_t* axes, double mid1,
                            double mid2, double end1, double end2) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num != 2 || !axes) return invalidArgRet();
    RpcMoveCirc2Wire args = {};
    args.len = ax_num;
    args.axis[0] = axes[0];
    args.axis[1] = axes[1];
    args.mid1 = mid1;
    args.mid2 = mid2;
    args.end1 = end1;
    args.end2 = end2;
    try {
        return callRpc(client, "movecirc2abs", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_group_select(ZML_HANDLE client, int32_t idx) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "groupselect", idx).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_group_setaxes(ZML_HANDLE client, int32_t ax_num, const int32_t* axes) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes) return invalidArgRet();
    RpcAxisListWire args = {};
    args.len = ax_num;
    for (int32_t i = 0; i < ax_num; ++i) args.axis[i] = axes[i];
    try {
        return callRpc(client, "groupsetaxes", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_group_enable(ZML_HANDLE client, int32_t ax_num, const int32_t* axes) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes) return invalidArgRet();
    RpcAxisListWire args = {};
    args.len = ax_num;
    for (int32_t i = 0; i < ax_num; ++i) args.axis[i] = axes[i];
    try {
        return callRpc(client, "groupenable", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_group_disable(ZML_HANDLE client, int32_t ax_num, const int32_t* axes) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes) return invalidArgRet();
    RpcAxisListWire args = {};
    args.len = ax_num;
    for (int32_t i = 0; i < ax_num; ++i) args.axis[i] = axes[i];
    try {
        return callRpc(client, "groupdisable", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
ZML_RESULT zml_group_reset(ZML_HANDLE client, int32_t ax_num, const int32_t* axes) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (ax_num <= 0 || ax_num > ZMC_RPC_MAX_AXES || !axes) return invalidArgRet();
    RpcAxisListWire args = {};
    args.len = ax_num;
    for (int32_t i = 0; i < ax_num; ++i) args.axis[i] = axes[i];
    try {
        return callRpc(client, "groupreset", args).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_wait_ticks(ZML_HANDLE client, uint32_t n_tick, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "waitticks", n_tick, axis, 0U, 2U).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_wait_seconds(ZML_HANDLE client, double seconds, int32_t axis) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    try {
        return callRpc(client, "waitseconds", seconds, axis, 0U, 2U).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_ax_wait_idle(ZML_HANDLE client, int32_t axis, uint32_t timeout_ms) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (timeout_ms == 0U) return invalidArgRet();
    try {
        // One blocking server-side wait shares ping's async-call timeout ownership.
        // GLM-5.3 (opencode), 2026-10-23.
        auto response = getClient(client).async_call("waitaxisidle", client->session, axis,
                                                     timeout_ms);
        if (response.wait_for(std::chrono::milliseconds(timeout_ms)) ==
            std::future_status::timeout) {
            return rpcClientRet(ZMC_ERR_MOTCMD_WAIT_TIMEOUT);
        }
        return response.get().as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_sleep_ms(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return rpcClientRet(ZMC_NO_ERROR);
}

ZML_RESULT zml_get_cmd_results(ZML_HANDLE client, ZML_CmdResultInfo* out, uint32_t* inout_len) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!inout_len) return invalidArgRet();
    try {
        const auto result = callRpc(client, "getcmdresults")
                                .as<std::tuple<int32_t, std::vector<RpcCmdResultInfoWire>>>();
        const int32_t ret = std::get<0>(result);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) {
            return ret;
        }
        const std::vector<RpcCmdResultInfoWire>& wireVec = std::get<1>(result);
        const uint32_t needed = static_cast<uint32_t>(wireVec.size());
        const uint32_t capacity = *inout_len;
        *inout_len = needed;
        if (needed > capacity || (needed > 0U && !out)) {
            return invalidArgRet();
        }
        for (uint32_t i = 0; i < needed; ++i) {
            out[i] = rpc_from_wire(wireVec[i]);
        }
        return ret;
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_io_metadata(ZML_HANDLE client, ZML_IoMetadataEntry* out,
                               uint32_t out_capacity, uint32_t* out_count) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_count || (out_capacity > 0U && !out)) return invalidArgRet();
    *out_count = 0;
    try {
        const auto result = callRpc(client, "getiometadata")
                                .as<std::tuple<int32_t, std::vector<RpcIoMetadataWire>>>();
        const int32_t ret = std::get<0>(result);
        if (ret != static_cast<int32_t>(ZMC_NO_ERROR)) return ret;
        const std::vector<RpcIoMetadataWire>& entries = std::get<1>(result);
        if (entries.size() > (std::numeric_limits<uint32_t>::max)()) return invalidArgRet();
        const uint32_t needed = static_cast<uint32_t>(entries.size());
        *out_count = needed;
        if (needed > out_capacity) return invalidArgRet();
        for (uint32_t i = 0; i < needed; ++i) {
            out[i].mask = entries[i].mask;
            out[i].io_number = entries[i].io_number;
            out[i].index = entries[i].index;
            out[i].bit_offset = entries[i].bit_offset;
            out[i].io_type = entries[i].io_type;
            out[i].source_kind = entries[i].source_kind;
            out[i].slave_idx = entries[i].slave_idx;
            out[i].subindex = entries[i].subindex;
            out[i].bit_size = entries[i].bit_size;
            out[i].is_input = entries[i].is_input;
        }
        return ret;
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_flow_status(ZML_HANDLE client, ZML_FlowCtrlStatus* out_status) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_status) return invalidArgRet();
    try {
        const auto result =
            callRpc(client, "getflowstatus").as<std::tuple<int32_t, RpcFlowCtrlStatusWire>>();
        const int32_t ret = std::get<0>(result);
        if (ret == static_cast<int32_t>(ZMC_NO_ERROR)) {
            *out_status = rpc_from_wire(std::get<1>(result));
        }
        return ret;
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_get_flow_config(ZML_HANDLE client, ZML_FlowCtrlConfig* out_config) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!out_config) return invalidArgRet();
    try {
        const auto result =
            callRpc(client, "getflowconfig").as<std::tuple<int32_t, RpcFlowCtrlConfigWire>>();
        const int32_t ret = std::get<0>(result);
        if (ret == static_cast<int32_t>(ZMC_NO_ERROR)) {
            *out_config = rpc_from_wire(std::get<1>(result));
        }
        return ret;
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}

ZML_RESULT zml_execute_command(ZML_HANDLE client, const char* cmd) {
    if (!getClientOrNull(client)) return rpcClientRet(ZMC_ERR_RPC_CLIENT_NOT_OPEN);
    if (!cmd) return invalidArgRet();
    try {
        return callRpc(client, "executecommand", cmd).as<int32_t>();
    } catch (const std::exception&) {
        return rpcClientRet(ZMC_ERR_RPC_CLIENT_CALL_FAILED);
    }
}
