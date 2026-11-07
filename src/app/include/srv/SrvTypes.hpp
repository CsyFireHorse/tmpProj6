#ifndef ZMC_SRV_TYPES_HPP_
#define ZMC_SRV_TYPES_HPP_

#include <string>
#include <vector>
#include "rpc/MotionRpcTypes.h"
#include "motbuf/MotionCmdTypes.hpp"
#include "io/IoMetadata.hpp"
// Model signature: GPT-6.
namespace zmc {
namespace srv {

// Request-local value snapshots; no runtime object or protocol library is exposed.
struct SrvIdentity {
    std::string version, author, description, code_date, device_name, channel_name;
    std::string eni_name, ini_name, controller_id;
    std::string eni_hash, ini_hash;
    std::string selected_eni_path, selected_eni_hash;
    std::string selected_ini_path, selected_ini_hash;
    std::vector<std::string> available_channel_names;
    bool device_open, channel_open, ecat_running;
};
struct SrvDeviceEntry {
    std::string device, name;
};
struct SrvFileEntry {
    std::string name;
    uint64_t size, time;
    std::string hash;  // Lowercase FNV-1a 128-bit hash of the file contents.
};
struct SrvEcatStatus {
    bool bus_is_running;
    uint8_t master_req_state, master_state;
    bool pi_enable;
    uint16_t slave_count, active_slave_count;
};
struct SrvAxisSnapshot {
    RpcAxisState2 state;
    bool is_extended;
};
struct SrvDashboard {
    std::vector<SrvAxisSnapshot> axes;
    RpcFlowCtrlStatus flow_status;
    std::vector<RpcCmdResultInfo> cmd_results;
};
struct SrvLatestStatus {
    bool has_axis;
    RpcAxisState axis;
    std::vector<RpcCmdResultInfo> cmd_results;
};
}  // namespace srv
}  // namespace zmc
#endif
