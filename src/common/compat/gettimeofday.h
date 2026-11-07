#ifndef _GETTIMEOFDAY_H_
#define _GETTIMEOFDAY_H_



#include <stdio.h>
#include <algorithm>
#include <sstream>
#include <memory>
#include <time.h>

#ifdef linux
#include <sys/time.h>
#elif  defined(WIN32)
#include <windows.h>

inline int gettimeofday(struct timeval *tp, void *tzp)
{
  time_t clock;
  struct tm tm;
  SYSTEMTIME wtm;
 
  GetLocalTime(&wtm);
  tm.tm_year   = wtm.wYear  - 1900;
  tm.tm_mon    = wtm.wMonth - 1;
  tm.tm_mday   = wtm.wDay;
  tm.tm_hour   = wtm.wHour;
  tm.tm_min    = wtm.wMinute;
  tm.tm_sec    = wtm.wSecond;
  tm.tm_isdst  = -1;
 
  clock = mktime(&tm);
  tp->tv_sec   = clock;
  tp->tv_usec  = wtm.wMilliseconds * 1000;
  return (0);
}
#else
#include <chrono>
#include <WinSock2.h>
inline void gettimeofday(timeval *tval, void *)
{
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> pass = now - start;
    uint64_t us = (uint64_t)pass.count();
    tval->tv_sec = (long)(us / 1000000);
    tval->tv_usec = (long)(us % 1000000);
}
#endif

#endif 