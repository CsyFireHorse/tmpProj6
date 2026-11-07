#include "fbt_axis_encoder.h"

// #include "ServoCmdDef.hpp"
// #include "Cia402Easy.hpp"
// #include "ServoFakerToy.h"
#include "fbt_axis_impl.h"
#include "fbt_axis_impl_cfg.hpp"
#include "ServoAdaptorBase.hpp"
namespace zmc{


inline AxisImpl* to_axis_impl(AxisEncoder* pAxis){
    return (AxisImpl*) pAxis;
}
inline const AxisImpl* to_axis_impl(const AxisEncoder* pAxis){
    return (AxisImpl*) pAxis;
}
inline const PdoEnableFlag& get_pdo_flag(const AxisEncoder* pAxis) {
    return static_cast<AxisInnerContent*>(to_axis_impl(pAxis)->native)->axisCfg.pdo_flag;
}



class ServoAdapterEncoder:public ServoAdapterBase{
public:
    void onInit(AxisBase& axis){
        AxisCiaConfig& cfg = axis.get_config_ref();
        cfg.debug.eAxisMtype = AXIS_TYPE_ENCODER;
        axis.reload_config();

        auto& ax =  (AxisImpl&)(axis);
        ax.bind_servo_adapter(new ServoAdapterEncoder());
    }
    void onBeforeRunCycle(AxisBase& axis) {
        auto pAxis = (AxisImpl*)&axis;
        pAxis->diActPosition = pAxis->diEnPos2diEnPos(pAxis->diEncoderPosition, pAxis->diEncoderTurn);
        // pAxis->diActPosition = pAxis->diEncoderPosition;
        // pAxis->wStatWord = srvFak.wStatusword;
    }    
};


using  AxisEncoderImpl= MixinAxisServoAdapter<ServoAdapterEncoder, AxisBase>;
    
AxisEncoder::AxisEncoder(){
    AxisCiaConfig cfg;
    AxisCiaConfig_default_init(&cfg);
    // cfg.is_axis_virtual = true;
    cfg.debug.eAxisMtype = AXIS_TYPE_ENCODER;
    set_config(&cfg);
}



AxisEncoder::~AxisEncoder(){
}
void AxisEncoder::set_encoder_pos(int diEncoderPosition){
    to_axis_impl(this)->diEncoderPosition = diEncoderPosition;
}



// ignore statusword, actPos, user control command, controlword. only use setPosition.
void AxisEncoder::runCycle() {
    auto pAxis = to_axis_impl(this);
    const auto& pdoEnFlag = get_pdo_flag(this);
    // if (get_config_ref().debug.eAxisMtype == AXIS_TYPE_ENCODER)
    pAxis->diActPosition = pAxis->diEnPos2diEnPos(pAxis->diEncoderPosition, pAxis->diEncoderTurn);
    pAxis->runCycle();

}


}