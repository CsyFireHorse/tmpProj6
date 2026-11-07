#ifndef _ZMC_WRAP_CMD_V2_H_
#define _ZMC_WRAP_CMD_V2_H_

#include <string>

namespace zmc {

class MotionCmdQueueBase;

int runCmdLoopV2(MotionCmdQueueBase& cmdIf);
int runCmdLoopV2(MotionCmdQueueBase& cmdIf, const std::string& startupScript, bool scriptExit);

}  // namespace zmc

#endif
