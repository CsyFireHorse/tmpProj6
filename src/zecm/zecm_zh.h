/*******************************************************************************
*                                   ZMC
*                       ----------------------------
*                          ZHIYUAN Machine Control
*
* Copyright (c) 2001-2020 Guangzhou ZHIYUAN Electronics Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
*******************************************************************************/
 
/*----------------------------------------------------------------------------*/
/**
 * \file
 * \brief KPA主站库简易API说明
 *
 * \internal
 * \par Modification history
 * - 1.00 2020-1-20  ZBL, 初次实现
 * - 1.01 2024-4-23  ZBL, 增加主站发送PDO前回调函数、定时器回调函数
 * \endinternal
 */
/*----------------------------------------------------------------------------*/
#ifndef ZMC_ZECM_ZH_H_
#define ZMC_ZECM_ZH_H_

#define ECAT_FAILED(status)  (((ECAT_RESULT)(status) & 0x8000) != 0)

#define ECAT_TRUE  1
#define ECAT_FALSE  0

#ifdef __cplusplus
#define ECAT_NULL  0
#else
#define ECAT_NULL  ((void*)0)
#endif

typedef long long int ECAT_LONGLONG;
typedef long long unsigned int ECAT_ULONGLONG;
typedef int ECAT_FILE;
typedef unsigned int ECAT_SIZE;
typedef short unsigned int ECAT_WCHAR;
typedef char ECAT_CHAR;
typedef ECAT_CHAR* ECAT_PCHAR;
typedef ECAT_CHAR* ECAT_STR;
typedef const ECAT_CHAR* ECAT_CSTR;
typedef ECAT_WCHAR* ECAT_PWCHAR;
typedef ECAT_WCHAR* ECAT_WSTR;
typedef const ECAT_WCHAR* ECAT_CWSTR;
typedef unsigned char ECAT_BYTE;
typedef ECAT_BYTE* ECAT_PBYTE;
typedef unsigned char ECAT_UINT8;
typedef signed char ECAT_INT8;
typedef short int ECAT_SWORD;
typedef short unsigned int ECAT_WORD;
typedef ECAT_WORD* ECAT_PWORD;
typedef short unsigned int ECAT_UINT16;
typedef short int ECAT_INT16;
typedef unsigned int ECAT_DWORD;
typedef int ECAT_SDWORD;
typedef int ECAT_INT32;
typedef unsigned int ECAT_UINT32;
typedef ECAT_DWORD* ECAT_PDWORD;
typedef ECAT_SDWORD* ECAT_PSDWORD;
typedef ECAT_LONGLONG ECAT_INT64;
typedef ECAT_ULONGLONG ECAT_UINT64;
typedef unsigned char ECAT_BOOL;
typedef int ECAT_RESULT;
typedef void* ECAT_HMODULE;
typedef void* ECAT_HANDLE;
typedef ECAT_HANDLE* ECAT_PHANDLE;
typedef ECAT_DWORD ECAT_TIME_MS;
typedef ECAT_ULONGLONG ECAT_TIME_US;
typedef ECAT_ULONGLONG ECAT_TIME_NS;
typedef ECAT_LONGLONG ECAT_TIME_STAMP;
typedef void* ECAT_LPVOID;
typedef void ECAT_VOID;
typedef float ECAT_REAL32;
typedef double ECAT_REAL64;
typedef ECAT_CHAR ECAT_TCHAR;
typedef ECAT_TCHAR* ECAT_TSTR;
typedef const ECAT_TCHAR* ECAT_TCSTR;

enum _EcatMasterError {
    ECAT_S_OK = 0,
    ECAT_S_SIZE_LIMIT = 1,
    ECAT_S_NOITEMS = 2,
    ECAT_S_PI_ALREADY_SYNC = 3,
    ECAT_S_NOITEMS_MAILBOX_WAIT = 4,
    ECAT_S_TIMEOUT = 5,
    ECAT_S_THREAD_EXIT = 6,
    ECAT_S_SYNC_CALL = 7,
    ECAT_S_REDUNDANCY = 8,
    ECAT_S_AGAIN = 9,
    ECAT_S_OBJECT = 10,
    ECAT_S_INVALID_LICENSE = 256,
    ECAT_S_DEMO_TIME_EXPIRED = 257,
    ECAT_S_PI_SYNC_LATE = 528,
    ECAT_S_PI_SYNC_FAST = 544,
    ECAT_E_OUTOFMEMORY = 32769,
    ECAT_E_INVALIDARG = 32770,
    ECAT_E_FAIL = 32771,
    ECAT_E_NOTIMPL = 32772,
    ECAT_E_XML_OPEN = 32773,
    ECAT_E_XML_PARSE = 32774,
    ECAT_E_MASTER_NOT_CONFIGURED = 32775,
    ECAT_E_MASTER_NOT_CONNECTED = 32776,
    ECAT_E_MASTER_ALREADY_CONNECTED = 32777,
    ECAT_E_INVALID_SLAVE_INDEX = 32778,
    ECAT_E_INVALID_TRANSITION = 32779,
    ECAT_E_DRIVER_LOAD = 32780,
    ECAT_E_DRIVER_INIT = 32781,
    ECAT_E_DRIVER_NOT_SUPPORTED = 32782,
    ECAT_E_DRIVER_NOT_LOADED = 32783,
    ECAT_E_SEND_FRAME_FAILED = 32784,
    ECAT_E_RECV_FRAME_FAILED = 32785,
    ECAT_E_TRACE_BUFFER_OVERFLOW = 32786,
    ECAT_E_NOTAVIABLE_TRANSITION = 32787,
    ECAT_E_TRANSITION_ERROR = 32788,
    ECAT_E_INVALID_POINTER = 32789,
    ECAT_E_INVALID_COMMAND_TYPE = 32790,
    ECAT_E_TRANSITION_FORBIDDEN = 32791,
    ECAT_E_SYNC_CALL_TIMEOUT = 32792,
    ECAT_E_MB_COE_TRANSITION_ABORTED = 32793,
    ECAT_E_INVALID_WC = 32794,
    ECAT_E_INVALID_SLAVE_STATE = 32795,
    ECAT_E_SNAPSHOT_DATA_LOSE = 32796,
    ECAT_E_XML_LICENSE_OPEN = 32797,
    ECAT_E_XML_LICENSE_PARSE = 32798,
    ECAT_E_XML_LICENSE_PRODUCT_NAME = 32799,
    ECAT_E_XML_LICENSE_LICENSED_TO = 32800,
    ECAT_E_XML_LICENSE_EXP_DATE = 32801,
    ECAT_E_XML_LICENSE_DEMO_MODE = 32802,
    ECAT_E_XML_LICENSE_HW_BINDING = 32803,
    ECAT_E_XML_LICENSE_PRODUCT_KEY = 32804,
    ECAT_E_CMD_ACYCLIC_SET_DATA = 32805,
    ECAT_E_CMD_CONTAINER_ADD_RES_CMD = 32806,
    ECAT_E_BUFFER_SIZE_LIMIT = 32807,
    ECAT_E_TELEGRAM_BUILD = 32808,
    ECAT_E_FRAME_BUILD = 32809,
    ECAT_E_LOAD_FORBIDDEN = 32810,
    ECAT_E_INVALID_CONFIGURATION = 32811,
    ECAT_E_READ_SLAVE_STATES = 32812,
    ECAT_E_TRANSIT_SLAVES = 32813,
    ECAT_E_SEND_MASTER_CMDS = 32814,
    ECAT_E_MASTER_ALREADY_STARTED = 32815,
    ECAT_E_THREAD_INITIALISE = 32816,
    ECAT_E_READ_EEPROM = 32817,
    ECAT_E_MB_NOT_SUPPORTED = 32818,
    ECAT_E_MB_COE_NOT_SUPPORTED = 32819,
    ECAT_E_PI_ALREADY_UNLOCKED = 32820,
    ECAT_E_INVALID_FRAME_SIZE = 32821,
    ECAT_E_INVALID_FRAME_TYPE = 32822,
    ECAT_E_INVALID_COMMAND_SIZE = 32823,
    ECAT_E_INVALID_COMMAND_DATA = 32824,
    ECAT_E_WRITE_EEPROM = 32825,
    ECAT_E_MASTER_NOT_STARTED = 32826,
    ECAT_E_ARRAY_BOUNDS_EXCEEDED = 32827,
    ECAT_E_INVALID_LICENSE = 32828,
    ECAT_E_RECV_FRAME_TIMEOUT = 32829,
    ECAT_E_EEPROM_ACCESS_DENIED = 32830,
    ECAT_E_EEPROM_ACCESS_DENIDED = 32830,
    ECAT_E_BUSY = 32831,
    ECAT_E_TIME = 32832,
    ECAT_E_ABORTED = 32833,
    ECAT_E_SLAVE_PI_NOT_EXIST = 32834,
    ECAT_E_DC_SYNC = 32835,
    ECAT_E_BAD_CONTEXT = 32836,
    ECAT_E_NOT_PERMITTED = 32837,
    ECAT_E_INI_BAD_CONTENT = 32838,
    ECAT_E_OBJECT = 32839,
    ECAT_E_CHANNEL_TRANSFER = 32840,
    ECAT_E_OPEN_FILE = 32848,
    ECAT_E_WRITE_FILE = 32849,
    ECAT_E_READ_FILE = 32850,
    ECAT_E_CANT_GET_HW_KEY = 32851,
    ECAT_E_CANT_ENCODE_DATA = 32852,
    ECAT_E_CANT_DECODE_DATA = 32853,
    ECAT_E_DRIVER_INVALID_NIC = 32864,
    ECAT_E_QUEUE_RESOURCE_LACK = 32880,
    ECAT_E_QUEUE_NO_ITEMS = 32881,
    ECAT_E_MB_COE_UNSUPPORTED_DATA_TYPE = 33024,
    ECAT_E_MB_COE_OBJECT_IS_READ_ONLY = 33025,
    ECAT_E_MB_COE_OBJECT_IS_WRITE_ONLY = 33026,
    ECAT_E_MB_COE_INCOMPLETE_ENTRY_DESC = 33027,
    ECAT_E_MB_COE_SDOINFO_ERROR = 33028,
    ECAT_E_MB_VOE_NOT_SUPPORTED = 33104,
    ECAT_E_MASTER_IS_LOCKED = 33280,
    ECAT_E_MASTER_NO_RIGHTS = 33281,
    ECAT_E_MASTER_USER_NOT_FOUND = 33282,
    ECAT_E_MASTER_NAME_CANNOT_BE_FOUND = 33283,
    ECAT_E_TASK_CANNOT_BE_FOUND = 33284,
    ECAT_E_AOE_ERROR_CODE = 33536,
    ECAT_E_AOE_CMD_ERROR = 33537,
    ECAT_E_AOE_INVALID_HEADER = 33538,
    ECAT_E_MB_AOE_NOT_SUPPORTED = 33539,
    ECAT_E_AOE_INVALID_ROUTE = 33540,
    ECAT_E_MB_SOE_NOT_SUPPORTED = 33552,
    ECAT_E_SOE_ERROR_CODE = 33553,
    ECAT_E_SOE_INVALID_HEADER = 33554,
    ECAT_E_SOE_FRAGMENT_LEFT = 33555,
    ECAT_E_MB_EOE_NOT_SUPPORTED = 33568,
    ECAT_E_MB_FOE_NOT_SUPPORTED = 33584,
    ECAT_E_OD_OBJECT_NOTFOUND = 33792,
    ECAT_E_OD_OBJECT_ALREADY_EXISTS = 33793,
    ECAT_E_OD_ENTRY_DESCRIPTION_ALREADY_EXISTS = 33794,
    ECAT_E_OD_ENTRY_DESCRIPTION_FAILED = 33795,
    ECAT_E_OD_INVALID_OBJECT_TYPE = 33796,
    ECAT_E_OD_INVALID_ACCESS_TYPE = 33797,
    ECAT_E_OD_INVALID_DATA_TYPE = 33798,
    ECAT_E_OD_ACCESS_DENIED = 33799,
    ECAT_E_OD_NOT_CREATED = 33800,
    ECAT_E_OD_SDO_SERVICE_NOT_SUPORTED = 33801,
    ECAT_E_OD_SDO_SIZE_INVALID_HEADER = 33802,
    ECAT_E_OD_SDO_SIZE_TOO_SHORT = 33803,
    ECAT_E_OD_SDO_INVALID_SIZE = 33804,
    ECAT_E_OD_SUB_OBJ_NOTFOUND = 33805,
    ECAT_E_OD_MORE_MAXIMUM_VALUE = 33806,
    ECAT_E_OD_LESS_MINIMUM_VALUE = 33807,
    ECAT_E_LICENSE_INIT = 34050,
    ECAT_E_LICENSE_LOAD = 34051,
    ECAT_E_LICENSE_INVALID_TARGET = 34052,
    ECAT_E_LICENSE_EXPIRED = 34053,
    ECAT_E_LICENSE_INVALID_HW = 34054,
    ECAT_E_NO_CONFIG = 34055,
    ECAT_E_INVALID_MASTER_CONFIGURATION = 34056,
    ECAT_E_ALREADY_INITIALIZED = 34057,
    ECAT_E_PI_MAP_OUT_OF_IMAGE = 34058,
    ECAT_E_PI_MAP_REJECTED = 34059,
    ECAT_E_NOT_INITIALIZED = 34060,
    ECAT_E_FRAME_GENERAL = 34305,
    ECAT_E_FRAME_QUEUE = 34306,
    ECAT_E_FRAME_TRANSFER = 34307,
    ECAT_E_FRAME_SEND = 34308,
    ECAT_E_FRAME_WAIT_TIMEOUT = 34309,
    ECAT_E_FRAME_SEND_TIMEOUT = 34310,
    ECAT_E_FRAME_SCHEDULER_DESTROY = 34311,
    ECAT_E_FRAME_CANCEL = 34312,
    ECAT_E_FRAME_TOO_LONG = 34313,
    ECAT_E_MBXREQ_NOT_STARTED = 34336,
    ECAT_E_MBXREQ_RESPONSE = 34337,
    ECAT_E_MBXREQ_NO_RESPONSE_DATA = 34338,
    ECAT_E_MBXREQ_FINISHED = 34339,
    ECAT_E_MB_FAIL = 34352,
    ECAT_E_MB_INVALID_SYNTAX = 34353,
    ECAT_E_MB_INVALID_CHANNEL = 34355,
    ECAT_E_MB_SERVICE_NOT_SUPPORTED = 34356,
    ECAT_E_MB_INVALID_PROTOCOL_HEADER = 34357,
    ECAT_E_MB_SIZE_TOO_SHORT = 34358,
    ECAT_E_MB_NO_MORE_MEMORY = 34359,
    ECAT_E_MB_INVALID_SIZE = 34360,
    ECAT_E_INIT_MODULE = 34560,
    ECAT_E_CONNECTION = 34817,
    ECAT_E_RPC_RESPOND = 34818,
    ECAT_E_RPC_INIT = 34819,
    ECAT_E_LIBRARY = 34820,
    ECAT_E_NOITEMS = 34821,
    ECAT_E_SYS = 34822
};
typedef enum _EcatMasterError EcatMasterError;
typedef ECAT_RESULT (*EcatExtCtrlTaskHandler)(const ECAT_LPVOID,ECAT_BYTE*,ECAT_BYTE*);

#include "zecm_enum_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
主站库初始化
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatMasterLibInit(
    ECAT_CSTR pszIniFileName
    );

/**
释放主站库资源
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatMasterLibExit(
    );

/**
请求切换主站状态
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatRequestMasterState(
    ECAT_HANDLE hMaster,
    ECAT_BYTE byState                            /* [IN]请求状态，详见EcatState */
    );

/**
获取当前主站状态
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatGetMasterState(
    ECAT_HANDLE hMaster,
    ECAT_BYTE* pbyState                          /* [OUT]指向当前状态变量的指针，参考：EcatState */
    );

/**
设置用户处理程序，每次收到过程数据时调用
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatSetExtCtrlTaskHandler(
    ECAT_HANDLE hMaster,
    EcatExtCtrlTaskHandler pfnHandler,           /* [IN]用户的回调函数指针 */
    ECAT_LPVOID pvCtrlTaskArg                    /* [IN]用户回调函数参数 */
    );

/**
设置用户处理程序，每次发送过程数据前调用
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatSetExtPrevWriteCtrlTaskHandler(
    ECAT_HANDLE hMaster,
    EcatExtCtrlTaskHandler pfnHandler,           /* [IN]用户的回调函数指针 */
    ECAT_LPVOID pvCtrlTaskArg                    /* [IN]用户回调函数参数 */
    );

/**
设置用户处理程序，每次过程镜像工作定时器到来时触发
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatSetExtTimerCtrlHandler(
    ECAT_HANDLE hMaster,
    EcatExtCtrlTaskHandler pfnHandler,           /* [IN]用户的回调函数指针 */
    ECAT_LPVOID pvCtrlTaskArg                    /* [IN]用户回调函数参数 */
    );

/**
复位外部任务处理程序的执行并设置为默认处理程序（即空处理程序）
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatResetExtCtrlTaskHandler(
    ECAT_HANDLE hMaster
    );

/**
读从站SDO
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatCoeSDOUpload(
    ECAT_HANDLE hMaster,                         /* [IN]主站句柄 */
    ECAT_WORD wSlaveId,                          /* 从节点ID */
    ECAT_WORD wIndex,                            /* SDO索引 */
    ECAT_BYTE bySubIndex,                        /* SDO子索引 */
    ECAT_BOOL bCompleteAccess,
    ECAT_DWORD* pdwDataLen,
    ECAT_BYTE* pbyData,
    ECAT_DWORD dwMaxDataLen,
    ECAT_DWORD dwTimeOut                         /* 超时 */
    );

/**
写从站SDO
*/
ECAT_RESULT                                      /* ECAT_S_OK-成功, ECAT_XXX-错误码 */
EcatCoeSDODownload(
    ECAT_HANDLE hMaster,                         /* [IN]主站句柄 */
    ECAT_WORD wSlaveId,                          /* 从节点ID */
    ECAT_WORD wIndex,                            /* SDO索引 */
    ECAT_BYTE bySubIndex,                        /* SDO子索引 */
    ECAT_BOOL bCompleteAccess,
    ECAT_DWORD* pdwDataLen,
    ECAT_BYTE* pbyData,
    ECAT_DWORD dwMaxDataLen,
    ECAT_DWORD dwTimeOut                         /* 超时 */
    );

/**
主站启动信息
*/
typedef struct ECAT_SIMPLE_START_CONTEXT_T {
    ECAT_HANDLE Master;
    ECAT_HANDLE Server;
    ECAT_CSTR NIC0;
    ECAT_CSTR NIC1;
    ECAT_CSTR ENI;
    ECAT_UINT32 MainCycleTimeUs;
    ECAT_UINT32 SubCycleTimeUs;
    ECAT_UINT32 AutoRecoveryTimeoutMs;
    ECAT_UINT32 RpcServerPort;
} ECAT_SIMPLE_START_CONTEXT;

/**
启动主站
*/
ECAT_RESULT
EcatSimpleStart(
    ECAT_SIMPLE_START_CONTEXT *ctx
    );

/**
停止主站
*/
ECAT_RESULT
EcatSimpleStop(
    ECAT_SIMPLE_START_CONTEXT *ctx
    );

/**
FOE文件写接口
*/
ECAT_RESULT EcatMbxFoE_Write(
    ECAT_HANDLE      hMaster,               /* [IN]主站句柄 */
    ECAT_WORD        wSlaveId,              /* [IN]从节点ID */
    ECAT_DWORD       dwPas,                 /* [IN]密码,0 为不使用密码*/
    const ECAT_CHAR *pszFileLocalName,      /* [IN]本地文件名 */
    const ECAT_CHAR *pszFileSlaveName);     /* [IN]存储在从站内文件名 */

/**
FOE文件读接口
*/
ECAT_RESULT EcatMbxFoE_Read(
    ECAT_HANDLE      hMaster,               /* [IN]主站句柄 */
    ECAT_WORD        wSlaveId,              /* [IN]从节点ID */
    ECAT_DWORD       dwPas,                 /* [IN]密码,0 为不使用密码*/  
    const ECAT_CHAR *pszFileLocalName,      /* [IN]本地文件名 */
    const ECAT_CHAR *pszFileSlaveName);     /* [IN]存储在从站内文件名 */

/** @brief 从站端口状态   */
typedef enum
{
    ECAT_NDIAG_SLAVE_PORT_STATUS_IN_OUT_REVERSE = 0,        // IN OUT端口反接
    ECAT_NDIAG_SLAVE_PORT_STATUS_IN_DISCONNECT,             // IN端口连接断开
} EcatNetDiagnosticsSlavePortStatus;

/** @brief 从站信息   */
typedef struct
{
    EcatNetDiagnosticsSlavePortStatus       byStatus;       // 从站状态
    ECAT_DWORD                              dwVendorId;     // 厂商ID
    ECAT_DWORD                              dwProductCode;  // 产品ID
    char                                    byName[256];    // 从站名称及路径
}EcatNetDiagnosticsSlaveInfo;

/** @brief 从站拓扑节点   */
typedef struct
{
    char                                    byName[24];     // 从站名称
    ECAT_DWORD                              dwVendorId;     // 厂商ID
    ECAT_DWORD                              dwProductCode;  // 产品ID
    ECAT_WORD                               wPort;          // 从站的端口，0-3：表示有效的端口，0xFFFF表示无效
    ECAT_WORD                               wPhyPos;        // 从站在当前分支下的硬件接线顺序，从1开始
    ECAT_WORD                               wIncAddr;       // 从站在当前拓扑中的顺序自增地址
    ECAT_WORD                               wFixAddr;       // 从站在ENI中的物理地址，0xFFFF表示无效
    ECAT_WORD                               wAlias;         // 当前从站的别名值，ESC寄存器0x0012的值
    ECAT_WORD                               wRequestingID;  // 当前从站的请求ID，ESC寄存器0x0134的值
    ECAT_WORD                               dwRes[2];       // 保留，未定义，固定为0
}EcatNetDiagnosticsSlaveTopologyNode;

/** @brief 从站拓扑   */
typedef struct
{
    EcatNetDiagnosticsSlavePortStatus       byStatus;       // 下标0的从站的状态
    ECAT_DWORD                              dwTopologyNum;  // 拓扑的有效数量
    EcatNetDiagnosticsSlaveTopologyNode     tTopology[10];  // 从站的当前拓扑，它是一颗树结构
                                                            // 下标0指向诊断出现错误的从站
                                                            // 下标1-9，表示其上1-9级父结点，最多输出10级
}EcatNetDiagnosticsSlaveTopology;


/**
 * @method EcatNetDiagnosticsGetSlaveLinkStatus
 * 网络诊断并返回连接有问题的从站信息
 *
 * @param {ECAT_HANDLE} [in]hMaster  主站句柄。
 * @param {EcatNetDiagnosticsSlaveInfo *} [out]pErrorInfos  存储错误的从站信息。
 * @param {ECAT_WORD} [in,out]pwErrorInfoNum  输入作为pErrorInfos的最大数量，输出作为错误的数量，如果为0，则表示从站均正常。
 *
 * @return 返回RET_OK表示成功，没有异常，否则接收调用失败。
 */
ECAT_RESULT EcatNetDiagnosticsGetSlaveLinkStatus(ECAT_HANDLE hMaster,
	EcatNetDiagnosticsSlaveInfo* pErrorInfos, ECAT_WORD *pwErrorInfosNum);

/**
 * @method EcatNetDiagnosticsGetSlaveLinkStatusEx
 * 网络诊断并返回连接有问题的从站拓扑
 *
 * @param {ECAT_HANDLE} [in]hMaster  主站句柄。
 * @param {EcatNetDiagnosticsSlaveInfo *} [out]pErrorInfos  存储错误的从站信息。
 * @param {ECAT_WORD} [in,out]pwErrorInfoNum  输入作为pErrorInfos的最大数量，输出作为错误的数量，如果为0，则表示从站均正常。
 *
 * @return 返回RET_OK表示成功，没有异常，否则接收调用失败。
 */
ECAT_RESULT EcatNetDiagnosticsGetSlaveLinkStatusEx(ECAT_HANDLE hMaster, 
    EcatNetDiagnosticsSlaveTopology* pErrorTopologys, ECAT_WORD *pwErrorTopologysNum);

#ifdef __cplusplus
}
#endif

#endif  // ZMC_ZECM_ZH_H_
/* end of file */


