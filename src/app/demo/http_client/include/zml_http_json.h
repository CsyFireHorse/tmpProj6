#ifndef ZML_HTTP_JSON_H
#define ZML_HTTP_JSON_H

#include "zml_motion_api.h"

/* HTTP JSON client and JSON Pointer access. GPT-6 Codex, 2026-11-05. */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ZML_JsonDocument* ZML_JSON;

typedef enum ZML_JSON_TYPE {
    ZML_JSON_NULL,
    ZML_JSON_BOOL,
    ZML_JSON_NUMBER,
    ZML_JSON_STRING,
    ZML_JSON_OBJECT,
    ZML_JSON_ARRAY
} ZML_JSON_TYPE;

/* The handle must be opened by the HTTP client. The result reports transport status;
   HTTP status and body are returned separately.
   GET requires a null body; POST requires a JSON text body. Release the body with
   zml_free_string. An HTTP error response is still returned to the caller. */
ZML_API ZML_RESULT zml_http_json_request(ZML_HANDLE handle, const char* method, const char* path,
                                         const char* request_body, int32_t* http_status,
                                         char** response_body);

/* Parse once, then read using JSON Pointer paths; an empty path selects the root. */
ZML_API ZML_RESULT zml_json_parse(const char* text, ZML_JSON* out_json);
/* Strings are decoded UTF-8; other values are JSON text. Length excludes the final NUL.
   Release each returned value with zml_free_string. */
ZML_API ZML_RESULT zml_json_get_value(ZML_JSON json, const char* path, ZML_JSON_TYPE* out_type,
                                      char** out_value, uint32_t* out_length);
ZML_API ZML_RESULT zml_json_array_size(ZML_JSON json, const char* path, uint32_t* out_count);
ZML_API void zml_json_free(ZML_JSON json);
ZML_API void zml_free_string(char* value);

#ifdef __cplusplus
}
#endif

#endif /* ZML_HTTP_JSON_H */
