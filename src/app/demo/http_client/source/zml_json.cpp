#include "zml_http_json.h"

#include <cstdlib>
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <string>
#include <utility>

#include "common/thirdlib/nlohmann/json.hpp"
#include "common/zmc/fbt_errorcode_def.h"

// GPT-6 Codex, 2026-11-05. One parsed document supports repeated JSON Pointer reads.
struct ZML_JsonDocument {
    nlohmann::json root;
};

namespace {

using Json = nlohmann::json;

ZML_JSON_TYPE valueType(const Json& value) {
    if (value.is_null()) return ZML_JSON_NULL;
    if (value.is_boolean()) return ZML_JSON_BOOL;
    if (value.is_number()) return ZML_JSON_NUMBER;
    if (value.is_string()) return ZML_JSON_STRING;
    if (value.is_object()) return ZML_JSON_OBJECT;
    return ZML_JSON_ARRAY;
}

}  // namespace

ZML_RESULT zml_json_parse(const char* text, ZML_JSON* out_json) {
    if (out_json) *out_json = nullptr;
    if (!text || !out_json) return ZMC_ERR_RPC_CLIENT_INVALID_ARG;
    try {
        Json root = Json::parse(text, nullptr, false);
        if (root.is_discarded()) return ZMC_ERR_RPC_JSON_INVALID;
        *out_json = new ZML_JsonDocument{std::move(root)};
        return ZMC_NO_ERROR;
    } catch (const std::exception&) {
        return ZMC_ERR_RPC_CLIENT_CALL_FAILED;
    }
}

ZML_RESULT zml_json_get_value(ZML_JSON json, const char* path, ZML_JSON_TYPE* out_type,
                               char** out_value, uint32_t* out_length) {
    if (out_value) *out_value = nullptr;
    if (out_length) *out_length = 0;
    if (!json || !path || !out_type || !out_value || !out_length)
        return ZMC_ERR_RPC_CLIENT_INVALID_ARG;
    try {
        const Json::json_pointer pointer(path);
        if (!json->root.contains(pointer)) return ZMC_ERR_RPC_JSON_MISSING_FIELD;
        const Json& value = json->root.at(pointer);
        const std::string text = value.is_string() ? value.get<std::string>() : value.dump();
        if (text.size() > (std::numeric_limits<uint32_t>::max)())
            return ZMC_ERR_RPC_CLIENT_CALL_FAILED;
        char* copy = static_cast<char*>(std::malloc(text.size() + 1));
        if (!copy) return ZMC_ERR_RPC_CLIENT_CALL_FAILED;
        std::memcpy(copy, text.data(), text.size());
        copy[text.size()] = '\0';
        *out_type = valueType(value);
        *out_length = static_cast<uint32_t>(text.size());
        *out_value = copy;
        return ZMC_NO_ERROR;
    } catch (const Json::out_of_range&) {
        return ZMC_ERR_RPC_JSON_MISSING_FIELD;
    } catch (const Json::exception&) {
        return ZMC_ERR_RPC_CLIENT_INVALID_ARG;
    } catch (const std::exception&) {
        return ZMC_ERR_RPC_CLIENT_CALL_FAILED;
    }
}

ZML_RESULT zml_json_array_size(ZML_JSON json, const char* path, uint32_t* out_count) {
    if (out_count) *out_count = 0;
    if (!json || !path || !out_count) return ZMC_ERR_RPC_CLIENT_INVALID_ARG;
    try {
        const Json::json_pointer pointer(path);
        if (!json->root.contains(pointer)) return ZMC_ERR_RPC_JSON_MISSING_FIELD;
        const Json& value = json->root.at(pointer);
        if (!value.is_array()) return ZMC_ERR_RPC_CLIENT_INVALID_ARG;
        if (value.size() > (std::numeric_limits<uint32_t>::max)())
            return ZMC_ERR_RPC_CLIENT_CALL_FAILED;
        *out_count = static_cast<uint32_t>(value.size());
        return ZMC_NO_ERROR;
    } catch (const Json::out_of_range&) {
        return ZMC_ERR_RPC_JSON_MISSING_FIELD;
    } catch (const Json::exception&) {
        return ZMC_ERR_RPC_CLIENT_INVALID_ARG;
    } catch (const std::exception&) {
        return ZMC_ERR_RPC_CLIENT_CALL_FAILED;
    }
}

void zml_json_free(ZML_JSON json) { delete json; }
void zml_free_string(char* value) { std::free(value); }
