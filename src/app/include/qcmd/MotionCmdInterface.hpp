#ifndef _ZMC_MOTION_CMD_INTERFACE_HPP_
#define _ZMC_MOTION_CMD_INTERFACE_HPP_
// Server/RPC command adapter (SingleAxis..Interface). Not for FbMoveWrapV3.hpp / AxisFbGraph.hpp.
// int API returns ZMC_NO_ERROR on success; errors use ZMC_ERR_MOTCMD_* in fbt_errorcode_def.h
// (packet/queue, script parse, invalid arg, script busy, etc.).

#include "qcmd/MotionCmdAxisGroup.hpp"

#include <vector>

namespace zmc {

class MotionCmdRobot : public MotionCmdAxisGroup {
   public:
    MotionCmdRobot() : MotionCmdAxisGroup() {}
};

class MotionCmdHerit : public MotionCmdRobot {
   private:
    // Parse and enqueue one textual command line.
    int executeSingleLineCommand(const char* cmd);
    // Axis list supplied by the script's independent group configuration command.
    std::vector<int> scriptGroupAxisIndices_;

   public:
    // Parse and enqueue one or more textual command lines.
    int executeCommand(const char* cmd);
    // Advance qcmd and complete sync requests while the caller holds the shared command mutex.
    int runOnce();
};

}  // namespace zmc
#include "qcmd/MotionCmdInterfaceExtra.hpp"

#endif
