#include "stdint.h"


typedef  uint32_t   U32;
typedef  uint16_t   U16;
typedef  uint8_t    U8;
typedef  int32_t    I32;
typedef  int16_t    I16;

#define MIN_LIMIT(T,L) ((T) < (L) ? (L) : (T))
#define MAX_LIMIT(T,L) ((T) > (L) ? (L) : (T))

#define LEN(x) sizeof(x)/sizeof(*(x))

enum {
    OP_MODE_PP = 1,      
    OP_MODE_PV = 3,    
    OP_MODE_PT = 4,      
    OP_MODE_CSP = 8,
    OP_MODE_CSV = 9,
    OP_MODE_CST = 10,
};

////////////////////////////////////////////////////////////////////////////////
// 过程映像
typedef union {
    struct {
        U16 switch_on: 1;
        U16 enable_voltage: 1;
        U16 quick_stop: 1;
        U16 enable_operation: 1;
        U16 pad0: 3;
        U16 fault_reset: 1;
        U16 halt: 1;
        U16 pad1: 1;
        U16 pad2: 6;
    } b;
    U16 v;
} servo_ctrl_word;

typedef union {
    struct {
        U16 ready_to_switch_on: 1;
        U16 switched_on: 1;
        U16 operation_enabled: 1;
        U16 fault: 1;
        U16 voltage_enabled: 1;
        U16 quick_stop: 1;
        U16 switch_on_disabled: 1;
        U16 warning: 1;
        U16 pad0: 1;
        U16 remote: 1;
        U16 target_reached: 1;
        U16 internal_limit_active: 1;
        U16 pad1: 4;
    } b;
    U16 v;
} servo_stat_word;

typedef struct {
    uint8_t             *mode;
    servo_ctrl_word     *ctrl;
    I32                 *p;
} servo_ctrl;

typedef struct {
    uint8_t             *mode;
    servo_stat_word     *stat;
    I32                 *p;
} servo_stat;



typedef struct {
    int32_t             po_mode_offset;
    int32_t             po_ctrl_word_offset;
    int32_t             po_p_offset;
    int32_t             pi_mode_offset;
    int32_t             pi_stat_offset;
    int32_t             pi_p_offset;
} pi_offset_config;


typedef struct {
    servo_ctrl_word sc;    // 新控制字
    servo_stat_word ss;    // 旧状态字
    int run_delay;         // 延时启动
    int fault_reset_delay; // 延时再错误恢复
    float zero_p;          // 零点位置
    float offset_p;        // 偏移输出位置
    I32 out_p;             // 输出位置
    I32 out_v;             // 输出转速
    I16 out_t;             // 输出扭矩
    bool reverse;          // 当前方向
}servos_t;



#ifdef _WIN32
#include <windows.h>
inline void usleep(int64_t usec){
    HANDLE timer;
    LARGE_INTEGER ft;
    ft.QuadPart = -(10*usec);
    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
#else
#include <unistd.h>
#endif