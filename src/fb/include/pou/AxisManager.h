#ifndef _ZMC_AXIS_MANAGER_H_
#define _ZMC_AXIS_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include "AxisRef.h"
#include "common/zmc/zmc_predef.h"

namespace zmc {

#define MAX_AXIS_COUNT MC_MOTION_MAX_AXES

// contain axis pointer, not axis object.
// only allow append or clear all axis. delete single axis is not allowed.
class SgAxisArr {
   public:
    static std::vector<AXIS_REF>& getInstance();

    static bool addAxis(void* value, const char* name = nullptr);
    static void* getAxisByIndex(uint32_t index);
    static int getIndexByAxis(void* axis);
    static void* getAxisByName(const char* name);
    static void* getAxisDefault();
    static std::string getAxisName(uint32_t index);
    static std::vector<std::string> getAllAxisNames();
    static uint32_t getSize();
    static bool replaceAxisName(uint32_t index, const char* newName);
    static void clear();

    SgAxisArr(const SgAxisArr&) = delete;
    SgAxisArr& operator=(const SgAxisArr&) = delete;

   private:
    SgAxisArr() = default;
};

}  // namespace zmc

#endif  // _ZMC_AXIS_MANAGER_H_
