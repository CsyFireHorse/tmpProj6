#ifndef _PDO_OFFSET_CFG_
#define _PDO_OFFSET_CFG_
#include "stdint.h"

typedef struct pi_offset_config_t{
    uint8_t             slave_idx;
    uint8_t             slave_channel_idx;
    int32_t             po_mode_offset;
    int32_t             po_ctrl_word_offset;
    int32_t             po_p_offset;
    int32_t             po_v_offset;
    int32_t             po_t_offset;

    int32_t             pi_mode_offset;
    int32_t             pi_stat_offset;
    int32_t             pi_error_offset;    
    int32_t             pi_p_offset;
    int32_t             pi_v_offset;  
    int32_t             pi_t_offset;      
} pi_offset_config;

#endif