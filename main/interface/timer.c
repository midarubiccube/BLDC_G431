#include "timer.h"


static void (*controll_task)() = NULL;
static void (*khz_task)() = NULL;
int16_t encoder_offset = 0;
#define PWM_PERIOD_COUNTS 4250.0f

void init_timer(void) {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
	__HAL_TIM_CLEAR_IT(&htim1, TIM_IT_UPDATE);
	HAL_TIM_Base_Start_IT(&htim1);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	HAL_TIM_Encoder_Start(&htim8, TIM_CHANNEL_ALL);
	__HAL_TIM_ENABLE_IT(&htim8, TIM_IT_IDX);

	encoder_offset = __HAL_TIM_GET_COUNTER(&htim8);
}

void set_control_task(void (*task)()) {
    controll_task = task;
}

void set_encoder_offset() {
	float duty_amp = 0.20f;

	uint16_t pwm_u = (uint16_t)(PWM_PERIOD_COUNTS * (0.5f + duty_amp));
	uint16_t pwm_v = (uint16_t)(PWM_PERIOD_COUNTS * (0.5f - duty_amp * 0.5f));
	uint16_t pwm_w = (uint16_t)(PWM_PERIOD_COUNTS * (0.5f - duty_amp * 0.5f));

	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pwm_u);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pwm_v);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pwm_w);
	HAL_Delay(100);
	__HAL_TIM_SET_COMPARE (&htim1, TIM_CHANNEL_1, 0);
	__HAL_TIM_SET_COMPARE (&htim1, TIM_CHANNEL_2, 0);
	__HAL_TIM_SET_COMPARE (&htim1, TIM_CHANNEL_3, 0);
	encoder_offset = __HAL_TIM_GET_COUNTER(&htim8);
}
void set_khz_task(void (*task)()) {
    __HAL_TIM_SET_COUNTER(&htim6, 0);
    __HAL_TIM_CLEAR_IT(&htim6, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&htim6);
    khz_task = task;
}
void timer_setDuty(float u, float v, float w) {
	if (u > 1.0f) u = 1.0f;
	if (u < -1.0f) u = -1.0f;
	if (v > 1.0f) v = 1.0f;
	if (v < -1.0f) v = -1.0f;
	if (w > 1.0f) w = 1.0f;
	if (w < -1.0f) w = -1.0f;
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 2125.0f - 1750.0f * u);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 2125.0f - 1750.0f * v);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 2125.0f - 1750.0f * w);
}



void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim1) {
        if (controll_task) {
            controll_task();
        }
	} else if (htim == &htim6) {
        if (khz_task) {
            khz_task();
        }
	}
}





