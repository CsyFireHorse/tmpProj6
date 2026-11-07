#ifndef _ZMC_HTTP_JSON_PARSE_DETAIL_HPP_
#define _ZMC_HTTP_JSON_PARSE_DETAIL_HPP_

#include <stdint.h>

#include <exception>
#include <string>

#include "common/thirdlib/nlohmann/json.hpp"
#include "common/thirdlib/yhirose/httplib.h"
#include "fbt_errorcode_def.h"

namespace zmc {
namespace http_json_parse_detail {

// AI modification signature: Cursor Grok 4.5.
// Build the common motion API response envelope.
inline nlohmann::json makeJsonEnvelope(int code, const std::string& message,
                                       const nlohmann::json& fields = nlohmann::json::object()) {
    nlohmann::json body = {{"code", code}, {"message", message}};
    if (fields.is_object()) {
        for (nlohmann::json::const_iterator it = fields.begin(); it != fields.end(); ++it) {
            body[it.key()] = it.value();
        }
    }
    return body;
}

// Write one JSON response body.
inline void setJsonResponse(httplib::Response& res, const nlohmann::json& body) {
    res.set_content(body.dump(2), "application/json");
}

// Write the common successful response envelope.
inline void replyJsonOk(httplib::Response& res,
                        const nlohmann::json& fields = nlohmann::json::object()) {
    res.status = 200;
    setJsonResponse(res, makeJsonEnvelope(0, "ok", fields));
}

// Write one business-error response envelope.
inline void replyJsonError(httplib::Response& res, int code, const std::string& message) {
    res.status = 200;
    setJsonResponse(res, makeJsonEnvelope(code, message));
}

// Write the standard invalid-argument response.
inline void replyJsonInvalidArg(httplib::Response& res, const std::string& message) {
    replyJsonError(res, static_cast<int>(ZMC_ERR_MOTCMD_INVALID_ARG), message);
}

// Write a simple API result: code 0 -> ok, otherwise fail with the given code.
inline void replyJsonResult(httplib::Response& res, int code) {
    if (code == 0) {
        replyJsonOk(res);
    } else {
        replyJsonError(res, code, "fail");
    }
}

// Map typed motion command returns to the existing HTTP response contract.
inline bool applyMotionApiRet(httplib::Response& res, int ret) {
    if (ret == static_cast<int>(ZMC_NO_ERROR)) {
        replyJsonOk(res);
        return true;
    }
    std::string message = "fail";
    if (ret == static_cast<int>(ZMC_ERR_MOTCMD_SCRIPT_BUSY_DISCARD)) {
        res.status = 503;
        message = "script busy";
    } else if (ret == static_cast<int>(ZMC_ERR_MOTCMD_ABORT_PUSH_FAILED)) {
        res.status = 500;
        message = "abort push failed";
    } else {
        res.status = 500;
    }
    setJsonResponse(res, makeJsonEnvelope(ret, message));
    return false;
}

// Parse one JSON request body and keep the detailed parser error for the caller.
inline bool parse_json_body(const httplib::Request& req, nlohmann::json& body,
                            std::string& error_message) {
    try {
        body = nlohmann::json::parse(req.body);
        return true;
    } catch (const std::exception& e) {
        error_message = std::string("invalid json body: ") + e.what();
        return false;
    }
}

// Parse one JSON body and reply with the standard invalid-argument envelope on failure.
inline bool parseJsonBody(const httplib::Request& req, httplib::Response& res,
                          nlohmann::json& body) {
    std::string error_message;
    if (parse_json_body(req, body, error_message)) {
        return true;
    }
    replyJsonInvalidArg(res, error_message);
    return false;
}

// Read one non-negative integer field after the caller has checked its presence.
inline bool read_uint64(const nlohmann::json& field, uint64_t maximum, bool allow_unsigned,
                        uint64_t& value) {
    if (allow_unsigned) {
        if (!field.is_number_integer() && !field.is_number_unsigned()) {
            return false;
        }
    } else if (!field.is_number_integer()) {
        return false;
    }

    if (field.is_number_unsigned()) {
        value = field.get<uint64_t>();
    } else {
        const int64_t signed_value = field.get<int64_t>();
        if (signed_value < 0) {
            return false;
        }
        value = static_cast<uint64_t>(signed_value);
    }
    return value <= maximum;
}

// Read one boolean field after the caller has checked its presence.
inline bool read_bool(const nlohmann::json& field, bool& value) {
    if (!field.is_boolean()) {
        return false;
    }
    value = field.get<bool>();
    return true;
}

}  // namespace http_json_parse_detail
}  // namespace zmc

#endif
