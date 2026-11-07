#ifndef _ZMC_TYPE_DEF_H_
#define _ZMC_TYPE_DEF_H_

#include "stdint.h"

#ifdef __cplusplus
typedef bool ZMC_BOOL;
#else
typedef uint8_t ZMC_BOOL;
#endif /*__cplusplus*/

typedef int8_t ZMC_SINT;
typedef int16_t ZMC_INT;
typedef int32_t ZMC_DINT;
typedef int64_t ZMC_LINT;

typedef uint8_t ZMC_USINT;
typedef uint16_t ZMC_UINT;
typedef uint32_t ZMC_UDINT;
typedef uint64_t ZMC_ULINT;

typedef uint8_t ZMC_BYTE;
typedef uint16_t ZMC_WORD;
typedef uint32_t ZMC_DWORD;
typedef uint64_t ZMC_LWORD;

typedef float ZMC_REAL;
typedef double ZMC_LREAL;

typedef char *ZMC_STRING;  // default length is 80

// Time data types are treated internally as DWORD. In the case of TIME, this
// leads to a resolution in milliseconds.
typedef int64_t ZMC_TIME;
typedef int64_t ZMC_TOD;
typedef int64_t ZMC_DT;
typedef int64_t ZMC_DATE;
typedef int64_t ZMC_TIME_OF_DAY;
typedef int64_t ZMC_DATE_AND_TIME;

// You can use the data type LTIME as a time base for high-resolution timer. A
// high-resolution timer has a resolution in nanoseconds.
typedef int64_t ZMC_LTIME;
typedef int64_t ZMC_LTOD;
typedef int64_t ZMC_LDT;
typedef int64_t ZMC_LDATE;
typedef int64_t ZMC_LTIME_OF_DAY;
typedef int64_t ZMC_LDATE_AND_TIME;
typedef char ZMC_CHAR;
typedef void *ZMC_VOID_ADR;
typedef void *ZMC_VOID_PTR;
// typedef wchar_t ZMC_WCHAR;

#endif /* _ZMC_TYPE_DEF_H_ */
