#ifndef _ZMC_FB_FACTORY_MANAGER_HPP_
#define _ZMC_FB_FACTORY_MANAGER_HPP_

#include "FbFactory.h"
// #include "AxisCiaConfig.hpp"
#include <queue>
#include <vector>
// #include <fstream>
// #include "common/thirdlib/json/json.hpp"
// #include "fbt_axis_base.h"
#include "AxFbVarMgr.h"
#include "AxFbVarMgr2.h"


struct RunConfig {
    double cycleTime;    // default 0.001s
    int exitCount;           // 0 for infinite loop
    int verbose;  
    uint16_t verboseModulo;
};

typedef struct AxFbCmdConfig {
    RunConfig runCfg;
    std::vector<AX_FACTORY_TYPE> axTypes;
    std::vector<AxisCiaConfig*> axCfgs;
    std::vector<FB_TYPE> fbTypes;
    std::vector<CommandStruct> cmds;
    std::vector<VarDefCfg> vars;
}AxFbCmdConfig;



namespace zmc {

class AxFbCmdManagerImpl;

class AxFbCmdManager {
   public:
    AxFbCmdManager();
    ~AxFbCmdManager();

    void createAxis(AX_FACTORY_TYPE axType);
    void createAxisArray(int num, AX_FACTORY_TYPE axType);

    void createFb(FB_TYPE fbType);
    void createFbArray(const std::vector<FB_TYPE>& fbTypes);

    void pushCommand(const CommandStruct& cmd);
    void pushCmds(const std::vector<CommandStruct>& cmds);
    void pushCmds(const std::vector<CommandStruct4>& cmds);
    void set_run_config(const RunConfig& cfg);
    void set_config(const AxFbCmdConfig& cfg);
    // bool loadConfig(const std::string& filename);

    void runCycle();
    void runCycles(int ticks);
    void run();

    AxisAttrWrap* getAxis(int id) { return cmdProc.getAxisWr(id); }
    FbWrapper* getFb(int id) { return cmdProc.getFb(id); }

    uint32_t getCurrentTick() const { return _tick; }
    bool isRunning() const { return !_exitFlag; }

    AxFbCmdManagerImpl* pImpl;
    RunConfig _runCfg;

   protected:
    void printHeader()const;
    virtual void printStatus(); // const
    virtual void processCommand(const CommandStruct& cmd);


   private:
    bool _exitFlag = false;
    uint32_t _tick = 0;
    void checkExit();

    CommandProcessor cmdProc;
   public:
    VariableVectManager_v2 varMgr;
   private:
    std::queue<CommandStruct> _cmdQueue;
};

}  // namespace zmc

#endif  // _ZMC_FB_FACTORY_MANAGER_HPP_