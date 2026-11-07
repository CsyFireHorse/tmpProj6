#ifndef _MC_AXIS_HPP_
#define _MC_AXIS_HPP_

#include "mc_fb_api.h"
#include "axis_cia402_api.h"
#include "fbt_axis_base.h"


class McAxis {
public:
    McAxis() {
        axis_cia402_init(&ax);
    }
    ~McAxis() {
        axis_cia402_deinit(&ax);
    }

    void runCycle() {
        axis_cia402_runcycle(&ax);
    }

    AxisCiaConfig& get_config_ref() {
        return *axis_cia402_get_config_addr(&ax);
    }

    void set_config(const AxisCiaConfig& cfg) {
        axis_cia402_set_config(&ax, &cfg);
    }

    void reload_config(){
        axis_cia402_set_config(&ax, &get_config_ref());
    }

private:
    AxisCia402InpWrap ax;
};
#endif