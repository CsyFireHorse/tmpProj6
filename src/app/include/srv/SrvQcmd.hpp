#ifndef ZMC_SRV_QCMD_HPP_
#define ZMC_SRV_QCMD_HPP_

#include "SrvFileService.hpp"
// AI modification: GPT-6 (Codex), 2026-10-26. Sync implementation belongs to qcmd.
// Model signature: GPT-6.
// Parameter mirror getters added by GPT-6 Codex, 2026-10-12.
namespace zmc {
namespace srv {
class SrvQcmd : public SrvFileService {
   public:
    // Wait for a motion-cycle confirmation of one no-op command.
    int ping(RpcSession s, uint32_t timeout_ms);
    // Select and submit as one serialized request; do not ignore selection errors.
    int poweron(RpcSession s, int32_t axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int poweroff(RpcSession s, int32_t axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int axisenable(RpcSession s, int32_t axis, bool enable);
    // Select and submit as one serialized request; do not ignore selection errors.
    // Units number/denominator setter; the wire method stays "setunits".
    // GLM-5.3 (opencode), 2026-10-23.
    int setunits_num_den(RpcSession s, int32_t axis, int32_t num, int32_t den);
    // Read the shared parameter mirror without submitting a query.
    int getunits(RpcSession s, int32_t axis, double& units);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setspeed(RpcSession s, int32_t axis, double speed);
    int getspeed(RpcSession s, int32_t axis, double& speed);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setaccel(RpcSession s, int32_t axis, double accel);
    int getaccel(RpcSession s, int32_t axis, double& accel);
    int getdecel(RpcSession s, int32_t axis, double& decel);
    int getjerk(RpcSession s, int32_t axis, double& jerk);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setdecel(RpcSession s, int32_t axis, double decel);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setjerk(RpcSession s, int32_t axis, double jerk);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setfastdecel(RpcSession s, int32_t axis, double value);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setspeedratio(RpcSession s, int32_t axis, double ratio);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setdirection(RpcSession s, int32_t axis, int32_t dir);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setbuffermode(RpcSession s, int32_t axis, int32_t mode);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setmasteraxis(RpcSession s, int32_t axis, int32_t master_axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int stop(RpcSession s, int32_t axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int halt(RpcSession s, int32_t axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int reset(RpcSession s, int32_t axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int movesgabs(RpcSession s, int32_t axis, double pos);
    // Select and submit as one serialized request; do not ignore selection errors.
    int movesgrel(RpcSession s, int32_t axis, double pos);
    // Select and submit as one serialized request; do not ignore selection errors.
    int moveadd(RpcSession s, int32_t axis, double pos);
    // Select and submit as one serialized request; do not ignore selection errors.
    int home(RpcSession s, int32_t axis, double pos);
    // Select and submit as one serialized request; do not ignore selection errors.
    int homing(RpcSession s, int32_t axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int gearin(RpcSession s, int32_t axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int gearout(RpcSession s, int32_t axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int setposition(RpcSession s, int32_t axis, double pos);
    // Select and submit as one serialized request; do not ignore selection errors.
    int datum(RpcSession s, int32_t axis, int32_t mode);
    // Select and submit as one serialized request; do not ignore selection errors.
    int movevelocity(RpcSession s, int32_t axis, int32_t dir);
    // Select and submit as one serialized request; do not ignore selection errors.
    int forward(RpcSession s, int32_t axis);
    // Select and submit as one serialized request; do not ignore selection errors.
    int reverse(RpcSession s, int32_t axis);
    // Start SDO-configured homing on the explicit target axis.
    int home_start(RpcSession s, int32_t axis, const RpcHomeSdoConfig& cfg);
    // Return the established RPC home-status numbering.
    int home_get_status(RpcSession s, int32_t axis, int32_t& status, int32_t& error_id);
    // Preserve explicit base selection for existing script semantics.
    int base(RpcSession s, int32_t axis);
    // Select the supplied axes before submitting the group move.
    int moveabs(RpcSession s, int32_t len, const int32_t* axes, const double* pos);
    // Select the supplied axes before submitting the group move.
    int moverel(RpcSession s, int32_t len, const int32_t* axes, const double* pos);
    // Select the supplied two axes before submitting the circular move.
    int movecirc(RpcSession s, int32_t len, const int32_t* axes, double end1, double end2,
                 double centre1, double centre2, int32_t direction);
    // Select the supplied two axes before submitting the absolute circular move.
    int movecircabs(RpcSession s, int32_t len, const int32_t* axes, double end1, double end2,
                    double centre1, double centre2, int32_t direction);
    // Select the supplied two axes before submitting the three-point circular move.
    int movecirc2(RpcSession s, int32_t len, const int32_t* axes, double mid1, double mid2,
                  double end1, double end2);
    // Select the supplied two axes before submitting the absolute three-point circular move.
    int movecirc2abs(RpcSession s, int32_t len, const int32_t* axes, double mid1, double mid2,
                     double end1, double end2);
    // Reuse the existing typed command or script path.
    int groupselect(RpcSession s, int32_t index);
    // Reuse the existing typed command or script path.
    int groupsetaxes(RpcSession s, int32_t len, const int32_t* axes);
    // Submit the supplied axes before the group control command.
    int groupenable(RpcSession s, int32_t len, const int32_t* axes);
    // Submit the supplied axes before the group control command.
    int groupdisable(RpcSession s, int32_t len, const int32_t* axes);
    // Submit the supplied axes before the group control command.
    int groupreset(RpcSession s, int32_t len, const int32_t* axes);
    // Reuse the existing typed command or script path.
    int waitticks(RpcSession s, uint32_t ticks, int32_t axis, uint32_t timeout_ms,
                  uint32_t poll_ms);
    // Reuse the existing typed command or script path.
    int waitseconds(RpcSession s, double seconds, int32_t axis, uint32_t timeout_ms,
                    uint32_t poll_ms);
    // Block the caller on the condition variable until the axis reaches standstill or ErrorStop.
    int waitaxisidle(RpcSession s, int32_t axis, uint32_t timeout_ms);
    // Reuse the existing typed command or script path.
    int executecommand(RpcSession s, const std::string& text);
    // Reuse the existing typed command or script path.
    int flush(RpcSession s);
    // Configure the shared timeout for synchronous numeric and IO reads.
    int set_sync_read_timeout(RpcSession s, uint32_t timeout_ms);
    // Return the configured timeout for synchronous numeric and IO reads.
    int get_sync_read_timeout(RpcSession s, uint32_t& timeout_ms);
    // Return one numeric parameter through the existing synchronous wait. GPT-6 Codex.
    int read_parameter(RpcSession s, int32_t axis, int32_t parameter_number, double& value);
    // Return the existing motion-cycle write confirmation. GPT-6 Codex.
    int write_parameter(RpcSession s, int32_t axis, int32_t parameter_number, double value);
    // Submit the existing asynchronous IO command.
    int read_di(RpcSession s, uint32_t io);
    // Return the current value after a directly submitted motion-cycle read.
    int read_di_sync(RpcSession s, uint32_t io, bool& value);
    // Submit the existing asynchronous IO command.
    int read_di_multi(RpcSession s, uint32_t first, uint32_t last);
    // Submit the existing asynchronous IO command.
    int read_do(RpcSession s, uint32_t io);
    // Return the retained value after a directly submitted motion-cycle read.
    int read_do_sync(RpcSession s, uint32_t io, bool& value);
    // Submit the existing asynchronous IO command.
    int read_do_multi(RpcSession s, uint32_t first, uint32_t last);
    // Submit the existing asynchronous IO command.
    int write_do(RpcSession s, uint32_t io, bool value);
    // Submit the existing asynchronous IO command.
    int write_do_multi(RpcSession s, uint32_t first, uint32_t last, uint32_t value);
    // Submit the existing asynchronous IO command.
    int read_ai(RpcSession s, uint32_t io);
    // Return the unscaled value after a directly submitted motion-cycle read.
    int read_ai_sync(RpcSession s, uint32_t io, uint32_t& value);
    // Submit the existing asynchronous IO command.
    int read_ao(RpcSession s, uint32_t io);
    // Return the retained unscaled value after a directly submitted motion-cycle read.
    int read_ao_sync(RpcSession s, uint32_t io, uint32_t& value);
    // Submit the existing asynchronous IO command.
    int write_ao(RpcSession s, uint32_t io, uint32_t value);
    // Submit the existing asynchronous IO command.
    int write_virtual_di(RpcSession s, uint32_t io, bool value);
    // Submit the existing asynchronous IO command.
    int write_virtual_ai(RpcSession s, uint32_t io, uint32_t value);
    // Preserve each transport's existing refresh and error contract.
    int getaxisstate(RpcSession s, int32_t axis, RpcAxisState& out, bool refresh);
    // Preserve each transport's existing refresh and error contract.
    int getaxisstate2(RpcSession s, int32_t axis, RpcAxisState2& out, bool refresh);
    // Copy the existing persistent command result snapshot.
    int getcmdresults(RpcSession s, std::vector<RpcCmdResultInfo>& out, bool refresh);
    // Read flow status without introducing another cache.
    int getflowstatus(RpcSession s, RpcFlowCtrlStatus& out, bool refresh);
    // Read the existing flow configuration.
    int getflowconfig(RpcSession s, RpcFlowCtrlConfig& out);
    // Set the existing flow configuration.
    int setflowconfig(RpcSession s, const RpcFlowCtrlConfig& cfg);
    // Read the persistent IO result snapshot.
    int io_results(RpcSession s, std::vector<CmdResult>& out);
    // Clear only the existing requested mirror scope.
    int dashboard_clear(RpcSession s);
    // Clear only the existing requested mirror scope.
    int cache_reset(RpcSession s);

   private:
    // Reuse service serialization and qcmd's parameter decoding.
    int getAxisParameter(RpcSession s, int32_t axis, uint16_t type, double& value);
    // Run one command-service iteration from the singleton worker.
    int runOnce(size_t devIdx);
    friend class MotSrv;
    friend class SrvCommand;
    // Construct the qcmd layer.
    SrvQcmd();
    // Release the qcmd layer.
    ~SrvQcmd();
};
}  // namespace srv
}  // namespace zmc
#endif
