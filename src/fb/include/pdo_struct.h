#ifndef _PDO_STRUCT_H_
#define _PDO_STRUCT_H_
#include "stdint.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct __servo_ctrl{
	uint16_t control_word;                       //6040#00     
	uint16_t _reserve;               
	int8_t   mode_of_operation;                  //6060#00 
	int8_t   _reserve1;
	int8_t   _reserve2;
	int8_t   _reserve3;
	int16_t  set_torque;                         //6071#00 
	int16_t  _reserve4;
	int32_t  set_position;                        //607A#00 
	int32_t  add_velocity_value;                 //60B1#00 
	int16_t  add_torque_value;                   //60B2#00 
	int16_t  _reserve5; 
	uint16_t touch_probe_function;               //60B8#00 
	uint16_t _reserve6;
	uint32_t digital_outputs;                     //60FE#00 
	int32_t  set_velocity;                       //60FF#00 
}servo_ctrl;


typedef struct __servo_stat{
	uint16_t status_word;                         //6041#00
	uint16_t _reserve;                 
	int8_t   mode_of_operation_display;           //6061#00
	int8_t   _reserve1;
	int8_t   _reserve2;
	int8_t   _reserve3;
	int32_t  actual_position;                       //6064#00
	int32_t  actual_velocity;                     //606C#00
	int16_t  actual_torque;                       //6077#00
	int16_t  _reserve4;
	uint16_t touch_probe_status;                  //60B9#00
	uint16_t _reserve5;
	int32_t  touch_probe_1_rising_edge;           //60BA#00
	int32_t  touch_probe_1_falling_edge;          //60BB#00
	int32_t  touch_probe_2_rising_edge;           //60BC#00
	int32_t  touch_probe_2_falling_edge;          //60BD#00
	int32_t  following_error;                     //60F4#00
	uint32_t  digital_inputs;                     //60FD#00
}servo_stat;

#ifdef __cplusplus
}
#endif

#endif