#include "pdo_misc.h"

namespace zmc{

bool ecat_ready(SERVOS_t* servos, int len, proc_img_i_addr *pi, proc_img_o_addr *po){
    bool servo_enable_flag = true;
    // Process each axis's input and output variables.
    for (uint32_t i = 0; i < len; i++) {
        auto &servo = servos[i];

    // Update the control word based on the drive state.
        bool run = servo_control(servo, i, servo.ss, *pi->servos[i].stat, servo.sc, false);

        // Stop motion while the servo is disabled or its post-enable delay is active.
        if (!run || servo.run_delay != 0) {
            // m->_target[i] = servo.out_p ;
            servo.reset(*pi->servos[i].p);
            *po->servos[i].p = *pi->servos[i].p;
        }

        // The servo is enabled; decrement the delay counter.
        if (run && servo.run_delay)
            servo.run_delay--;
    
        // The servo is enabled and the delay has expired; proceed with normal control.
        if (run && !servo.run_delay) {   
            // servo.out_p = pos;
        }else servo_enable_flag = false; 
        
        // Update output variables.
        *po->servos[i].ctrl = servo.sc;

        // _ASSERT(po->servos[i].p);
        *po->servos[i].p = *pi->servos[i].p;
        // *po->servos[i].p = pos;
    }

    if (!servo_enable_flag){
        // printf("servo not ready!\n");    // The servo is not ready; wait.
        
    }
    return servo_enable_flag;
}

bool servo_control_enable(SERVOS_t& servo, int idx, servo_stat_word &s0, servo_stat_word &s1, servo_ctrl_word &c, bool show){
    bool servo_enable_flag = true;

        // Update the control word based on the drive state.
    bool run = servo_control(servo, idx, servo.ss, s1, servo.sc, show);
    // Stop motion while the servo is disabled or its post-enable delay is active.
    if (!run || servo.run_delay != 0) {
        // m->_target[i] = servo.out_p ;
        // servo.reset(*pi->servos[i].p);
        // *po->servos[i].p = *pi->servos[i].p;
    }

    // The servo is enabled; decrement the delay counter.
    if (run && servo.run_delay)
        servo.run_delay--;

    // The servo is enabled and the delay has expired; proceed with normal control.
    if (run && !servo.run_delay) {   
        // servo.out_p = pos;
    }else servo_enable_flag = false; 
    
    // Update output variables.
    c = servo.sc;

    // _ASSERT(po->servos[i].p);
    // *po->servos[i].p = *pi->servos[i].p;    
    return servo_enable_flag;
}


bool servo_control(SERVOS_t& _servo, int axis, servo_stat_word &s0, servo_stat_word &s1, servo_ctrl_word &c, bool show)
{
    auto& servo = _servo;
    bool status_change = false;

    do {
        c.v = 0;
        c.b.enable_voltage = 1;
        c.b.quick_stop = 1;
        c.b.enable_operation = s1.b.switched_on || s1.b.operation_enabled;
        c.b.switch_on = s1.b.ready_to_switch_on;

        // The servo status word indicates a fault.
        if (s1.b.fault && !servo.fault_reset_delay) {
            // Wait 1 second before retrying fault recovery to prevent rapid fault/recovery cycling from damaging the circuit.
            _DBG_LINE_("axis %d: delay fault reset", axis);
            servo.fault_reset_delay = 2000; // 2s = 2000 * cycle
            servo.run_delay = 2000; // After fault recovery succeeds and the servo is enabled, wait 2 seconds before accelerating; adjust for brake timing.
            break;
        }

        // The servo has a fault and is waiting for the delay to expire.
        if (servo.fault_reset_delay) {
            servo.fault_reset_delay--;
            if (!servo.fault_reset_delay) {
                // The delay has reached zero; attempt fault recovery.
                _DBG_LINE_("axis %d: start fault reset", axis);
                c.b.fault_reset = 1;
            }
            break;
        }
    } while (0);

    // Ignore some states to reduce log output.
    s1.b.target_reached = 0;
    s1.b.pad0 = 0;
    s1.b.pad1 = 0;

    if (s0.v != s1.v)
        status_change = true;
    if (status_change && show) {
        _DBG_("+++ axis=%d", axis);
        _DBG_("old_stat:%s%s%s%s%s%s%s%s%s%s%s%s%s",
            BIT_STR(s0, ready_to_switch_on),
            BIT_STR(s0, switched_on),
            BIT_STR(s0, operation_enabled),
            BIT_STR(s0, fault),
            BIT_STR(s0, voltage_enabled),
            BIT_STR(s0, quick_stop),
            BIT_STR(s0, switch_on_disabled),
            BIT_STR(s0, warning),
            BIT_STR(s0, pad0),
            BIT_STR(s0, remote),
            BIT_STR(s0, target_reached),
            BIT_STR(s0, internal_limit_active),
            BIT_STR(s0, pad1)
            );
        _DBG_("new_stat:%s%s%s%s%s%s%s%s%s%s%s%s%s",
            BIT_STR(s1, ready_to_switch_on),
            BIT_STR(s1, switched_on),
            BIT_STR(s1, operation_enabled),
            BIT_STR(s1, fault),
            BIT_STR(s1, voltage_enabled),
            BIT_STR(s1, quick_stop),
            BIT_STR(s1, switch_on_disabled),
            BIT_STR(s1, warning),
            BIT_STR(s1, pad0),
            BIT_STR(s1, remote),
            BIT_STR(s1, target_reached),
            BIT_STR(s1, internal_limit_active),
            BIT_STR(s1, pad1)
            );
        _DBG_("ctrl:%s%s%s%s%s%s%s%s%s",
            BIT_STR(c, switch_on),
            BIT_STR(c, enable_voltage),
            BIT_STR(c, quick_stop),
            BIT_STR(c, enable_operation),
            BIT_STR(c, pad0),
            BIT_STR(c, fault_reset),
            BIT_STR(c, halt),
            BIT_STR(c, pad1),
            BIT_STR(c, pad2)
            );
        _DBG_("---");
    }
    s0.v = s1.v;

    return s1.b.operation_enabled;
}

}
