/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority
 * GPT-5 Codex
*/
#include "sdo_rt_thread.hpp"

#include "common/zmc/zmc_predef.h"

#ifdef ZMC_SDO_USE_PTHREAD
#include <pthread.h>
#else
#include <thread>
#endif

namespace zmc {

#ifdef ZMC_SDO_USE_PTHREAD
#else
typedef std::thread Thread;
#endif

SdoRtThread::SdoRtThread() : SdoRt() {
    static_assert(sizeof(SdoRtThread) == sizeof(SdoRt), "SdoRtThread must equal to SdoRt");
}

SdoRtThread::~SdoRtThread() { stop(); }

void SdoRtThread::start() {
    if (sdo_started_) {
        return;
    }
    if (thread_) {
        stop();
    }
#ifdef ZMC_SDO_USE_PTHREAD
    pthread_t *tid = new pthread_t();
    const int rc = pthread_create(tid, nullptr, &SdoRtThread::thread_entry, this);
    if (rc != 0) {
        delete tid;
        return;
    }
    thread_ = tid;
#else
    thread_ = new Thread([this]() { run_blocking(); });
#endif
    sdo_started_ = true;
}

void SdoRtThread::stop() {
    if (!thread_) {
        clear_runtime_queues_after_join();
        sdo_started_ = false;
        return;
    }
    request_stop();
#ifdef ZMC_SDO_USE_PTHREAD
    pthread_t *tid = static_cast<pthread_t *>(thread_);
    if (tid) {
        (void)pthread_join(*tid, nullptr);
        delete tid;
    }
#else
    Thread *thread = static_cast<Thread *>(thread_);
    if (thread && thread->joinable()) {
        thread->join();
        delete thread;
    }
#endif
    thread_ = nullptr;
    clear_runtime_queues_after_join();
    sdo_started_ = false;
}

void *SdoRtThread::thread_entry(void *arg) {
    SdoRtThread *instance = static_cast<SdoRtThread *>(arg);
    if (instance) {
        instance->run_blocking();
    }
    return nullptr;
}

void SdoRtThread::clear_adapter() {
    stop();
    SdoRt::clear_adapter();
}

}  // namespace zmc
