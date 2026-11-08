/*
 * @Author: ChenSy
 * @LastEditors: Chen Sy
 * @Date: 2022-05-07 13:47:19
 * @LastEditTime: 2023-02-10 14:59:42
 * @Description: Modify here please
 */

#ifndef _ZMC_COMMON_H_
#define _ZMC_COMMON_H_

// #include "zmc_errno.h"

// #define USE_OS_THREAD

#define DLL_PUBLIC __attribute__ ((visibility("default")))

#ifndef ZMC_DLLPORT
#  if defined(WIN32) /* Windows platform. */
#    if defined(ZMC_DLLBUILD)
#      define ZMC_DLLPORT __declspec (dllexport)
#    else
#       define ZMC_DLLPORT
#    endif
#  elif defined(__linux__) /* Linux platform. */
#    if defined(ZMC_DLLBUILD)
#      define ZMC_DLLPORT __attribute__((visibility("default")))
#    else
#      define ZMC_DLLPORT __attribute__((visibility("default")))
#    endif
#  elif defined(__GNUC__) // mingw32,mingw64,msys
#    define ZMC_DLLPORT
#  else /* Other platforms. */
#    define ZMC_DLLPORT extern
#  endif
#endif

// #define ZMC_DLLPORT

#endif  // _ZMC_COMMON_H_
/* end of file */
