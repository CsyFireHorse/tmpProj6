#ifndef _ZMC_MOTION_RPC_TYPES_H_
#define _ZMC_MOTION_RPC_TYPES_H_

#include <stdint.h>

#include "motbuf/MotionCmdResult.h"
#include "../../demo/rpc_client/include/zml_motion_types.h"

/* AI modification signature: GPT-5 Codex */

typedef uint64_t RpcSession;

#define ZMC_RPC_MAX_AXES 64

/* Internal names alias the public C API layouts. */
typedef ZML_AxisState RpcAxisState;
typedef ZML_AxisStateEx RpcAxisState2;
typedef ZML_FlowCtrlConfig RpcFlowCtrlConfig;
typedef ZML_FlowCtrlStatus RpcFlowCtrlStatus;
typedef ZML_HomeSdoConfig RpcHomeSdoConfig;

/* Keep native command results internal to server-side code. */
typedef CmdResult RpcCmdResultInfo;

#endif
