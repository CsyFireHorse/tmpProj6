#ifndef _SMC_PARAMETER_UTILS_H_
#define _SMC_PARAMETER_UTILS_H_

#include <stdint.h>
#include "common/zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Converts CANopen object index/subindex pair to SoftMotion parameter number
 * 
 * This function translates from a CANopen object index/subindex pair to a SoftMotion 
 * parameter number that can be used with function blocks like MC_ReadParameter, 
 * MC_ReadBoolParameter, MC_WriteParameter, and MC_WriteBoolParameter.
 * 
 * Formula: ParameterNumber = DWORD_TO_DINT(
 *     SHL(USINT_TO_DWORD(usiDataLength), 24) +
 *     SHL(UINT_TO_DWORD(uiIndex), 8) +
 *     usiSubIndex
 * )
 * 
 * @param index The object index, e.g. 0x6061
 * @param subIndex The object subindex, e.g. 0
 * @param dataLength The data length in bytes (1, 2, or 4). Only used when writing a parameter.
 * @return int32_t The SoftMotion parameter number
 */
ZMC_DLLPORT int32_t zmc_parameter_number_coe(uint16_t index, uint8_t subIndex, uint8_t dataLength);

/**
 * @brief Decodes a SoftMotion parameter number produced by zmc_parameter_number_coe
 *
 * This is the single validation authority for the CoE encoding: a number only
 * decodes when the highest byte is nonzero AND names a usable expedited
 * payload length (1, 2 or 4). Callers must not re-validate the length rule.
 *
 * @param paraNo The parameter number to decode
 * @param index Out: the object index
 * @param subIndex Out: the object subindex
 * @param dataLength Out: the validated payload length (1, 2 or 4)
 * @return 1 when paraNo is a valid CoE encoded number, 0 otherwise
 */
ZMC_DLLPORT int32_t zmc_parameter_coe_decode(int32_t paraNo, uint16_t* index,
                                             uint8_t* subIndex, uint8_t* dataLength);

/**
 * @brief Converts SoE object (IDN + element) to SoftMotion parameter number
 * 
 * This function translates from a SoE (servodrive over EtherCAT) object (IDN + element) 
 * to a SoftMotion parameter number that can be used with function blocks like 
 * MC_ReadParameter, MC_ReadBoolParameter, MC_WriteParameter, and MC_WriteBoolParameter.
 * 
 * Formula: ParameterNumber = DWORD_TO_DINT(
 *     SHL(USINT_TO_DWORD(usiDataLength), 24) +
 *     SHL(UINT_TO_DWORD(IDN), 8) +
 *     element
 * )
 * 
 * @param IDN The IDN
 * @param element 2: Name, 3: Attribute, 4: Unit, 5: Minimum, 6: Maximum, 0 or 7: Value
 * @param dataLength The data length in bytes (1, 2, or 4). Only used when writing a parameter.
 * @return int32_t The SoftMotion parameter number
 */
ZMC_DLLPORT int32_t zmc_parameter_number_soe(uint16_t IDN, uint8_t element, uint8_t dataLength);

ZMC_DLLPORT double zmc_braking_distance(double v0, double a0, double deceleration, double jerk);


#ifdef __cplusplus
}
#endif

#endif // _SMC_PARAMETER_UTILS_H_ 