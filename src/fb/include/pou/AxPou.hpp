#ifndef _ZMC_AX_POU_H_
#define _ZMC_AX_POU_H_
#include <string>
#include <vector>

#include "AxVerboseFlag.h"
#include "AxisRef.h"  // for AXIS_REF
#include "common/misc/pou_base.hpp"
#include "pou/AxisManager.h"
#include "pou/Ctx.h"

class AxisCiaConfig;
namespace zmc {
class AxPouImpl;
// class AxisImplBase;

class AxPou : public RunnerBase {
   public:
    AxPou();
    AxPou(const std::string& iniPath);
    AxPou(const std::string& iniPath, const std::string& iniContent, unsigned int axNum = 1);
    AxPou(int axNum);
    virtual ~AxPou();
    int configure(const std::string& iniPath, const std::string& iniContent,
                  unsigned int axNum = 1);
    void reset_runtime();
    virtual void preRun();
    virtual void postRunCycle();
    void axRunCycle();

    uint32_t axis_num() const;
    uint32_t axis_cap_num() const;
    uint32_t enabled_axis_num() const;
    int set_enabled_axis_num(uint32_t n);
    std::vector<AXIS_REF>& get_axis_ref_arr() { return axisRefArr; }
    AXIS_REF axisRef(int i) { return (AXIS_REF)axisRefArr[i]; }
    AXIS_REF axisRef(int i) const { return (const AXIS_REF)axisRefArr[i]; }
    // AxisImplBase& axRef(int i) { return *(AxisImplBase*)axisRefArr[i]; }
    // const AxisImplBase& axRef(int i) const { return *(const AxisImplBase*)axisRefArr[i]; }
    void print(int verbose) const;
    void printAxis() const;
    void setAxisSimulated(int idx, bool bSimulated);
    void _setAllAxisVirtual();
    AxisCiaConfig& get_config_ref(int i);
    void reload_config(int i);
    void set_cycle_time(int us);
    void set_cycles(int cycles);
    void set_verbose_flag(int flag);
    int get_cycle_time() const;
    int get_verbose_flag() const;

   protected:
    std::vector<AXIS_REF> axisRefArr;
};

class AxPouEnhanced : public AxPou {
   public:
    using AxPou::AxPou;
};

class AxPouTaskBinder : public AxPouEnhanced {
   public:
    // This uses C++11 "using constructor inheritance":
    // It allows AxPouTaskBinder to inherit all of AxPouEnhanced's constructors without manual
    // forwarding.
    using AxPouEnhanced::AxPouEnhanced;
    void bind(int i, PouTaskBase* pObj) { taskArr.push_back(pObj); }
    void runCycle() override {
        for (auto& task : taskArr) task->runCycle();
    }
    void postRun() override {
        for (auto& task : taskArr) task->postRun();
    }

   private:
    std::vector<PouTaskBase*> taskArr;
};

/**
 * @brief AxPouTaskBinder with automatic axis registration and cleanup
 */
class AxPouTaskBinderAuto : public AxPouTaskBinder {
   public:
    // AxPouTaskBinderAuto(const std::string& iniPath, const std::string& namePrefix = "axis_");
    AxPouTaskBinderAuto();
    AxPouTaskBinderAuto(const std::string& iniPath, const std::string& iniContent,
                        unsigned int axNum = 1, const std::string& namePrefix = "axis_");
    AxPouTaskBinderAuto(int axNum, const std::string& namePrefix = "axis_");
    int configure(const std::string& iniPath, const std::string& iniContent,
                  unsigned int axNum = 1, const std::string& namePrefix = "axis_");
    void reset_runtime();
    virtual ~AxPouTaskBinderAuto();
    static AXIS_REF getAxisByIndex(uint32_t index);
    static int getAxisCount();
    // Apply configured IO aliases to the owned context.
    int applyIoBinding();
    /**
     * @brief Register all axes with the AxisManager
     * @param namePrefix Optional prefix for axis names (default: "axis_")
     * @return Number of axes successfully registered
     */
    uint32_t registerAllAxis(const std::string& namePrefix = "axis_");

    // Owned context; motbuf holds a non-owning pointer to this.
    Ctx& ctx() { return ctx_; }
    const Ctx& ctx() const { return ctx_; }

    template <typename T>
    void bindRun() {
        T task;
        this->bind(0, &task);
        this->run();
    }

   private:
    std::string axisNamePrefix;
    Ctx ctx_;
};

}  // namespace zmc
#endif
