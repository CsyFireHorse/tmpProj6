#ifndef _ECAT_PDOIO_WRAP_H_
#define _ECAT_PDOIO_WRAP_H_

#include "stdint.h"


/*  初始化 缓冲区大小
 *
 *  core    主站索引、cpu索引
 *  tx_sz   txpdo尺寸
 *  rx_sz   rxpdo尺寸
 */
int init_txrxpdo_data(uint16_t core, uint16_t tx_sz, uint16_t rx_sz);
void* get_txrxpdo(uint16_t core);


/* 读取 txpdo 输入
 * core     \param[in]      主站索引、cpu索引
 * offset   \param[in]      pdo偏置
 * size     \param[in]      读取尺寸
 * val      \param[out]     读取值
 * \return      返回错误码
 */
int get_txpdo_data(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val);
// 读取 rxpdo 输入
int get_rxpdo_data(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val);

/*      写入 rxpdo
 * core     \param[in]      主站索引、cpu索引
 * offset   \param[in]      pdo偏置
 * size     \param[in]      读取尺寸
 * val      \param[in]      设置值
 * \return      返回错误码
 */
int set_rxpdo_data(uint16_t core, uint16_t offset, uint16_t size, const uint8_t* val);

// 排列组合： { get_txpdo get_rxpdo, set_rxpdo, set_rxpdo_inverse} * { data, mask_data, mask_byte, mask_word, mask_dword }

int get_rxpdo_data_mask(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val, const uint8_t* mask);
int get_txpdo_data_mask(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val, const uint8_t* mask);

// 更改掩模 mask 对应的bit 为 指定值
int set_rxpdo_data_mask(uint16_t core, uint16_t offset, uint16_t size, const uint8_t* val, const uint8_t* mask);

// 更改翻转掩模 mask 对应的bit
int set_rxpdo_data_inverse(uint16_t core, uint16_t offset, uint16_t size, const uint8_t* mask);

int get_rxpdo_byte_mask(uint16_t core, uint16_t offset, uint8_t mask, uint8_t* val);
int get_rxpdo_word_mask(uint16_t core, uint16_t offset, uint16_t mask, uint16_t* val);
int get_rxpdo_dword_mask(uint16_t core, uint16_t offset, uint32_t mask, uint32_t* val);

int get_txpdo_byte_mask(uint16_t core, uint16_t offset, uint8_t mask, uint8_t* val);
int get_txpdo_word_mask(uint16_t core, uint16_t offset, uint16_t mask, uint16_t* val);
int get_txpdo_dword_mask(uint16_t core, uint16_t offset, uint32_t mask, uint32_t* val);

int set_rxpdo_byte_mask(uint16_t core, uint16_t offset, uint8_t val, uint8_t mask);
int set_rxpdo_word_mask(uint16_t core, uint16_t offset, uint16_t val, uint16_t mask);
int set_rxpdo_dword_mask(uint16_t core, uint16_t offset, uint32_t val, uint32_t mask);

int set_rxpdo_byte_inverse(uint16_t core, uint16_t offset, uint8_t mask);
int set_rxpdo_word_inverse(uint16_t core, uint16_t offset, uint16_t mask);
int set_rxpdo_dword_inverse(uint16_t core, uint16_t offset, uint32_t mask);

#endif