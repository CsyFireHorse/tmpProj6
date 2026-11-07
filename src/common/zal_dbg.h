#ifndef __ZAL_DBG_H_
#define __ZAL_DBG_H_

#include "zal_config.h"

#ifdef ZAL_DEBUG

#ifdef ZAL_DEBUG_USE_SPDLOG
    #include "../mhelper/zspdlog.h"
    #define _TRACE_(...) get_logger()->trace(__VA_ARGS__)
    #define _DBG_(...) get_logger()->debug(__VA_ARGS__)
    #define _INFO_(...) get_logger()->info(__VA_ARGS__)
    #define _WARN_(...) get_logger()->warn(__VA_ARGS__)
    #define _ERROR_(...) get_logger()->error(__VA_ARGS__)
    #define _CRITC_(...) get_logger()->critical(__VA_ARGS__)
#else
    #define _TRACE_(...)
    #define _DBG_(...)
    #define _INFO_(...)
    #define _WARN_(...)
    #define _ERROR_(...)
    #define _CRITC_(...)
#endif

#ifdef ZAL_DEBUG_USE_PRINT
    #include "stdio.h"
    #if defined(__linux__) || defined(__GNUC__) // linux  mingw32,mingw64,msys
        #define _PRT_(...)  printf(__VA_ARGS__)
        #define _EVAL_(ABC) (ABC)
    #  else /* else platform */
        #define _PRT_(...)  printf(__VA_ARGS__)
        #define _EVAL_(ABC) (ABC)
    #endif
#else
    #define _PRT_(...)
    #define _EVAL_(ABC)
#endif


#else
#if defined(__linux__)  || defined(__GNUC__) // linux  mingw32,mingw64,msys
    #define _PRT_(...)
    #define _TRACE_(...)
    #define _DBG_(...)
    #define _INFO_(...)
    #define _WARN_(...)
    #define _ERROR_(...)
    #define _CRITC_(...)
    #define _EVAL_(ABC)
#else /* else platform */
    #define _PRT_(...)
    #define _TRACE_(...)
    #define _DBG_(...)
    #define _INFO_(...)
    #define _WARN_(...)
    #define _ERROR_(...)
    #define _CRITC_(...)
    #define _EVAL_(ABC)
#endif
#endif

#endif