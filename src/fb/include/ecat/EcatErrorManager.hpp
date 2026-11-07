/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-19
@ArtifactStatusAuthority GPT-5 Codex
*/
#ifndef _ZMC_ECAT_ERROR_MANAGER_HPP_
#define _ZMC_ECAT_ERROR_MANAGER_HPP_

#include "bridge/AxisFbBinding.hpp"
#include "ecat/EcatState.hpp"

namespace zmc {

// Own the Bridge-local EtherCAT state producer and consume its state snapshots.
class EcatErrorManager {
   public:
    EcatErrorManager() = default;

    // Bind the bridge-owned axis references and axis-to-slave mapping.
    void bind(const AxisFbBinding& binding);
    // Bind callbacks without querying EtherCAT state on the binding thread.
    int bind_adapter(const EcatStateAdapter* adapter);
    // Clear the adapter, snapshot, and pending initial master check.
    void reset();

    // Run the master and slave checks in PDO-cycle order.
    void update();
    // Apply the current slave-derived errors to bound physical axes.
    void apply() const;
    // Check master state once on the first PDO cycle after binding.
    void check_master_state();
    // Refresh the complete slave portion of the current state snapshot.
    void check_slave_states();
    // Apply only slave-derived communication errors to bound physical axes.
    void apply_slave_errors() const;

   private:
    // Drop all collected state.
    void clear_state();

    EcatState ecat_state_;
    EcatStateResult state_result_{};
    bool adapter_bound_{false};
    bool master_check_pending_{false};
    AxisFbBinding binding_{};
};

}  // namespace zmc

#endif  // _ZMC_ECAT_ERROR_MANAGER_HPP_
