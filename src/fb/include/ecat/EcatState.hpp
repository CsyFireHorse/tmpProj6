/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-19
@ArtifactStatusAuthority GPT-5 Codex
*/
#ifndef _ZMC_ECAT_STATE_HPP_
#define _ZMC_ECAT_STATE_HPP_

#include <cstdint>

#include "common/zmc/zmc_predef.h"
#include "ecat_state_adapter.h"

namespace zmc {

constexpr uint8_t ECAT_STATE_MASK = 0x0Fu;
constexpr uint8_t ECAT_STATE_OPERATIONAL = 0x08u;
constexpr uint8_t ECAT_STATE_ERROR_FLAG = 0x10u;

// Store one complete EtherCAT state snapshot and its acquisition status.
struct EcatStateResult {
    int32_t master_query_error;
    int32_t slave_count_query_error;
    uint16_t slave_count;
    uint8_t master_state;
    uint8_t master_state_valid;
    uint8_t slave_count_valid;
    uint8_t slave_scan_complete;
    uint8_t slave_states[MAX_NUM_SLAVES];
    int32_t slave_query_errors[MAX_NUM_SLAVES];
};

// Query EtherCAT state synchronously through its dedicated adapter.
class EcatState {
   public:
    EcatState() = default;
    explicit EcatState(const EcatStateAdapter* adapter);

    // Copy callback bindings without taking ownership of their context.
    void bind_adapter(const EcatStateAdapter* adapter);
    // Clear all callback bindings.
    void clear_adapter();
    // Store the configured ENI slave count used by the slave-state scan.
    void set_eni_slave_num(uint16_t eni_slave_num);
    // Update the master and active-slave-count portions of the supplied snapshot.
    int32_t query_master(EcatStateResult* out);
    // Update only the slave portion of the supplied snapshot.
    int32_t query_slaves(EcatStateResult* out);

   private:
    EcatStateAdapter adapter_{};
    uint16_t eni_slave_num_{0u};
};

}  // namespace zmc

#endif  // _ZMC_ECAT_STATE_HPP_
