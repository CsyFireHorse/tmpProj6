#ifndef _ECAT_PDOIO_WRAP_H_
#define _ECAT_PDOIO_WRAP_H_

#include "stdint.h"


/*  Initialize buffer sizes.
 *
 *  core    master and CPU indices
 *  tx_sz   TxPDO size
 *  rx_sz   RxPDO size
 */
int init_txrxpdo_data(uint16_t core, uint16_t tx_sz, uint16_t rx_sz);
void* get_txrxpdo(uint16_t core);


/* Read TxPDO input.
 * core     \param[in]      master and CPU indices
 * offset   \param[in]      PDO offset
 * size     \param[in]      number of bytes to read
 * val      \param[out]     value read
 * \return                  error code
 */
int get_txpdo_data(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val);
// Read RxPDO input.
int get_rxpdo_data(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val);

/* Write RxPDO.
 * core     \param[in]      master and CPU indices
 * offset   \param[in]      PDO offset
 * size     \param[in]      number of bytes to write
 * val      \param[in]      value to write
 * \return                  error code
 */
int set_rxpdo_data(uint16_t core, uint16_t offset, uint16_t size, const uint8_t* val);

// Combinations: { get_txpdo, get_rxpdo, set_rxpdo, set_rxpdo_inverse } * { data, mask_data, mask_byte, mask_word, mask_dword }.

int get_rxpdo_data_mask(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val, const uint8_t* mask);
int get_txpdo_data_mask(uint16_t core, uint16_t offset, uint16_t size, uint8_t* val, const uint8_t* mask);

// Set the bit selected by mask to the specified value.
int set_rxpdo_data_mask(uint16_t core, uint16_t offset, uint16_t size, const uint8_t* val, const uint8_t* mask);

// Toggle the bit selected by the inversion mask.
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
