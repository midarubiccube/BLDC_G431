#include "main_app.hpp"

#include "main.h"
#include <math.h>

#include "interface/current.h"
#include "interface/timer.h"
#include "interface/canfd.hpp"
#include "interface/FullColorLED.hpp"
#include "melody_defines.h"
#include "esc.h"

#include "ID_format.h"
#include "BLDC_format.h"
#include "stm32g4xx_hal.h"

CANFD* canfd;
ID own_id;

FullColorLED led{&htim3, TIM_CHANNEL_4};
extern float encoder_target;
extern int16_t encoder_diff;

bool monitorflag = false;
int monitor_freq = 10;
float gear_ratio = 0;
int encoder_resolution = 4096;

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
		canfd->rx_interrupt_task();
	}
}

extern "C" void main_setup(void){
	
    HAL_GPIO_WritePin(SD_U_GPIO_Port, SD_U_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_V_GPIO_Port, SD_V_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_W_GPIO_Port, SD_W_Pin, GPIO_PIN_SET);

	own_id.fields.board_num = 5;
	own_id.fields.data_type = DataType::BLDC_COMMAND;

	canfd = new CANFD(&hfdcan1);
	canfd->set_filter_mask(0, own_id.id, 0xFF);
	canfd->start();

	init_timer();
	init_adc();
	set_encoder_offset();

	led.set_rgb(255, 0, 0);
  	led.start();
	
	/* Send a remote frame to request data from the other node */
	CANFD_Frame remote;
	remote.is_remote = true;
	remote.id = own_id.id;
	canfd->tx(remote);

	while(!canfd->rx_available()) HAL_Delay(100);
	CANFD_Frame rx_frame;
	canfd->rx(rx_frame);

	led.set_rgb(0, 255, 0);
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

	while (1) {
		HAL_Delay(1000);
	}
}

void khz_task() {
	static uint32_t counter;
	static uint32_t last_count;
	if (canfd->rx_available()) {
		CANFD_Frame rx_frame;
		if (canfd->rx(rx_frame)) {
			if (rx_frame.size >= sizeof(BLDCTX_CANPacket)) {
				BLDCTX_CANPacket* packet = reinterpret_cast<BLDCTX_CANPacket*>(rx_frame.data);
				gear_ratio = packet->gear_ratio;
				encoder_resolution = packet->encoder_resolution;
				encoder_target = packet->rps_target * ((gear_ratio * encoder_resolution) / 1000.0f);
				monitorflag = packet->monitor_flag;
				monitor_freq = packet->monitor_freq;
				last_count = counter;
			}
		}
	}

	if(monitorflag) {
		if (counter % monitor_freq == 0) {
			BLDCRX_CANPacket packet;
			packet.rps = (encoder_diff * 1000.0f) / (gear_ratio * encoder_resolution);
			CANFD_Frame tx_frame;
			tx_frame.id = own_id.id;
			tx_frame.size = sizeof(BLDCRX_CANPacket);
			memcpy(tx_frame.data, &packet, sizeof(BLDCRX_CANPacket));
			canfd->tx(tx_frame);
		}
	}
	rps_task();

	counter++;
	if (counter - last_count > 1000) {
		encoder_target = 0;
	}
}


extern "C" void disable_irq_nest() {
	if (irq_cnt == 0) __disable_irq();
	if (irq_cnt < UINT32_MAX) irq_cnt++;
}

extern "C" void enable_irq_nest() {
	if (irq_cnt > 0) irq_cnt--;
	if (irq_cnt == 0) __enable_irq();
}

