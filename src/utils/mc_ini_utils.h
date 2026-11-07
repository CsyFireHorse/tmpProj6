/*
 * @Author: ChenShengyong
 * @Date: 2026-05-26
 * @Description: INI parsing utility functions (transitional C version)
 *                Will be replaced by C++ INIReader in the future
 */

#ifndef __MC_INI_UTILS_H
#define __MC_INI_UTILS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum line length for INI file parsing */
#define MC_INI_MAX_LINE_LENGTH 512

/**
 * @brief Trim leading and trailing whitespace from string (in-place)
 * @param str Input string to trim
 * @return Pointer to trimmed string
 */
char* mc_ini_trim_whitespace(char* str);

/**
 * @brief Parse INI content from memory buffer and extract [base] section values
 *
 * Extracts:
 *   - eni_path: ENI file path (required)
 *   - axis_num: Number of axes (optional, defaults to 1)
 *
 * @param content INI content string (null-terminated)
 * @param eni_path_out Output buffer for eni_path
 * @param eni_path_size Size of eni_path_out buffer
 * @param axis_num_out Output for axis_num
 * @return MC_COMM_S_OK on success, or ZMC_ERR_INI_* on error
 */
int mc_ini_parse_buffer(const char* content, char* eni_path_out, size_t eni_path_size, uint32_t* axis_num_out);

/**
 * @brief Extract directory from file path
 * @param filepath Full file path
 * @param dir_out Output buffer for directory
 * @param dir_size Size of dir_out buffer
 */
void mc_ini_get_directory(const char* filepath, char* dir_out, size_t dir_size);

/**
 * @brief Combine directory and filename into full path
 * @param dir Directory path (can be empty)
 * @param filename Filename
 * @param out_path Output buffer for full path
 * @param out_size Size of out_path buffer
 */
void mc_ini_combine_path(const char* dir, const char* filename, char* out_path, size_t out_size);

#ifdef __cplusplus
}
#endif

#endif /* __MC_INI_UTILS_H */
