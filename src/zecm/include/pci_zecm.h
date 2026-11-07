#ifndef PCI_ZECM_H
#define PCI_ZECM_H

#include <stdint.h>
#include <stdbool.h>

#include "zecm_enum_def.h"
#ifdef __cplusplus
extern "C" {
#endif

// Basic types
typedef void *ECAT_HANDLE;
typedef int ECAT_RESULT;

// Error codes
#define ECAT_S_OK 0
#define ECAT_S_TIMEOUT 5

// PI area definitions
enum {
    PI_AREA_LOCAL_OUTPUT = 0,
    PI_AREA_LOCAL_INPUT,
    PI_AREA_FAST_OUTPUT,
    PI_AREA_SHADOW_OUTPUT,
    PI_AREA_SHADOW_INPUT,
    PI_AREA_NUM,
};

// ENI info structure
typedef struct {
    uint32_t u32MainCycle;         //!< 主循环周期，PDO周期
    uint32_t u32MailboxCycle;      //!< 邮箱周期
    uint32_t u32PIOutputByteSize;  //!< 镜像输出有效字节大小
    uint32_t u32PIInputByteSize;   //!< 镜像输入有效字节大小
} ECAT_ENI_INFO;

// API declarations
int32_t EcatOpen(ECAT_HANDLE *phHandle, const char *sBoardIf, uint32_t u32EcatChannel);
int32_t EcatClose(ECAT_HANDLE hHandle);
int32_t EcatBusRun(ECAT_HANDLE hHandle, const char *cEniFileName);
int32_t EcatBusStop(ECAT_HANDLE hHandle);
int32_t EcatRequestMasterState(ECAT_HANDLE hHandle, uint8_t u8State);
int32_t EcatGetMasterState(ECAT_HANDLE hHandle, uint8_t *pu8State);
int32_t EcatGetEniInfo(ECAT_HANDLE hHandle, ECAT_ENI_INFO *ptInfo);

/**
  @brief  COE命令扩展：写从站SDO

  @param  hHandle               ECAT控制句柄
  @param  u16SlaveId            从站位置索引，范围：0~SlaveCount-1
  @param  u16Index              SDO索引
  @param  u8SubIndex            SDO子索引，若CompleteAccess == True，该值为0或1
  @param  bCompleteAccess       false: 非完全访问，true:
  完全访问，写完整的对象，子索引应为0（包含子索引0）或1（不包含子索引0）
  @param  u32DataLen            数据的字节长度，通常为：1,2,4
  @param  pvData                写入的数据，发送多字节时，须以小端排列
  @param  u32TimeOutMs          超时时间，以毫秒为单位
  @return  返回ECAT_S_OK(0)表示成功，非0表示失败
*/
int32_t EcatCoeSDODownloadEx(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t u16Index,
                             uint8_t u8SubIndex, uint8_t bCompleteAccess, uint32_t u32DataLen,
                             const void *pvData, uint32_t u32TimeOutMs);

/**
  @brief  COE命令扩展：读从站SDO

  @param  hHandle               ECAT控制句柄
  @param  u16SlaveId            从站位置索引，范围：0~SlaveCount-1
  @param  u16Index              SDO索引
  @param  u8SubIndex            SDO子索引，若CompleteAccess == True，该值为0或1
  @param  bCompleteAccess       false: 非完全访问，true:
  完全访问，读完整的对象，子索引应为0（包含子索引0）或1（不包含子索引0）
  @param[INOUT]  pu32DataLen IN：作为pu8Data的最大长度，OUT：作为读取到的有效字节数，通常为：1,2,4
  @param[OUT]  pvData           存储读取到的数据，发送多字节时，须以小端排列
  @param  u32TimeOutMs          超时时间，以毫秒为单位
  @return  返回ECAT_S_OK(0)表示成功，非0表示失败
*/
int32_t EcatCoeSDOUploadEx(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t u16Index,
                           uint8_t u8SubIndex, uint8_t bCompleteAccess, uint32_t *pu32DataLen,
                           void *pvData, uint32_t u32TimeOutMs);

int32_t PcieEcatCoeSDODownload(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t u16Index,
                               uint8_t u8SubIndex, uint32_t u32DataLen, const void *pvData,
                               uint32_t u32TimeOutMs);

int32_t PcieEcatCoeSDOUpload(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t u16Index,
                             uint8_t u8SubIndex, uint32_t *pu32DataLen, void *pvData,
                             uint32_t u32TimeOutMs);

int32_t EcatPIEnable(ECAT_HANDLE hHandle);
int32_t EcatPIRead(ECAT_HANDLE hHandle, uint32_t u32PiArea, uint32_t u32Offset,
                   uint32_t u32DataLen, void *pvData);
int32_t EcatPIWrite(ECAT_HANDLE hHandle, uint32_t u32PiArea, uint32_t u32Offset,
                    uint32_t u32DataLen, const void *pvData);
int32_t EcatPIInputWaitNotify(ECAT_HANDLE hHandle, uint32_t u32TimeOutMs);
int32_t EcatGetSlaveCount(ECAT_HANDLE hHandle, uint16_t *pu16Count);
int32_t EcatGetActiveSlaveCount(ECAT_HANDLE hHandle, uint16_t *pu16Count);

int32_t EcatGetSlaveState(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint8_t *pu8State);
int32_t EcatGetSlaveDLStatus(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t *pu16DLStatus);
const char *EcatErrorGetErrorMessage(ECAT_RESULT nError);

/*******************************************************************************
  板卡接口
*******************************************************************************/
#define BOARD_IDX(buff, index) (sprintf(buff, "ID%d", (int)index), buff)
#define BOARD_ALIAS(buff, alias) (sprintf(buff, "AL%d", (int)alias), buff)

#ifdef __cplusplus
}
#endif

#endif  // PCI_ZECM_H
