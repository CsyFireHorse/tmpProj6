#ifndef _AXIS_FB_BRIDGE_WITH_THREAD_H_
#define _AXIS_FB_BRIDGE_WITH_THREAD_H_

#include "AxisFbBridge.h"

namespace zmc {

// AxisFbBridge + SDO worker thread lifecycle.
// Default: std::thread; define ZMC_SDO_USE_PTHREAD for pthread_create.
class AxisFbBridgeWithThread {
   public:
    AxisFbBridgeWithThread();
    ~AxisFbBridgeWithThread();

    AxisFbBridgeWithThread(const AxisFbBridgeWithThread&) = delete;
    AxisFbBridgeWithThread& operator=(const AxisFbBridgeWithThread&) = delete;

    AxisFbBridge& bridge();
    const AxisFbBridge& bridge() const;

    int reset();
    int start_sdo();
    int stop_sdo();
    bool sdo_started() const;

   private:
    static int join_and_clear_worker(SdoRt* sdo);

    AxisFbBridge bridge_;
};

}  // namespace zmc

#endif
