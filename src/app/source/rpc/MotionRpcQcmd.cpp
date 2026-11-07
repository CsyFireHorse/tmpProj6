#include "rpc/MotRpcQcmd.hpp"
#include "srv/MotSrv.hpp"
#include "rpc/MotionRpcWireTypes.h"
#include "rpc/server.h"
#include <tuple>
namespace zmc {
// RPC adapters only preserve wire shapes and call business methods. Model signature: GPT-6.
// Parameter getters added by GPT-6 Codex, 2026-10-12.
MotRpcQcmd::MotRpcQcmd(const std::string& host, int port) : MotRpcFile(host, port) {}

void MotRpcQcmd::bindQcmd() {
    // Numeric reads use the existing service confirmation path. GPT-6 Codex.
    server_->bind("read_parameter", [this](RpcSession s, int32_t axis, int32_t parameter_number) {
        double value = 0;
        const int ret =
            service_->read_parameter(s, axis, parameter_number, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    // Forward the write's execution ACK without adding transport state. GPT-6 Codex.
    server_->bind("write_parameter", [this](RpcSession s, int32_t axis, int32_t parameter_number,
                                       double value) {
        return static_cast<int32_t>(service_->write_parameter(
            s, axis, parameter_number, value));
    });
    server_->bind("read_di", [this](RpcSession s, uint32_t io) {
        bool value = false;
        const int ret = service_->read_di_sync(s, io, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    server_->bind("read_do", [this](RpcSession s, uint32_t io) {
        bool value = false;
        const int ret = service_->read_do_sync(s, io, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    server_->bind("read_ai", [this](RpcSession s, uint32_t io) {
        uint32_t value = 0;
        const int ret = service_->read_ai_sync(s, io, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    server_->bind("read_ao", [this](RpcSession s, uint32_t io) {
        uint32_t value = 0;
        const int ret = service_->read_ao_sync(s, io, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    server_->bind("write_do", [this](RpcSession s, uint32_t io, bool value) {
        return static_cast<int32_t>(service_->write_do(s, io, value));
    });
    server_->bind("write_ao", [this](RpcSession s, uint32_t io, uint32_t value) {
        return static_cast<int32_t>(service_->write_ao(s, io, value));
    });
    server_->bind("write_virtual_di", [this](RpcSession s, uint32_t io, bool value) {
        return static_cast<int32_t>(service_->write_virtual_di(s, io, value));
    });
    server_->bind("write_virtual_ai", [this](RpcSession s, uint32_t io, uint32_t value) {
        return static_cast<int32_t>(service_->write_virtual_ai(s, io, value));
    });
    server_->bind("getiometadata", [this](RpcSession s) {
        std::vector<IoMetadataEntry> entries;
        std::vector<RpcIoMetadataWire> out;
        bool layout_ready = false;
        const int ret = service_->io_metadata(s, entries, layout_ready);
        if (ret == 0) {
            out.resize(entries.size());
            for (size_t i = 0; i < entries.size(); ++i) {
                out[i].mask = entries[i].mask;
                out[i].io_number = entries[i].io_number;
                out[i].index = entries[i].index;
                out[i].bit_offset = entries[i].bit_offset;
                out[i].io_type = static_cast<uint8_t>(entries[i].kind);
                out[i].source_kind = static_cast<uint8_t>(entries[i].source_kind);
                out[i].slave_idx = entries[i].slave_idx;
                out[i].subindex = entries[i].subindex;
                out[i].bit_size = entries[i].bit_size;
                out[i].is_input = entries[i].is_input ? 1U : 0U;
            }
        }
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("ping", [this](RpcSession s, uint32_t timeout_ms) {
        return static_cast<int32_t>(service_->ping(s, timeout_ms));
    });
    server_->bind("poweron", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->poweron(s, axis));
    });
    server_->bind("poweroff", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->poweroff(s, axis));
    });
    server_->bind("axisenable", [this](RpcSession s, int32_t axis, bool enable) {
        return static_cast<int32_t>(
            service_->axisenable(s, axis, enable));
    });
    // Units carry the explicit num/den ratio on the wire. GLM-5.3 (opencode).
    server_->bind("setunits", [this](RpcSession s, int32_t axis, int32_t num, int32_t den) {
        return static_cast<int32_t>(
            service_->setunits_num_den(s, axis, num, den));
    });
    server_->bind("getunits", [this](RpcSession s, int32_t axis) {
        double value = 0;
        const int ret = service_->getunits(s, axis, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    server_->bind("setspeed", [this](RpcSession s, int32_t axis, double speed) {
        return static_cast<int32_t>(service_->setspeed(s, axis, speed));
    });
    server_->bind("getspeed", [this](RpcSession s, int32_t axis) {
        double value = 0;
        const int ret = service_->getspeed(s, axis, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    server_->bind("setaccel", [this](RpcSession s, int32_t axis, double accel) {
        return static_cast<int32_t>(service_->setaccel(s, axis, accel));
    });
    server_->bind("getaccel", [this](RpcSession s, int32_t axis) {
        double value = 0;
        const int ret = service_->getaccel(s, axis, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    server_->bind("getdecel", [this](RpcSession s, int32_t axis) {
        double value = 0;
        const int ret = service_->getdecel(s, axis, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    server_->bind("getjerk", [this](RpcSession s, int32_t axis) {
        double value = 0;
        const int ret = service_->getjerk(s, axis, value);
        return std::make_tuple(static_cast<int32_t>(ret), value);
    });
    server_->bind("setdecel", [this](RpcSession s, int32_t axis, double decel) {
        return static_cast<int32_t>(service_->setdecel(s, axis, decel));
    });
    server_->bind("setjerk", [this](RpcSession s, int32_t axis, double jerk) {
        return static_cast<int32_t>(service_->setjerk(s, axis, jerk));
    });
    server_->bind("setfastdecel", [this](RpcSession s, int32_t axis, double value) {
        return static_cast<int32_t>(
            service_->setfastdecel(s, axis, value));
    });
    server_->bind("setspeedratio", [this](RpcSession s, int32_t axis, double ratio) {
        return static_cast<int32_t>(
            service_->setspeedratio(s, axis, ratio));
    });
    server_->bind("setdirection", [this](RpcSession s, int32_t axis, int32_t dir) {
        return static_cast<int32_t>(
            service_->setdirection(s, axis, dir));
    });
    server_->bind("setbuffermode", [this](RpcSession s, int32_t axis, int32_t mode) {
        return static_cast<int32_t>(
            service_->setbuffermode(s, axis, mode));
    });
    server_->bind("setmasteraxis", [this](RpcSession s, int32_t axis, int32_t master_axis) {
        return static_cast<int32_t>(
            service_->setmasteraxis(s, axis, master_axis));
    });
    server_->bind("datum", [this](RpcSession s, int32_t axis, int32_t mode) {
        return static_cast<int32_t>(service_->datum(s, axis, mode));
    });
    server_->bind("movevelocity", [this](RpcSession s, int32_t axis, int32_t dir) {
        return static_cast<int32_t>(
            service_->movevelocity(s, axis, dir));
    });
    server_->bind("stop", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->stop(s, axis));
    });
    server_->bind("home_stop", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->stop(s, axis));
    });
    server_->bind("halt", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->halt(s, axis));
    });
    server_->bind("reset", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->reset(s, axis));
    });
    server_->bind("homing", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->homing(s, axis));
    });
    server_->bind("gearin", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->gearin(s, axis));
    });
    server_->bind("gearout", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->gearout(s, axis));
    });
    server_->bind("forward", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->forward(s, axis));
    });
    server_->bind("reverse", [this](RpcSession s, int32_t axis) {
        return static_cast<int32_t>(service_->reverse(s, axis));
    });
    server_->bind("movesgabs", [this](RpcSession s, int32_t axis, double pos) {
        return static_cast<int32_t>(service_->movesgabs(s, axis, pos));
    });
    server_->bind("movesgrel", [this](RpcSession s, int32_t axis, double pos) {
        return static_cast<int32_t>(service_->movesgrel(s, axis, pos));
    });
    server_->bind("movesg", [this](RpcSession s, int32_t axis, double pos) {
        return static_cast<int32_t>(service_->movesgrel(s, axis, pos));
    });
    server_->bind("moveadd", [this](RpcSession s, int32_t axis, double pos) {
        return static_cast<int32_t>(service_->moveadd(s, axis, pos));
    });
    server_->bind("home", [this](RpcSession s, int32_t axis, double pos) {
        return static_cast<int32_t>(service_->home(s, axis, pos));
    });
    server_->bind("setposition", [this](RpcSession s, int32_t axis, double pos) {
        return static_cast<int32_t>(
            service_->setposition(s, axis, pos));
    });
    server_->bind("groupselect", [this](RpcSession s, int32_t index) {
        return static_cast<int32_t>(service_->groupselect(s, index));
    });
    server_->bind("groupenable", [this](RpcSession s, const RpcAxisListWire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(
            service_->groupenable(s, len, args.axis));
    });
    server_->bind("groupdisable", [this](RpcSession s, const RpcAxisListWire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(
            service_->groupdisable(s, len, args.axis));
    });
    server_->bind("groupreset", [this](RpcSession s, const RpcAxisListWire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(
            service_->groupreset(s, len, args.axis));
    });
    server_->bind("waitticks", [this](RpcSession s, uint32_t ticks, int32_t axis, uint32_t timeout_ms,
                                uint32_t poll_ms) {
        return static_cast<int32_t>(
            service_->waitticks(s, ticks, axis, timeout_ms, poll_ms));
    });
    server_->bind("waitseconds", [this](RpcSession s, double seconds, int32_t axis, uint32_t timeout_ms,
                                  uint32_t poll_ms) {
        return static_cast<int32_t>(service_->waitseconds(
            s, seconds, axis, timeout_ms, poll_ms));
    });
    server_->bind("waitaxisidle", [this](RpcSession s, int32_t axis, uint32_t timeout_ms) {
        return static_cast<int32_t>(
            service_->waitaxisidle(s, axis, timeout_ms));
    });
    server_->bind("executecommand", [this](RpcSession s, const std::string& text) {
        return static_cast<int32_t>(service_->executecommand(s, text));
    });
    server_->bind("moveabs", [this](RpcSession s, const RpcMoveWire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(
            service_->moveabs(s, len, args.axis, args.distance));
    });
    server_->bind("move", [this](RpcSession s, const RpcMoveWire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(
            service_->moverel(s, len, args.axis, args.distance));
    });
    server_->bind("moverel", [this](RpcSession s, const RpcMoveWire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(
            service_->moverel(s, len, args.axis, args.distance));
    });
    server_->bind("groupsetaxes", [this](RpcSession s, const RpcAxisListWire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(
            service_->groupsetaxes(s, len, args.axis));
    });
    server_->bind("movecirc", [this](RpcSession s, const RpcMoveCircWire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(service_->movecirc(
            s, len, args.axis, args.end1, args.end2, args.centre1, args.centre2, args.direction));
    });
    server_->bind("movecircabs", [this](RpcSession s, const RpcMoveCircWire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(service_->movecircabs(
            s, len, args.axis, args.end1, args.end2, args.centre1, args.centre2, args.direction));
    });
    server_->bind("movecirc2", [this](RpcSession s, const RpcMoveCirc2Wire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(service_->movecirc2(
            s, len, args.axis, args.mid1, args.mid2, args.end1, args.end2));
    });
    server_->bind("movecirc2abs", [this](RpcSession s, const RpcMoveCirc2Wire& args) {
        const int32_t len =
            args.len < 0 ? 0 : (args.len > ZMC_RPC_MAX_AXES ? ZMC_RPC_MAX_AXES : args.len);
        return static_cast<int32_t>(service_->movecirc2abs(
            s, len, args.axis, args.mid1, args.mid2, args.end1, args.end2));
    });
    server_->bind("home_start", [this](RpcSession s, int32_t axis, const RpcHomeSdoConfigWire& cfg) {
        return static_cast<int32_t>(
            service_->home_start(s, axis, rpc_from_wire(cfg)));
    });
    server_->bind("home_get_status", [this](RpcSession s, int32_t axis) {
        int32_t status = 0, error_id = 0;
        const int ret =
            service_->home_get_status(s, axis, status, error_id);
        return std::make_tuple(static_cast<int32_t>(ret), status, error_id);
    });
    server_->bind("setflowconfig", [this](RpcSession s, const RpcFlowCtrlConfigWire& cfg) {
        return static_cast<int32_t>(
            service_->setflowconfig(s, rpc_from_wire(cfg)));
    });
    server_->bind("getaxisstate", [this](RpcSession s, int32_t axis) {
        RpcAxisStateWire out = {};
        const int ret =
            service_->getaxisstate(s, axis, rpc_from_wire(out), false);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("getaxisstate2", [this](RpcSession s, int32_t axis) {
        RpcAxisState2Wire out = {};
        const int ret =
            service_->getaxisstate2(s, axis, rpc_from_wire(out), false);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("getflowstatus", [this](RpcSession s) {
        RpcFlowCtrlStatusWire out = {};
        const int ret =
            service_->getflowstatus(s, rpc_from_wire(out), false);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("getflowconfig", [this](RpcSession s) {
        RpcFlowCtrlConfigWire out = {};
        const int ret = service_->getflowconfig(s, rpc_from_wire(out));
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
    server_->bind("getcmdresults", [this](RpcSession s) {
        std::vector<RpcCmdResultInfo> values;
        const int ret = service_->getcmdresults(s, values, false);
        std::vector<RpcCmdResultInfoWire> out(values.size());
        for (size_t i = 0; i < values.size(); ++i) out[i] = rpc_to_wire(values[i]);
        return std::make_tuple(static_cast<int32_t>(ret), out);
    });
}
}  // namespace zmc
