/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#include "fb/smc_parameter_utils.h"

/**
 * @brief Converts CANopen object index/subindex pair to SoftMotion parameter number
 * 
 * Formula: ParameterNumber = DWORD_TO_DINT(
 *     SHL(USINT_TO_DWORD(usiDataLength), 24) +
 *     SHL(UINT_TO_DWORD(uiIndex), 8) +
 *     usiSubIndex
 * )
 */
int32_t zmc_parameter_number_coe(uint16_t index, uint8_t subIndex, uint8_t dataLength) {
    // Convert types to uint32_t for calculation
    uint32_t dataLengthDword = (uint32_t)dataLength;
    uint32_t indexDword = (uint32_t)index;
    uint32_t subIndexDword = (uint32_t)subIndex;
    
    // Calculate according to formula
    // SHL(dataLength, 24) + SHL(index, 8) + subIndex
    uint32_t result = (dataLengthDword << 24) + (indexDword << 8) + subIndexDword;
    
    // Convert to int32_t
    return (int32_t)result;
}

/**
 * @brief Decodes a CoE-encoded SoftMotion parameter number back into its parts
 *
 * The layout mirrors zmc_parameter_number_coe: dataLength in the highest byte,
 * index in bits 8..23 and subIndex in the lowest byte. This is the single
 * validation authority for the CoE encoding: a number only decodes when the
 * highest byte is nonzero AND names a usable expedited length (1, 2 or 4).
 */
int32_t zmc_parameter_coe_decode(int32_t paraNo, uint16_t* index, uint8_t* subIndex,
                                 uint8_t* dataLength) {
    // Output pointers are required to carry the decoded fields
    if (!index || !subIndex || !dataLength) {
        return 0;
    }
    const uint32_t enc = (uint32_t)paraNo;
    const uint8_t len = (uint8_t)(enc >> 24);
    if (len != 1u && len != 2u && len != 4u) {
        return 0;  // Not a CoE number with a usable expedited payload length
    }
    // Split the encoded layout into its three fields
    *dataLength = len;
    *index = (uint16_t)((enc >> 8) & 0xFFFFu);
    *subIndex = (uint8_t)(enc & 0xFFu);
    return 1;
}

/**
 * @brief Converts SoE object (IDN + element) to SoftMotion parameter number
 * 
 * For SoE, we use the same formula but with IDN as the index and element as subIndex
 */
int32_t zmc_parameter_number_soe(uint16_t IDN, uint8_t element, uint8_t dataLength) {
    // Convert types to uint32_t for calculation
    uint32_t dataLengthDword = (uint32_t)dataLength;
    uint32_t idnDword = (uint32_t)IDN;
    uint32_t elementDword = (uint32_t)element;
    
    // Calculate according to formula
    // SHL(dataLength, 24) + SHL(IDN, 8) + element
    uint32_t result = (dataLengthDword << 24) + (idnDword << 8) + elementDword;
    
    // Convert to int32_t
    return (int32_t)result;
}

double zmc_braking_distance(double v0, double a0, double deceleration, double jerk){
    // FIXME: jerk is not used
    if (v0 > 1e-6){
        return v0*v0/2/deceleration;
    }else if(v0 < -1e-6){
        return -v0*v0/2/deceleration;
    }else{
        return 0;
    }
}