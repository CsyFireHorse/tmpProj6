#ifndef _ZMC_FBT_AXIS_IMPL_H_
#define _ZMC_FBT_AXIS_IMPL_H_

#include "fbt_axis_base.h"
#include "fbt_axis_log.h"

namespace zmc {

class ServoAdapterBase;
class AxisImplBaseWithLog : public AxisImplBase {
public:
    void setLogger(std::unique_ptr<AxLogger> logger);
    AxLogger* getLogger();
    uint8_t is_ready_to_print()const;
};

class AxisImpl : public  AxisImplLog<AxisImplBaseWithLog> {
   public:
    AxisImpl();
    ~AxisImpl();
    FbStatEnum cur_fb_stat() const;
    FbStatEnum prev_fb_stat() const;
    void runCycle();

    int fPos2diPos(double pos);
    double diPos2fPos(int pos);
    double diVel2fVel(int vel)const;
    int fVel2diVel(double vel)const;


    int32_t fSetPos2diSetPos(double x, int& cnt)const;    
    int32_t fSetPos2diSetPos(double x, int& cnt, float& res) const;
    double diActPos2fActPos(int32_t pos, int& cnt)const;
    // double calc_modulo(double v) const;
    void bind_servo_adapter(ServoAdapterBase* pServoAdapter);
    void init_position(int pos, int vel=0);
    void reset_runtime();
    void print(int)const;
private:
    void _runCycle();
    void _calcCmd();
    void _use_last_position();
    void _use_last_pva();
    void _use_save_pos();
    void onResetError();
    void _onError();
    void _onSwErrorstop();
    void _onSwError(uint16_t error_id);
    void _onLagError();
    void _onDriveError();
    // Hold the last command and request power-off while PDO feedback is unreliable.
    void _onCommunicationError();
    void pushActPosVelAcc(int diPosNew, int diVelNew, int diAcc, int flag);
    void pushActPosVelAccAtFirst(int diPosNew, int diVelNew, int diAcc, int flag);
    void _updateMotionstat();
    void _updateState();
    bool checkNewPva();
    bool checkLag();

    void _onConstantVelocityCoasting();
};


}  // namespace zmc
#endif
