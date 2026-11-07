#ifndef _ZMC_SDO_RT_THREAD_HPP_
#define _ZMC_SDO_RT_THREAD_HPP_

#include "sdo_rt.hpp"

namespace zmc {

class SdoRtThread : public SdoRt {
   public:
    SdoRtThread();
    ~SdoRtThread();

    void start();
    void stop();
    void clear_adapter();

    SdoRtThread(const SdoRtThread&) = delete;
    SdoRtThread& operator=(const SdoRtThread&) = delete;
    static void* thread_entry(void* arg);
};

}  // namespace zmc

#endif  // _ZMC_SDO_RT_THREAD_HPP_

