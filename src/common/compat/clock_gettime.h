#ifndef _ZMC_CLOCK_GETTIME_H_
#define _ZMC_CLOCK_GETTIME_H_


#ifdef linux
#include <time.h>
#else
#include "Windows.h"

#define CLOCK_REALTIME  0// : 是指系统时间，随着系统时间的改变而改变。系统时钟会被用户而改变。并非不变的时间戳。
#define CLOCK_MONOTONIC  1//  指从系统启动时开始计时。不受系统设置影响，也不会被用户改变。
#define CLOCK_PROCESS_CPUTIME_ID 2 //  指这个进程运行到当前代码时，CPU花费的时间。
#define CLOCK_THREAD_CPUTIME_ID  3//  指这个线程运行到当前代码时，CPU花费的时间。

// struct timespec { 
//     long tv_sec; 
//     long tv_nsec; 
// }timespec;    //header part


#define BILLION                             (1E9)
inline
int clock_gettime(int dummy, timespec *ct)
{
    static BOOL g_first_time = 1;
    static LARGE_INTEGER g_counts_per_sec;

    LARGE_INTEGER count;

    if (g_first_time)
    {
        g_first_time = 0;

        if (0 == QueryPerformanceFrequency(&g_counts_per_sec))
        {
            g_counts_per_sec.QuadPart = 0;
        }
    }

    if ((NULL == ct) || (g_counts_per_sec.QuadPart <= 0) ||
            (0 == QueryPerformanceCounter(&count)))
    {
        return -1;
    }

    ct->tv_sec = count.QuadPart / g_counts_per_sec.QuadPart;
    ct->tv_nsec = ((count.QuadPart % g_counts_per_sec.QuadPart) * BILLION) / g_counts_per_sec.QuadPart;

    return 0;
}

#endif 


#endif /* _ZMC_CLOCK_GETTIME_H_ */