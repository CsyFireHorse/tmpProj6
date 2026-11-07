#ifndef __MC_ERRNO_H
#define __MC_ERRNO_H


#include <stdint.h>
#include "common/zmc/fbt_errorcode_def.h"

typedef MC_ERROR_CODE MCCommResult;

#ifdef __cplusplus
extern "C" {
#endif

const char* McCommResultGetMessage(MCCommResult code);

MCCommResult McCommResultFromMailbox(int32_t raw);

#ifdef __cplusplus
}
#endif
#endif //__MC_ERRNO_H
