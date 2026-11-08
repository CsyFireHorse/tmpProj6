#ifndef _CIA402_EASY_HPP_
#define _CIA402_EASY_HPP_

#include "stdint.h"

namespace zmc{
class Cia402Easy{
public:
    // Status word states.
    typedef enum
    {
        axis_sm_not_ready_to_switch_on = 0,         //!< 0: Not started; before initialization.
        axis_sm_switch_on_disabled,                 //!< 1: Switch-on disabled; after initialization or power-off, not ready to power on.
        axis_sm_ready_to_switch_on,                 //!< 2: Ready to switch on; ready to power on.
        axis_sm_switched_on,                        //!< 3: Switched on; after power-on, operation disabled.
        axis_sm_operation_enabled,                  //!< 4: Operation enabled; after power-on, operation enabled.
        axis_sm_quick_stop_active,                  //!< 5: Quick stop active.
        axis_sm_fault_reaction_active,              //!< 6: Fault reaction active.
        axis_sm_fault,                              //!< 7: Fault.
        axis_sm_halt,             
        axis_sm_home, 
    } axis_sm_t;

    typedef enum {
        STATWORD_SWITCH_ON_DISABLED = 0x40,
        STATWORD_READY_TO_SWITCH_ON = 0x21,
        STATWORD_SWITCH_ON = 0x23,   
        STATWORD_OPERATION_ENABLED = 0x27,
        STATWORD_QUICKSTOP = 0x07,   
        STATWORD_FAULT_REACTION_ACTIVE = 0x0f, 
        STATWORD_FAULT = 0x08,
        STATWORD_HALT_REACHED = 0x427,
        STATWORD_HOME_ATTAINED = 0x1027,
        STATWORD_HOME_ERROR = 0x2027,
    }statword_t;

    // Control words and their corresponding states are not one-to-one; some are duplicated.
    enum sm_cmd_t
    {
        //                                       BITS 0–3 AND 7    Transitions
        sm_cmd_shutdown             = 0x06,       //!< 0 x 1 1 0    2,6,8       => power off &  ready to power
        sm_cmd_switch_on            = 0x07,       //!< 0 0 1 1 1    3*          ready to power on => power on & disabled
        sm_cmd_disable_voltage      = 0x00,       //!< 0 x x 0 x    7,9,10,12   => power off & not ready to power
        sm_cmd_quick_stop           = 0x02,       //!< 0 x 0 1 x    7,10,11
        sm_cmd_disable_operation    = 0x07,       //!< 0 0 1 1 1    5           enabled => disabled
        sm_cmd_enable_operation     = 0x0F,       //!< 0 1 1 1 1    4,16        disabled => enabled
        sm_cmd_fault_reset          = 0x80,       //!< 1 x x x x    15
        sm_cmd_home                 = 0x1F,       //!< 0 1 1 1 1    4            home
        sm_cmd_halt                 = 0x10F,       //!< 0 1 1 1 1   4            halt
    };
    /**
  \brief  Define composite axis commands corresponding to control words.
    */
    typedef enum
    {
        axis_cmd_nop,                           //!< 
        axis_cmd_power_off,                     //!< Power off.
        axis_cmd_ready_power,                   //!< Prepare power.
        axis_cmd_power_on,                      //!< Power on.
        axis_cmd_disable,                       //!< Disable the axis.
        axis_cmd_enable,                        //!< Enable the axis.
        axis_cmd_ready_poweron,                 //!< Prepare power and power on.
        axis_cmd_ready_poweron_enable,          //!< Prepare power, power on, and enable the axis.
        axis_cmd_poweron_enable,                //!< Power on and enable the axis.
        axis_cmd_alarm_clear,                   //!< Clear the alarm.
        axis_cmd_quick_stop,                    //!< Emergency stop.
        axis_cmd_halt,                          //!< halt
        axis_cmd_home,                          //!< home
        axis_cmd_poweroff_home,                 //!< power_off to home
        axis_cmd_enable_position,               //!< Enable the axis and enter CSP mode.
        axis_cmd_end,                           //!< end of axis_cmd
    } axis_cmd_t;
    /** \brief Define the initialization-complete state. */
    #define axis_sm_init_done                   (1 << axis_sm_switch_on_disabled) | (1 << axis_sm_ready_to_switch_on)


    /**
      \brief  Return the state machine corresponding to the current status word.

      \param  status            Status word.
      \return  State machine corresponding to the current status word.
    */
    inline static axis_sm_t get_sm(uint16_t status)
    {
        // Status word 6041H, bits 0–3, 5, and 6:
        //       0: xxxx xxxx x0xx 0000 Not ready to switch on
        //       1: xxxx xxxx x1xx 0000 Switch on disabled
        //       2: xxxx xxxx x01x 0001 Ready to switch on
        //       3: xxxx xxxx x01x 0011 Switched on
        //       4: xxxx xxxx x01x 0111 Operation enabled
        //       5: xxxx xxxx x00x 0111 Quick stop active
        //       6: xxxx xxxx x0xx 1111 Fault reaction active
        //       7: xxxx xxxx x0xx 1000 Fault
        // 
        static const uint8_t _get_sm_table[][16] = 
        {
            //0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
            { 0, 7, 7, 7, 7, 7, 7, 5, 7, 7, 7, 7, 7, 7, 7, 6 }, // 00
            { 0, 2, 7, 3, 7, 7, 7, 4, 7, 7, 7, 7, 7, 7, 7, 6 }, // 01
            { 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 }, // 10
            { 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 }, // 11
        };
        return (axis_sm_t)_get_sm_table[status >> 5 & 0x03][status & 0x0F];
    }
    inline static bool is_ctrlw_halt(uint16_t ctrl){
        return (ctrl & sm_cmd_halt) == sm_cmd_halt;
    }
    inline static bool is_ctrlw_home(uint16_t ctrl){
        return (ctrl & sm_cmd_home) == sm_cmd_home;
    }
    inline static bool is_halt_done(uint16_t status){
        // TARGET REACHED
        return (status & STATWORD_HALT_REACHED) == STATWORD_HALT_REACHED;
    }
    inline static bool is_home_done(uint16_t status){
        // Homing attained
        return (status & STATWORD_HOME_ATTAINED) == STATWORD_HOME_ATTAINED; 
    }
    inline static bool is_home_error(uint16_t status){
        // Homing error
        return (status & STATWORD_HOME_ERROR) == STATWORD_HOME_ERROR;
    }
    /**
      \brief  Return the status word corresponding to the current status-word state machine.

      \param  sm            Status-word state machine.
      \param  status        Initial status-word value.
      \return  Status word.
    */
    inline static uint16_t get_stat(axis_sm_t sm, uint16_t stat){
        static const uint16_t _table[][2] = {
            {0x4f, 0},
            {0x4f, STATWORD_SWITCH_ON_DISABLED},
            {0x6f, STATWORD_READY_TO_SWITCH_ON},
            {0x6f, STATWORD_SWITCH_ON},
            {0x6f, STATWORD_OPERATION_ENABLED},
            {0x6f, STATWORD_QUICKSTOP},
            {0x4f, STATWORD_FAULT_REACTION_ACTIVE},
            {0x4f, STATWORD_FAULT}
        };
        uint16_t mask = _table[(uint8_t)sm][0];
        uint16_t val = _table[(uint8_t)sm][1];
        stat = (stat & ~mask ) | (mask & val);
        return stat;
    }
    
    /**
      \brief  Return the command corresponding to the current control word.

      \param  status            Control word.
      \return  Command.
    */
    inline static sm_cmd_t get_ctrl_st(uint16_t ctrl)
    {
        static const uint8_t _get_ctrl_sm_table[][16] = 
        {
            { sm_cmd_disable_voltage, sm_cmd_disable_voltage, sm_cmd_quick_stop, sm_cmd_quick_stop, 
                sm_cmd_disable_voltage, sm_cmd_disable_voltage, sm_cmd_shutdown, sm_cmd_disable_operation, 
                sm_cmd_disable_voltage, sm_cmd_disable_voltage, sm_cmd_quick_stop, sm_cmd_quick_stop, 
                sm_cmd_disable_voltage, sm_cmd_disable_voltage, sm_cmd_shutdown, sm_cmd_enable_operation }, // 00
        };
        return (sm_cmd_t) ((ctrl&0x80) ?sm_cmd_fault_reset:_get_ctrl_sm_table[0][ctrl & 0x0F]);
    }

    // Calculate the new state from the control word and current state.
    static uint16_t get_new_state(uint16_t ctrl, uint16_t stat){
        axis_sm_t st = get_sm(stat);
        sm_cmd_t cmd = get_ctrl_st(ctrl);
                // axis_sm_t out = stat;
        return get_stat(get_new_state_by_cmd(cmd, st), stat);
    }
    struct axis_flags{
        uint8_t is_moved:1;
        uint8_t is_moving:1;
        uint8_t is_home_mode:1;
        uint8_t is_home_error:1;
    };
    static uint16_t get_new_stateEx(uint16_t ctrl, uint16_t stat, axis_flags flg){
        axis_sm_t sm = get_sm(stat);
        sm_cmd_t cmd = get_ctrl_st(ctrl);
        axis_sm_t sm2 = get_new_state_by_cmd(cmd, sm);

        if (sm2 == Cia402Easy::axis_sm_switched_on && sm == Cia402Easy::axis_sm_operation_enabled && flg.is_moving){
            sm2 = sm; // rollback statusword
        }else if(sm2 == Cia402Easy::axis_sm_fault  && sm == Cia402Easy::axis_sm_fault_reaction_active && flg.is_moved){
            sm2 = sm;
        }else if (sm == Cia402Easy::axis_sm_quick_stop_active && sm2 == Cia402Easy::axis_sm_quick_stop_active && !flg.is_moved){
            sm2 = Cia402Easy::axis_sm_switch_on_disabled;
        }else if ( sm == Cia402Easy::axis_sm_operation_enabled && sm2 == Cia402Easy::axis_sm_operation_enabled){
            if (is_ctrlw_halt(ctrl)){ // halt
                if (flg.is_moved){
                    return STATWORD_OPERATION_ENABLED;
                }else{
                    return STATWORD_HALT_REACHED;
                }
            }else if(is_ctrlw_home(ctrl)){ // home
                if (flg.is_home_mode){
                    if (flg.is_home_error){
                        return STATWORD_HOME_ERROR;
                    }else if (!flg.is_moved){
                        return STATWORD_HOME_ATTAINED;
                    }else{
                        return STATWORD_OPERATION_ENABLED;
                    }
                }
            }else{
                if (!flg.is_moved){
                    return STATWORD_OPERATION_ENABLED;
                }
            }
        }
        return get_stat(sm2, stat);
    }

    inline static axis_sm_t get_new_state_by_cmd(sm_cmd_t cmd, axis_sm_t st){
        axis_sm_t out = st;
        switch (st){
        case axis_sm_not_ready_to_switch_on:
            out = axis_sm_switch_on_disabled;  // Transition 1
            break;
        case axis_sm_switch_on_disabled:
            if (cmd == sm_cmd_shutdown)
                out = axis_sm_ready_to_switch_on;  // Transition 2
            break;
        case axis_sm_ready_to_switch_on:
            if (cmd == sm_cmd_disable_voltage || cmd == sm_cmd_quick_stop)
                out = axis_sm_switch_on_disabled; // Transition 7
            else if (cmd == sm_cmd_switch_on)  
                out = axis_sm_switched_on; // Transition 3
            break;
        // power on
        case axis_sm_switched_on:
            if (cmd == sm_cmd_disable_voltage || cmd == sm_cmd_quick_stop)
                out = axis_sm_switch_on_disabled; // Transition 10
            else if (cmd == sm_cmd_shutdown)
                out = axis_sm_ready_to_switch_on; // Transition 6
            else if (cmd == sm_cmd_enable_operation) 
                out = axis_sm_operation_enabled; // Transition 4
            break;
        case axis_sm_operation_enabled:
            if (cmd == sm_cmd_disable_voltage)
                out = axis_sm_switch_on_disabled; // Transition 9
            else if (cmd == sm_cmd_shutdown)
                out = axis_sm_ready_to_switch_on; // Transition 8
            else if (cmd == sm_cmd_disable_operation)
                out = axis_sm_switched_on; // Transition 5
            else if (cmd == sm_cmd_quick_stop)
                out = axis_sm_quick_stop_active; // Transition 11
            break;
        case axis_sm_quick_stop_active:
            // wait until 'Quick Stop' is completed.
            if (cmd == sm_cmd_disable_voltage) // Transition 12
                out = axis_sm_switch_on_disabled;
            if (cmd == sm_cmd_enable_operation)
                ; // transition 16, supporting not recommended
            break;
        case axis_sm_fault_reaction_active: // Transition 13
            // wait until fault is completed.
            out = axis_sm_fault; // Transition 14
            break;
        case axis_sm_fault:
            if (cmd == sm_cmd_fault_reset) // Transition 15
               out = axis_sm_switch_on_disabled; 
            break;
        default:
            break;
        }
        return out;
    }

    // Calculate control words from the command and current state (some commands require multiple control words).
    static uint16_t get_ctrl(axis_cmd_t cmd, uint16_t stat){
        switch (cmd){
        case axis_cmd_nop:
            switch (get_sm(stat)){
            case axis_sm_switch_on_disabled:
                return sm_cmd_disable_voltage;
            case axis_sm_ready_to_switch_on:  
                return sm_cmd_shutdown;       
            case axis_sm_switched_on:
                return sm_cmd_switch_on;
            case axis_sm_operation_enabled:
                return sm_cmd_enable_operation;
            case axis_sm_quick_stop_active:
                return sm_cmd_quick_stop;
            case axis_sm_fault_reaction_active:
            case axis_sm_fault:
            default:
                return sm_cmd_disable_voltage;
            }
        case axis_cmd_power_off:
            return sm_cmd_disable_voltage;
        case axis_cmd_ready_power:
            return sm_cmd_shutdown;
        case axis_cmd_power_on:
            return sm_cmd_switch_on;
        case axis_cmd_disable:
            return sm_cmd_disable_operation;
        case axis_cmd_enable:
            return sm_cmd_enable_operation;
        case axis_cmd_ready_poweron:
            if (get_sm(stat) == axis_sm_ready_to_switch_on || get_sm(stat) == axis_sm_ready_to_switch_on)
                return sm_cmd_switch_on;
            else
                return sm_cmd_shutdown;
        case axis_cmd_ready_poweron_enable:
            if (get_sm(stat) == axis_sm_switched_on || get_sm(stat) == axis_sm_operation_enabled)
                return sm_cmd_enable_operation;
            else if (get_sm(stat) == axis_sm_ready_to_switch_on)
                return sm_cmd_switch_on;
            else
                return sm_cmd_shutdown;
        case axis_cmd_poweron_enable:
            if (get_sm(stat) == axis_sm_switched_on || get_sm(stat) == axis_sm_operation_enabled)
                return sm_cmd_enable_operation;
            else
                return sm_cmd_switch_on;
        case axis_cmd_alarm_clear:
            return sm_cmd_fault_reset;
        case axis_cmd_quick_stop:
            return sm_cmd_quick_stop;
        case axis_cmd_halt:
            return sm_cmd_halt;
        case axis_cmd_home:
            return sm_cmd_home;
        case axis_cmd_poweroff_home:
            switch (get_sm(stat))
            {
            case axis_sm_operation_enabled:
                return sm_cmd_home;
            case axis_sm_switched_on:
                return sm_cmd_enable_operation;
            case axis_sm_ready_to_switch_on:
                return sm_cmd_switch_on; 
            case axis_sm_switch_on_disabled:
            default:
                return sm_cmd_shutdown;
            }
        case axis_cmd_enable_position:
            return sm_cmd_enable_operation;
        default:
            return sm_cmd_disable_voltage;
        }
    }
};


}
#endif
