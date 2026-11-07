#include "eni_json_slice.h"

#include <limits.h>
#include <stddef.h>

#include "fbt_errorcode_def.h"

static const char JSON_DATA_START[] = "<!-- @@##ENI_INFO##@@";
static const char JSON_DATA_END[] = " -->";

/* Advance a fixed-pattern matcher without reading ahead in the input stream. */
static size_t advance_match(char value, const char* pattern, size_t matched) {
    if (value == pattern[matched]) {
        return matched + 1u;
    }
    return value == pattern[0] ? 1u : 0u;
}

int extract_eni_json_slice(const char* eni_content, int* ofs, int* length) {
    size_t start_matched = 0u;
    size_t end_matched = 0u;
    size_t json_ofs = 0u;
    size_t pos = 0u;
    int start_found = 0;

    if (ofs != NULL) {
        *ofs = 0;
    }
    if (length != NULL) {
        *length = 0;
    }
    if (eni_content == NULL || ofs == NULL || length == NULL) {
        return ZMC_ERR_DEV_NULL_ARG;
    }

    while (eni_content[pos] != '\0') {
        if (!start_found) {
            start_matched = advance_match(eni_content[pos], JSON_DATA_START, start_matched);
            if (JSON_DATA_START[start_matched] == '\0') {
                json_ofs = pos + 1u;
                start_found = 1;
            }
        } else {
            end_matched = advance_match(eni_content[pos], JSON_DATA_END, end_matched);
            if (JSON_DATA_END[end_matched] == '\0') {
                const size_t end_ofs = pos + 1u - (sizeof(JSON_DATA_END) - 1u);
                const size_t json_length = end_ofs - json_ofs;
                if (json_ofs > INT_MAX || json_length > INT_MAX) {
                    return ZMC_ERR_ENI_JSON_INVALID;
                }
                *ofs = (int)json_ofs;
                *length = (int)json_length;
                return 0;
            }
        }
        ++pos;
    }

    return ZMC_ERR_ENI_JSON_NOT_FOUND;
}
