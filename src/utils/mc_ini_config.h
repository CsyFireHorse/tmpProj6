/*
 * @Author: ChenShengyong
 * @Date: 2026-05-26
 * @Description: INI Configuration structure for [base] section
 */

#ifndef __MC_INI_CONFIG_H
#define __MC_INI_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "common/ecat/ecat_pdo_key.h"
#include "common/zmc/zmc_predef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum path length for ENI file path */
#define MC_INI_MAX_PATH_LENGTH 256

/* Maximum axis number supported */
#define MC_INI_MAX_AXIS_NUM MC_MOTION_MAX_AXES

/* Default values */
#define MC_INI_DEFAULT_AXIS_NUM  1

/**
 * @brief Base configuration structure corresponding to [base] section in INI file
 *
 * INI format:
 * [base]
 * axis_num = 1          ; Number of logical axes (1-64)
 * eni_path = xxx.xml    ; ENI file path (relative or absolute)
 */
typedef struct BaseConfig {
    uint32_t axis_num;                    /* Number of axes, default: 1 */
    char     eni_path[MC_INI_MAX_PATH_LENGTH]; /* ENI file path */
    uint8_t  init_opmode;                 /* Initial 0x6060 watch value for non-PDO opmode axes */
    bool     has_custom_config;           /* Flag: true if user provided custom config */
} BaseConfig;

/**
 * @brief Initialize BaseConfig with default values
 */
static inline void BaseConfig_InitDefault(BaseConfig* cfg) {
    if (cfg) {
        cfg->axis_num = MC_INI_DEFAULT_AXIS_NUM;
        cfg->eni_path[0] = '\0';
        cfg->init_opmode = OP_MODE_CSP;
        cfg->has_custom_config = false;
    }
}

/**
 * @brief Initialize BaseConfig from parsed values
 */
static inline void BaseConfig_Init(BaseConfig* cfg,
                                   uint32_t axis_num,
                                   const char* eni_path,
                                   uint8_t init_opmode,
                                   bool custom) {
    if (cfg) {
        cfg->axis_num = (axis_num > 0 && axis_num <= MC_INI_MAX_AXIS_NUM) ? axis_num : MC_INI_DEFAULT_AXIS_NUM;
        cfg->eni_path[0] = '\0';
        if (eni_path) {
            strncpy(cfg->eni_path, eni_path, MC_INI_MAX_PATH_LENGTH - 1);
            cfg->eni_path[MC_INI_MAX_PATH_LENGTH - 1] = '\0';
        }
        cfg->init_opmode = init_opmode;
        cfg->has_custom_config = custom;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* __MC_INI_CONFIG_H */
