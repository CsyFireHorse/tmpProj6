#include "AxisFbBridgeWithThread.h"

#include "common/zmc/fbt_errorcode_def.h"
#include "sdo/sdo_rt.hpp"
#include "common/zmc/zmc_predef.h"

#ifdef ZMC_SDO_USE_PTHREAD
#include <pthread.h>
#else
#include <thread>
#endif

namespace zmc {

#ifdef ZMC_SDO_USE_PTHREAD
namespace {

void* sdo_worker_entry(void* arg) {
    SdoRt* sdo = static_cast<SdoRt*>(arg);
    if (sdo) {
        sdo->run_blocking();
    }
    return nullptr;
}

}  // namespace
#else
typedef std::thread Thread;
#endif

int AxisFbBridgeWithThread::join_and_clear_worker(SdoRt* sdo) {
    if (!sdo || !sdo->thread_) {
        return ZMC_ERR_BRIDGE_SDO_NOT_STARTED;
    }
    sdo->resume();
    sdo->request_stop();
#ifdef ZMC_SDO_USE_PTHREAD
    pthread_t* tid = static_cast<pthread_t*>(sdo->thread_);
    if (tid) {
        (void)pthread_join(*tid, nullptr);
        delete tid;
    }
#else
    Thread* thread = static_cast<Thread*>(sdo->thread_);
    if (thread && thread->joinable()) {
        thread->join();
        delete thread;
    }
#endif
    sdo->thread_ = nullptr;
    sdo->clear_runtime_queues_after_join();
    sdo->sdo_started_ = false;
    return 0;
}

AxisFbBridgeWithThread::AxisFbBridgeWithThread() {}

AxisFbBridgeWithThread::~AxisFbBridgeWithThread() { stop_sdo(); }

AxisFbBridge& AxisFbBridgeWithThread::bridge() { return bridge_; }

const AxisFbBridge& AxisFbBridgeWithThread::bridge() const { return bridge_; }

int AxisFbBridgeWithThread::reset() {
    return bridge_.reset();
}

bool AxisFbBridgeWithThread::sdo_started() const {
    const SdoRt* sdo = bridge_.native_sdo();
    return sdo && sdo->sdo_started();
}

int AxisFbBridgeWithThread::start_sdo() {
    SdoRt* sdo = bridge_.native_sdo();
    if (!sdo) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    if (sdo->sdo_started()) {
        sdo->resume();
        return 0;
    }
    if (sdo->thread_) {
        join_and_clear_worker(sdo);
    }
#ifdef ZMC_SDO_USE_PTHREAD
    pthread_t* tid = new pthread_t();
    if (!tid) {
        return ZMC_ERR_BRIDGE_SDO_THREAD_START_FAILED;
    }
    const int rc = pthread_create(tid, nullptr, sdo_worker_entry, sdo);
    if (rc != 0) {
        delete tid;
        return ZMC_ERR_BRIDGE_SDO_THREAD_START_FAILED;
    }
    sdo->thread_ = tid;
#else
    sdo->thread_ = new Thread([sdo]() { sdo->run_blocking(); });
#endif
    sdo->sdo_started_ = true;
    return 0;
}

int AxisFbBridgeWithThread::stop_sdo() {
    SdoRt* sdo = bridge_.native_sdo();
    if (!sdo || !sdo->sdo_started()) {
        return ZMC_ERR_BRIDGE_SDO_NOT_STARTED;
    }
    return join_and_clear_worker(sdo);
}

}  // namespace zmc
