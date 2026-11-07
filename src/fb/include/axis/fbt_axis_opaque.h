#ifndef _ZMC_FBT_OPAQUE_H_
#define _ZMC_FBT_OPAQUE_H_

#include "AxisRef.h"
#include "AxisCiaConfig.hpp"
#include "AxisStorage.h"
namespace zmc {

class AxisOpaque : public AxisStorage {
   public:
    AxisOpaque();
    ~AxisOpaque();
    AXIS_REF axis();
    void set_config(const AxisCiaConfig* config);
    void reload_config();
    AxisCiaConfig get_config() const;
    AxisCiaConfig& get_config_ref();
    AxisCiaConfig* get_config_addr();
    const AxisCiaConfig* get_config_addr() const;
    void runCycle();
    void print(int) const;
    bool is_valid() const;
    uint32_t get_count() const;
    void reset_runtime();
    void bind_extra_data(int32_t* pDwExtraData);
};

// class Axis:public AxisOpaque{};
// class :public AxisBase{};

}  // namespace zmc

typedef class AxisBase : public zmc::AxisOpaque {
} AxisBase;

namespace zmc {
void assign_axis_id(AxisBase*, int);
void printAxisHeader(int verbose_flag);
void axisRequestSwError(AXIS_REF axis, uint16_t error_id);
void axisSetCommunError(AXIS_REF axis, bool val);
// Project the current non-OP Slave state without changing the latched axis error.
void axisSetNotOperationalError(AXIS_REF axis, bool val);
bool axisGetCommunError(AXIS_REF axis);
void axisSetDriveInterfaceError(AXIS_REF axis, uint16_t error);
uint16_t axisGetDriveInterfaceError(AXIS_REF axis);
void axisSetCycleTime(AXIS_REF axis, double val);
double axisGetCycleTime(AXIS_REF axis);
}  // namespace zmc
#endif
