#ifndef __ECAT_PRINT_H__
#define __ECAT_PRINT_H__

#include "stdint.h"

#ifdef ZMC_DEBUG
#if defined(__linux__) /* Linux platform. */ || defined(__GNUC__) // mingw32,mingw64,msys
    #define _MSG_(flt,fmt,args...)  printf(flt ": %s<%d>: " fmt "\n", __PRETTY_FUNCTION__, __LINE__, ##args)
    #define _INF_(fmt,args...)  _MSG_("INF", fmt, ##args)
    #define _WRN_(fmt,args...)  _MSG_("WRN", fmt, ##args)
    #define _ERR_(fmt,args...)  _MSG_("ERR", fmt, ##args)
    #define _DBG_(fmt,args...)  _MSG_("DBG", fmt, ##args)
    #define _MSG_LINE_(flt,fmt,args...)  printf(flt ": %s<%d>: " fmt "\n",__FUNCTION__,__LINE__, ##args)
    #define _DBG_LINE_(fmt,args...)  _MSG_LINE_("RUN", fmt, ##args)
    #define __DBG_RT__(...)    if ((ret = __VA_ARGS__) != rt_bus_err_none) {  printf("fall on %d!! call: %s\n", ret, #__VA_ARGS__); }
#  else /* Other platforms. */
    #define _MSG_(flt,fmt,...) printf(flt ": %s<%d>: " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)
    #define _INF_(fmt, ...)  _MSG_("INF", fmt, __VA_ARGS__)
    #define _WRN_(fmt, ...)  _MSG_("WRN", fmt, __VA_ARGS__)
    #define _ERR_(fmt, ...)  _MSG_("ERR", fmt, __VA_ARGS__)
    #define _DBG_(fmt, ...)  _MSG_("DBG", fmt, __VA_ARGS__)
    #define _MSG_LINE_(flt,fmt, ...)  printf(flt ": %s<%d>: " fmt "\n",__FUNCTION__,__LINE__, __VA_ARGS__)
    #define _DBG_LINE_(fmt, ...)  _MSG_LINE_("RUN", fmt, __VA_ARGS__)
    #define __DBG_RT__(...)    if ((ret = __VA_ARGS__) != rt_bus_err_none) {  printf("fall on %d!! call: %s\n", ret, #__VA_ARGS__); }
#endif
#else
#if defined(__linux__) /* Linux platform. */ || defined(__GNUC__) // mingw32,mingw64,msys
    #define _MSG_(flt,fmt,args...)
    #define _INF_(fmt,args...) _MSG_("INF", fmt, ##args)
    #define _WRN_(fmt,args...) _MSG_("WRN", fmt, ##args)
    #define _ERR_(fmt,args...) _MSG_("ERR", fmt, ##args)
    #define _DBG_(fmt,args...) _MSG_("DBG", fmt, ##args)
    #define _MSG_LINE_(flt,fmt,args...)
    #define _DBG_LINE_(fmt,args...)
    #define __DBG_RT__(...)
#  else /* Other platforms. */
    #define _MSG_(flt,fmt,...)
    #define _INF_(fmt, ...) _MSG_("INF", fmt, __VA_ARGS__)
    #define _WRN_(fmt, ...) _MSG_("WRN", fmt, __VA_ARGS__)
    #define _ERR_(fmt, ...) _MSG_("ERR", fmt, __VA_ARGS__)
    #define _DBG_(fmt, ...) _MSG_("DBG", fmt, __VA_ARGS__)
    #define _MSG_LINE_(flt,fmt, ...)
    #define _DBG_LINE_(fmt, ...) 
    #define __DBG_RT__(...)
#endif
#endif

#define BOOL_TO_STR(bl) (bl)? "true" : "false"
#define BREAK_IF_FAILED(ret, expr)  { ret = (expr); if (ECAT_FAILED(ret)) { _ERR_(#expr " failed: err=%d", ret); break; } else { _INF_(#expr " succeeded"); } }
#define LOG_RET(ret, expr)          { ret = (expr); if (ECAT_FAILED(ret)) { _ERR_(#expr " failed: err=%d", ret);        } else { _INF_(#expr " succeeded"); } }
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))
#define BIT_STR(v,i)  (v.b.i ? (" " #i) : "")


#endif
