#ifndef PCIE_COMPAT_H_
#define PCIE_COMPAT_H_

#include <cstddef>
#include <cstdint>
#include "common/zmc/zmc_predef.h"

#if defined(_WIN32)
#include <chrono>
#include <thread>
#else
#include <unistd.h>
#endif

#if defined(ZMC_USE_PCIE) || defined(MC_HOST_MOCK)
#include "delay/delay.h"
#else
#include <chrono>
inline uint32_t get_tick_ms() {
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count());
}
#endif

#if defined(ZMC_USE_PCIE) || defined(MC_HOST_MOCK)
#include "common/spsc_fifo.hpp"
template <typename T, size_t Capacity>
using SPSCQueue = zmc::SpscFifo<T, Capacity>;

#else
#include "common/container/SpscQueue.hpp"
// template <typename T, size_t Capacity>
// using PcieCompatPacketQueue = ::SPSCQueue<T, Capacity>;
#endif


#if defined(_WIN32)
// Provide the POSIX-style sleep helper expected by legacy worker loops on Windows.
inline void usleep(unsigned int usec) { std::this_thread::sleep_for(std::chrono::microseconds(usec)); }
#endif

#endif  // PCIE_COMPAT_H_
