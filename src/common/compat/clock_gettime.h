#ifndef _ZMC_CLOCK_GETTIME_H_
#define _ZMC_CLOCK_GETTIME_H_


#ifdef linux
#include <time.h>
#else
#include "Windows.h"

#define CLOCK_REALTIME  0// System time, which changes when the system clock is adjusted; it is not a monotonic timestamp.
#define CLOCK_MONOTONIC  1// Time elapsed since system startup; unaffected by system clock adjustments.
#define CLOCK_PROCESS_CPUTIME_ID 2 // CPU time consumed by this process up to the current point.
#define CLOCK_THREAD_CPUTIME_ID  3// CPU time consumed by this thread up to the current point.

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
