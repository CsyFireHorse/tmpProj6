/*
 * @Author: ChenShengyong
 * @Date: 2026-05-26
 * @Description: INI parsing utility functions (transitional C version)
 *                Will be replaced by C++ INIReader in the future
 */

#include "utils/mc_ini_utils.h"
#include "common/zmc/mc_errno.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {

char* mc_ini_trim_whitespace(char* str)
{
    char* end;
    if (!str || *str == '\0') return str;

    /* Trim leading space */
    while (isspace((unsigned char)*str)) str++;

    if (*str == '\0') return str;

    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';
    return str;
}

int mc_ini_parse_buffer(const char* content, char* eni_path_out, size_t eni_path_size, uint32_t* axis_num_out)
{
    if (!content) {
        return ZMC_ERR_DEV_NULL_ARG;
    }

    /* Duplicate content for modification */
    char* content_copy = (char*)malloc(strlen(content) + 1);
    if (!content_copy) {
        return ZMC_ERR_DEV_MALLOC_FAILED;
    }
    strcpy(content_copy, content);

    /* Set defaults */
    if (axis_num_out) *axis_num_out = 1;
    if (eni_path_out && eni_path_size > 0) eni_path_out[0] = '\0';

    char* line = content_copy;
    char* next_line = NULL;
    int in_base_section = 0;
    int found_eni_path = 0;

    while (line) {
        /* Find next line */
        next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }

        char* trimmed = mc_ini_trim_whitespace(line);

        /* Skip empty lines and comments */
        if (*trimmed == '\0' || *trimmed == ';' || *trimmed == '#') {
            line = next_line;
            continue;
        }

        /* Check for section header */
        if (trimmed[0] == '[') {
            char* end_bracket = strchr(trimmed, ']');
            if (end_bracket) {
                *end_bracket = '\0';
                char* section_name = mc_ini_trim_whitespace(trimmed + 1);
                in_base_section = (strcmp(section_name, "base") == 0);
            }
            line = next_line;
            continue;
        }

        /* Parse key=value pairs only in [base] section */
        if (in_base_section) {
            char* equals = strchr(trimmed, '=');
            if (equals) {
                *equals = '\0';
                char* key = mc_ini_trim_whitespace(trimmed);
                char* value = mc_ini_trim_whitespace(equals + 1);

                /* Remove inline comments from value */
                char* comment = strchr(value, ';');
                if (comment) *comment = '\0';
                comment = strchr(value, '#');
                if (comment) *comment = '\0';
                value = mc_ini_trim_whitespace(value);

                if (strcmp(key, "eni_path") == 0) {
                    if (eni_path_out && eni_path_size > 0) {
                        strncpy(eni_path_out, value, eni_path_size - 1);
                        eni_path_out[eni_path_size - 1] = '\0';
                        found_eni_path = 1;
                    }
                } else if (strcmp(key, "axis_num") == 0) {
                    if (axis_num_out) {
                        *axis_num_out = (uint32_t)atoi(value);
                        if (*axis_num_out == 0) *axis_num_out = 1;
                    }
                }
            }
        }

        line = next_line;
    }

    free(content_copy);

    /* eni_path is required */
    return found_eni_path ? MC_COMM_S_OK : ZMC_ERR_INI_MISSING_ENI_PATH;
}

void mc_ini_get_directory(const char* filepath, char* dir_out, size_t dir_size)
{
    if (!filepath || !dir_out || dir_size == 0) return;

    const char* last_sep = NULL;
    const char* p = filepath;

    /* Find last path separator */
    while (*p) {
        if (*p == '/' || *p == '\\') {
            last_sep = p;
        }
        p++;
    }

    if (last_sep) {
        size_t len = (size_t)(last_sep - filepath);
        if (len < dir_size) {
            strncpy(dir_out, filepath, len);
            dir_out[len] = '\0';
        } else {
            strncpy(dir_out, filepath, dir_size - 1);
            dir_out[dir_size - 1] = '\0';
        }
    } else {
        /* No separator - use current directory */
        if (dir_size > 0) {
            dir_out[0] = '.';
            dir_out[1] = '\0';
        }
    }
}

void mc_ini_combine_path(const char* dir, const char* filename, char* out_path, size_t out_size)
{
    if (!out_path || out_size == 0) return;

    if (!dir || dir[0] == '\0') {
        strncpy(out_path, filename, out_size - 1);
    } else {
        int needs_sep = (dir[strlen(dir) - 1] != '/' && dir[strlen(dir) - 1] != '\\');
        snprintf(out_path, out_size, "%s%s%s", dir, needs_sep ? "/" : "", filename);
    }
    out_path[out_size - 1] = '\0';
}

} /* extern "C" */
