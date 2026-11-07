#ifndef MC_ZECM_H
#define MC_ZECM_H

/*
 * mc_zecm: thin single-handle facade over pci_zecm.
 *
 * Wraps a process-global ECAT_HANDLE so that demo loops such as
 * mc_pdo_thread can call MC_Ecat* without threading the handle through
 * every function. Stateless wrt master state - the underlying Ecat* APIs
 * still own all bus state.
 */

#include <stdbool.h>
#include <stdint.h>

// #include "pci_zecm.h"
#include "zecm_enum_def.h"

#define EXPORT
#define IN
#define OUT

#ifdef __cplusplus
extern "C" {
#endif

typedef int MCCommResult;

/* Extension placeholder. Identical to ECAT_ENI_INFO today; extra fields
 * (PDO offsets, servo mask, ...) can be appended without breaking ABI of
 * the demo. */
typedef struct ecat_eni_info_ex {
    uint32_t u32MainCycle;
    uint32_t u32MailboxCycle;
    uint32_t u32PIOutputByteSize;
    uint32_t u32PIInputByteSize;
} ecat_eni_info_ex;

/* Re-export the OP state constant under MC_ namespace so demos can stay
 * decoupled from zecm_enum_def. */
#define MC_EcatStateO ((uint8_t)EcatStateO)

/*
 * Lifecycle: Open performs EcatOpen + EcatBusRun on the same handle.
 * board_if may be NULL (defaults to "ID0"); eni_path must not be NULL.
 * Returns 0 on success, non-zero (ECAT_RESULT) on failure.
 */
int MC_EcatHostOpen(const char* board_if, uint32_t channel, const char* eni_path);
int MC_EcatHostClose(void);

/**
  @brief  启动ECAT总线运行

  @param  hHandle               ECAT控制句柄
  @param  cEniFileName          ENI配置文件名
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatBusRun(IN const char* eniFile);

MCCommResult MC_EcatBusStop(void);

/**
 * @brief 判断主站是否启动
 *
 * @return 返回主站是否启动
 */
MCCommResult MC_EcatBusIsRunning(bool* runFlag);
/**
 * @brief 获取运行的ENI信息
 *
 * @param eniInfo
 * @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
 */
MCCommResult MC_EcatGetEniInfo(ecat_eni_info_ex* eniInfo);
/**
  @brief  请求切换主站状态

  @param  u8State               切换到主站的状态，@see EcatState
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatRequestMasterState(IN uint8_t u8State);
/**
  @brief  获取当前主站请求状态

  @param[OUT]  pu8State         主站当前的状态，@see EcatState
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatGetMasterReqState(uint8_t* pu8State);
/**
  @brief  获取当前主站状态

  @param[OUT]  pu8State         主站当前的状态，@see EcatState
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatGetMasterState(OUT uint8_t* pu8State);

/**
  @brief  使能PI镜像缓存跟总线进行数据交换，等价于调用 EcatPIEnableSet(xx, 1)

  @param[IN]  flag
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatPIEnableSet(bool flag);

/**
  @brief  获取PI使能的状态

  @param[OUT]  flag
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatPIEnableGet(bool* flag);

/**
  @brief  等待总线更新输入数据，非线程安全

  @param  u32TimeOutMs          超时时间，以毫秒为单位
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatPIInputWaitNotify(IN uint32_t u32TimeOutMs);

/**
  @brief  从输入PDO读取镜像数据

  @param  u32Offset             区域的偏移地址
  @param  u32DataLen            读取的数据长度
  @param[OUT]  pvData           读取的数据
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatPIRead(IN uint32_t u32Offset, IN uint32_t u32DataLen, OUT void* pvData);

/**
  @brief  从输出PDO读取镜像数据

  @param  u32Offset             区域的偏移地址
  @param  u32DataLen            读取的数据长度
  @param[OUT]  pvData           读取的数据
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatPIReadTx(IN uint32_t u32Offset, IN uint32_t u32DataLen, OUT void* pvData);

/**
  @brief  写镜像输出数据到输出PDO

  @param  u32Offset             区域的偏移地址
  @param  u32DataLen            写入的数据长度
  @param  pvData                写入的数据
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatPIWrite(IN uint32_t u32Offset, IN uint32_t u32DataLen, IN const void* pvData);

/**
  @brief  COE命令扩展：写从站SDO

  @param  u16SlaveId            从站位置索引，范围：0~SlaveCount-1
  @param  u16Index              SDO索引
  @param  u8SubIndex            SDO子索引，若CompleteAccess == True，该值为0或1
  @param  bCompleteAccess       false: 非完全访问，true:
  完全访问，写完整的对象，子索引应为0（包含子索引0）或1（不包含子索引0）
  @param  u32DataLen            数据的字节长度，通常为：1,2,4
  @param  pvData                写入的数据，发送多字节时，须以小端排列
  @param  u32TimeOutMs          超时时间，以毫秒为单位
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatCoeSDODownloadEx(IN uint16_t u16SlaveId, IN uint16_t u16Index,
                                     IN uint8_t u8SubIndex, IN uint8_t bCompleteAccess,
                                     IN uint32_t u32DataLen, IN const void* pvData,
                                     IN uint32_t u32TimeOutMs);

/**
  @brief  COE命令扩展：读从站SDO

  @param  u16SlaveId            从站位置索引，范围：0~SlaveCount-1
  @param  u16Index              SDO索引
  @param  u8SubIndex            SDO子索引，若CompleteAccess == True，该值为0或1
  @param  bCompleteAccess       false: 非完全访问，true:
  完全访问，读完整的对象，子索引应为0（包含子索引0）或1（不包含子索引0）
  @param[INOUT]  pu32DataLen IN：作为pu8Data的最大长度，OUT：作为读取到的有效字节数，通常为：1,2,4
  @param[OUT]  pvData           存储读取到的数据，发送多字节时，须以小端排列
  @param  u32TimeOutMs          超时时间，以毫秒为单位
  @return  返回PCI_ZECM_RESULT(0)表示成功，非0表示失败
*/
MCCommResult MC_EcatCoeSDOUploadEx(IN uint16_t u16SlaveId, IN uint16_t u16Index,
                                   IN uint8_t u8SubIndex, IN uint8_t bCompleteAccess,
                                   IN OUT uint32_t* pu32DataLen, OUT void* pvData,
                                   IN uint32_t u32TimeOutMs);

/**
 * @brief 获得当前从站的状态
 *
 * @param u16SlaveIdx       从站位置索引，范围：0~SlaveCount-1
 * @param pu8Status         输出的从站状态
 */
MCCommResult MC_EcatGetSlaveState(uint16_t u16SlaveIdx, uint8_t* pu8Status);
/**
 * @brief 获得当前从站的状态，需要外部提供空间，容量为u16SlaveNum字节
 *
 * @param u16SlaveNum       查询的从站数量
 * @param pu8Status         输出的状态数组
 */
MCCommResult MC_EcatGetSlaveStateEx(uint16_t u16SlaveNum, uint8_t* pu8Status);
/**
 * @brief 获得当前从站的DL状态
 *
 * @param u16SlaveIdx       从站位置索引，范围：0~SlaveCount-1
 * @param pu16DLStatus         输出的从站DL状态
 */
MCCommResult MC_EcatGetSlaveDLStatus(uint16_t u16SlaveIdx, uint16_t* pu16DLStatus);
/**
 * @brief 获得当前从站的AL状态
 *
 * @param u16SlaveIdx       从站位置索引，范围：0~SlaveCount-1
 * @param pu16ALStatus         输出的从站AL状态
 */
MCCommResult MC_EcatGetSlaveALStatus(uint16_t u16SlaveIdx, uint16_t* pu16ALStatus);
/**
 * @brief 清除当前从站的异常状态
 *
 * @param u16SlaveIdx       从站位置索引，范围：0~SlaveCount-1
 */
MCCommResult MC_EcatClrSlaveStateError(uint16_t u16SlaveIdx);
/**
 * @brief 获得当前从站的Vendor
 *
 * @param u16SlaveIdx       从站位置索引，范围：0~SlaveCount-1
 * @param pu32VendorID         输出的从站Vendor
 */
MCCommResult MC_EcatGetSlaveVendorID(uint16_t u16SlaveIdx, uint32_t* pu32VendorID);
/**
 * @brief 获得当前从站的产品码
 *
 * @param u16SlaveIdx       从站位置索引，范围：0~SlaveCount-1
 * @param pu32ProductCode         输出的从站产品码
 */
MCCommResult MC_EcatGetSlaveProductCode(uint16_t u16SlaveIdx, uint32_t* pu32ProductCode);
/**
 * @brief 获得当前从站的修订号
 *
 * @param u16SlaveIdx       从站位置索引，范围：0~SlaveCount-1
 * @param pu32RevisionNo         输出的从站修订号
 */
MCCommResult MC_EcatGetSlaveRevisionNo(uint16_t u16SlaveIdx, uint32_t* pu32RevisionNo);
/**
 * @brief 获得当前从站的别名
 *
 * @param u16SlaveIdx       从站位置索引，范围：0~SlaveCount-1
 * @param pu32Alias         输出的从站别名
 */
MCCommResult MC_EcatGetSlaveAlias(uint16_t u16SlaveIdx, uint32_t* pu32Alias);
/**
 * @brief 获得当前从站的物理地址
 *
 * @param u16SlaveIdx       从站位置索引，范围：0~SlaveCount-1
 * @param pu16FixAddr       输出的从站物理地址
 */
MCCommResult MC_EcatGetSlaveFixAddr(uint16_t u16SlaveIdx, uint16_t* pu16FixAddr);
/**
 * @brief 获得当前ENI中从站数量
 *
 * @param pu16SlaveCount       输出从站数量
 */
MCCommResult MC_EcatGetSlaveCount(uint16_t* pu16SlaveCount);

MCCommResult MC_EcatGetActiveSlaveCount(uint16_t* pu16SlaveCount);

#ifdef __cplusplus
}
#endif

#endif /* MC_ZECM_H */
