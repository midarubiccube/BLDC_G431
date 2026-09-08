#include "main_app.hpp"

#include "main.h"
#include <math.h>

#include "interface/current.h"
#include "interface/timer.h"
#include "interface/canfd.hpp"
#include "interface/FullColorLED.hpp"
#include "melody_defines.h"
#include "esc.h"

CANFD* canfd;
FullColorLED led{&htim3, TIM_CHANNEL_4};
extern uint16_t encoder_target;

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET) {
		canfd->rx_interrupt_task();
	}
}

enum class DataType : uint8_t {
    COMMON_COMAND = 0x01,
    POWERBOARD_COMANND = 0x02,
    BLDC_COMANND = 0x03,
    MOTORBOARDC_COMAND = 0x04,
    // Add other data types as needed
};

union ID {
    uint16_t id;
    struct {
        uint16_t board_num : 4;
        DataType data_type : 4;
        uint16_t priority : 3;
    } fields;
};

extern "C" void main_setup(void){
	led.set_rgb(255, 0, 0);
  	led.start();
	
    HAL_GPIO_WritePin(SD_U_GPIO_Port, SD_U_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_V_GPIO_Port, SD_V_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SD_W_GPIO_Port, SD_W_Pin, GPIO_PIN_SET);

	ID own_id;
	own_id.fields.board_num = 2;
	own_id.fields.data_type = DataType::BLDC_COMANND;

	canfd = new CANFD(&hfdcan1);
	canfd->set_filter_mask(0, own_id.id, 0xFF);
	canfd->start();
	
	CANFD_Frame remote;
	remote.is_remote = true;
	remote.id = own_id.id;
	canfd->tx(remote);

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

	while (1) {
		if (canfd->rx_available()) {
			CANFD_Frame rx_frame;
			if (canfd->rx(rx_frame)) {
				if (rx_frame.size >= sizeof(BLDCPacket)) {
					BLDCPacket* packet = reinterpret_cast<BLDCPacket*>(rx_frame.data);
					encoder_target = packet->rps_target;
				}
			}
		}
		HAL_Delay(10);
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

