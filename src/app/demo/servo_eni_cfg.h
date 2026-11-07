#include "ecat_common.h"

/**************************motor config*************************/
#define ENI_FILE "zcpb.xml"
#define NUM_SERVOS  1           // 总电机数

typedef struct {
    servo_ctrl servos[NUM_SERVOS];
} proc_img_o;

typedef struct {
    servo_stat servos[NUM_SERVOS];
} proc_img_i;

// servo index to slave index, because some of slaves has no servo
const static uint8_t servo_to_slave_index[] = { 1 };

static const pi_offset_config  __pi_config[] =
{
    {
        -1, // po_mode_offset
        0, // po_ctrl_word_offset
        2, // po_p_offset
        -1, // pi_mode_offset
        0, // pi_stat_offset
        2, // pi_p_offset
    },
};


#define INIT_OP_MODE OP_MODE_CSP
