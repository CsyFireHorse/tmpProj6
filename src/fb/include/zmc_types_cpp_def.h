#ifndef _ZMC_TYPE_CPP_DEF_H_
#define _ZMC_TYPE_CPP_DEF_H_

#include "stdint.h"

#ifdef __cplusplus
namespace zmc {
typedef bool BOOL_T;
typedef bool BOOL;

typedef int8_t SINT;
typedef int16_t INT;
typedef int32_t DINT;
typedef int64_t LINT;

typedef uint8_t USINT;
typedef uint16_t UINT;
typedef uint32_t UDINT;
typedef uint64_t ULINT;

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t LWORD;

// typedef float REAL;
typedef double REAL;
typedef double LREAL;

// Time data types are treated internally as DWORD. In the case of TIME, this
// leads to a resolution in milliseconds.
typedef int64_t TIME;
typedef int64_t TOD;
typedef int64_t DT;
typedef int64_t DATE;
typedef int64_t TIME_OF_DAY;
typedef int64_t DATE_AND_TIME;

// You can use the data type LTIME as a time base for high-resolution timer. A
// high-resolution timer has a resolution in nanoseconds.
typedef int64_t LTIME;
typedef int64_t LTOD;
typedef int64_t LDT;
typedef int64_t LDATE;
typedef int64_t LTIME_OF_DAY;
typedef int64_t LDATE_AND_TIME;

typedef char *STRING;  // default length is 80

typedef uint16_t ZMC_ERROR;
typedef BOOL_T Error;
}  // namespace zmc

#endif
#endif /* _ZMC_TYPE_CPP_DEF_H_ */
