#ifndef ZMC_SRV_COMMAND_HPP_
#define ZMC_SRV_COMMAND_HPP_

#include "SrvQcmd.hpp"
// Model signature: GPT-6.
namespace zmc {
namespace srv {
class SrvCommand : public SrvQcmd {
   public:
    // Select the configuration using the owning motion service.
    int eni_select(RpcSession s, const std::string& path);
    // Select the configuration using the owning motion service.
    int ini_select(RpcSession s, const std::string& path);
    // Start and bind PDO in one critical section; roll back a failed binding.
    int startup(RpcSession s, const std::string& path);
    // Start with explicit ENI and INI paths without changing selected candidates.
    // GPT-6 (Codex), 2026-10-29.
    int startup(RpcSession s, const std::string& eni_path, const std::string& ini_path);
    // Start and bind PDO in one critical section; roll back a failed binding.
    int startup_ini(RpcSession s, const std::string& path);
    // Start and bind PDO in one critical section; roll back a failed binding.
    int ecat_start(RpcSession s);
    // Stop the bus and cancel pending commands, wait gates and cached status.
    int ecat_stop(RpcSession s);
    // Use RPC integer/size semantics with complete access zero and timeout 1000 ms.
    int ecat_coe_sdo_download(RpcSession s, uint16_t slave_id, uint16_t index, uint8_t subindex,
                              uint32_t size, uint32_t data);
    // Use RPC integer/size semantics with complete access zero and timeout 1000 ms.
    int ecat_coe_sdo_upload(RpcSession s, uint16_t slave_id, uint16_t index, uint8_t subindex,
                            uint32_t size, uint32_t& data);
    // Read configuration content through its owning service.
    int eni_json(RpcSession s, std::string& text);
    // Read configuration content through its owning service.
    int eni_download(RpcSession s, std::string& text);
    // Read configuration content through its owning service.
    int ini_content(RpcSession s, std::string& text);
    // Return value metadata without exposing the IO runtime.
    int io_metadata(RpcSession s, std::vector<IoMetadataEntry>& out, bool& layout_ready);
    // Return the existing closed-device setup snapshot.
    int session_status(RpcSession s, SrvIdentity& out);
    // Read the aggregate bus snapshot without nested service locking.
    int ecat_status(RpcSession s, SrvEcatStatus& out);
    // Preserve HTTP nullable-axis and empty-result aggregation semantics.
    int status_latest(RpcSession s, int32_t axis, SrvLatestStatus& out);
    // Capture one mirror snapshot; formatting stays in the HTTP adapter.
    int dashboard(RpcSession s, const std::vector<int32_t>& axes, bool include_flow,
                  bool include_results, SrvDashboard& out);

   private:
    // Construct only through the derived service.
    SrvCommand();
    // Release the complete service after adapter threads finish.
    ~SrvCommand();

   private:
    friend class MotSrv;
    // Share startup and PDO rollback under the existing command lock.
    int start(RpcSession s, const std::string& path, bool use_ini,
              const std::string& ini_path = std::string());
};
}  // namespace srv
}  // namespace zmc
#endif
