#ifndef _COMPAT_WINDOWS_H_
#define _COMPAT_WINDOWS_H_


#ifdef __linux__
#include <pthread.h>
#include <unistd.h>
#include <termio.h>
#else
#include "windows.h"
inline int sleep(unsigned int s) { Sleep(s*1000); return 0;}
inline int usleep(unsigned int us) { Sleep(us/1000); return 0;}

inline int ioctl(int d, int state, void*){ return 0;}
inline int open(const char*, int, int) { return -1;};
inline int close(int) { return 0;};
#endif


#include <thread>
#include <chrono>

inline void sleep_for_s(unsigned int ms){
    std::this_thread::sleep_for(std::chrono::seconds(ms));
}

inline void sleep_for_ms(unsigned int ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
inline void sleep_for_us(unsigned int us){
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

#endif