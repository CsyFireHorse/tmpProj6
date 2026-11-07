/*
 * @Author: ChenShengyong 
 * @Date: 2024-03-01 17:15:05 
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-11-28 17:32:07
 */
#ifndef _ZMC_FB_IMPL_BASE_H_
#define _ZMC_FB_IMPL_BASE_H_


#include "mc_enum_def.h"
#include "zmc_types_cpp_def.h"
#include "fbt_errorcode_def.h"
#include "FbStatUn.h"
#include "AxisRef.h"
#include "mc_motion_def.h"

namespace zmc{

class FbImplBase{
public:   
    void setAxis(AXIS_REF axis){
        _axis = axis;
    }
    inline AXIS_REF getAxis(){
        return _axis;
    }
    void setBufferMode(MC_BUFFER_MODE bufferMode){
        ctrlParam.bufferMode = bufferMode;
    };
    void runCycle(){};
    virtual void axisRunCycle(){};

    bool isEnabled()const { return true;}
    bool isError() const{
        return _fbOStat.error_;
    };
    ZMC_ERROR getErrorID()const{
        return _error_id;
    };
    inline void setErrorIdStat(ZMC_ERROR err){
        _error_id = err;
        _fbOStat.set_error();
    }
    inline MC_BUFFER_MODE bufferMode() const { return (MC_BUFFER_MODE) ctrlParam.bufferMode;}
    inline uint16_t get_fb_stat()const { return _fbOStat.ostat; }
    inline uint16_t get_fb_type() const { return (uint16_t)_mode; }
    inline FB_TYPE getType()const { return _mode;}
    void syncStat(uint16_t){}
    MC_MOTION_MODE moveMode()const {  return (_mode < mcMoveCamOut)? (MC_MOTION_MODE)((int)_mode) : mcNoMoveType;}
    inline bool is_high_edge()const { return _fbOStat.executed_ && !_fbOStat.executed_bak;}
    
    FB_TYPE _mode = FB_TYPE_NOP;
    FbCtrlUn ctrlParam = { 0 };
    FbStatUn _fbOStat = { 0 };
    ZMC_ERROR _error_id = ZMC_NO_ERROR;
    AXIS_REF _axis;
};


} /* namespace zmc */
#endif