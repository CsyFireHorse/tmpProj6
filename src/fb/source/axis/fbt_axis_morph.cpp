#include "fbt_axis_morph.h"

#include "fbt_axis_encoder.h"
#include "fbt_axis_virtual.h"
#include "impl/fbt_impl_enum_def.h"


namespace zmc {

AxisMorph::AxisMorph() : AxisCia402() {}

void AxisMorph::set_encoder_pos(int diEncoderPosition) {
    // AxisCiaConfig& cfg = get_config_ref();
    // if (cfg.debug.eAxisMtype == AXIS_TYPE_ENCODER) {
    static_cast<AxisEncoder*>((AxisBase*)this)->set_encoder_pos(diEncoderPosition);
    // }
}

void AxisMorph::runCycle() {
    switch (get_config_ref().debug.eAxisMtype) {
        case AXIS_TYPE_CIA402_SIMULATED:
            // static_cast<AxisCia402Fak*>((AxisBase*)this)->runCycle();
            break;
        case AXIS_TYPE_CIA402:
            static_cast<AxisCia402*>((AxisBase*)this)->runCycle();
            break;
        case AXIS_TYPE_ENCODER:
            static_cast<AxisEncoder*>((AxisBase*)this)->runCycle();
            break;
        case AXIS_TYPE_VIRTUAL:
        default:
            static_cast<AxisVirtual*>((AxisBase*)this)->runCycle();
            break;
    }
}

}  // namespace zmc