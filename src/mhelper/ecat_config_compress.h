#ifndef _ECAT_CONFIG_COMPRESS_H_
#define _ECAT_CONFIG_COMPRESS_H_

#include <stdint.h>
#include <vector>
#include <string>

namespace zmc {

// Transition state enum (uint4: 0-11)
enum class TransitionState : uint8_t {
    IP = 0,  // INIT -> PREOP
    PI = 1,  // PREOP -> INIT
    BI = 2,  // BOOT -> INIT
    SI = 3,  // SAFEOP -> INIT
    OI = 4,  // OP -> INIT
    SP = 5,  // SAFEOP -> PREOP
    OP = 6,  // PREOP -> OP
    OS = 7,  // OP -> SAFEOP
    PS = 8,  // PREOP -> SAFEOP
    SO = 9,  // SAFEOP -> OP
    IB = 10, // INIT -> BOOT
    II = 11  // INIT -> INIT
};

/*
 * ============================================================================
 * InitCmd Binary Protocol Specification
 * ============================================================================
 * 
 * This pseudo-struct describes the binary layout (NOT an actual C struct)
 * Used for documentation and implementation reference only.
 * 
 * struct InitCmdProtocol {
 *     uint8_t flags;              // Status flags (7 bits used, 1 reserved)
 *     uint8_t cmd:4;              // Command (4 bits)
 *     uint8_t transition_len:4;   // Number of transitions (4 bits, 0-15)
 *     uint8_t trans[...];         // Packed: ceil(transition_len/2) bytes
 *     uint16_t ado;               // Address offset (little-endian)
 *     
 *     // === DataLength vs Data: Mutually exclusive ===
 *     
 *     [if flags & HAS_DATA_LENGTH] {
 *         // DataLength mode: explicit uint16_t value (supports any value)
 *         uint16_t data_length;   // little-endian
 *     }
 *     [else] {  // Data mode
 *         uint8_t dataLen;        // Length descriptor (NOT a real field)
 *         
 *         [if flags & HAS_VALIDATE] {
 *             uint8_t timeout_compact;
 *             uint8_t data[dataLen];
 *             uint8_t valid_data[dataLen];
 *             [if flags & HAS_DATA_MASK] {
 *                 uint8_t valid_mask[dataLen];
 *             }
 *         } [else] {
 *             uint8_t data[dataLen];
 *         }
 *     }
 * };
 * 
 * Key Points:
 * - DataLength: Real XML field, saved as explicit uint16_t (2 bytes)
 * - Data: Real data array, needs 1 length byte + actual data bytes
 * - dataLen: NOT a field, just describes array length in protocol
 * - Requires: Uses 1 flag bit for "cycle" value (most common case)
 * - Comment: NOT saved (discarded during serialization)
 * - No MULTI_TRANS flag: transition_len directly in byte 1 (high 4 bits)
 * - No data length constraints: supports 0-255 bytes
 * - Variable length design: adapts to actual data size
 * ============================================================================
 */

// InitCmd flags bit definition (7 bits used, 1 reserved)
#define INITCMD_FLAG_BEFORE_SLAVE    0x01  // bit 0: hasBeforeSlave
#define INITCMD_FLAG_HAS_DATA_LENGTH 0x02  // bit 1: hasDataLength (else hasData)
#define INITCMD_FLAG_REQUIRES_CYCLE  0x04  // bit 2: Requires="cycle"
#define INITCMD_FLAG_ADP_1001        0x08  // bit 3: adp value (0=0, 1=1001)
#define INITCMD_FLAG_RETRIES_5000    0x10  // bit 4: retries value (0=3, 1=5000)
#define INITCMD_FLAG_HAS_VALIDATE    0x20  // bit 5: hasValidate
#define INITCMD_FLAG_HAS_DATA_MASK   0x40  // bit 6: hasDataMask (only if hasValidate)
// bit 7: reserved for future use

// ProcessImage structure (still compact, no variable fields)
#pragma pack(push, 1)
struct CompactProcessImage {
    uint16_t input_byte_size;
    uint16_t output_byte_size;
};
#pragma pack(pop)

// ============================================================================
// Variable-Length Serialization API
// ============================================================================

// Top-level API
int parse_xml_to_compact(const char* xml_file, std::vector<uint8_t>& compressed_data);
int save_compressed_config(const char* output_file, const std::vector<uint8_t>& compressed_data);
int load_compressed_config(const char* input_file, std::vector<uint8_t>& compressed_data);
int decompress_to_xml(const std::vector<uint8_t>& compressed_data, const char* output_xml);

// ============================================================================
// Utility functions
// ============================================================================

TransitionState parse_transition(const char* trans_str);
const char* transition_to_string(TransitionState state);
uint8_t parse_cmd(const char* cmd_str);

// Timeout compression: {100, 200, 3000, 5000, 10000} => {1, 2, 30, 50, 100}
uint8_t timeout_to_compact(uint16_t timeout);
uint16_t timeout_from_compact(uint8_t val);

// ============================================================================
// Friendly Structures - for XML parsing/export (using STL containers)
// ============================================================================

struct FriendlyValidateInfo {
    std::vector<uint8_t> data;
    std::vector<uint8_t> mask;  // empty if no mask
    uint16_t timeout = 0;
};

struct FriendlyInitCmd {
    std::vector<TransitionState> transitions;
    bool before_slave = false;
    std::string comment;     // Optional comment
    std::string requires;    // Optional requires
    uint8_t cmd = 0;
    uint16_t adp = 0;
    uint16_t ado = 0;
    uint16_t retries = 3;
    
    // Either data_length or data (mutually exclusive)
    bool has_data_length = false;
    uint16_t data_length = 0;
    std::vector<uint8_t> data;
    
    // Optional validate
    bool has_validate = false;
    FriendlyValidateInfo validate;
};

struct FriendlyMasterInfo {
    std::string name;
    uint8_t dest_mac[6];
    uint8_t src_mac[6];
    uint16_t ether_type = 0;
};

struct FriendlyMasterConfig {
    std::string nic;
    std::string rnic;
    uint8_t initial_state = 0;
    uint16_t cycle_time = 0;
    uint16_t mailbox_cycle_time = 0;
    uint8_t auto_recovery_timeout = 0;
    uint16_t process_image_update_period = 0;
    uint16_t statistics_update_period = 0;
    uint16_t max_frame_size = 0;
    bool separate_io_update = false;
    uint32_t dc_user_shift_time = 0;
    
    FriendlyMasterInfo info;
    std::vector<FriendlyInitCmd> init_cmds;
};

struct FriendlySlaveInfo {
    std::string name;
    std::string product_revision;
    uint16_t phys_addr = 0;
    uint16_t auto_inc_addr = 0;
    std::string physics;  // Can be multi-char like "YY"
    uint32_t vendor_id = 0;
    uint32_t product_code = 0;
    uint32_t revision_no = 0;
    uint32_t serial_no = 0;
};

struct FriendlySlaveConfig {
    std::string physics;  // Can be multi-char like "YY"
    bool hot_plug = false;
    uint8_t segment = 0;
    uint8_t segment_sub_index = 0;
    
    FriendlySlaveInfo info;
    std::vector<FriendlyInitCmd> init_cmds;
};

struct FriendlyEtherCATConfig {
    std::string crc;
    std::vector<FriendlyMasterConfig> masters;
    std::vector<FriendlySlaveConfig> slaves;
    CompactProcessImage process_image;
};

// ============================================================================
// Friendly Structure APIs - XML parsing/export
// ============================================================================

// Parse XML to friendly structure
int parse_xml_to_friendly(const char* xml_file, FriendlyEtherCATConfig& config);

// Export friendly structure to XML
int export_friendly_to_xml(const FriendlyEtherCATConfig& config, const char* output_xml);

// Test XML roundtrip (parse -> export -> parse -> compare)
int test_xml_roundtrip(const char* input_xml, const char* output_xml);


// InitCmd serialization
size_t serialize_init_cmd(const FriendlyInitCmd& cmd, std::vector<uint8_t>& buffer);
bool deserialize_init_cmd(const uint8_t* data, size_t len, size_t& offset, FriendlyInitCmd& cmd);

// Master serialization
size_t serialize_master(const FriendlyMasterConfig& master, std::vector<uint8_t>& buffer);
bool deserialize_master(const uint8_t* data, size_t len, size_t& offset, FriendlyMasterConfig& master);

// Slave serialization
size_t serialize_slave(const FriendlySlaveConfig& slave, std::vector<uint8_t>& buffer);
bool deserialize_slave(const uint8_t* data, size_t len, size_t& offset, FriendlySlaveConfig& slave);

std::string auto_generate_eni_from_embedded(const char* filename = nullptr);
} // namespace zmc

#endif // _ECAT_CONFIG_COMPRESS_H_
