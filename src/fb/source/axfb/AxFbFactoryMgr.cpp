#include "axfb/AxFbCmdManager.h"
#include "fbt_axis_base.h"
#include "fbt_axis_opaque.h"
#include "stdio.h"
#include <unordered_map>
#include "AxFbVarMgr.h"

namespace zmc {

struct AxFbCmdManagerImpl {
    std::vector<uint8_t> fbRunFlag;
    // std::unordered_map<int, void*> varDict;
};

AxFbCmdManagerImpl& getImplRef(AxFbCmdManager* pThis) {
    return *((AxFbCmdManagerImpl*)(pThis->pImpl));
}
const AxFbCmdManagerImpl& getImplRef(const AxFbCmdManager* pThis) {
    return *((const AxFbCmdManagerImpl*)(pThis->pImpl));
}
AxFbCmdManager::AxFbCmdManager() { pImpl = new AxFbCmdManagerImpl(); }

AxFbCmdManager::~AxFbCmdManager() {
    delete pImpl;
    pImpl = nullptr;
}
void AxFbCmdManager::createAxis(AX_FACTORY_TYPE axType) {
    cmdProc.createAxisArr(1, axType);
    auto p = cmdProc.getAxisWr(cmdProc.getAxisNum()-1);
    if (p) {
        p->getAxis()->get_config_ref().cycle_time = _runCfg.cycleTime * 1e6;
        p->getAxis()->reload_config();
    }
}

void AxFbCmdManager::createAxisArray(int num, AX_FACTORY_TYPE axType) {
    // cmdProc.createAxisArr(num, axType);
    for (int j = 0; j < num; j++) {
        createAxis(axType);
    }
}

void AxFbCmdManager::createFb(FB_TYPE fbType){
    cmdProc.createFb(fbType);
    getImplRef(this).fbRunFlag.resize(cmdProc.getFbNum());
}

void AxFbCmdManager::createFbArray(const std::vector<FB_TYPE>& fbTypes) {
    for (auto fbType : fbTypes) {
        cmdProc.createFb(fbType);
    }
    getImplRef(this).fbRunFlag.resize(cmdProc.getFbNum());
}

void AxFbCmdManager::pushCommand(const CommandStruct& cmd) { _cmdQueue.push(cmd); }

void AxFbCmdManager::pushCmds(const std::vector<CommandStruct>& cmds) {
    for (auto& cmd : cmds) {
        _cmdQueue.push(cmd);
    }
}
void AxFbCmdManager::pushCmds(const std::vector<CommandStruct4>& cmds) {
    for (auto& cmd : cmds) {
        _cmdQueue.push((CommandStruct&) cmd);
    }
}
void AxFbCmdManager::set_run_config(const RunConfig& cfg){
    _runCfg = cfg;
}

void AxFbCmdManager::set_config(const AxFbCmdConfig& cfg) {
    _runCfg = cfg.runCfg;
    
    for (auto& axType : cfg.axTypes) {
        createAxis(axType);
    }

    int i = 1;
    for (auto& axCfg : cfg.axCfgs) {
        if (axCfg) {
            auto p = cmdProc.getAxisWr(i);
            if (p) p->set_config(axCfg);
        }
        i++;
    }
    createFbArray(cfg.fbTypes);
    for (auto& cmd : cfg.cmds) {
        pushCommand(cmd);
    }
    for (const auto& var : cfg.vars) {
        varMgr.registerVar(var.name, (VarTag) var.type, var.size);
    }
}

void AxFbCmdManager::runCycle() {
    while (!_cmdQueue.empty() && _cmdQueue.front().tick <= _tick) {
        const auto& cmd = _cmdQueue.front();
        processCommand(cmd);
        _cmdQueue.pop();
    }
    const auto& rFlg = getImplRef(this).fbRunFlag;
    for (unsigned int i = 1; i < cmdProc.getFbNum(); i++) {
        if (rFlg[i]) {
            cmdProc.fbRunCycle(i);
        }
    }
    for (unsigned int i = 1; i < cmdProc.getAxisNum(); i++) {
        cmdProc.axisRunCycle(i);
    }

    if (_runCfg.verbose && (_runCfg.verboseModulo == 0 || (_tick % _runCfg.verboseModulo == 0))) {
        printStatus();
    }

    _tick++;
    checkExit();
}

void AxFbCmdManager::runCycles(int ticks){
    for (int i = 0; i < ticks; i++) {
        runCycle();
    }
}

void AxFbCmdManager::printHeader()const{
    if (_runCfg.verbose){
        if (_runCfg.verbose & 2)
            printf("idx,axId,time,axSt,mePos,mPos,pos,vel,acc\n");
        else 
            printf("idx,axId,time,axSt,mPos,pos,vel,acc\n");
    }
}
            
void AxFbCmdManager::run() {
    // _tick = 0;
    // _exitFlag = false;
    printHeader();
    while (!_exitFlag) {
        runCycle();
    }
}


void AxFbCmdManager::processCommand(const CommandStruct& cmd) {
    if (!cmd.fbId) return;
    
    switch(cmd.action){
        case AXFB_NOP:{
            if (cmd.attr == FB_ATTR_AUTORUN) {
                getImplRef(this).fbRunFlag[cmd.fbId] = (int)cmd.val;
                break;
            }
        }
        case FB_ATTR_SET:
        case AX_ATTR_SET:{
            cmdProc.process(cmd);
            break;
        }
        case AX_ATTR_GET: {
            double val;
            cmdProc.getAxAttr(cmd.fbId, cmd.attr + AX_ATTR_NOP, val);
            break;
        }
        case FB_ATTR_GET: {
            double val;
            cmdProc.getFbAttr(cmd.fbId, cmd.attr, val);
            break;
        }
        case AX_ATTR_SET_EX: // TOTEST
        case FB_ATTR_SET_EX:{
            auto cmd2 = cmd;
            // void* ptr = varMgr[(int)cmd.val].pAddr;
            // double& val = reinterpret_cast<double&>(ptr);
            cmd2.val = varMgr[(int)cmd.val].value.dVal;
            cmdProc.process(cmd2);
            break;
        }
        case AX_ATTR_GET_EX:   {
            // void* ptr = varMgr[(int)cmd.val].pAddr;
            // double& val = reinterpret_cast<double&>(ptr);
            double val = varMgr.getDouble((int)cmd.val);
            cmdProc.getAxAttr(cmd.fbId, cmd.attr + AX_ATTR_NOP, val);
            varMgr.setDouble((int)cmd.val, val);
            break;
        }
        case FB_ATTR_GET_EX:  {
            // void* ptr = varMgr[(int)cmd.val].pAddr;
            // double& val = reinterpret_cast<double&>(ptr);
            double val = varMgr.getDouble((int)cmd.val);
            cmdProc.getFbAttr(cmd.fbId, cmd.attr, val);
            varMgr.setDouble((int)cmd.val, val);
            break;
        }
        // use for object (size more than sizeof(double))
        case FB_ATTR_SET_EX_SPECIAL:{
            if (cmd.attr == FB_ATTR_CAMTABLEID){
                // from switch to camIn
                char buf[80];
                void* ptr = (void*)buf;
                cmdProc.getFb((int) cmd.val)->getPtrAttr(cmd.attr, ptr);
                cmdProc.setFbAttr(cmd.fbId, cmd.attr, buf);
            }else if (cmd.attr == FB_ATTR_TAPPETS){
                // from camIn to getTappets
                void* ptr;
                cmdProc.getFb((int) cmd.val)->getPtrAttr(cmd.attr, ptr);
                double& val =(double&) ptr;
                auto cmd2 = cmd; 
                cmd2.action = AXFB_NOP;
                cmd2.val = val;
                cmdProc.process(cmd2);
                // cmdProc.setFbAttr(cmd.fbId, cmd.attr, val);
            }else if (cmd.attr == FB_ATTR_SFILENAME){
                // from var to writeCam or readCam
                void* ptr = varMgr.getObjData((int)cmd.val) ;//[(int)cmd.val].getD;
                double& val = reinterpret_cast<double&>(ptr);
                auto cmd2 = cmd; 
                cmd2.action = AXFB_NOP;
                cmd2.val = val;
                cmdProc.process(cmd2);
            }
        }
        default:
            break;
    }
}


void AxFbCmdManager::printStatus(){
    for (unsigned int i = 1; i < cmdProc.getAxisNum(); i++) {
        auto ax = cmdProc.getAxis(i);
        auto axis = get_axis_impl(ax);

        ax->print(_runCfg.verbose);
        // if (_runCfg.verbose & 2)
        //     // printf("%u,%u,%f,%d,%d,%f,%f,%f,%f\n", _tick, i, _tick * _runCfg.cycleTime, 
        // else
        // printf("%u,%u,%f,%d,%f,%f,%f,%f\n", _tick, i, _tick * _runCfg.cycleTime, axis->axisStat,
        //        axis->fActPosition, axis->fSetPosition, axis->fSetVelocity, axis->fSetAcceleration);
    }
}

void AxFbCmdManager::checkExit() {
    if (_runCfg.exitCount > 0 && _tick >= _runCfg.exitCount) {
        _exitFlag = true;
    }
}

}  // namespace zmc