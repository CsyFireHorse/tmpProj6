#include "zml_http_json.h"
#include "ZmlHttpContext.hpp"

#include <cstdlib>
#include <cstring>
#include <exception>

#include "common/thirdlib/yhirose/httplib.h"
#include "common/zmc/fbt_errorcode_def.h"

// GPT-6 Codex, 2026-11-05. Reuse the active HTTP client and its session header.
ZML_RESULT zml_http_json_request(ZML_HANDLE handle, const char* method, const char* path,
                                 const char* request_body, int32_t* http_status,
                                 char** response_body) {
    if (http_status) *http_status = 0;
    if (response_body) *response_body = nullptr;
    if (!handle || !handle->client) return ZMC_ERR_RPC_CLIENT_NOT_OPEN;
    if (!method || !path || path[0] != '/' || !http_status || !response_body)
        return ZMC_ERR_RPC_CLIENT_INVALID_ARG;
    const bool get = std::strcmp(method, "GET") == 0;
    const bool post = std::strcmp(method, "POST") == 0;
    if ((!get && !post) || (get && request_body) || (post && !request_body))
        return ZMC_ERR_RPC_CLIENT_INVALID_ARG;

    try {
        httplib::Result result = get ? handle->client->Get(path)
                                     : handle->client->Post(path, request_body, "application/json");
        handle->connectionState = result ? 1 : 2;
        if (!result) return ZMC_ERR_RPC_CLIENT_CALL_FAILED;
        *http_status = result->status;
        char* body = static_cast<char*>(std::malloc(result->body.size() + 1));
        if (!body) return ZMC_ERR_RPC_CLIENT_CALL_FAILED;
        std::memcpy(body, result->body.data(), result->body.size());
        body[result->body.size()] = '\0';
        *response_body = body;
        return ZMC_NO_ERROR;
    } catch (const std::exception&) {
        return ZMC_ERR_RPC_CLIENT_CALL_FAILED;
    }
}
