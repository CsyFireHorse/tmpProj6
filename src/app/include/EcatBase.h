#ifndef _ECAT_BASE_H_
#define _ECAT_BASE_H_


#include "stdint.h"
#include "stdio.h"
#include <string.h>
#include <string>
#include "common/ecat/servo_word.h"
#include "common/ecat/ecat_print.h"

typedef int ECAT_RESULT;
typedef int (*EcatExtCtrlTaskHandler)(void*, uint8_t*, uint8_t*);

typedef struct ECAT_CONTEXT_T {
    void* Master;
    void* Server;
    char* NIC0;
    char* NIC1;
    char* ENI;
    uint32_t MainCycleTimeUs;
    uint32_t SubCycleTimeUs;
    uint32_t AutoRecoveryTimeoutMs;
    uint32_t RpcServerPort;
} ECAT_CONTEXT;

class EcatBase{
public:
    EcatBase(const char* eth=nullptr);
    EcatBase(const std::string& ecat_name);
    ~EcatBase();
    int set_eni(const char* eni);
    void use_default_cfg();
    int simple_start();
    int simple_start_wrap();
    int start();
    int stop();
    bool request_state(int stat, int retry=30);
    // Request a master state transition.
    int request_master_state(uint8_t state);
    // Query whether the master is running according to its current state.
    int bus_is_running(bool* running);
    // Query the last successfully requested master state.
    int get_master_req_state(uint8_t* state) const;
    // Query current EtherCAT master state (see EcatState).
    int get_master_state(uint8_t* state);
    // Query configured and active EtherCAT slave counts.
    int get_slave_count(uint16_t* count);
    int get_active_slave_count(uint16_t* count);
    // Query one EtherCAT slave state by its configured position.
    int get_slave_state(uint16_t slave_id, uint8_t* state);
    // Clear the EtherCAT slave error flag by its configured position.
    int clear_slave_state_error(uint16_t slave_id);
    // Store the PI enable state without touching the underlying PI exchange.
    int pi_enable_set(bool enable);
    int pi_enable_get(bool* enable) const;
    // Transfer raw CoE SDO bytes through the current master session.
    int coe_sdo_download(uint16_t slave_id, uint16_t index, uint8_t subindex,
                         uint8_t complete_access, const void* data, uint32_t data_len,
                         uint32_t timeout_ms);
    int coe_sdo_upload(uint16_t slave_id, uint16_t index, uint8_t subindex,
                       uint8_t complete_access, uint32_t* data_len, void* data,
                       uint32_t timeout_ms);
    // Set the PDO callback.
    int bind(EcatExtCtrlTaskHandler cb, void* obj);
    void unbind();
    int query_by_sdo(uint32_t slave, uint32_t index, uint8_t subindex, uint32_t len, int32_t* val);
    int write_by_sdo(uint32_t slave, uint32_t index, uint8_t subindex, uint32_t len, int32_t val);
    int set_opmode_by_sdo(int num_servos, uint32_t op_mode);
    ECAT_CONTEXT ctx;

private:
    bool pi_enabled_ = false;
    uint8_t master_req_state_ = 0;
    bool master_req_state_valid_ = false;
};

// bool ecat_context_request_state(ECAT_SIMPLE_START_CONTEXT *ctx, ECAT_BYTE state, int retry);
// int query_by_sdo(ECAT_SIMPLE_START_CONTEXT* ctx, uint32_t slave, uint32_t index, uint8_t subindex, uint32_t len, int32_t* val);
// int write_by_sdo(ECAT_SIMPLE_START_CONTEXT* ctx, uint32_t slave, uint32_t index, uint8_t subindex, uint32_t len, int32_t val);
// ECAT_RESULT servo_set_opmode_by_sdo(ECAT_SIMPLE_START_CONTEXT& ctx, int _num_servos, uint32_t op_mode);

/*----------------------------------------------------------------------------*/
/**
  \brief Initialize EtherCAT environment variables.
*/
/*----------------------------------------------------------------------------*/
class ethercat_env
{
public:
    ethercat_env();
    ~ethercat_env();
};

#endif
