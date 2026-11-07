#ifndef _ZMC_WRAP_CMD_H_
#define _ZMC_WRAP_CMD_H_

// #include "FbMoveWrap.hpp"

namespace zmc {
class FbMoveWrap;
int runCmdLoop(FbMoveWrap& fbGrp);


int runCmdLoop2(int argc, char** argv);
}
#endif