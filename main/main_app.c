#include "main_app.h"

#include "main.h"
#include <math.h>

#include "interface/current.h"
#include "interface/timer.h"
#include "melody_defines.h"
#include "esc.h"

void main_setup(void){
    HAL_GPIO_WritePin(SD_U_GPIO_Port, SD_U_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_V_GPIO_Port, SD_V_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_W_GPIO_Port, SD_W_Pin, GPIO_PIN_SET);

	init_timer();
	init_adc();
	set_encoder_offset();
	
    set_control_task(melodyControlTask);
	melody_freq = MELODY_C;
	melody_volume = 0.5f;
	HAL_Delay(200);
	melody_freq = MELODY_D;
	melody_volume = 0.5;
	HAL_Delay(200);
	melody_freq = MELODY_G;
	melody_volume = 0.5f;
	HAL_Delay(200);
	melody_volume = 0.0f;
	HAL_Delay(200);
	
    motor_controller_setup();
    set_control_task(MotorControlTask);
	set_khz_task(khz_task);
}


void disable_irq_nest() {
	if (irq_cnt == 0) __disable_irq();
	if (irq_cnt < UINT32_MAX) irq_cnt++;
}

void enable_irq_nest() {
	if (irq_cnt > 0) irq_cnt--;
	if (irq_cnt == 0) __enable_irq();
}

